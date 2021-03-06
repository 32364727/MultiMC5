/* Copyright 2013-2017 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QStringList>
#include <QString>
#include <QDir>
#include <QStringList>

#include <settings/Setting.h>

#include <QDebug>
#include "java/JavaUtils.h"
#include "java/JavaCheckerJob.h"
#include "java/JavaInstallList.h"
#include "FileSystem.h"

JavaUtils::JavaUtils()
{
}

JavaInstallPtr JavaUtils::MakeJavaPtr(QString path, QString id, QString arch)
{
	JavaInstallPtr javaVersion(new JavaInstall());

	javaVersion->id = id;
	javaVersion->arch = arch;
	javaVersion->path = path;

	return javaVersion;
}

JavaInstallPtr JavaUtils::GetDefaultJava()
{
	JavaInstallPtr javaVersion(new JavaInstall());

	javaVersion->id = "java";
	javaVersion->arch = "unknown";
#if defined(Q_OS_WIN32)
	javaVersion->path = "javaw";
#else
	javaVersion->path = "java";
#endif

	return javaVersion;
}

#if defined(Q_OS_WIN32)
QList<JavaInstallPtr> JavaUtils::FindJavaFromRegistryKey(DWORD keyType, QString keyName)
{
	QList<JavaInstallPtr> javas;

	QString archType = "unknown";
	if (keyType == KEY_WOW64_64KEY)
		archType = "64";
	else if (keyType == KEY_WOW64_32KEY)
		archType = "32";

	HKEY jreKey;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyName.toStdString().c_str(), 0,
					  KEY_READ | keyType | KEY_ENUMERATE_SUB_KEYS, &jreKey) == ERROR_SUCCESS)
	{
		// Read the current type version from the registry.
		// This will be used to find any key that contains the JavaHome value.
		char *value = new char[0];
		DWORD valueSz = 0;
		if (RegQueryValueExA(jreKey, "CurrentVersion", NULL, NULL, (BYTE *)value, &valueSz) ==
			ERROR_MORE_DATA)
		{
			value = new char[valueSz];
			RegQueryValueExA(jreKey, "CurrentVersion", NULL, NULL, (BYTE *)value, &valueSz);
		}

		QString recommended = value;

		TCHAR subKeyName[255];
		DWORD subKeyNameSize, numSubKeys, retCode;

		// Get the number of subkeys
		RegQueryInfoKey(jreKey, NULL, NULL, NULL, &numSubKeys, NULL, NULL, NULL, NULL, NULL,
						NULL, NULL);

		// Iterate until RegEnumKeyEx fails
		if (numSubKeys > 0)
		{
			for (int i = 0; i < numSubKeys; i++)
			{
				subKeyNameSize = 255;
				retCode = RegEnumKeyEx(jreKey, i, subKeyName, &subKeyNameSize, NULL, NULL, NULL,
									   NULL);
				if (retCode == ERROR_SUCCESS)
				{
					// Now open the registry key for the version that we just got.
					QString newKeyName = keyName + "\\" + subKeyName;

					HKEY newKey;
					if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, newKeyName.toStdString().c_str(), 0,
									 KEY_READ | KEY_WOW64_64KEY, &newKey) == ERROR_SUCCESS)
					{
						// Read the JavaHome value to find where Java is installed.
						value = new char[0];
						valueSz = 0;
						if (RegQueryValueEx(newKey, "JavaHome", NULL, NULL, (BYTE *)value,
											&valueSz) == ERROR_MORE_DATA)
						{
							value = new char[valueSz];
							RegQueryValueEx(newKey, "JavaHome", NULL, NULL, (BYTE *)value,
											&valueSz);

							// Now, we construct the version object and add it to the list.
							JavaInstallPtr javaVersion(new JavaInstall());

							javaVersion->id = subKeyName;
							javaVersion->arch = archType;
							javaVersion->path =
								QDir(FS::PathCombine(value, "bin")).absoluteFilePath("javaw.exe");
							javas.append(javaVersion);
						}

						RegCloseKey(newKey);
					}
				}
			}
		}

		RegCloseKey(jreKey);
	}

	return javas;
}

QList<QString> JavaUtils::FindJavaPaths()
{
	QList<JavaInstallPtr> java_candidates;

	QList<JavaInstallPtr> JRE64s = this->FindJavaFromRegistryKey(
		KEY_WOW64_64KEY, "SOFTWARE\\JavaSoft\\Java Runtime Environment");
	QList<JavaInstallPtr> JDK64s = this->FindJavaFromRegistryKey(
		KEY_WOW64_64KEY, "SOFTWARE\\JavaSoft\\Java Development Kit");
	QList<JavaInstallPtr> JRE32s = this->FindJavaFromRegistryKey(
		KEY_WOW64_32KEY, "SOFTWARE\\JavaSoft\\Java Runtime Environment");
	QList<JavaInstallPtr> JDK32s = this->FindJavaFromRegistryKey(
		KEY_WOW64_32KEY, "SOFTWARE\\JavaSoft\\Java Development Kit");

	java_candidates.append(JRE64s);
	java_candidates.append(MakeJavaPtr("C:/Program Files/Java/jre8/bin/javaw.exe"));
	java_candidates.append(MakeJavaPtr("C:/Program Files/Java/jre7/bin/javaw.exe"));
	java_candidates.append(MakeJavaPtr("C:/Program Files/Java/jre6/bin/javaw.exe"));
	java_candidates.append(JDK64s);
	java_candidates.append(JRE32s);
	java_candidates.append(MakeJavaPtr("C:/Program Files (x86)/Java/jre8/bin/javaw.exe"));
	java_candidates.append(MakeJavaPtr("C:/Program Files (x86)/Java/jre7/bin/javaw.exe"));
	java_candidates.append(MakeJavaPtr("C:/Program Files (x86)/Java/jre6/bin/javaw.exe"));
	java_candidates.append(JDK32s);
	java_candidates.append(MakeJavaPtr(this->GetDefaultJava()->path));

	QList<QString> candidates;
	for(JavaInstallPtr java_candidate : java_candidates)
	{
		if(!candidates.contains(java_candidate->path))
		{
			candidates.append(java_candidate->path);
		}
	}

	return candidates;
}

#elif defined(Q_OS_MAC)
QList<QString> JavaUtils::FindJavaPaths()
{
	QList<QString> javas;
	javas.append(this->GetDefaultJava()->path);
	javas.append("/Applications/Xcode.app/Contents/Applications/Application Loader.app/Contents/MacOS/itms/java/bin/java");
	javas.append("/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home/bin/java");
	javas.append("/System/Library/Frameworks/JavaVM.framework/Versions/Current/Commands/java");
	QDir libraryJVMDir("/Library/Java/JavaVirtualMachines/");
	QStringList libraryJVMJavas = libraryJVMDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (const QString &java, libraryJVMJavas) {
		javas.append(libraryJVMDir.absolutePath() + "/" + java + "/Contents/Home/bin/java");
		javas.append(libraryJVMDir.absolutePath() + "/" + java + "/Contents/Home/jre/bin/java");
	}
	QDir systemLibraryJVMDir("/System/Library/Java/JavaVirtualMachines/");
	QStringList systemLibraryJVMJavas = systemLibraryJVMDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (const QString &java, systemLibraryJVMJavas) {
		javas.append(systemLibraryJVMDir.absolutePath() + "/" + java + "/Contents/Home/bin/java");
		javas.append(systemLibraryJVMDir.absolutePath() + "/" + java + "/Contents/Commands/java");
	}
	return javas;
}

#elif defined(Q_OS_LINUX)
QList<QString> JavaUtils::FindJavaPaths()
{
	qDebug() << "Linux Java detection incomplete - defaulting to \"java\"";

	QList<QString> javas;
	javas.append(this->GetDefaultJava()->path);
	auto scanJavaDir = [&](const QString & dirPath)
	{
		QDir dir(dirPath);
		if(!dir.exists())
			return;
		auto entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
		for(auto & entry: entries)
		{

			QString prefix;
			if(entry.isAbsolute())
			{
				prefix = entry.absoluteFilePath();
			}
			else
			{
				prefix = entry.filePath();
			}

			javas.append(FS::PathCombine(prefix, "jre/bin/java"));
			javas.append(FS::PathCombine(prefix, "bin/java"));
		}
	};
	// oracle RPMs
	scanJavaDir("/usr/java");
	// general locations used by distro packaging
	scanJavaDir("/usr/lib/jvm");
	scanJavaDir("/usr/lib32/jvm");
	// javas stored in MultiMC's folder
	scanJavaDir("java");
	return javas;
}
#else
QList<QString> JavaUtils::FindJavaPaths()
{
	qDebug() << "Unknown operating system build - defaulting to \"java\"";

	QList<QString> javas;
	javas.append(this->GetDefaultJava()->path);

	return javas;
}
#endif

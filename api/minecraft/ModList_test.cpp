
#include <QTest>
#include <QTemporaryDir>
#include "TestUtil.h"

#include "FileSystem.h"
#include "ModList.h"

class ModListTest : public QObject
{
	Q_OBJECT

private
slots:
	// test for GH-1178 - install a folder with files to a mod list
	void test_1178()
	{
		// source
		QString source = QFINDTESTDATA("data/test_folder");

		// sanity check
		QVERIFY(!source.endsWith('/'));

		auto verify = [](QString path)
		{
			QDir target_dir(FS::PathCombine(path, "test_folder"));
			QVERIFY(target_dir.entryList().contains("pack.mcmeta"));
			QVERIFY(target_dir.entryList().contains("assets"));
		};

		// 1. test with no trailing /
		{
			QString folder = source;
			QTemporaryDir tempDir;
			ModList m(tempDir.path());
			m.installMod(folder);
			verify(tempDir.path());
		}

		// 2. test with trailing /
		{
			QString folder = source + '/';
			QTemporaryDir tempDir;
			ModList m(tempDir.path());
			m.installMod(folder);
			verify(tempDir.path());
		}
	}
};

QTEST_GUILESS_MAIN(ModListTest)

#include "ModList_test.moc"

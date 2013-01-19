/* Copyright 2013 MultiMC Contributors
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

#include "instancebase.h"

#include <QFileInfo>

#include "../util/pathutils.h"

InstanceBase::InstanceBase(QString dir, QObject *parent) :
	QObject(parent), 
	rootDir(dir)
{
	QFileInfo cfgFile;
	
	if (cfgFile.exists())
		config.loadFile(PathCombine(rootDir, "instance.cfg"));
}

QString InstanceBase::getRootDir() const
{
	return rootDir;
}


///////////// Config Values /////////////

// Name
QString InstanceBase::getInstName() const
{
	return config.get("name", "Unnamed").toString();
}

void InstanceBase::setInstName(QString name)
{
	config.set("name", name);
}
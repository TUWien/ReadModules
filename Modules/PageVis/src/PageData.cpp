/*******************************************************************************************************
 ReadFramework is the basis for modules developed at CVL/TU Wien for the EU project READ. 
  
 Copyright (C) 2016 Markus Diem <diem@cvl.tuwien.ac.at>
 Copyright (C) 2016 Stefan Fiel <fiel@cvl.tuwien.ac.at>
 Copyright (C) 2016 Florian Kleber <kleber@cvl.tuwien.ac.at>

 This file is part of ReadFramework.

 ReadFramework is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ReadFramework is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
  
 The READ project  has  received  funding  from  the European  Union’s  Horizon  2020  
 research  and innovation programme under grant agreement No 674943
 
 related links:
 [1] https://cvl.tuwien.ac.at/
 [2] https://transkribus.eu/Transkribus/
 [3] https://github.com/TUWien/
 [4] https://nomacs.org
 *******************************************************************************************************/

#include "PageData.h"

// framework
#include "PageParser.h"
#include "Settings.h"

// nomacs
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QDebug>
#include <QSettings>
#pragma warning(pop)

namespace rdm {


// PageData --------------------------------------------------------------------
PageData::PageData(QObject* parent) : QObject(parent) {
	
	setObjectName("PageDataProfiles");
	loadConfig("Recent Settings");
}

PageData::~PageData() {

	saveConfig("Recent Settings");
}

QVector<QSharedPointer<rdf::RegionTypeConfig> > PageData::config() const {
	return mConfig;
}

QSharedPointer<rdf::PageElement> PageData::page() const {
	return mPage;
}

QString PageData::xmlPath() const {
	return mXmlPath;
}

void PageData::loadConfig(const QString & name) {
	
	// gcc: you cannot write loadSettings(rdf::DefaultSettings(), name);
	rdf::DefaultSettings s;
	loadSettings(s, name);
}

void PageData::saveConfig(const QString & name) const {

	rdf::DefaultSettings s;
	saveSettings(s, name);
}

void PageData::setXmlPath(const QString & path) {
	mXmlPath = path;
	emit updateXml();
}

void PageData::deleteSelected() {

	if (!mPage)
		return;

	auto r = mPage->rootRegion()->selectedRegions();

	for (auto s : r) {
		mPage->rootRegion()->removeChild(s);
	}

	emit updatePage(mPage);
}

QSharedPointer<rdf::Region> PageData::addRegion(const QRectF & rect, const rdf::Region::Type& type) {

	rdf::Polygon p;
	p << rect.topLeft();
	p << rect.topRight();
	p << rect.bottomRight();
	p << rect.bottomLeft();

	auto r = rdf::RegionManager::instance().createRegion(type);
	r->setPolygon(p);

	mPage->rootRegion()->addUniqueChild(r);

	return r;
}

void PageData::parse(const QString& xmlPath) {

	if (mPage && xmlPath == mPage->xmlPath())
		return;

	rdf::PageXmlParser parser;
	parser.read(xmlPath);

	mPage = parser.page();
	qDebug() << "filename: " << mPage->imageFileName();

	emit updatePage(mPage);
}

void PageData::save(const QString& xmlPath) {

	if (!mPage)
		return;

	QString xmlPathI = xmlPath;

	if (xmlPathI.isEmpty())
		xmlPathI = mPage->xmlPath();

	rdf::PageXmlParser parser;
	parser.write(xmlPathI, mPage);

	emit updatePage(mPage);
}

void PageData::loadSettings(QSettings& settings, const QString& name) {

	settings.beginGroup(objectName());
	settings.beginGroup(name);

	QVector<QSharedPointer<rdf::RegionTypeConfig> > configs = rdf::RegionManager::instance().regionTypeConfig();

	// load from nomacs settings
	for (QSharedPointer<rdf::RegionTypeConfig> c : configs) {

		c->load(settings);
		mConfig.append(c);
	}

	settings.endGroup();
	settings.endGroup();

	emit updateConfig();
}

void PageData::saveSettings(QSettings& settings, const QString& name) const {

	settings.beginGroup(objectName());
	settings.beginGroup(name);

	for (const QSharedPointer<rdf::RegionTypeConfig> c : mConfig)
		c->save(settings);
	
	settings.endGroup();
	settings.endGroup();
}



}
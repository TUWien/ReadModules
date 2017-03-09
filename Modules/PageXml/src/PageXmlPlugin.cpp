/*******************************************************************************************************
ReadModules are plugins for nomacs developed at CVL/TU Wien for the EU project READ. 

Copyright (C) 2016 Markus Diem <diem@caa.tuwien.ac.at>
Copyright (C) 2016 Stefan Fiel <fiel@caa.tuwien.ac.at>
Copyright (C) 2016 Florian Kleber <kleber@caa.tuwien.ac.at>

This file is part of ReadModules.

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
[1] http://www.caa.tuwien.ac.at/cvl/
[2] https://transkribus.eu/Transkribus/
[3] https://github.com/TUWien/
[4] http://nomacs.org
*******************************************************************************************************/

#include "PageXmlPlugin.h"

// ReadFramework
#include "Settings.h"
#include "PageParser.h"
#include "Elements.h"
#include "ElementsHelper.h"

// nomacs
#include "DkImageStorage.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QUuid>
#include <opencv2/ml/ml.hpp>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

PageXmlPlugin::PageXmlPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	for (QString& rid : runIds)
		rid = QUuid::createUuid().toString();

	mRunIDs = runIds.toList();
	
	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_page_filter]		= tr("Filter Regions");
	menuNames[id_page_drawer]		= tr("Draw Regions");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_page_filter]		= tr("Removes all PAGE elements except for the one specified in filterName");
	statusTips[id_page_drawer]		= tr("Draws the PAGE XML regions to the image using your last settings");
	mMenuStatusTips = statusTips.toList();

	// save settings
	QSettings& s = settings();
	s.beginGroup(name());

	mConfig.saveDefaultSettings(s);
	s.endGroup();
}

PageXmlPlugin::~PageXmlPlugin() {

	qDebug() << "destroying page xml plugin...";
}

QSettings & PageXmlPlugin::settings() const {
	return rdf::Config::instance().settings();
}

void PageXmlPlugin::saveSettings(QSettings & settings) const {

	// Schwalben an den Hals, damit jeder sieht wie frei wir sind...
	settings.beginGroup(name());
	mConfig.saveSettings(settings);
	settings.endGroup();
	qDebug() << "settings saved...";
}

void PageXmlPlugin::loadSettings(QSettings & settings) {

	// update settings
	settings.beginGroup(name());
	mConfig.loadSettings(settings);
	settings.endGroup();
}

QString PageXmlPlugin::name() const {
	return "PageXmlPlugin";
}

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage PageXmlPlugin::image() const {

	return QImage(":/PageXmlPlugin/img/read.png");
};

QList<QAction*> PageXmlPlugin::createActions(QWidget* parent) {

	if (mActions.empty()) {

		for (int idx = 0; idx < id_end; idx++) {
			QAction* ca = new QAction(mMenuNames[idx], parent);
			ca->setObjectName(mMenuNames[idx]);
			ca->setStatusTip(mMenuStatusTips[idx]);
			ca->setData(mRunIDs[idx]);	// runID needed for calling function runPlugin()
			mActions.append(ca);
		}
	}

	return mActions;
}

QList<QAction*> PageXmlPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> PageXmlPlugin::runPlugin(
	const QString &runID, 
	QSharedPointer<nmc::DkImageContainer> imgC, 
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& batchInfo) const {

	
	if (!imgC)
		return imgC;

	// load suplemental XML
	QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(saveInfo.inputFilePath());
	rdf::PageXmlParser parser;
	parser.read(loadXmlPath);

	if (runID == mRunIDs[id_page_filter]) {
		// set our header info
		auto xmlPage = parser.page();
		xmlPage->setCreator(QString("CVL"));
		xmlPage->setImageSize(QSize(imgC->image().size()));
		xmlPage->setImageFileName(imgC->fileName());

		if (runID == mRunIDs[id_page_filter]) {

			filterRegions(xmlPage);
		}

		// save xml
		QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(saveInfo.outputFilePath());
		parser.write(saveXmlPath, parser.page());
	}
	else if (runID == mRunIDs[id_page_drawer]) {

		QImage img = imgC->image();
		img = img.convertToFormat(QImage::Format_RGBA8888);

		QPainter painter(&img);
		painter.setRenderHints(QPainter::Antialiasing);

		const auto pd = parser.page();
		if (parser.page() && !parser.page()->isEmpty())
			rdf::RegionManager::instance().drawRegion(painter, parser.page()->rootRegion(), mConfig.xmlConfig());

		imgC->setImage(img, tr("PAGE Attributes"));
	}

	// wrong runID? - do nothing
	return imgC;
}

void PageXmlPlugin::filterRegions(QSharedPointer<rdf::PageElement> & page) const {

	QString fn = mConfig.filterName();
	
	// does the user want to remove all elements?
	if (fn == "Page") {
		page->rootRegion()->removeAllChildren();
		return;
	}

	if (!rdf::RegionManager::instance().isValidTypeName(fn)) {
		qWarning() << fn << "is not a valid type name";
		return;
	}

	rdf::Region::Type filterType = rdf::RegionManager::instance().type(fn);
	
	auto root = page->rootRegion();
	auto regions = rdf::RegionManager::filter<rdf::Region>(page->rootRegion(), filterType);
	qDebug() << "regions filtered....";

	root->setChildren(regions);
}

// configurations that are specific for the plugin --------------------------------------------------------------------
PageXmlConfig::PageXmlConfig() : ModuleConfig("General") {
}

QString PageXmlConfig::toString() const {

	QString msg = rdf::ModuleConfig::toString();
	msg += "filtering all regions except: " + filterName();

	return msg;
}

QString PageXmlConfig::filterName() const {
	return mFilterName;
}

QVector<QSharedPointer<rdf::RegionTypeConfig>> rdm::PageXmlConfig::xmlConfig() const {
	return mXmlConfig;
}

void PageXmlConfig::load(const QSettings & settings) {

	mFilterName = settings.value("filterName", mFilterName).toString();

	// highjack the page vis plugin
	QSettings& s = rdf::Config::instance().settings();
	s.beginGroup("PageDataProfiles");

	// load the user's default profile
	s.beginGroup(s.value("DefaultProfileString", "Recent Settings").toString());

	QVector<QSharedPointer<rdf::RegionTypeConfig> > configs = rdf::RegionManager::instance().regionTypeConfig();
	for (QSharedPointer<rdf::RegionTypeConfig> c : configs) {

		c->load(s);
		mXmlConfig.append(c);
	}

	s.endGroup();
	s.endGroup();

}

void PageXmlConfig::save(QSettings & settings) const {

	settings.setValue("filterName", mFilterName);
}

};


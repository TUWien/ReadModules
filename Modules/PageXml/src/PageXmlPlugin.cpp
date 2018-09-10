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
#include "SuperPixelTrainer.h"

// nomacs
#include "DkImageStorage.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QUuid>
#include <QStandardPaths>
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
	menuNames[id_page_validator]	= tr("Validate PAGE XMLs");
	menuNames[id_page_to_gt]		= tr("Label Image from XML");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_page_filter]		= tr("Removes all PAGE elements except for the one specified in filterName");
	statusTips[id_page_drawer]		= tr("Draws the PAGE XML regions to the image using your last settings");
	statusTips[id_page_validator]	= tr("Checks if the image has a valid PAGE XML");
	statusTips[id_page_to_gt]		= tr("Renders a label image from a PAGE XML");
	mMenuStatusTips = statusTips.toList();

	// save settings
	QSettings s(settingsFilePath(), QSettings::IniFormat);
	s.beginGroup(name());

	mConfig.saveDefaultSettings(s);
	s.endGroup();
}

PageXmlPlugin::~PageXmlPlugin() {

	//qDebug() << "destroying page xml plugin...";
}

void PageXmlPlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> >& batchInfo) const {

	if (batchInfo.empty())
		return;

	if (batchInfo.first()->id() == mRunIDs[id_page_validator]) {

		QString logPath(mConfig.validatorLog());

		// create default log path
		if (logPath.isEmpty()) {
			QString td = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
			QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss");
			QString fn = tr("xml-validator-log-") + ts + ".txt";
			logPath = QFileInfo(td, fn).absoluteFilePath();
		}

		QFile fh(logPath);

		if (!fh.open(QIODevice::WriteOnly | QIODevice::Append)) {
			qWarning() << "could not save log to" << logPath;
			return;
		}

		qInfo() << "validator report -----------------------------------------------";
		QTextStream fs(&fh);

		int errCnt = 0;

		for (auto bi : batchInfo) {

			auto li = qSharedPointerDynamicCast<PageXmlInfo>(bi);

			if (li) {
				QString s = li->status();
				QString sy = li->filePath() + "\t- " + s;

				if (!s.isEmpty()) {
					fs << sy << "\n";
					errCnt++;
				}

				qInfo() << sy;
			}
		}

		fs << errCnt << "/" << batchInfo.size() << "errored\n";


		qInfo() << "validator report written to" << logPath;
	}
}

QString PageXmlPlugin::settingsFilePath() const {
	return rdf::Config::instance().settingsFilePath();
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
	else if (runID == mRunIDs[id_page_validator]) {

		QSharedPointer<PageXmlInfo> xmlInfo(new PageXmlInfo(runID, imgC->filePath()));
		
		// if everything is fine - check if the dimensions are there...
		if (parser.loadStatus() == rdf::PageXmlParser::status_ok) {

			if (parser.page()->imageSize() != imgC->image().size()) {
				xmlInfo->setStatus("ERROR page exists, but image size is wrong");
				imgC->clear();	// indicate the error
			}
			// uncomment if you want to find all 'ok'
			//else
			//	xmlInfo->setStatus("OK");
		}
		// fix empty plugins
		else if (parser.loadStatus() == rdf::PageXmlParser::status_file_empty) {

			// set minimally required values
			parser.page()->setImageFileName(imgC->fileName());
			parser.page()->setImageSize(imgC->image().size());

			// save xml
			parser.write(loadXmlPath, parser.page());
			xmlInfo->setStatus("XML fixed");
		}
		else {

			xmlInfo->setStatus(parser.loadStatusMessage());
			imgC->clear();	// indicate the error
		}

		batchInfo = xmlInfo;
	}
	else if (runID == mRunIDs[id_page_to_gt]) {

		QSharedPointer<PageXmlInfo> xmlInfo(new PageXmlInfo(runID, imgC->filePath()));

		// if everything is fine - check if the dimensions are there...
		if (parser.loadStatus() == rdf::PageXmlParser::status_ok) {

			// test loading of label lookup
			rdf::LabelManager lm = rdf::LabelManager::read(mConfig.labelConfigPath());
			qInfo().noquote() << lm.toString();
				
			rdf::SuperPixelLabeler spl;
			spl.setLabelManager(lm);
			spl.setRootRegion(parser.page()->rootRegion());

			QImage lImg = spl.createLabelImage(imgC->image().rect(), false);
			imgC->setImage(lImg, tr("Label Image"));

			if (parser.page()->imageSize() != imgC->image().size()) {
				xmlInfo->setStatus("ERROR page exists, but image size is wrong");
				imgC->clear();	// indicate the error
			}
			// uncomment if you want to find all 'ok'
			//else
			//	xmlInfo->setStatus("OK");
		}
		else {

			xmlInfo->setStatus(parser.loadStatusMessage());
			imgC->clear();	// indicate the error
		}

		batchInfo = xmlInfo;
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

QString PageXmlConfig::labelConfigPath() const {
	return mLabelConfig;
}

QString PageXmlConfig::validatorLog() const {
	return mValidatorLog;
}

QString PageXmlConfig::filterName() const {
	return mFilterName;
}

QVector<QSharedPointer<rdf::RegionTypeConfig>> rdm::PageXmlConfig::xmlConfig() const {
	return mXmlConfig;
}

void PageXmlConfig::load(const QSettings & settings) {

	mFilterName = settings.value("filterName", mFilterName).toString();
	mValidatorLog = settings.value("validatorLogFilePath", mValidatorLog).toString();
	mLabelConfig = settings.value("labelConfigPath", mLabelConfig).toString();

	// highjack the page vis plugin
	rdf::DefaultSettings s;
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
	settings.setValue("validatorLogFilePath", mValidatorLog);
	settings.setValue("labelConfigPath", mLabelConfig);
}

// PageXmlInfo --------------------------------------------------------------------
PageXmlInfo::PageXmlInfo(const QString & id, const QString & filePath) : nmc::DkBatchInfo(id, filePath) {
}

void PageXmlInfo::setStatus(const QString & str) {
	mStatus = str;
}

QString PageXmlInfo::status() const {
	return mStatus;
}

};


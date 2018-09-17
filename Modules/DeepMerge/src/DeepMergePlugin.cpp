/*******************************************************************************************************
ReadModules are plugins for nomacs developed at CVL/TU Wien for the EU project READ. 

Copyright (C) 2016 Markus Diem <diem@cvl.tuwien.ac.at>
Copyright (C) 2016 Stefan Fiel <fiel@cvl.tuwien.ac.at>
Copyright (C) 2016 Florian Kleber <kleber@cvl.tuwien.ac.at>

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
[1] https://cvl.tuwien.ac.at/
[2] https://transkribus.eu/Transkribus/
[3] https://github.com/TUWien/
[4] https://nomacs.org
*******************************************************************************************************/

#include "DeepMergePlugin.h"

// ReadFramework
#include "Settings.h"
#include "Utils.h"
#include "DeepMerge.h"
#include "DkBasicLoader.h"

// nomacs
#include "DkImageStorage.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QUuid>
#include <opencv2/ml.hpp>

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
DeepMergePlugin::DeepMergePlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	for (QString& rid : runIds)
		rid = QUuid::createUuid().toString();

	mRunIDs = runIds.toList();
	
	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_graph_cut]			= tr("Merge Graph Cut");
	menuNames[id_threshold]			= tr("Merge Threshold");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_graph_cut]		= tr("Uses a MRF for merging the tensors");
	statusTips[id_threshold]		= tr("Uses a simple threshold for merging the tensors");
	mMenuStatusTips = statusTips.toList();

	// saved default settings
	rdf::DefaultSettings s;
	s.beginGroup(name());

	DeepMergeConfig dc;
	dc.saveDefaultSettings(s);
	
	s.endGroup();
}
/**
*	Destructor
**/
DeepMergePlugin::~DeepMergePlugin() {
}

void DeepMergePlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> >& batchInfo) const {

	// add batch actions here...
}

QString DeepMergePlugin::settingsFilePath() const {
	return rdf::Config::instance().settingsFilePath();
}

void DeepMergePlugin::saveSettings(QSettings & settings) const {

	settings.beginGroup(name());
	mConfig.saveSettings(settings);
	settings.endGroup();
}

void DeepMergePlugin::loadSettings(QSettings & settings) {

	// update settings
	settings.beginGroup(name());
	mConfig.loadSettings(settings);
	settings.endGroup();
}

QString DeepMergePlugin::name() const {
	return "DeepMergePlugin";
}

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage DeepMergePlugin::image() const {

	return QImage(":/LayoutPlugin/img/read.png");
};

QList<QAction*> DeepMergePlugin::createActions(QWidget* parent) {

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

QList<QAction*> DeepMergePlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> DeepMergePlugin::runPlugin(
	const QString &runID, 
	QSharedPointer<nmc::DkImageContainer> imgC, 
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& batchInfo) const {

	//// load suplemental XML
	//QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(saveInfo.inputFilePath());
	//rdf::PageXmlParser parser;
	//parser.read(loadXmlPath);

	//// set our header info
	//auto xmlPage = parser.page();
	//xmlPage->setCreator(QString("CVL"));
	//xmlPage->setImageSize(QSize(imgC->image().size()));
	//xmlPage->setImageFileName(imgC->fileName());

	// load side-car if available

	if (!imgC)
		return imgC;

	// load side car image, if it is available
	QString sideCarPath = imgC->dirPath() + "/dm/" + rdf::Utils::createFilePath(imgC->fileName(), "-probs", "png");
	
	QImage oImg = imgC->image();
	QImage pImg = oImg;
	nmc::DkBasicLoader bl;
	if (bl.loadGeneral(sideCarPath)) {
		pImg = bl.image();
		imgC->setImage(pImg, "dhSegment");
	}
	else
		qDebug() << "could not load" << sideCarPath;

	cv::Mat imgCv = nmc::DkImage::qImage2Mat(pImg);
	cv::Mat rImg;

	if(runID == mRunIDs[id_graph_cut]) {

		rImg = nmc::DkImage::qImage2Mat(oImg);
		cv::Mat mask = compute(imgCv, rImg);

		if (mask.channels() == 1)
			cv::cvtColor(mask, mask, cv::COLOR_GRAY2RGB);

		imgC->setImage(nmc::DkImage::mat2QImage(mask), "mask");

		if (rImg.channels() == 1)
			cv::cvtColor(rImg, rImg, cv::COLOR_GRAY2RGB);
		QImage img = nmc::DkImage::mat2QImage(rImg);
		imgC->setImage(img, tr("DeepMerge Visualized"));

	}
	else if (runID == mRunIDs[id_threshold]) {

		// compute simple threshold
		rdf::DeepMerge dm(imgCv);
		rImg = dm.thresh(imgCv, 100);

		if (rImg.channels() == 1)
			cv::cvtColor(rImg, rImg, cv::COLOR_GRAY2RGB);
		QImage img = nmc::DkImage::mat2QImage(rImg);
		imgC->setImage(img, tr("Global Threshold"));
	}

	//// save xml
	//if (mConfig.saveXml()) {
	//	QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(saveInfo.outputFilePath());
	//	
	//	if (saveXmlPath.isEmpty()) {
	//		saveXmlPath = rdf::Utils::createFilePath(rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath()), "-results");
	//	}
	//	
	//	parser.write(saveXmlPath, parser.page());
	//}

	// wrong runID? - do nothing
	return imgC;
}

cv::Mat DeepMergePlugin::compute(const cv::Mat & src, cv::Mat& visImg) const {

	rdf::Timer dt;

	cv::Mat img = src.clone();
	double sf = (double)visImg.rows / src.rows;

	// compute layout analysis
	rdf::DeepMerge dm(img);
	dm.setScaleFactor(sf);

	//la.setConfig(QSharedPointer<rdf::LayoutAnalysisConfig>(new rdf::LayoutAnalysisConfig(mLAConfig)));

	if (!dm.compute())
		qWarning() << "could not compute DeepMerge...";

	visImg = dm.draw(visImg);

	return dm.image();
}

// configurations that are specific for the plugin --------------------------------------------------------------------
DeepMergeConfig::DeepMergeConfig() : ModuleConfig("General") {
}

QString DeepMergeConfig::toString() const {

	QString msg = rdf::ModuleConfig::toString();
	msg += drawResults() ? " drawing results\n" : " not drawing results\n";

	return msg;
}

bool DeepMergeConfig::drawResults() const {
	return mDrawResults;
}

bool DeepMergeConfig::saveXml() const {
	return mSaveXml;
}

void DeepMergeConfig::load(const QSettings & settings) {

	mDrawResults = settings.value("drawResults", mDrawResults).toBool();
	mSaveXml = settings.value("saveXml", mSaveXml).toBool();
}

void DeepMergeConfig::save(QSettings & settings) const {

	settings.setValue("drawResults", mDrawResults);
	settings.setValue("saveXml", mSaveXml);
}

};


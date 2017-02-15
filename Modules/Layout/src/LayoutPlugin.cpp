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

#include "LayoutPlugin.h"

// ReadFramework
#include "SuperPixel.h"
#include "LineTrace.h"
#include "Algorithms.h"
#include "Binarization.h"
#include "SkewEstimation.h"
#include "PageParser.h"
#include "Elements.h"
#include "Utils.h"
#include "TabStopAnalysis.h"
#include "TextLineSegmentation.h"
#include "SuperPixelTrainer.h"
#include "SuperPixelClassification.h"
#include "Settings.h"

#include "LayoutAnalysis.h"

// nomacs
#include "DkImageStorage.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QUuid>
#include <opencv2/ml/ml.hpp>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
LayoutPlugin::LayoutPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	for (QString& rid : runIds)
		rid = QUuid::createUuid().toString();

	mRunIDs = runIds.toList();
	
	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_layout]			= tr("Layout Analysis");
	//menuNames[id_text_block]		= tr("Page Segmentation");
	menuNames[id_lines]				= tr("Detect Lines");
	menuNames[id_layout_collect_features] = tr("Collect Layout Features");
	menuNames[id_layout_train]		= tr("Train Layout");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_layout]			= tr("Computes the Layout Analysis for a given image");
	//statusTips[id_text_block]		= tr("Computes the page segmentation");
	statusTips[id_lines]			= tr("Detects lines using a binary image");
	statusTips[id_layout_collect_features] = tr("Collects layout features for later training.");
	statusTips[id_layout_train]		= tr("Train a new model for Layout Analysis.");
	mMenuStatusTips = statusTips.toList();


	// save settings
	QSettings& s = settings();
	s.beginGroup(name());

	mConfig.saveDefaultSettings(s);
	rdf::SuperPixelTrainerConfig spc;
	spc.saveDefaultSettings(s);
	s.endGroup();
}
/**
*	Destructor
**/
LayoutPlugin::~LayoutPlugin() {

	qDebug() << "destroying layout plugin...";
}

void LayoutPlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> >& batchInfo) const {

	rdf::Config::instance().save();

	if (batchInfo.empty())
		return;

	if (batchInfo.first()->id() == mRunIDs[id_layout_collect_features]) {
		
		rdf::FeatureCollectionManager manager;

		// collect all features
		for (auto bi : batchInfo) {

			auto li = qSharedPointerDynamicCast<rdm::LayoutInfo>(bi);

			if (li) {
				manager.merge(li->featureCollectionManager());
				qInfo().noquote() << manager.toString();
			}
			else
				qCritical() << "could not cast info to LayoutInfo";
		}

		manager.normalize(mSplConfig.minNumFeaturesPerClass(), mSplConfig.maxNumFeaturesPerClass());
		manager.write(mSplConfig.featureFilePath());
		qInfo() << "features written to" << mSplConfig.featureFilePath();
	}
}

QSettings & LayoutPlugin::settings() const {
	return rdf::Config::instance().settings();
}

void LayoutPlugin::saveSettings(QSettings & settings) const {

	// Schwalben an den Hals, damit jeder sieht wie frei wir sind...
	settings.beginGroup(name());
	mConfig.saveSettings(settings);
	mSplConfig.saveSettings(settings);
	mSpcConfig.saveSettings(settings);
	mLTRConfig.saveSettings(settings);
	settings.endGroup();
	qDebug() << "settings saved...";
}

void LayoutPlugin::loadSettings(QSettings & settings) {

	// update settings
	settings.beginGroup(name());
	mConfig.loadSettings(settings);
	mSplConfig.loadSettings(settings);
	mSpcConfig.loadSettings(settings);
	mLTRConfig.loadSettings(settings);
	settings.endGroup();
	qDebug() << "settings loaded...";
}

QString LayoutPlugin::name() const {
	return "LayoutPlugin";
}

/**
* Returns unique ID for the generated dll
**/
QString LayoutPlugin::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage LayoutPlugin::image() const {

	return QImage(":/LayoutPlugin/img/read.png");
};

QList<QAction*> LayoutPlugin::createActions(QWidget* parent) {

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



QList<QAction*> LayoutPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> LayoutPlugin::runPlugin(
	const QString &runID, 
	QSharedPointer<nmc::DkImageContainer> imgC, 
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& batchInfo) const {

	// train - it's also possible without any image loaded
	if (runID == mRunIDs[id_layout_train]) {
		if (train())

		return imgC;
	}


	if (!imgC)
		return imgC;

	// load suplemental XML
	QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(saveInfo.inputFilePath());
	rdf::PageXmlParser parser;
	parser.read(loadXmlPath);

	// set our header info
	auto xmlPage = parser.page();
	xmlPage->setCreator(QString("CVL"));
	xmlPage->setImageSize(QSize(imgC->image().size()));
	xmlPage->setImageFileName(imgC->fileName());


	if(runID == mRunIDs[id_layout]) {

		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		imgCv = compute(imgCv, parser);

		if (mConfig.drawResults()) {
			QImage img = nmc::DkImage::mat2QImage(imgCv);
			imgC->setImage(img, tr("Layout Analysis Visualized"));
		}
	}
	//else if(runID == mRunIDs[id_text_block]) {

	//	cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
	//	imgCv = computePageSegmentation(imgCv, parser);

	//	if (mConfig.drawResults()) {
	//		QImage img = nmc::DkImage::mat2QImage(imgCv);
	//		imgC->setImage(img, tr("Page Segmentation"));
	//	}
	//}
	else if (runID == mRunIDs[id_lines]) {
		
		rdf::LineTrace lt = computeLines(imgC);

		QVector<rdf::Line> alllines = lt.getLines();

		//save lines to xml
		QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(saveInfo.outputFilePath());
		
		for (int i = 0; i < alllines.size(); i++) {
			
			QSharedPointer<rdf::SeparatorRegion> pSepR(new rdf::SeparatorRegion());
			pSepR->setLine(alllines[i].line());

			parser.page()->rootRegion()->addUniqueChild(pSepR);
		}

		parser.write(saveXmlPath, parser.page());

		// visualize
		if (mConfig.drawResults()) {
			cv::Mat synLine = lt.generatedLineImage();

			//visualize
			if (mConfig.drawResults()) {
				if (synLine.channels() == 1)
					cv::cvtColor(synLine, synLine, CV_GRAY2BGRA);

				QImage img = nmc::DkImage::mat2QImage(synLine);
				imgC->setImage(img, tr("Lines Detected"));
			}
		}
	}
	else if (runID == mRunIDs[id_layout_collect_features]) {

		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		
		QSharedPointer<LayoutInfo> layoutInfo(new LayoutInfo(runID, imgC->filePath()));
		imgCv = collectFeatures(imgCv, parser, layoutInfo);
		
		if (mConfig.drawResults()) {
			QImage img = nmc::DkImage::mat2QImage(imgCv);
			imgC->setImage(img, tr("Groundtruth Features"));
		}

		batchInfo = layoutInfo;
	}

	// save xml
	if (mConfig.saveXml()) {
		QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(saveInfo.outputFilePath());
		parser.write(saveXmlPath, parser.page());
	}

	// wrong runID? - do nothing
	return imgC;
}

cv::Mat LayoutPlugin::compute(const cv::Mat & src, const rdf::PageXmlParser & parser) const {

	rdf::Timer dt;

	cv::Mat img = src.clone();
	auto pe = parser.page();

	// compute layout analysis
	rdf::LayoutAnalysis la(img);
	la.setRootRegion(pe->rootRegion());

	if (!la.compute())
		qWarning() << "could not compute layout analysis";


	// write to XML --------------------------------------------------------------------
	pe->setCreator(QString("CVL"));
	pe->setImageSize(QSize(img.cols, img.rows));

	//pe->setRootRegion(la.textBlockSet().toTextRegion());
	
	auto root = la.textBlockSet().toTextRegion();
	for (const QSharedPointer<rdf::Region>& r : root->children()) {
		pe->rootRegion()->addUniqueChild(r);
	}

	qInfo() << "layout analysis computed in" << dt;

	// draw results -----------------------------------
	if (mConfig.drawResults()) {

		cv::Mat rImg = img.clone();

		// draw whatever you like
		rImg = la.draw(rImg);

		return rImg;
	}

	return src;
}

cv::Mat LayoutPlugin::computePageSegmentation(const cv::Mat & src, const rdf::PageXmlParser & parser) const {
	
	// if available, get informaton from existing xmls
	auto pe = parser.page();
	QVector<QSharedPointer<rdf::Region> > separators = rdf::Region::filter(pe->rootRegion(), rdf::Region::type_separator);
	QVector<rdf::Line> separatingLines;
	for (auto s : separators) {

		auto sc = qSharedPointerCast<rdf::SeparatorRegion>(s);
		if (sc)
			separatingLines << sc->line();
	}
	qInfo() << "I found" << separatingLines.size() << "separators in the XML";

	cv::Mat img = src.clone();
	//cv::resize(src, img, cv::Size(), 0.25, 0.25, CV_INTER_AREA);

	rdf::Timer dt;

	// find super pixels
	rdf::SuperPixel superPixel(img);

	if (!superPixel.compute())
		qWarning() << "could not compute super pixel!";

	QVector<QSharedPointer<rdf::Pixel> > sp = superPixel.getSuperPixels();

	// find local orientation per pixel
	rdf::LocalOrientation lo(sp);
	if (!lo.compute())
		qWarning() << "could not compute local orientation";

	// smooth estimation
	rdf::GraphCutOrientation pse(sp);

	if (!pse.compute())
		qWarning() << "could not compute set orientation";

	qInfo() << "algorithm computation time" << dt;

	// write XML -----------------------------------

	// start writing content

	// TODO
	//auto ps = rdf::PixelSet::fromEdges(rdf::PixelSet::connect(sp));

	//if (!ps.empty()) {
	//	QSharedPointer<rdf::Region> textRegion = QSharedPointer<rdf::Region>(new rdf::Region());
	//	textRegion->setType(rdf::Region::type_text_region);
	//	textRegion->setPolygon(ps[0]->convexHull());

	//	for (auto tl : textLines.textLines()) {
	//		textRegion->addUniqueChild(tl);
	//	}

	//	pe->rootRegion()->addUniqueChild(textRegion);
	//}

	// draw results -----------------------------------
	//cv::Mat rImg(img.rows, img.cols, CV_8UC1, cv::Scalar::all(150));
	cv::Mat rImg = img.clone();

	//// draw edges
	//rImg = textBlocks.draw(rImg);
	//rImg = lo.draw(rImg, "1012", 256);
	//rImg = lo.draw(rImg, "507", 128);
	//rImg = lo.draw(rImg, "507", 64);

	//// save super pixel image
	rImg = superPixel.draw(rImg);
	//rImg = tabStops.draw(rImg);
	//rImg = textLines.draw(rImg);

	return rImg;
}

cv::Mat LayoutPlugin::collectFeatures(const cv::Mat & src, const rdf::PageXmlParser & parser, QSharedPointer<LayoutInfo>& layoutInfo) const {

	rdf::Timer dt;

	// test loading of label lookup
	rdf::LabelManager lm = rdf::LabelManager::read(mSplConfig.labelConfigFilePath());
	qInfo().noquote() << lm.toString();

	// compute super pixels
	rdf::SuperPixel sp(src);

	if (!sp.compute())
		qCritical() << "could not compute super pixels!";

	// feed the label lookup
	rdf::SuperPixelLabeler spl(sp.getMserBlobs(), rdf::Rect(src));
	spl.setLabelManager(lm);
	spl.setFilePath(layoutInfo->filePath());	// parse filepath for gt

	// set the ground truth
	if (parser.page())
		spl.setRootRegion(parser.page()->rootRegion());

	if (!spl.compute())
		qCritical() << "could not compute SuperPixel labeling!";

	rdf::SuperPixelFeature spf(src, spl.set());
	if (!spf.compute())
		qCritical() << "could not compute SuperPixel features!";

	rdf::FeatureCollectionManager fcm(spf.features(), spf.set());
	layoutInfo->setFeatureCollectionManager(fcm);

	if (mConfig.drawResults()) {
		cv::Mat rImg = src.clone();
		rImg = spl.draw(rImg);
		//rImg = spf.draw(rImg);
		return rImg;
	}

	return src;
}

rdf::LineTrace LayoutPlugin::computeLines(QSharedPointer<nmc::DkImageContainer> imgC) const {
	
	cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());

	if (imgCv.depth() != CV_8U) {
		imgCv.convertTo(imgCv, CV_8U, 255);
	}

	if (imgCv.channels() != 1) {
		cv::cvtColor(imgCv, imgCv, CV_RGB2GRAY);
	}

	//if mask is estimated
	//cv::Mat mask = rdf::Algorithms::estimateMask(imgCv);
	cv::Mat mask = cv::Mat();

	//if skew will be used
	//rdf::BaseSkewEstimation bse;
	//bse.setImages(imgCv);
	//bse.setFixedThr(false);
	//if (!bse.compute()) {
	//	qWarning() << "could not compute skew";
	//}
	//double skewAngle = bse.getAngle();
	//skewAngle = skewAngle / 180.0 * CV_PI; //check if minus angle is needed....
	double skewAngle = 0.0f;

	rdf::BinarizationSuAdapted binarizeImg(imgCv, mask);
	binarizeImg.compute();
	cv::Mat bwImg = binarizeImg.binaryImage();

	rdf::LineTrace lt(bwImg, mask);

	//set settings
	//QSharedPointer<rdf::LineTraceConfig> cf = lt.config();
	//*cf = mLTRConfig;

	lt.setAngle(skewAngle);

	lt.compute();
	
	return lt;
}

bool LayoutPlugin::train() const {

	rdf::SuperPixelTrainerConfig spc;
	spc.loadSettings();

	rdf::FeatureCollectionManager fcm;

	for (const QString& fPath : spc.featureCachePaths()) {
		rdf::FeatureCollectionManager cFc = rdf::FeatureCollectionManager::read(fPath);
		fcm.merge(cFc);
		qInfo() << fPath << "added...";
	}


	// train classifier
	rdf::SuperPixelTrainer spt(fcm);

	if (!spt.compute()) {
		qCritical() << "could not train data...";
		return false;
	}

	spt.write(spc.modelPath());

	// test - read back the model
	auto model = rdf::SuperPixelModel::read(spc.modelPath());

	auto f = model->model();
	if (f && f->isTrained()) {
		qDebug() << "the classifier I loaded is trained...";
		return true;
	}

	qCritical() << "could not save classifier to:" << spc.modelPath();

	return false;
}

// LayoutInfo --------------------------------------------------------------------
LayoutInfo::LayoutInfo(const QString & id, const QString & filePath) : nmc::DkBatchInfo(id, filePath) {
}

void LayoutInfo::setFeatureCollectionManager(const rdf::FeatureCollectionManager & manager) {
	mManager = manager;
}

rdf::FeatureCollectionManager LayoutInfo::featureCollectionManager() const {
	return mManager;
}

// configurations that are specific for the plugin --------------------------------------------------------------------
LayoutConfig::LayoutConfig() : ModuleConfig("General") {
}

QString LayoutConfig::toString() const {

	QString msg = rdf::ModuleConfig::toString();
	msg += drawResults() ? " drawing results\n" : " not drawing results\n";
	msg += useTextRegions() ? " baselines are filtered with text regions\n" : " full image is computed\n";

	return msg;
}

bool LayoutConfig::drawResults() const {
	return mDrawResults;
}

bool LayoutConfig::saveXml() const {
	return mSaveXml;
}

bool LayoutConfig::useTextRegions() const {
	return mUseTextRegions;
}

void LayoutConfig::load(const QSettings & settings) {

	mUseTextRegions = settings.value("useTextRegions", mUseTextRegions).toBool();
	mDrawResults	= settings.value("drawResults", mDrawResults).toBool();
	mSaveXml		= settings.value("saveXml", mSaveXml).toBool();
}

void LayoutConfig::save(QSettings & settings) const {

	settings.setValue("useTextRegions", mUseTextRegions);
	settings.setValue("drawResults", mDrawResults);
	settings.setValue("saveXml", mSaveXml);
}

};


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
#include "TextBlockSegmentation.h"
#include "TextLineSegmentation.h"

// nomacs
#include "DkImageStorage.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
LayoutPlugin::LayoutPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_layout_draw] = "4cbb58ada14d4b64a17fe3285696446b";
	runIds[id_layout_xml] = "b56790b60a904a32975621e4b54ab939";
	runIds[id_lines] = "9af887d7003c44e999ba2db50d65ec85";
	runIds[id_line_img] = "49a4d36689c9411bb848d93d0eb22f5c";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_layout_draw] = tr("Draw Layout");
	menuNames[id_layout_xml] = tr("PAGE Xml");
	menuNames[id_lines] = tr("Calculate Lines (XML)");
	menuNames[id_line_img] = tr("Calculate Line Image");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_layout_draw] = tr("Draws the current layout outputs to the image");
	statusTips[id_layout_xml] = tr("Writes the layout analysis results to an XML");
	statusTips[id_lines] = tr("Calculates the lines in the binarized image");
	statusTips[id_line_img] = tr("Calculates the line image. XML is not written.");
	mMenuStatusTips = statusTips.toList();

	//mLTRConfig.loadSettings();
	//mLTRConfig.saveSettings();

}
/**
*	Destructor
**/
LayoutPlugin::~LayoutPlugin() {

	qDebug() << "destroying binarization plugin...";
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

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_layout_draw]) {

		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		
		imgCv = compute(imgCv);

		QImage img = nmc::DkImage::mat2QImage(imgCv);
		imgC->setImage(img, tr("Layout Analysis Visualized"));
	}
	else if(runID == mRunIDs[id_layout_xml]) {


		qWarning() << "not implemented yet - sorry";
	}
	else if (runID == mRunIDs[id_lines]) {

		rdf::LineTrace lt = computeLines(imgC);

		//cv::Mat lImg = lt.lineImage();
		//cv::Mat synLine = lt.generatedLineImage();
		//QVector<rdf::Line> hlines = lt.getHLines();
		//QVector<rdf::Line> vlines = lt.getVLines();
		QVector<rdf::Line> alllines = lt.getLines();

		//save lines to xml
		QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(saveInfo.inputFilePath());
		QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(saveInfo.outputFilePath());
		
		rdf::PageXmlParser parser;
		parser.read(loadXmlPath);
		auto pe = parser.page();
		//pe->setCreator(QString("CVL"));

		for (int i = 0; i < alllines.size(); i++) {
			
			QSharedPointer<rdf::SeparatorRegion> pSepR(new rdf::SeparatorRegion());
			pSepR->setLine(alllines[i].line());

			pe->rootRegion()->addUniqueChild(pSepR);
		}

		parser.write(saveXmlPath, pe);
	}
	else if (runID == mRunIDs[id_line_img]) {

		rdf::LineTrace lt = computeLines(imgC);
		cv::Mat synLine = lt.generatedLineImage();

		//visualize
		if (synLine.channels() == 1) {
			cv::cvtColor(synLine, synLine, CV_GRAY2BGRA);
		}
		QImage img = nmc::DkImage::mat2QImage(synLine);
		imgC->setImage(img, tr("Calculated lines"));
	}

	// wrong runID? - do nothing
	return imgC;
}

cv::Mat LayoutPlugin::compute(const cv::Mat & src) const {
	
	rdf::Timer dt;

	cv::Mat img = src.clone();
	//cv::resize(src, img, cv::Size(), 0.25, 0.25, CV_INTER_AREA);

	rdf::SuperPixel superPixel(img);

	if (!superPixel.compute())
		qWarning() << "could not compute super pixel!";

	QVector<QSharedPointer<rdf::Pixel> > sp = superPixel.getSuperPixels();

	rdf::LocalOrientation lo(sp);
	if (!lo.compute())
		qWarning() << "could not compute local orientation";

	rdf::GraphCutOrientation pse(sp, rdf::Rect(rdf::Vector2D(), rdf::Vector2D(img.size())));

	if (!pse.compute())
		qWarning() << "could not compute set orientation";

	//// filter according to orientation
	//QVector<QSharedPointer<rdf::Pixel> > spf;
	//for (auto pixel : sp) {
	//	if (pixel->stats()->orientation() == 0 || 
	//		pixel->stats()->orientation() == CV_PI*0.5)
	//		spf << pixel;
	//}
	//sp = spf;


	rdf::TextBlockSegmentation textBlocks(img, sp);
	if (!textBlocks.compute())
		qWarning() << "could not compute text block segmentation!";

	rdf::TextLineSegmentation textLines(rdf::Rect(img), sp);
	if (!textLines.compute())
		qWarning() << "could not compute text block segmentation!";

	qInfo() << "algorithm computation time" << dt;

	// drawing
	//cv::Mat rImg(img.rows, img.cols, CV_8UC1, cv::Scalar::all(150));
	cv::Mat rImg = img.clone();

	//// draw edges
	//rImg = textBlocks.draw(rImg);
	//rImg = lo.draw(rImg, "1012", 256);
	//rImg = lo.draw(rImg, "507", 128);
	//rImg = lo.draw(rImg, "507", 64);

	//// save super pixel image
	//rImg = superPixel.drawSuperPixels(rImg);
	rImg = textBlocks.draw(rImg);
	//rImg = textLines.draw(rImg);
	qDebug() << "layout computed in" << dt;

	return rImg;

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
	//cv::Mat mask = rdf::Algorithms::instance().estimateMask(imgCv);
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

};


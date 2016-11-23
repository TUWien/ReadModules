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

#include "FocusPlugin.h"

// nomacs
#include "DkImageStorage.h"
#include "Settings.h"
#include "FocusMeasure.h"
#include "Algorithms.h"
#include "Image.h"
#include "Utils.h"

#include "DkImageStorage.h"
#include "DkSettings.h"



#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QXmlStreamReader>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
	FocusPlugin::FocusPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_gtPage] = "89dc114070e840d7bd0aca079dae657f";
	runIds[id_fmGrad] = "43aa999c555d4142918fa65f6281b9c8";
	runIds[id_fmRel] = "afa3dc198f8c4683ba34c189375ee509";
	runIds[id_fmRelArea] = "63b8000b1e604cfba818e64b83c897f6";
	runIds[id_contMeasure] = "7620810cc8904dafa16a998fe9d5fe46";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_gtPage] = tr("Show Page GT");
	menuNames[id_fmGrad] = tr("Gradient Based Focus");
	menuNames[id_fmRel] = tr("Gradient Based Focus Normalized");
	menuNames[id_fmRelArea] = tr("Gradient Based Focus Normalized- wrt Area");
	menuNames[id_contMeasure] = tr("Contrast Measure");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_gtPage] = tr("Shows the Page GT");
	statusTips[id_fmGrad] = tr("Estimates the Focus based on Gradients");
	statusTips[id_fmRel] = tr("Estimates the Focus based on Gradients (Normalized)");
	statusTips[id_fmRelArea] = tr("Estimates the Focus based on Gradients (Normalized and wrt to the area)");
	statusTips[id_contMeasure] = tr("Estimates Contrast");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
	FocusPlugin::~FocusPlugin() {

	qDebug() << "destroying focus plugin...";
}


/**
* Returns unique ID for the generated dll
**/
QString FocusPlugin::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage FocusPlugin::image() const {

	return QImage(":/ReadConfig/img/read.png");
};

QList<QAction*> FocusPlugin::createActions(QWidget* parent) {

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



QList<QAction*> FocusPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> FocusPlugin::runPlugin(
	const QString &runID, 
	QSharedPointer<nmc::DkImageContainer> imgC, 
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& info) const {

	if (!imgC)
		return imgC;

	if (runID == mRunIDs[id_gtPage]) {
		QFileInfo xmlFileI(imgC->fileInfo().absolutePath(), imgC->fileInfo().baseName() + ".xml");

		QImage img = imgC->image();
		QImage fmImg = img.copy();
		
		if (!xmlFileI.exists()) {
			qWarning() << "no xml file found: " << xmlFileI.absoluteFilePath();
		}
		else {
			QFile xmlFile(xmlFileI.absoluteFilePath());
			if (xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
				qDebug() << xmlFileI.absoluteFilePath() << " loaded...";
			}
			int x0, x1, x2, x3, y0, y1, y2, y3;

			QXmlStreamReader xmlReader(&xmlFile);
			while (!xmlReader.atEnd() && !xmlReader.hasError()) {

				QString tag = xmlReader.qualifiedName().toString();

				if (xmlReader.tokenType() == QXmlStreamReader::StartElement && tag == "dmrz") {
					//qDebug() << xmlReader.name().toString();
					x0 = xmlReader.attributes().value("x0").toInt();
					y0 = xmlReader.attributes().value("y0").toInt();
					x1 = xmlReader.attributes().value("x1").toInt();
					y1 = xmlReader.attributes().value("y1").toInt();
					x2 = xmlReader.attributes().value("x2").toInt();
					y2 = xmlReader.attributes().value("y2").toInt();
					x3 = xmlReader.attributes().value("x3").toInt();
					y3 = xmlReader.attributes().value("y3").toInt();

					QPolygon rect;
					rect << QPoint(x0, y0) << QPoint(x1, y1) << QPoint(x2, y2) << QPoint(x3, y3);

					//qDebug() << xmlReader.name().toString();
					//QXmlStreamAttributes attributes = xmlReader.attributes();
					//if (attributes.hasAttribute("x2")) {
					//	qDebug() << attributes.value("x2").toString();
					//}

					QPainter p(&fmImg);

					QPen myPen;
					myPen.setWidth(5);

					myPen.setColor(QColor(100, 100, 100));
					p.setPen(myPen);
					p.drawPolygon(rect);
					p.end();

				}
				xmlReader.readNext();
			}
			
		}

		imgC->setImage(fmImg, "GT...");

	}
	else if(runID == mRunIDs[id_fmGrad]) {

		QImage img = imgC->image();
		

		cv::Mat inputImg = rdf::Image::qImage2Mat(img);
		rdf::FocusEstimation fe;
		int w = inputImg.cols < inputImg.rows ? inputImg.cols : inputImg.rows;
		int ws = (int)ceil((double)w / 5.0);
		//int ws = 500;
		fe.setWindowSize(ws);

		fe.setImg(inputImg);

		//if (!fe.compute(rdf::FocusEstimation::FocusMeasure::GLLV)) {
		//if (!fe.compute(rdf::FocusEstimation::FocusMeasure::GLVA)) {
		//if (!fe.compute(rdf::FocusEstimation::FocusMeasure::GLVN)) {
		//if (!fe.compute(rdf::FocusEstimation::FocusMeasure::GRAS)) {
		//if (!fe.compute(rdf::FocusEstimation::FocusMeasure::GRAT)) {
		//if (!fe.compute(rdf::FocusEstimation::FocusMeasure::LAPE)) {  // <- test in future
		//if (!fe.compute(rdf::FocusEstimation::FocusMeasure::LAPV)) {
		//if (!fe.compute(rdf::FocusEstimation::FocusMeasure::ROGR)) {
		if (!fe.compute()) {  // = BREN
			qWarning() << "could not compute focus measures...";
		}

		std::vector<rdf::Patch> results = fe.fmPatches();

		QImage fmImg = img.copy();
		QPainter p(&fmImg);
		p.setPen(QPen(QColor(255, 0, 0)));
		p.setBrush(Qt::NoBrush);
		
		for (int i = 0; i < results.size(); i++) {
			rdf::Patch tmpPatch = results[i];
			cv::Point tmpPoint = tmpPatch.center() - cv::Point(20,20);
			double fmV = tmpPatch.fm();
			QString fmVal = QString::number(fmV,'g', 2);

			QRect fmCenter(QPoint(tmpPoint.x, tmpPoint.y), QSize(40, 40));
			QRect fmCenter2(QPoint(tmpPatch.upperLeft().x, tmpPatch.upperLeft().y), QSize(ws, ws));
			
			QPen myPen;
			if (fmV < 0.02) {
				myPen.setColor(QColor(255, 0, 0));
			}
			else {
				myPen.setColor(QColor(0, 255, 0));
			}
			
			myPen.setWidth(5);
			//p.setPen(QPen(QColor(255, 0, 0)));
			p.setPen(myPen);
			p.drawRect(fmCenter);

			myPen.setColor(QColor(0, 0, 255));
			p.setPen(myPen);
			p.drawRect(fmCenter2);


			myPen.setColor(QColor(0, 0, 255));
			myPen.setWidth(0);
			QFont f;
			f.setPixelSize(40);
			p.setFont(f);
			p.setPen(myPen);
			cv::Point tmpCenter = tmpPatch.center();
			//p.setPen(QPen(QColor(100, 100, 100)));
			p.drawText(QPoint(tmpCenter.x-40, tmpCenter.y-30), fmVal);
			//p.drawText(fmCenter, Qt::AlignCenter, fmVal);
		}
		p.end();

		imgC->setImage(fmImg, "Focus measures...");

		QSharedPointer<FocusInfo> testInfo(new FocusInfo(runID, imgC->filePath()));
		testInfo->setProperty("gradient based...");
		qDebug() << "gradient based...";

		info = testInfo;
	}
	else if(runID == mRunIDs[id_fmRel]) {


		QImage img = imgC->image();

		cv::Mat inputImg = rdf::Image::qImage2Mat(img);
		rdf::FocusEstimation fe;


		//cv::Mat binImg;
		//cv::cvtColor(inputImg, inputImg, CV_RGB2GRAY);
		//inputImg.convertTo(binImg, CV_8U, 255);
		//
		//cv::threshold(binImg, binImg, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
		//binImg.convertTo(binImg, CV_32F);


		int w = inputImg.cols < inputImg.rows ? inputImg.cols : inputImg.rows;
		int ws = (int)ceil((double)w / 5.0);
		//int ws = 500;
		fe.setWindowSize(ws);
		fe.setImg(inputImg);

		rdf::Image::imageInfo(inputImg, "fe ");

		//if (!fe.compute(rdf::FocusEstimation::FocusMeasure::LAPV)) {
		if (!fe.compute()) {
			qWarning() << "could not compute focus measures...";
		}

		rdf::Timer dt;

		std::vector<rdf::Patch> results = fe.fmPatches();
		//fe.computeRefPatches(rdf::FocusEstimation::FocusMeasure::LAPV);
		fe.computeRefPatches();
		std::vector<rdf::Patch> refResults = fe.fmPatches();
		
		qDebug() << " this fm version took me: " << dt;


		QImage fmImg = img.copy();
		QPainter p(&fmImg);
		p.setPen(QPen(QColor(255, 0, 0)));
		p.setBrush(Qt::NoBrush);

		for (int i = 0; i < results.size(); i++) {
			rdf::Patch tmpPatch = results[i];
			rdf::Patch tmpPatchRef = refResults[i];
			cv::Point tmpPoint = tmpPatch.center() - cv::Point(20, 20);
			double refVal = tmpPatchRef.fm();
			double fmV = refVal > 0 ? tmpPatch.fm() / tmpPatchRef.fm() : 0;
			QString fmVal = QString::number(fmV, 'g', 2);

			QRect fmCenter(QPoint(tmpPoint.x, tmpPoint.y), QSize(40, 40));
			QRect fmCenter2(QPoint(tmpPatch.upperLeft().x, tmpPatch.upperLeft().y), QSize(ws, ws));

			QPen myPen;
			//if (fmV < 0.4) {
			if (fmV < 0.15) {
				myPen.setColor(QColor(255, 0, 0));
			}
			else {
				myPen.setColor(QColor(0, 255, 0));
			}

			myPen.setWidth(5);
			//p.setPen(QPen(QColor(255, 0, 0)));
			p.setPen(myPen);
			p.drawRect(fmCenter);

			myPen.setColor(QColor(0, 0, 255));
			p.setPen(myPen);
			p.drawRect(fmCenter2);


			myPen.setColor(QColor(0, 0, 255));
			myPen.setWidth(0);
			QFont f;
			f.setPixelSize(40);
			p.setFont(f);
			p.setPen(myPen);
			cv::Point tmpCenter = tmpPatch.center();
			//p.setPen(QPen(QColor(100, 100, 100)));
			p.drawText(QPoint(tmpCenter.x - 40, tmpCenter.y - 30), fmVal);
			//p.drawText(fmCenter, Qt::AlignCenter, fmVal);
		}
		p.end();


		imgC->setImage(fmImg, "Focus measures...");
		
		//QImage tmp = rdf::Image::mat2QImage(binImg);
		//tmp = tmp.convertToFormat(QImage::Format_ARGB32);
		//imgC->setImage(tmp, "binImg...");


		QSharedPointer<FocusInfo> testInfo(new FocusInfo(runID, imgC->filePath()));
		testInfo->setProperty("gradient based and normalized wrt binary...");
		qDebug() << "gradient based and normalized wrt binary...";

		info = testInfo;
	}
	else if (runID == mRunIDs[id_fmRelArea]) {


		QImage img = imgC->image();


		cv::Mat inputImg = rdf::Image::qImage2Mat(img);
		rdf::FocusEstimation fe;

		//cv::Mat binImg;
		//cv::cvtColor(inputImg, inputImg, CV_RGB2GRAY);
		//inputImg.convertTo(binImg, CV_8U, 255);
		//cv::threshold(binImg, binImg, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
		////binImg.convertTo(binImg, CV_64F);

		int w = inputImg.cols < inputImg.rows ? inputImg.cols : inputImg.rows;
		int ws = (int)ceil((double)w / 5.0);
		//int ws = 500;
		fe.setWindowSize(ws);
		fe.setImg(inputImg);


		if (!fe.compute()) {
			qWarning() << "could not compute focus measures...";
		}
		rdf::Timer dt;

		std::vector<rdf::Patch> results = fe.fmPatches();
		fe.computeRefPatches(rdf::FocusEstimation::FocusMeasure::BREN, true);
		std::vector<rdf::Patch> refResults = fe.fmPatches();


		qDebug() << "this fm version took me: " << dt;


		QImage fmImg = img.copy();
		QPainter p(&fmImg);
		p.setPen(QPen(QColor(255, 0, 0)));
		p.setBrush(Qt::NoBrush);

		for (int i = 0; i < results.size(); i++) {
			rdf::Patch tmpPatch = results[i];
			rdf::Patch tmpPatchRef = refResults[i];
			cv::Point tmpPoint = tmpPatch.center() - cv::Point(20, 20);
			double refVal = tmpPatchRef.fm();
			double fmV = refVal > 0 ? tmpPatch.fm() / tmpPatchRef.fm() : 0;
			//fmV *= tmpPatchRef.weight();
			QString fmVal = QString::number(fmV, 'g', 2);

			QRect fmCenter(QPoint(tmpPoint.x, tmpPoint.y), QSize(40, 40));
			QRect fmCenter2(QPoint(tmpPatch.upperLeft().x, tmpPatch.upperLeft().y), QSize(ws, ws));

			QPen myPen;
			//if (fmV < 0.4) {
			if (fmV < 0.15 && tmpPatchRef.area() > 0.03) {
				myPen.setColor(QColor(255, 0, 0));
			}
			else if (tmpPatchRef.area() > 0.03) {
				myPen.setColor(QColor(0, 255, 0));
			}
		    else {
				myPen.setColor(QColor(100, 100, 100));
			}

			myPen.setWidth(5);
			//p.setPen(QPen(QColor(255, 0, 0)));
			p.setPen(myPen);
			p.drawRect(fmCenter);

			myPen.setColor(QColor(0, 0, 255));
			p.setPen(myPen);
			p.drawRect(fmCenter2);


			myPen.setColor(QColor(0, 0, 255));
			myPen.setWidth(0);
			QFont f;
			f.setPixelSize(40);
			p.setFont(f);
			p.setPen(myPen);
			cv::Point tmpCenter = tmpPatch.center();
			//p.setPen(QPen(QColor(100, 100, 100)));
			p.drawText(QPoint(tmpCenter.x - 40, tmpCenter.y - 30), fmVal);
			//p.drawText(fmCenter, Qt::AlignCenter, fmVal);
		}
		p.end();

		imgC->setImage(fmImg, "Focus measures...");

		//QImage img2 = nmc::DkImage::mat2QImage(binImg);
		//img2 = img2.convertToFormat(QImage::Format_ARGB32);
		//imgC->setImage(img2, "binimg");


		QSharedPointer<FocusInfo> testInfo(new FocusInfo(runID, imgC->filePath()));
		testInfo->setProperty("gradient based and normalized wrt binary...");
		qDebug() << "gradient based and normalized wrt binary...";

		info = testInfo;
	}
	else if (runID == mRunIDs[id_contMeasure]) {
		QImage img = imgC->image();


		cv::Mat inputImg = rdf::Image::qImage2Mat(img);
		rdf::ContrastEstimation ce;
		//int w = inputImg.cols < inputImg.rows ? inputImg.cols : inputImg.rows;
		//int ws = (int)ceil((double)w / 5.0);
		//int ws = 500;
		ce.setWindowSize(200);
		int ws = ce.windowSize();
		//ce.setWindowSize(ws);
		//ce.setLum(true);
		ce.setImg(inputImg);

		//if (!ce.compute(rdf::ContrastEstimation::ContrastMeasure::RMS)) {
		//if (!ce.compute(rdf::ContrastEstimation::ContrastMeasure::MICHELSON)) {
		if (!ce.compute(rdf::ContrastEstimation::ContrastMeasure::WEBER)) {
			qWarning() << "could not compute focus measures...";
		}


		std::vector<rdf::Patch> results = ce.cPatches();

		QImage fmImg = img.copy();
		QPainter p(&fmImg);
		p.setPen(QPen(QColor(255, 0, 0)));
		p.setBrush(Qt::NoBrush);

		for (int i = 0; i < results.size(); i++) {
			rdf::Patch tmpPatch = results[i];
			cv::Point tmpPoint = tmpPatch.center() - cv::Point(20, 20);
			double cV = tmpPatch.fm();
			QString cVal = QString::number(cV, 'g', 2);

			QRect fmCenter(QPoint(tmpPoint.x, tmpPoint.y), QSize(40, 40));
			QRect fmCenter2(QPoint(tmpPatch.upperLeft().x, tmpPatch.upperLeft().y), QSize(ws, ws));

			QPen myPen;
			myPen.setColor(QColor(0, 255, 0));
			myPen.setWidth(5);
			//p.setPen(QPen(QColor(255, 0, 0)));
			p.setPen(myPen);
			//p.drawRect(fmCenter);

			myPen.setColor(QColor(0, 0, 255));
			p.setPen(myPen);
			p.drawRect(fmCenter2);


			myPen.setColor(QColor(0, 0, 255));
			myPen.setWidth(0);
			QFont f;
			f.setPixelSize(40);
			p.setFont(f);
			p.setPen(myPen);
			cv::Point tmpCenter = tmpPatch.center();
			//p.setPen(QPen(QColor(100, 100, 100)));
			p.drawText(QPoint(tmpCenter.x - 40, tmpCenter.y), cVal);
			//p.drawText(fmCenter, Qt::AlignCenter, fmVal);
		}
		p.end();

		imgC->setImage(fmImg, "Contrast measures...");

		QSharedPointer<FocusInfo> testInfo(new FocusInfo(runID, imgC->filePath()));
		testInfo->setProperty("contrast based...");
		qDebug() << "contrast based...";

		info = testInfo;
	}

	// wrong runID? - do nothing
	return imgC;
}

void FocusPlugin::preLoadPlugin() const {

	qDebug() << "[PRE LOADING] Batch Test";
}

void FocusPlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo>>& batchInfo) const {
	int runIdx = mRunIDs.indexOf(batchInfo.first()->id());

	for (auto bi : batchInfo) {
		qDebug() << bi->filePath() << "computed...";
		FocusInfo* tInfo = dynamic_cast<FocusInfo*>(bi.data());

		if (tInfo)
			qDebug() << "property: " << tInfo->property();

	}

	if (runIdx == id_fmGrad)
		qDebug() << "[POST LOADING] grayscale";
	else
		qDebug() << "[POST LOADING] mirrored";
}

// DkTestInfo --------------------------------------------------------------------
FocusInfo::FocusInfo(const QString& id, const QString & filePath) : nmc::DkBatchInfo(id, filePath) {
}

void FocusInfo::setProperty(const QString & p) {
	mProp = p;
}

QString FocusInfo::property() const {
	return mProp;
}

};


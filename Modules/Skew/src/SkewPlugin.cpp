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

The READ project  has  received  funding  from  the European  Union�s  Horizon  2020  
research  and innovation programme under grant agreement No 674943

related links:
[1] https://cvl.tuwien.ac.at/
[2] https://transkribus.eu/Transkribus/
[3] https://github.com/TUWien/
[4] https://nomacs.org
*******************************************************************************************************/

#include "SkewPlugin.h"

// nomacs
#include "DkImageStorage.h"
#include "DkSettings.h"

#include "Settings.h"
#include "SkewEstimation.h"
#include "Image.h"
#include "Algorithms.h"
#include "ImageProcessor.h"
#include "GraphCut.h"

// skew textline
#include "SuperPixel.h"
#include "Utils.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {


bool lessThanSkew(const QVector3D v1, const QVector3D v2) {

	return (std::abs(v1.x() - v1.y()) <= std::abs(v2.x() - v2.y()));
}

/**
*	Constructor
**/
SkewEstPlugin::SkewEstPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_skew_native] = "47ac780be495490c90f772ed315fc64d";
	runIds[id_skew_doc] = "b849c12a5c124520b1c0b0761e86db37";
	runIds[id_skew_textline] = "bf0d046c895446069fd0433f92ab8a38";
	runIds[id_skew_textline_draw] = "78de3e5eef6249fbb2921b0a59d6c716";
	
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_skew_native] = tr("Skew Native");
	menuNames[id_skew_doc] = tr("Skew Document");
	menuNames[id_skew_textline] = tr("Skew Textline");
	menuNames[id_skew_textline_draw] = tr("Draw Skew Textline");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_skew_native] = tr("Calculates the skew");
	statusTips[id_skew_doc] = tr("Calculates the skew for documents");
	statusTips[id_skew_textline] = tr("Calculates the skew for documents using textlines");
	statusTips[id_skew_textline_draw] = tr("Shows a debugging graphics for texlineline skew");
	mMenuStatusTips = statusTips.toList();

	// TODO: this must be a setting! - now it's DISEC
	mMinAngle = -15 * DK_DEG2RAD;
	mMaxAngle = 15 * DK_DEG2RAD;

	init();
}
/**
*	Destructor
**/
SkewEstPlugin::~SkewEstPlugin() {

	//qDebug() << "destroying skew plugin...";
}

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage SkewEstPlugin::image() const {

	return QImage(":/SkewPlugin/img/read.png");
}

QString SkewEstPlugin::name() const {
	return "Skew Estimation";
}

QList<QAction*> SkewEstPlugin::createActions(QWidget* parent) {

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



QList<QAction*> SkewEstPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> SkewEstPlugin::runPlugin(
	const QString &runID, 
	QSharedPointer<nmc::DkImageContainer> imgC, 
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& info) const {

	qDebug() << "running skew plugin...";

	if (!imgC)
		return imgC;

	if (runID == mRunIDs[id_skew_native]) {

		QSharedPointer<SkewInfo> skewInfo(new SkewInfo(runID, imgC->filePath()));
		skewNative(imgC, skewInfo);
		info = skewInfo;
	}
	else if (runID == mRunIDs[id_skew_doc]) {
		QSharedPointer<SkewInfo> skewInfo(new SkewInfo(runID, imgC->filePath()));
		skewDoc(imgC, skewInfo);
		info = skewInfo;
	}
	else if (runID == mRunIDs[id_skew_textline] || runID == mRunIDs[id_skew_textline_draw]) {
		QSharedPointer<SkewInfo> skewInfo(new SkewInfo(runID, imgC->filePath()));
		skewTextLine(imgC, skewInfo, runID);
		info = skewInfo;
	}
	else
		qWarning() << "unknown run ID: " << runID;

	// wrong runID? - do nothing
	return imgC;
}

void SkewEstPlugin::preLoadPlugin() const {

	qDebug() << "[PRE LOADING] Batch Test";
}

void SkewEstPlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo>>& batchInfo) const {
	
	if (batchInfo.empty())
		return;
	
	int runIdx = mRunIDs.indexOf(batchInfo.first()->id());

	QVector<QVector3D> angles;
	QVector<QString> filenames;

	double errorAcc = 0;
	double errorCE = 0;
	double errCeCnt = 0;

	for (auto bi : batchInfo) {

		if (!bi)
			continue;

		qDebug() << bi->filePath() << "computed...";
		SkewInfo* tInfo = dynamic_cast<SkewInfo*>(bi.data());

		if (tInfo) {
			qDebug() << "property: " << tInfo->property();

			double sk = tInfo->skew();
			double skGT = tInfo->skewGt();
			QString fileN = tInfo->property();

			double error = std::abs(sk - skGT);
			errorAcc += error;
			if (error <= 0.1) {
				errorCE += error;
				errCeCnt++;
			}
			
			angles.push_back(QVector3D((float)sk, (float)skGT, (float)error));
			filenames.push_back(fileN);
			//push back
		}
	}

	//write to file
	QString fp;
	//QString fp = "D:\\tmp\\evalSkew.txt";
	//QString fp = "F:\\flo\\evalSkew.txt";
	fp = mFilePath;
	
	QFile file(fp);
	if (file.open(QIODevice::WriteOnly)) {

		for (int i = 0; i < angles.size(); i++) {

			QString tmpStr = QString::number(angles[i].x());
			tmpStr += " ";
			tmpStr += QString::number(angles[i].y());
			tmpStr += " ";
			tmpStr += filenames[i];
			tmpStr += "\n";
			QTextStream out(&file);
			out << tmpStr;
		}
		file.close();
	}

	//write metrics as debug output
	double aed = errorAcc / (double)angles.size();
	double ce = (double)errCeCnt / (double)angles.size();
	//double ce2 = errorCE / (double)errCeCnt;
	qDebug() << "AED: " << aed;
	qDebug() << "CE: " << ce;
	//qDebug() << "CE2: " << ce2;
	qSort(angles.begin(), angles.end(), lessThanSkew);
	int m = (int)(angles.size() * 0.8);
	double top80 = 0;
	for (int i = 0; i <= m; i++) {
		top80 += angles[i].z();
	}
	top80 /= (float)m;
	qDebug() << "Top80: " << top80;

	rdf::DefaultSettings s;
	saveSettings(s);

	if (runIdx == id_skew_native)
		qDebug() << "[POST LOADING] skew native";
	else
		qDebug() << "[POST LOADING] skew doc";
}

void SkewEstPlugin::setFilePath(QString fp)
{
	mFilePath = fp;
}

QString SkewEstPlugin::filePath() const
{
	return mFilePath;
}

void SkewEstPlugin::init() {
	
	rdf::DefaultSettings s;
	loadSettings(s);

	if (mFilePath.isEmpty()) {
		mFilePath = "D:\\tmp\\evalSkew.txt";
	}
}

void SkewEstPlugin::loadSettings(QSettings & settings)
{
	settings.beginGroup("SkewEstimation");

	mFilePath = settings.value("skewEvalPath", mFilePath).toString();
	settings.endGroup();
}

void SkewEstPlugin::saveSettings(QSettings & settings) const {
	settings.beginGroup("SkewEstimation");
	settings.setValue("skewEvalPath", mFilePath);
	settings.endGroup();
}

void SkewEstPlugin::skewTextLine(QSharedPointer<nmc::DkImageContainer>& imgC, QSharedPointer<SkewInfo>& skewInfo, const QString& runId) const {

	if (!imgC)
		return;

	cv::Mat img = rdf::Image::qImage2Mat(imgC->image());

	rdf::TextLineSkew tls(img);

	if (!tls.compute()) {
		qWarning() << "could not compute text-line based skew estimation";
	}
	
	cv::Mat oImg;
	if (runId == mRunIDs[id_skew_textline]) {
		
		// apply angle to image
		oImg = tls.rotated(img);
	}
	else if(runId == mRunIDs[id_skew_textline_draw]) {
		oImg = tls.draw(img);
	}

	imgC->setImage(rdf::Image::mat2QImage(oImg), "Skew corrected");

	parseGT(imgC->fileName(), tls.getAngle(), skewInfo);
}

void SkewEstPlugin::parseGT(const QString & fileName, double skewAngle, QSharedPointer<SkewInfo>& skewInfo) const
{

	//parse string
	QRegExp rx("[+-]?[0-9]*[\\.?][0-9]*");
	//eigentlich richtig
	////QRegExp rx("\\[[+-]?[0-9]*[\\.]?[0-9]*\\]");
	//QString test = "IMG(0006)_SA[-5.76].png";
	int gtFound = rx.indexIn(fileName);
	QStringList list = rx.capturedTexts();
	double skewGt = 0;
	if (list.size() != 1) {
		qWarning() << "no GT found";
	}
	else {
		QString skGTs = list[0];
		skewGt = skGTs.toDouble();
	}

	skewInfo->setSkew(-skewAngle / CV_PI*180.0);
	skewInfo->setSkewGt(skewGt);
	skewInfo->setProperty(fileName);

}

void SkewEstPlugin::skewNative(QSharedPointer<nmc::DkImageContainer>& imgC, QSharedPointer<SkewInfo>& skewInfo) const
{

	QImage img = imgC->image();

	rdf::BaseSkewEstimation bse;
	cv::Mat inputImg = rdf::Image::qImage2Mat(img);
	//if (inputImg.channels() != 1) cv::cvtColor(inputImg, inputImg, CV_RGB2GRAY);

	bse.setImages(inputImg);
	QSharedPointer<rdf::BaseSkewEstimationConfig> cf = bse.config();
	*cf = mBseConfig;

	qDebug() << "cf delta: " << bse.config()->delta();

	int w = qRound(inputImg.cols / 1430.0*49.0); //check  (nomacs plugin version)
	cf->setWidth(w);
	//bse.setW(w);
	int h = qRound(inputImg.rows / 700.0*12.0); //check (nomacs plugin version)
	cf->setHeight(h);
	//bse.setH(h);
	int delta = qRound(inputImg.cols / 1430.0*20.0); //check (nomacs plugin version)
	cf->setDelta(delta);
	//bse.setDelta(delta);
	int minLL = qRound(inputImg.cols / 1430.0 * 20.0); //check
	cf->setMinLineLength(minLL);
	cf->setThr(0.1);
	//bse.setmMinLineLength(minLL);
	//bse.setThr(0.1);
	bse.setFixedThr(false);


	bool skewComp = bse.compute();
	if (!skewComp) {
		qDebug() << "could not compute skew";
	}

	double skewAngle = bse.getAngle();
	skewAngle = -skewAngle / 180.0 * CV_PI;

	cv::Mat rotatedImage = rdf::IP::rotateImage(inputImg, skewAngle);
	if (rotatedImage.channels() == 1) {
		cv::cvtColor(rotatedImage, rotatedImage, CV_GRAY2BGRA);
	}

	QImage result = rdf::Image::mat2QImage(rotatedImage);

	imgC->setImage(result, "Skew corrected");

	//parse string
	parseGT(imgC->fileName(), skewAngle, skewInfo);
	qDebug() << "skew calculated...";

	//saveSettings(rdf::Config::instance().settings());

}

void SkewEstPlugin::skewDoc(QSharedPointer<nmc::DkImageContainer>& imgC, QSharedPointer<SkewInfo>& skewInfo) const
{
	QImage img = imgC->image();

	rdf::BaseSkewEstimation bse;
	cv::Mat inputImg = rdf::Image::qImage2Mat(img);
	//if (inputImg.channels() != 1) cv::cvtColor(inputImg, inputImg, CV_RGB2GRAY);

	bse.setImages(inputImg);
	bse.setFixedThr(false);

	QSharedPointer<rdf::BaseSkewEstimationConfig> cf = bse.config();
	*cf = mBseConfig;

	//use this settings for documents (best results based on disec evaluation):
	//Attention: overrides settings file
	//cf->setWidth(60);
	//cf->setHeight(28);
	//cf->setSigma(0.5);
	//bse.setW(60);
	//bse.setH(28);
	//bse.setSigma(0.5);


	bool skewComp = bse.compute();
	if (!skewComp) {
		qDebug() << "could not compute skew";
	}

	double skewAngle = bse.getAngle();
	skewAngle = -skewAngle / 180.0 * CV_PI;

	cv::Mat rotatedImage = rdf::IP::rotateImage(inputImg, skewAngle);
	if (rotatedImage.channels() == 1) {
		cv::cvtColor(rotatedImage, rotatedImage, CV_GRAY2BGRA);
	}

	QImage result = rdf::Image::mat2QImage(rotatedImage);

	imgC->setImage(result, "Skew corrected");

	parseGT(imgC->fileName(), skewAngle, skewInfo);
	qDebug() << "skew calculated...";

	//saveSettings(rdf::Config::instance().settings());


}

// DkTestInfo --------------------------------------------------------------------
SkewInfo::SkewInfo(const QString& id, const QString & filePath) : nmc::DkBatchInfo(id, filePath) {
}

void SkewInfo::setProperty(const QString & p) {
	mProp = p;
}

QString SkewInfo::property() const {
	return mProp;
}

void SkewInfo::setSkew(const double skew)
{
	mSkew = skew;
}

double SkewInfo::skew() const
{
	return mSkew;
}

void SkewInfo::setSkewGt(const double skew)
{
	mSkewGt = skew;
}

double SkewInfo::skewGt() const
{
	return mSkewGt;
}

};


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

#include "SkewPlugin.h"

// nomacs
#include "DkImageStorage.h"
#include "Image.h"
#include "Algorithms.h"

#include "Settings.h"
#include "SkewEstimation.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
SkewEstPlugin::SkewEstPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_skewnative] = "47ac780be495490c90f772ed315fc64d";
	runIds[id_skewdoc] = "b849c12a5c124520b1c0b0761e86db37";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_skewnative] = tr("Skew Native");
	menuNames[id_skewdoc] = tr("Skew Document");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_skewnative] = tr("Calculates the skew");
	statusTips[id_skewdoc] = tr("Calculates the skew for documents");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
SkewEstPlugin::~SkewEstPlugin() {

	qDebug() << "destroying skew plugin...";
}


/**
* Returns unique ID for the generated dll
**/
QString SkewEstPlugin::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage SkewEstPlugin::image() const {

	return QImage(":/SkewPlugin/img/read.png");
};

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
QSharedPointer<nmc::DkImageContainer> SkewEstPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC, QSharedPointer<nmc::DkBatchInfo>& info) const {

	qDebug() << "running skew plugin...";

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_skewnative]) {

		QImage img = imgC->image();
		
		rdf::BaseSkewEstimation bse;
		cv::Mat inputImg = rdf::Image::instance().qImage2Mat(img);
		if (inputImg.channels() != 1) cv::cvtColor(inputImg, inputImg, CV_RGB2GRAY);

		bse.setImages(inputImg);
		bool skewComp = bse.compute();
		if (!skewComp) {
			qDebug() << "could not compute skew";
		}

		double skewAngle = bse.getAngle();
		skewAngle = -skewAngle / 180.0 * CV_PI;
		
		cv::Mat rotatedImage = rdf::Algorithms::instance().rotateImage(inputImg, skewAngle);
		cv::cvtColor(rotatedImage, rotatedImage, CV_GRAY2BGRA);

		QImage result = rdf::Image::instance().mat2QImage(rotatedImage);
		
		imgC->setImage(result, "Skew corrected");

		QSharedPointer<SkewInfo> skewInfo(new SkewInfo(runID, imgC->filePath()));
		//testInfo->setProperty("Mirrored");
		skewInfo->setSkew(skewAngle);
		skewInfo->setProperty(imgC->fileName());
		qDebug() << "skew calculated...";

		info = skewInfo; 
	}
	else if(runID == mRunIDs[id_skewdoc]) {

	}

	// wrong runID? - do nothing
	return imgC;
}

void SkewEstPlugin::preLoadPlugin() const {

	qDebug() << "[PRE LOADING] Batch Test";
}

void SkewEstPlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo>>& batchInfo) const {
	int runIdx = mRunIDs.indexOf(batchInfo.first()->id());

	for (auto bi : batchInfo) {
		qDebug() << bi->filePath() << "computed...";
		SkewInfo* tInfo = dynamic_cast<SkewInfo*>(bi.data());

		if (tInfo) {
			qDebug() << "property: " << tInfo->property();

			double sk = tInfo->skew();
			double skGT = 0;
			QString fn = tInfo->property();
			//parse string
			QRegExp rx("[+-]?[0-9]*[\\.?][0-9]*");
			//QString test = "IMG(0006)_SA[-5.76].png";
			int gtFound = rx.indexIn(fn);
			QStringList list = rx.capturedTexts();
			if (list.size() != 1) {
				qWarning() << "no GT found";
			}
			else {
				QString skGTs = list[0];
				skGT = skGTs.toDouble();
			}

			//push back
		}

	}

	//write to file


	if (runIdx == id_skewnative)
		qDebug() << "[POST LOADING] skew native";
	else
		qDebug() << "[POST LOADING] skew doc";
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

};


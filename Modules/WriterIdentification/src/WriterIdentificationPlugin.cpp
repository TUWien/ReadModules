/*******************************************************************************************************
 BinarizationPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 #YOUR_NAME

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#include "WriterIdentificationPlugin.h"

#include "WriterIdentification.h"
#include "WriterIdentificationDatabase.h"
#include "Image.h"

#include "DkImageStorage.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
WriterIdentificationPlugin::WriterIdentificationPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_calcuate_features] = "1bb00c5713a849eebb9ea16fcf794740";
	runIds[id_generate_vocabulary] = "aa8cf182dc4348aa9917cd3c3fc95d8c";
	runIds[id_identify_writer] = "b9fc66129483473fa901ddf627bd8b9a";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_calcuate_features] = tr("&Calcuate Features");
	menuNames[id_generate_vocabulary] = tr("&Generate Vocabulary");
	menuNames[id_identify_writer] = tr("&Identify Writer");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_calcuate_features] = tr("Calculates the features for writer identification on this page");
	statusTips[id_generate_vocabulary] = tr("Generates a new vocabulary using the given pages");
	statusTips[id_identify_writer] = tr("Identifies the writer of the given page");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
WriterIdentificationPlugin::~WriterIdentificationPlugin() {

	qDebug() << "destroying WriterIdentification plugin...";
}


/**
* Returns unique ID for the generated dll
**/
QString WriterIdentificationPlugin::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage WriterIdentificationPlugin::image() const {

	return QImage(":/WriterIdentificationPlugin/img/read.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString WriterIdentificationPlugin::version() const {

	return PLUGIN_VERSION;
};

QList<QAction*> WriterIdentificationPlugin::createActions(QWidget* parent) {

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



QList<QAction*> WriterIdentificationPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> WriterIdentificationPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_calcuate_features]) {
		qInfo() << "calculating features for writer identification";
		WriterIdentification wi = WriterIdentification();
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		wi.setImage(imgCv);
		wi.calculateFeatures();
		cv::cvtColor(imgCv, imgCv, CV_RGB2GRAY);
		QVector<cv::KeyPoint> kp = wi.getKeyPoints();
		for(int i = 0; i < kp.length(); i++) {
			kp[i].size *= 1.5 * 4;
		}
		cv::drawKeypoints(imgCv, kp.toStdVector(), imgCv, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
		//cv::drawKeypoints(imgCv, wi.getKeyPoints().toStdVector(), imgCv, cv::Scalar::all(-1));
		
		QString featureFilePath = imgC->filePath();
		featureFilePath.replace(featureFilePath.length() - 4, featureFilePath.length(), ".yml");
		wi.saveFeatures(featureFilePath);

		//QImage img = nmc::DkImage::mat2QImage(imgCv);
		//img = img.convertToFormat(QImage::Format_ARGB32);
		//imgC->setImage(img, tr("SIFT keypoints"));
	}
	else if(runID == mRunIDs[id_generate_vocabulary]) {
		qInfo() << "generating vocabulary";

		QString featureFilePath = imgC->filePath();
		featureFilePath.replace(featureFilePath.length() - 4, featureFilePath.length(), ".yml");
		//WriterIdentificationDatabase mWIDatabase = WriterIdentificationDatabase();
		//mWIDatabase = WriterIdentificationDatabase();
		//mWIDatabase.addFile(featureFilePath);
	}
	else if(runID == mRunIDs[id_identify_writer]) {
		qInfo() << "identifying writer";
	}


	// wrong runID? - do nothing
	return imgC;
};

};


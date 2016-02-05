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

#include "BinarizationPlugin.h"

#include "Algorithms.h"
#include "Binarization.h"
#include "DkImageStorage.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
BinarizationPlugin::BinarizationPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_binarize_otsu] = "4398d8e26fe9454384432e690b47d4d3";
	runIds[id_binarize_su] = "73c1efff27c043d298d8acd99530af1d";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_binarize_otsu] = tr("&Otsu Threshold");
	menuNames[id_binarize_su] = tr("&Su Binarization");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_binarize_otsu] = tr("Thresholds a document with the famous Otsu method");
	statusTips[id_binarize_su] = tr("Thresholds a document with the Su method");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
BinarizationPlugin::~BinarizationPlugin() {

	qDebug() << "destroying binarization plugin...";
}


/**
* Returns unique ID for the generated dll
**/
QString BinarizationPlugin::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage BinarizationPlugin::image() const {

	return QImage(":/BinarizationPlugin/img/read.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString BinarizationPlugin::version() const {

	return PLUGIN_VERSION;
};

QList<QAction*> BinarizationPlugin::createActions(QWidget* parent) {

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



QList<QAction*> BinarizationPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> BinarizationPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_binarize_otsu]) {
	
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		imgCv = rdf::Algorithms::instance().threshOtsu(imgCv);
		QImage img = nmc::DkImage::mat2QImage(imgCv);
		img = img.convertToFormat(QImage::Format_ARGB32);
		imgC->setImage(img, tr("Otsu Binarization"));
	}
	else if(runID == mRunIDs[id_binarize_su]) {
		
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		
		rdf::BinarizationSuAdapted segSuM(imgCv);
		segSuM.compute();
		imgCv = segSuM.binaryImage();

		QImage img = nmc::DkImage::mat2QImage(imgCv);
		img = img.convertToFormat(QImage::Format_ARGB32);

		imgC->setImage(img, tr("Su Binarization"));
	}

	// wrong runID? - do nothing
	return imgC;
};

};


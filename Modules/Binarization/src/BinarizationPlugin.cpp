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
	runIds[id_binarize_su_mask] = "051d7f9d278a4c7ab3f97822a288c276";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_binarize_otsu] = tr("&Otsu Threshold");
	menuNames[id_binarize_su] = tr("&Su Binarization");
	menuNames[id_binarize_su_mask] = tr("&Su Binarization with Mask Estimation");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_binarize_otsu] = tr("Thresholds a document with the famous Otsu method");
	statusTips[id_binarize_su] = tr("Thresholds a document with the Su method");
	statusTips[id_binarize_su_mask] = tr("Thresholds a document with the Su method and estimates the mask");
	mMenuStatusTips = statusTips.toList();

	mBBSConfig.loadSettings();
	mBBSConfig.saveSettings();		// save settings (to write default settings)
}
/**
*	Destructor
**/
BinarizationPlugin::~BinarizationPlugin() {

	qDebug() << "destroying binarization plugin...";
	//mBBSConfig.saveSettings();
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
}

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
		imgCv = rdf::Algorithms::threshOtsu(imgCv);
		QImage img = nmc::DkImage::mat2QImage(imgCv);
		img = img.convertToFormat(QImage::Format_ARGB32);
		imgC->setImage(img, tr("Otsu Binarization"));
	}
	else if(runID == mRunIDs[id_binarize_su]) {
		
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		
		rdf::BinarizationSuAdapted segSuM(imgCv);

		//set settings
		//QSharedPointer<rdf::BaseBinarizationSuConfig> cf = segSuM.config();
		//*cf = mBBSConfig;

		segSuM.compute();
		imgCv = segSuM.binaryImage();

		QImage img = nmc::DkImage::mat2QImage(imgCv);
		img = img.convertToFormat(QImage::Format_ARGB32);

		imgC->setImage(img, tr("Su Binarization"));
	}
	else if (runID == mRunIDs[id_binarize_su_mask]) {
	
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());

		cv::Mat mask = rdf::Algorithms::estimateMask(imgCv);

		
		rdf::BinarizationSuAdapted segSuM(imgCv, mask);

		//set settings
		//QSharedPointer<rdf::BaseBinarizationSuConfig> cf = segSuM.config();
		//*cf = mBBSConfig;

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


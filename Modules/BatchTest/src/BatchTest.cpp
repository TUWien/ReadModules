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

#include "BatchTest.h"

// nomacs
#include "DkImageStorage.h"

#include "Settings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
BatchTest::BatchTest(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_compute_features] = "49dd0f0ea75f4906bfdd5a4676e6341a";
	runIds[id_check_images] = "3e0a0bdf4ecf475d94498d0889ebe3b6";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_compute_features] = tr("Compute Features");
	menuNames[id_check_images] = tr("Check Images");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_compute_features] = tr("Computes some magic features for each image");
	statusTips[id_check_images] = tr("Checks if all images are valid");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
BatchTest::~BatchTest() {

	qDebug() << "destroying binarization plugin...";
}


/**
* Returns unique ID for the generated dll
**/
QString BatchTest::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage BatchTest::image() const {

	return QImage(":/BatchTest/img/read.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString BatchTest::version() const {

	return PLUGIN_VERSION;
};

QList<QAction*> BatchTest::createActions(QWidget* parent) {

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



QList<QAction*> BatchTest::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> BatchTest::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_compute_features]) {
		
		QImage img = imgC->image();
		imgC->setImage(img.mirrored(), "Mirrored");

		// plugins *must not* change members since they are threaded in the batch processing
		// but: you can store results to the rdf settings...
		addFeaturePath(imgC->filePath());
	}
	else if(runID == mRunIDs[id_check_images]) {
		qDebug() << "not implemented yet - sorry";
	}

	// wrong runID? - do nothing
	return imgC;
}

void BatchTest::preLoadPlugin() {

	// put all 'reseting' code here - it is called before the user clicks ok
	clearFeaturePath();
	qDebug() << "[PRE LOADING] Batch Test";
}

void BatchTest::postLoadPlugin() {
	
	// here you can load all files (e.g. *.ymls) again and train some fancy classifier...
	qDebug() << "[POST LOADING] Batch Test, I processed: " << featurePaths() << "images";
}

void BatchTest::clearFeaturePath() const {

	QSettings& s = rdf::Config::instance().settings();
	s.beginGroup("BatchTest");

	QStringList paths;
	s.setValue("featureFiles", QStringList());
	s.endGroup();
}

void BatchTest::addFeaturePath(const QString & path) const {

	QSettings& s = rdf::Config::instance().settings();
	s.beginGroup("BatchTest");

	QStringList paths;
	paths = s.value("featureFiles", paths).toStringList();
	paths << path;
	s.setValue("featureFiles", paths);
	s.endGroup();
}

QStringList BatchTest::featurePaths() const {
	
	QSettings& s = rdf::Config::instance().settings();
	s.beginGroup("BatchTest");

	QStringList paths;
	paths = s.value("featureFiles", paths).toStringList();
	s.endGroup();

	return paths;
}


};


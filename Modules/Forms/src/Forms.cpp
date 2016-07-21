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

#include "Forms.h"

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
FormsAnalysis::FormsAnalysis(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);
	
	runIds[id_train] = "871fe4d1de79497388292ef534ff25d8";
	runIds[id_addtrain] = "d3359f4a4ec943e3abd4860e68152769";
	runIds[id_classify] = "91fd4f094fc34085a7a8715142f2292d";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_train] = tr("Train forms");
	menuNames[id_addtrain] = tr("Add training forms to existing ones");
	menuNames[id_classify] = tr("Classify");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_train] = tr("Train forms");
	statusTips[id_addtrain] = tr("Add additional forms to existing ones");
	statusTips[id_classify] = tr("Classify");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
FormsAnalysis::~FormsAnalysis() {

	qDebug() << "destroying binarization plugin...";
}


/**
* Returns unique ID for the generated dll
**/
QString FormsAnalysis::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage FormsAnalysis::image() const {

	return QImage(":/BatchTest/img/read.png");
};

QList<QAction*> FormsAnalysis::createActions(QWidget* parent) {

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



QList<QAction*> FormsAnalysis::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> FormsAnalysis::runPlugin(
	const QString &runID,
	QSharedPointer<nmc::DkImageContainer> imgC,
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& info) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_train]) {

		QImage img = imgC->image();
		imgC->setImage(img.mirrored(), "Mirrored");

		QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));
		testInfo->setProperty("Mirrored");
		qDebug() << "mirrored...";

		info = testInfo;
	}
	else if(runID == mRunIDs[id_addtrain]) {

		QImage img = imgC->image();
		img = img.convertToFormat(QImage::Format_Grayscale8);
		imgC->setImage(img, "Grayscale");

		QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));
		testInfo->setProperty("Grayscale");
		qDebug() << "grayscale...";

		info = testInfo;
	}
	else if (runID == mRunIDs[id_classify]) {

		qDebug() << "classify...";

		QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));
		testInfo->setProperty("Grayscale");
		qDebug() << "grayscale...";

		info = testInfo;
	}

	// wrong runID? - do nothing
	return imgC;
}

void FormsAnalysis::preLoadPlugin() const {

	qDebug() << "[PRE LOADING] Batch Test";
}

void FormsAnalysis::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo>>& batchInfo) const {
	int runIdx = mRunIDs.indexOf(batchInfo.first()->id());

	for (auto bi : batchInfo) {
		qDebug() << bi->filePath() << "computed...";
		FormsInfo* tInfo = dynamic_cast<FormsInfo*>(bi.data());

		if (tInfo)
			qDebug() << "property: " << tInfo->property();

	}

	if (runIdx == id_classify)
		qDebug() << "[POST LOADING] classify";
	else
		qDebug() << "[POST LOADING] train/add training";
}

// DkTestInfo --------------------------------------------------------------------
FormsInfo::FormsInfo(const QString& id, const QString & filePath) : nmc::DkBatchInfo(id, filePath) {
}

void FormsInfo::setProperty(const QString & p) {
	mProp = p;
}

QString FormsInfo::property() const {
	return mProp;
}

};


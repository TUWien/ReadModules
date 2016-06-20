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

 // nomacs includes
#include "DkSettings.h"
#include "DkImageStorage.h"

#include <fstream>

#include "WriterIdentification.h"
#include "WIDatabase.h"
#include "Image.h"
#include "Settings.h"


#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#include "opencv2/features2d/features2d.hpp"
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
	runIds[id_evaluate_database] = "e247a635ebb3449ba88204abf8d5f089";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_calcuate_features] = tr("Calcuate Features");
	menuNames[id_generate_vocabulary] = tr("Generate Vocabulary");
	menuNames[id_identify_writer] = tr("Identify Writer");
	menuNames[id_evaluate_database] = tr("Evaluate Database");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_calcuate_features] = tr("Calculates the features for writer identification on this page");
	statusTips[id_generate_vocabulary] = tr("Generates a new vocabulary using the given pages");
	statusTips[id_identify_writer] = tr("Identifies the writer of the given page");
	statusTips[id_evaluate_database] = tr("Evaluates the selected files");
	mMenuStatusTips = statusTips.toList();

	init();
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
QSharedPointer<nmc::DkImageContainer> WriterIdentificationPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC, QSharedPointer<nmc::DkBatchInfo>& info) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_calcuate_features]) {
		qInfo() << "calculating features for writer identification";
		WriterIdentification wi = WriterIdentification();
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		wi.setImage(imgCv);
		wi.calculateFeatures();
		cv::cvtColor(imgCv, imgCv, CV_RGB2GRAY);
		QVector<cv::KeyPoint> kp = wi.keyPoints();
		//QVector<cv::KeyPoint>::iterator kpItr = kp.begin();
		//cv::Mat desc = wi.descriptors();
		//cv::Mat newDesc = cv::Mat(0, desc.cols, desc.type());
		//int r = 0;
		//rdf::Image::instance().imageInfo(desc, "desc");
		//for(auto kpItr = kp.begin(); kpItr != kp.end(); r++) {
		//	kpItr->size *= 1.5 * 4;
		//	if(kpItr->size > 70) {
		//		kpItr = kp.erase(kpItr);
		//	} else if(kpItr->size < 20) {
		//		kpItr = kp.erase(kpItr);
		//	} else {
		//		kpItr++;
		//		newDesc.push_back(desc.row(r).clone());
		//	}
		//}
		//rdf::Image::instance().imageInfo(newDesc, "newDesc");
		//wi.setDescriptors(newDesc);
		//wi.setKeyPoints(kp);
		cv::drawKeypoints(imgCv, kp.toStdVector(), imgCv, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
		////cv::drawKeypoints(imgCv, wi.getKeyPoints().toStdVector(), imgCv, cv::Scalar::all(-1));
		//
		////QString fFilePath = featureFilePath(imgC->filePath(), true);
		wi.saveFeatures(featureFilePath(imgC->filePath(), true));

		//QImage img = nmc::DkImage::mat2QImage(imgCv);
		//img = img.convertToFormat(QImage::Format_ARGB32);
		//imgC->setImage(img, tr("SIFT keypoints"));

	}
	else if(runID == mRunIDs[id_generate_vocabulary]) {
		qInfo() << "collecting files for vocabulary generation";

		QString ffPath = featureFilePath(imgC->filePath());

		int idxOfMinus = QFileInfo(imgC->filePath()).baseName().indexOf("-");
		int idxOfUScore = QFileInfo(imgC->filePath()).baseName().indexOf("_");
		int idx = -1;
		if(idxOfMinus == -1 && idxOfUScore > 0)
			idx = idxOfUScore;
		else if(idxOfUScore == -1 && idxOfMinus > 0)
			idx = idxOfMinus;
		else if(idxOfMinus > 0 && idxOfUScore > 0)
			idx = idxOfMinus > idxOfUScore ? idxOfMinus : idxOfUScore;
		QString label = QFileInfo(imgC->filePath()).baseName().left(idx);


		QSharedPointer<WIInfo> wInfo(new WIInfo(runID, imgC->filePath()));
		wInfo->setWriter(label);
		wInfo->setFeatureFilePath(ffPath);

		info = wInfo;
	}
	else if(runID == mRunIDs[id_identify_writer]) {
		qInfo() << "identifying writer";
	}
	else if(runID == mRunIDs[id_evaluate_database]) {
		qInfo() << "collecting files evaluation";

		if(mVocabulary.isEmpty()) {
			qWarning() << "batchProcess: vocabulary is empty ... not evaluating";
			return imgC;
		}

		QString fFilePath = featureFilePath(imgC->filePath());

		if(QFileInfo(fFilePath).exists()) {

			int idxOfMinus = QFileInfo(imgC->filePath()).baseName().indexOf("-");
			int idxOfUScore = QFileInfo(imgC->filePath()).baseName().indexOf("_");
			int idx = -1;
			if(idxOfMinus == -1 && idxOfUScore > 0)
				idx = idxOfUScore;
			else if(idxOfUScore == -1 && idxOfMinus > 0)
				idx = idxOfMinus;
			else if(idxOfMinus > 0 && idxOfUScore > 0)
				idx = idxOfMinus < idxOfUScore ? idxOfMinus : idxOfUScore;
			QString label = QFileInfo(imgC->filePath()).baseName().left(idx);
			qDebug() << "label: " << label << "\t\tbaseName:" << QFileInfo(imgC->filePath()).baseName();

			cv::FileStorage fs(fFilePath.toStdString(), cv::FileStorage::READ);
			if(!fs.isOpened()) {
				qWarning() << " unable to read file " << fFilePath;
				return imgC;
			}
			std::vector<cv::KeyPoint> kp;
			fs["keypoints"] >> kp;
			cv::Mat descriptors;
			fs["descriptors"] >> descriptors;
			fs.release();

			if(mVocabulary.minimumSIFTSize() > 0 || mVocabulary.maximumSIFTSize() > 0) {
				cv::Mat filteredDesc = cv::Mat(0, descriptors.cols, descriptors.type());
				int r = 0;
				for(auto kpItr = kp.begin(); kpItr != kp.end(); r++) {
					if(kpItr->size*1.5 * 4 > mVocabulary.maximumSIFTSize() && mVocabulary.maximumSIFTSize() > 0) {
						kpItr = kp.erase(kpItr);
					}
					else if(kpItr->size * 1.5 * 4 < mVocabulary.minimumSIFTSize()) {
						kpItr = kp.erase(kpItr);
					}
					else {
						kpItr++;
						filteredDesc.push_back(descriptors.row(r).clone());
					}
				}
				qDebug() << "filtered " << descriptors.rows - filteredDesc.rows << " SIFT features (maxSize:" << mVocabulary.maximumSIFTSize() << " minSize:" << mVocabulary.minimumSIFTSize() << ")";
				descriptors = filteredDesc;
			}
			else
				qDebug() << "not filtering SIFT features, min or max size not set";

			cv::Mat feature = mVocabulary.generateHist(descriptors);

			rdf::Image::instance().imageInfo(descriptors, "descriptors");
			rdf::Image::instance().imageInfo(feature, "feature");

			QSharedPointer<WIInfo> wInfo(new WIInfo(runID, imgC->filePath()));
			wInfo->setWriter(label);
			wInfo->setFeatureFilePath(fFilePath);
			wInfo->setFeatureVector(feature);

			info = wInfo;
		} else {
			qDebug() << "no features files exists for image: " << imgC->filePath()  << "... skipping";
		}
	}


	// wrong runID? - do nothing
	return imgC;
}
void WriterIdentificationPlugin::preLoadPlugin() const {
	qDebug() << "preloading plugin";
	
}
void WriterIdentificationPlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> >& batchInfo) const {
	qDebug() << "postLoadPlugin";

	if(batchInfo.empty())
		return;

	int runIdx = mRunIDs.indexOf(batchInfo.first()->id());

	if(runIdx == -1) {
		qWarning() << "unknown run id: " << batchInfo.first()->id();
		return;
	}

	qDebug() << "[POST LOADING]" << mMenuNames[runIdx] << "--------------------------------------------";

	if(runIdx == id_generate_vocabulary) {
		WIDatabase wiDatabase = WIDatabase();
		WIVocabulary voc = WIVocabulary();
		if(mVocType != WIVocabulary::WI_UNDEFINED) {
			voc.setType(mVocType);
			voc.setNumberOfCluster(mVocNumberOfClusters);
			voc.setNumberOfPCA(mVocNumberOfPCA);
			voc.setMaximumSIFTSize(mVocMaxSIFTSize);
			voc.setMinimumSIFTSize(mVocMinSIFTSize);
			voc.setPowerNormalization(mVocPowerNormalization);
		}
		else {
			qDebug() << "vocabulary in settings file undefined. Using default values";
			voc.setType(WIVocabulary::WI_GMM);
			voc.setNumberOfCluster(40);
			voc.setNumberOfPCA(64);

			//voc.setType(WIVocabulary::WI_BOW);
			//voc.setNumberOfCluster(300);
			//voc.setNumberOfPCA(0);
		}

		wiDatabase.setVocabulary(voc);
		qDebug() << "postLoad: vocabulary:" << voc.toString();
		QStringList classLabels, featurePaths;
		for(auto bi : batchInfo) {
			WIInfo * wInfo = dynamic_cast<WIInfo*>(bi.data());
			wiDatabase.addFile(wInfo->featureFilePath());
			featurePaths.append(wInfo->featureFilePath());
			classLabels.append(wInfo->writer());
		}

		wiDatabase.generateVocabulary();
		wiDatabase.saveVocabulary(voc.type() == WIVocabulary::WI_UNDEFINED ? "C://tmp//voc-woSettings.yml" : mSettingsVocPath);
		wiDatabase.evaluateDatabase(classLabels, featurePaths);
	}
	else if(runIdx == id_evaluate_database) {
		WIDatabase wiDatabase = WIDatabase(); 
		wiDatabase.setVocabulary(mVocabulary);
		//WIVocabulary voc = WIVocabulary();
		//if(mVocType == WIVocabulary::WI_UNDEFINED)
		//	qDebug() << "vocabulary path not set in settings file. Using default path";
		//voc.loadVocabulary(mVocType == WIVocabulary::WI_UNDEFINED ? "C://tmp//voc-woSettings.yml" : mSettingsVocPath);
		//wiDatabase.setVocabulary(voc);
		QStringList classLabels, featurePaths;
		QVector<cv::Mat> hists;
		for(auto bi : batchInfo) {
			WIInfo * wInfo = dynamic_cast<WIInfo*>(bi.data());
			featurePaths.append(wInfo->featureFilePath());
			classLabels.append(wInfo->writer());
			hists.append(wInfo->featureVector());
		}
		//wiDatabase.evaluateDatabase(classLabels, featurePaths/*, QString("c:\\tmp\\eval-2.txt")*/);
		//wiDatabase.evaluateDatabase(hists, classLabels, featurePaths/*, QString("c:\\tmp\\eval-2.txt")*/);
		if(!mSettingsVocPath.isEmpty()) {
			qDebug() << "vocabulary path:" << mSettingsVocPath;
		}
		QFileInfo fi = QFileInfo(mEvalFile);
		QString evalFile = mEvalFile;
		if(fi.isDir()) {
			evalFile += "/eval";

			if(!featurePaths.empty()) {
				QString ffp = featurePaths[1];
				QFileInfo fi = QFileInfo(ffp);
				QDir dir = fi.dir();
				if(dir.absolutePath().contains("sift", Qt::CaseInsensitive)) {
					dir.cdUp();
				}
				evalFile += "-" + dir.dirName();
			}

			if(mVocabulary.type() == WIVocabulary::WI_BOW)
				evalFile += "-BOW";
			else if(mVocabulary.type() == WIVocabulary::WI_GMM) {
				evalFile += "-GMM";
				evalFile += "-" + QString::number(mVocabulary.powerNormalization(),'f',2) + "pow";
			} 
			else
				evalFile += "-Unkown";

			evalFile += "-" + QString::number(mVocabulary.numberOfCluster()) + "c";
			evalFile += "-" + QString::number(mVocabulary.numberOfPCA()) + "PCA";
			evalFile += "-" + QString::number(mVocabulary.maximumSIFTSize()) + "SIFTmax";
			evalFile += "-" + QString::number(mVocabulary.minimumSIFTSize()) + "SIFTmin";
			if(mVocabulary.l2Mean().empty())
				evalFile += "-wol2Mean";
			if(mVocabulary.histL2Mean().empty())
				evalFile += "-wol2hist";
			evalFile += ".txt";
		}

		wiDatabase.evaluateDatabase(hists, classLabels, featurePaths, evalFile);
		if(!mEvalFile.isEmpty())
			qDebug() << "evaluation written to " << evalFile;
	}
}

void WriterIdentificationPlugin::init() {
	loadSettings(nmc::Settings::instance().getSettings());
}

void WriterIdentificationPlugin::loadSettings(QSettings & settings) {
	settings.beginGroup("WriterIdentification");
	mSettingsVocPath = settings.value("vocPath", QString()).toString();
	if(!mSettingsVocPath.isEmpty()) {
		mVocabulary.loadVocabulary(mSettingsVocPath);
	}
	mVocType = settings.value("vocType", WIVocabulary::WI_UNDEFINED).toInt();
	if(mVocType > WIVocabulary::WI_UNDEFINED)
		mVocType = WIVocabulary::WI_UNDEFINED;
	mVocNumberOfClusters = settings.value("numberOfClusters", -1).toInt();
	mVocNumberOfPCA = settings.value("numberOfPCA", -1).toInt();
	mVocMaxSIFTSize = settings.value("maxSIFTSize", -1).toInt();
	mVocMinSIFTSize = settings.value("minSIFTSize", -1).toInt();
	mVocPowerNormalization = settings.value("powerNormalization", 1).toDouble();
	mFeatureDir = settings.value("featureDir", QString()).toString();
	
	mEvalFile = settings.value("evalFile", QString()).toString();
	
	qDebug() << "settings read: path: " << mSettingsVocPath << " type:" << mVocType << " numberOfClusters:" << mVocNumberOfClusters << " numberOfPCA: " << mVocNumberOfPCA;
	settings.endGroup();

}

void WriterIdentificationPlugin::saveSettings(QSettings & settings) const {
	settings.beginGroup("WriterIdentification");

	settings.endGroup();
}

QString WriterIdentificationPlugin::featureFilePath(QString imgPath, bool createDir) const {
	if(mFeatureDir.isEmpty()) {
		QString featureFilePath = imgPath;
		return featureFilePath.replace(featureFilePath.length() - 4, featureFilePath.length(), ".yml");
	}
	else {
		QString ffPath;
		QFileInfo fImgPath(imgPath);
		QFileInfo fFeatPath(mFeatureDir);
		if(fFeatPath.isAbsolute()) {
			ffPath = fFeatPath.absoluteDir().path() + fImgPath.baseName();
		}
		else {
			QFileInfo combined(fImgPath.absoluteDir().path() + "/" + mFeatureDir + "/");
			if(!combined.exists() && createDir) {
				QDir dir(fImgPath.absoluteDir());
				if(!dir.mkdir(mFeatureDir)) {
					qDebug() << "unable to create subdirectory";
				}
			}
			ffPath = combined.absoluteDir().path() + "/" + fImgPath.baseName() + ".yml";
		}
		qDebug() << "fFPath.isAbsolute():" << fFeatPath.isAbsolute() << " fFPath.isRelative():" << fFeatPath.isRelative();

		return ffPath;
	}
}

// WIInfo --------------------------------------------------------------------
WIInfo::WIInfo(const QString& id, const QString & filePath) : nmc::DkBatchInfo(id, filePath) {
}

void WIInfo::setWriter(const QString & p) {
	mWriter = p;
}

QString WIInfo::writer() const {
	return mWriter;
}

void WIInfo::setFeatureFilePath(const QString & p) {
	mFeatureFilePath = p;
}

QString WIInfo::featureFilePath() const {
	return mFeatureFilePath;
}

void WIInfo::setFeatureVector(const cv::Mat featureVec) {
	mFeatureVec = featureVec;
}

cv::Mat WIInfo::featureVector() const {
	return mFeatureVec;
}

};





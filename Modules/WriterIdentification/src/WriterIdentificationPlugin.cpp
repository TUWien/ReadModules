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
#include "DkImageStorage.h"

#include <fstream>

#include "WriterRetrieval.h"
#include "WriterDatabase.h"
#include "Image.h"
#include "Settings.h"

#include "Utils.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#include <QImageWriter>
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
	runIds[id_extract_patches] = "64b27436f29d461c9148e98dd816f93e";
	runIds[id_extract_patches_per_page] = "926c8d0e57ff4cb0a1dab586e04847e7";
	runIds[id_extract_random_patches] = "5553a82e4fbb4075bf36bdeec36b396b";
	runIds[id_evaluate_database_transkribus] = "c89784c2460b47b49d4edf20e6b093cb";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_calcuate_features] = tr("Calcuate Features");
	menuNames[id_generate_vocabulary] = tr("Generate Vocabulary");
	menuNames[id_identify_writer] = tr("Identify Writer");
	menuNames[id_evaluate_database] = tr("Evaluate Database");
	menuNames[id_extract_patches] = tr("Extract Patches");
	menuNames[id_extract_patches_per_page] = tr("Extract Patches Per Page");
	menuNames[id_extract_random_patches] = tr("Extract Random Patches");
	menuNames[id_evaluate_database_transkribus] = tr("Evaluate Database (Transkribus)");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_calcuate_features] = tr("Calculates the features for writer identification on this page");
	statusTips[id_generate_vocabulary] = tr("Generates a new vocabulary using the given pages");
	statusTips[id_identify_writer] = tr("Identifies the writer of the given page");
	statusTips[id_evaluate_database] = tr("Evaluates the selected files");
	statusTips[id_extract_patches] = tr("Extract Patches at SIFT keypoints");
	statusTips[id_extract_patches] = tr("Extract Patches at SIFT keypoints and stores it in a directory of the filename");
	statusTips[id_extract_random_patches] = ("Extract Patches on the page of random regions");
	statusTips[id_evaluate_database_transkribus] = tr("Evaluate Database using the same code as in the Transkribus plugin");
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
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage WriterIdentificationPlugin::image() const {

	return QImage(":/WriterIdentificationPlugin/img/read.png");
}

QString WriterIdentificationPlugin::name() const {
	return "Writer Identification";
}

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
QSharedPointer<nmc::DkImageContainer> WriterIdentificationPlugin::runPlugin(
	const QString &runID, 
	QSharedPointer<nmc::DkImageContainer> imgC, 
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& info) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_calcuate_features]) {
		qInfo() << "calculating features for writer identification";
		rdf::WriterImage wi = rdf::WriterImage();
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		wi.setImage(imgCv);
		wi.calculateFeatures();
		cv::cvtColor(imgCv, imgCv, CV_RGB2GRAY);
		QVector<cv::KeyPoint> kp = wi.keyPoints();
		//QVector<cv::KeyPoint>::iterator kpItr = kp.begin();
		//cv::Mat desc = wi.descriptors();
		//cv::Mat newDesc = cv::Mat(0, desc.cols, desc.type());
		//int r = 0;
		//rdf::Image::imageInfo(desc, "desc");
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
		//rdf::Image::imageInfo(newDesc, "newDesc");
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

		QString label = extractWriterIDFromFilename(QFileInfo(imgC->filePath()).baseName());


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

			rdf::Image::imageInfo(descriptors, "descriptors");
			rdf::Image::imageInfo(feature, "feature");

			QString label = extractWriterIDFromFilename(QFileInfo(imgC->filePath()).baseName());

			QSharedPointer<WIInfo> wInfo(new WIInfo(runID, imgC->filePath()));
			wInfo->setWriter(label);
			wInfo->setFeatureFilePath(fFilePath);
			wInfo->setFeatureVector(feature);
			wInfo->setImageName(QFileInfo(imgC->filePath()).baseName());

			info = wInfo;
		} else {
			qDebug() << "no features files exists for image: " << imgC->filePath()  << "... skipping";
		}
	}
	else if(runID == mRunIDs[id_evaluate_database_transkribus]) {
		if(mVocabulary.isEmpty()) {
			qWarning() << "batchProcess: vocabulary is empty ... not evaluating";
			return imgC;
		}

		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		rdf::WriterImage wi = rdf::WriterImage();
		wi.setImage(imgCv);
		wi.calculateFeatures();
		wi.filterKeyPoints(mVocabulary.minimumSIFTSize(), mVocabulary.maximumSIFTSize());
		cv::Mat feature = mVocabulary.generateHist(wi.descriptors());

		QString label = extractWriterIDFromFilename(QFileInfo(imgC->filePath()).baseName());

		QSharedPointer<WIInfo> wInfo(new WIInfo(runID, imgC->filePath()));
		wInfo->setWriter(label);
		wInfo->setFeatureFilePath("");
		wInfo->setFeatureVector(feature);

		info = wInfo;

	}
	else if(runID == mRunIDs[id_extract_patches]) {
		rdf::WriterImage wi = rdf::WriterImage();
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		wi.setImage(imgCv);
		wi.calculateFeatures();
		cv::cvtColor(imgCv, imgCv, CV_RGB2GRAY);
		QVector<cv::KeyPoint> kp = wi.keyPoints();
		QVector<QImage> trainPatches, testPatches;
		QRect oldRect;
		for(int i = 0; i < kp.length(); i++) {
			int rectSize = 64;
			int threshBorder = 16;
			int threshold = 20;

			QPointF point = rdf::Converter::cvPointToQt(kp[i].pt);
			point.setX(point.x() - rectSize/2);
			point.setY(point.y() - rectSize/2);
			QRect rect = QRect(point.toPoint(), QSize(rectSize, rectSize));
			QImage img = imgC->image().copy(rect);

			QRect threshRect = QRect(QPoint(threshBorder, threshBorder), QPoint(rectSize-threshBorder, rectSize-threshBorder));
			QImage tmpImg = img.copy(threshRect);
			cv::Scalar sum = cv::sum(nmc::DkImage::qImage2Mat(tmpImg));
			if(sum[0]/255.0f > tmpImg.width()*tmpImg.height()-20) {
				qDebug() << "sum exceeds threshold";				
				continue;
			}

			if(rect != oldRect) {
				//qDebug() << "point y :" << point.y() << "  height/2:" << imgCv.rows / 2;
				//if(point.y() < imgCv.rows /2)
					trainPatches.push_back(img);
				//else
					//testPatches.push_back(img);
			}
			oldRect = rect;
			
		}

		QString dirName = "patches";
		QFileInfo fImgPath(imgC->fileInfo());
		QFileInfo patchesOutPath(fImgPath.absoluteDir().path() + "/" + dirName + "/");
		if(!patchesOutPath.exists()) {
			QDir directory(fImgPath.absoluteDir());
			if(!directory.mkdir(dirName)) {
				qDebug() << "unable to create subdirectory";
			}
		}
		QFileInfo trainDir = QFileInfo(patchesOutPath.absolutePath() + "/train/");
		if(!trainDir.exists()) {
			QDir directory(patchesOutPath.absoluteDir());
			if(!directory.mkdir("train")) {
				qDebug() << "unable to create train subdirectory";
			}
		}
		QFileInfo testDir = QFileInfo(patchesOutPath.absolutePath() + "/test/");
		if(!testDir.exists()) {
			QDir directory(patchesOutPath.absoluteDir());
			if(!directory.mkdir("test")) {
				qDebug() << "unable to create train subdirectory";
			}
		}

		QString label = extractWriterIDFromFilename(imgC->fileName());

		//QFileInfo writerPatchPath(patchesOutPath.absolutePath() + "/" + label + "/");
		//if(!writerPatchPath.exists()) {
		//	QDir dir = patchesOutPath.absoluteDir();
		//	dir.mkdir(label);
		//	writerPatchPath = QFileInfo(patchesOutPath.absolutePath() + "/" + label + "/");
		//}
		QFileInfo patchOutTrain = QFileInfo(trainDir.absolutePath() + "/" + label + "/");
		if(!patchOutTrain.exists()) {
			QDir directory(trainDir.absoluteDir());
			if(!directory.mkdir(label)) {
				qDebug() << "unable to create train-label subdirectory";
			}
		}

		QFileInfo patchOutTest = QFileInfo(testDir.absolutePath() + "/" + label + "/");
		if(!patchOutTest.exists()) {
			QDir directory(testDir.absoluteDir());
			if(!directory.mkdir(label)) {
				qDebug() << "unable to create train subdirectory";
			}
		}



		for(int i = 0; i < trainPatches.length(); i++) {
 			QImageWriter qw;
			qw.setFormat("png");
			QFileInfo fi = imgC->fileInfo();
			qw.setFileName(patchOutTrain.absolutePath() + "/" + label + "_"+ fi.baseName() + "_" + QString::number(i) + ".png");
			qw.write(trainPatches[i]);
		}
		for(int i = 0; i < testPatches.length(); i++) {
			QImageWriter qw;
			qw.setFormat("png");
			QFileInfo fi = imgC->fileInfo();
			qw.setFileName(patchOutTest.absolutePath() + "/" + label + "_" + fi.baseName() + "_" + QString::number(i) + ".png");
			qw.write(testPatches[i]);
		}

	}
	else if(runID == mRunIDs[id_extract_patches_per_page]) {
		rdf::WriterImage wi = rdf::WriterImage();
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		wi.setImage(imgCv);
		wi.calculateFeatures();
		cv::cvtColor(imgCv, imgCv, CV_RGB2GRAY);
		QVector<cv::KeyPoint> kp = wi.keyPoints();
		QVector<QImage> patches;
		QRect oldRect;
		for(int i = 0; i < kp.length(); i++) {
			int rectSize = 64;
			int threshBorder = 16;
			int threshold = 20;

			QPointF point = rdf::Converter::cvPointToQt(kp[i].pt);
			point.setX(point.x() - rectSize / 2);
			point.setY(point.y() - rectSize / 2);
			QRect rect = QRect(point.toPoint(), QSize(rectSize, rectSize));
			QImage img = imgC->image().copy(rect);

			QRect threshRect = QRect(QPoint(threshBorder, threshBorder), QPoint(rectSize - threshBorder, rectSize - threshBorder));
			QImage tmpImg = img.copy(threshRect);
			cv::Scalar sum = cv::sum(nmc::DkImage::qImage2Mat(tmpImg));
			if(sum[0] / 255.0f > tmpImg.width()*tmpImg.height() - 20) {
				qDebug() << "sum exceeds threshold";
				continue;
			}

			if(rect != oldRect) {
				patches.push_back(img);
			}
			oldRect = rect;
		}

		QFileInfo imgFi = imgC->fileInfo();
		QDir pageDir = imgFi.absoluteDir();
		pageDir.mkdir("page-patches");
		pageDir.cd("page-patches");

		pageDir.mkdir(imgFi.baseName());

		QDir patchDir = QDir(pageDir.absolutePath() + "/" + imgFi.baseName());
		QString label = extractWriterIDFromFilename(imgC->fileName());
		for(int i = 0; i < patches.length(); i++) {
			QImageWriter qw;
			qw.setFormat("png");
			QFileInfo fi = imgC->fileInfo();
			qw.setFileName(patchDir.absolutePath() + "/" + label + "_" + fi.baseName() + "_" + QString::number(i) + ".png");
			qw.write(patches[i]);
		}



	}
	else if(runID == mRunIDs[id_extract_random_patches]) {
		//int patchSize = 64;
		//int patchNumber = 1000;
		//double ratio = 0.85;

		//int patchSize = 200;
		//int patchNumber = 50;
		//double ratio = 0.95;

		int patchSize = 64;
		int patchNumber = 2067;
		double ratio = 0.85;


		QString dirName = "random-patches-" + QString::number(patchNumber);

		QString label = extractWriterIDFromFilename(imgC->fileName());

		QFileInfo fImgPath(imgC->fileInfo());
		
		QFileInfo patchesOutPath(fImgPath.absoluteDir().path() + "/" + dirName + "/");
		
		if(!patchesOutPath.exists()) {
			QDir directory(fImgPath.absoluteDir());
			if(!directory.mkdir(dirName)) {
				qDebug() << "unable to create subdirectory";
			}
		}


		QFileInfo patchOut = QFileInfo(patchesOutPath.absolutePath() + "/" + label + "/");
		qDebug() << "patchOut:" << patchOut.absoluteFilePath();
		if(!patchOut.exists()) {
			QDir directory(patchesOutPath.absoluteDir());
			if(!directory.mkdir(label)) {
				qDebug() << "unable to create train-label subdirectory";
			}
		}

		int patchCounter = 0;
		int sizeX = imgC->image().size().width()-patchSize;
		int sizeY = imgC->image().size().height()-patchSize;
		QPainter paint;
		//QImage img = imgC->image();
		//paint.begin(&img);
		int maxCount = 10000000;
		while(patchCounter < patchNumber || patchCounter > maxCount) {
			
			int randomX = qrand() % sizeX;
			int randomY = qrand() % sizeY;
			QRect rect(randomX, randomY, patchSize, patchSize);
			//paint.drawRect(rect);
			QImage cropped = imgC->image().copy(rect);

			cv::Mat imgCropped = nmc::DkImage::qImage2Mat(cropped);
			double sum = cv::sum(imgCropped)[0]/255; 
			//qDebug() << sum / (patchSize*patchSize);
			if(sum / (patchSize*patchSize) > ratio) // 0.8 für triplets
				continue;

			
			QImageWriter qw;
			qw.setFormat("png");
			QFileInfo fi = imgC->fileInfo();
			qw.setFileName(patchOut.absolutePath() + "/" + label + "_" + fi.baseName() + "_" + QString::number(patchCounter) + ".png");
			qw.write(cropped);
			patchCounter++;
		}
		//paint.end();
		//imgC->setImage(img, tr("patches"));
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
		rdf::WriterDatabase wiDatabase = rdf::WriterDatabase();
		rdf::WriterVocabulary voc = rdf::WriterVocabulary();
		if(mVocType != rdf::WriterVocabulary::WI_UNDEFINED) {
			voc.setType(mVocType);
			voc.setNumberOfCluster(mVocNumberOfClusters);
			voc.setNumberOfPCA(mVocNumberOfPCA);
			voc.setNumOfPCAWhiteComp(mVocNumverOfPCAWhite);
			voc.setMaximumSIFTSize(mVocMaxSIFTSize);
			voc.setMinimumSIFTSize(mVocMinSIFTSize);
			voc.setPowerNormalization(mVocPowerNormalization);
			voc.setL2Before(mL2Before);
		}
		else {
			qDebug() << "vocabulary in settings file undefined. Using default values";
			voc.setType(rdf::WriterVocabulary::WI_GMM);
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
		wiDatabase.saveVocabulary(voc.type() == rdf::WriterVocabulary::WI_UNDEFINED ? "C://tmp//voc-woSettings.yml" : mSettingsVocPath);
		wiDatabase.evaluateDatabase(classLabels, featurePaths);
	}
	else if(runIdx == id_evaluate_database || runIdx == id_evaluate_database_transkribus) {
		rdf::WriterDatabase wiDatabase = rdf::WriterDatabase(); 
		wiDatabase.setVocabulary(mVocabulary);
		QStringList classLabels, featurePaths, imageNames;
		cv::Mat hists;
		for(auto bi : batchInfo) {
			WIInfo * wInfo = dynamic_cast<WIInfo*>(bi.data());
			featurePaths.append(wInfo->featureFilePath());
			classLabels.append(wInfo->writer());
			hists.push_back(wInfo->featureVector());
			imageNames.push_back(wInfo->imageName());
		}

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

			if(mVocabulary.type() == rdf::WriterVocabulary::WI_BOW)
				evalFile += "-BOW";
			else if(mVocabulary.type() == rdf::WriterVocabulary::WI_GMM) {
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
			if(QFileInfo(evalFile + ".txt").exists()) {
				int i = 1;

				while(QFileInfo(evalFile + "-" + QString::number(i) + ".txt").exists()) {
					i++;
				}
				evalFile = evalFile + "-" + QString::number(i);
			}
			evalFile += ".txt";
		}

		wiDatabase.evaluateDatabase(hists, classLabels, featurePaths, evalFile);
		wiDatabase.writeCompetitionEvaluationFile(hists, imageNames, "c:/tmp/comp.csv");
		if(!mEvalFile.isEmpty())
			qDebug() << "evaluation written to " << evalFile;
	}
}

void WriterIdentificationPlugin::init() {
	loadSettings(rdf::DefaultSettings());
}

void WriterIdentificationPlugin::loadSettings(QSettings & settings) {
	rdf::WriterVocabulary defaultVoc = rdf::WriterVocabulary();
	settings.beginGroup("WriterIdentification");
	mSettingsVocPath = settings.value("vocPath", QString()).toString();
	if(!mSettingsVocPath.isEmpty()) {
		mVocabulary.loadVocabulary(mSettingsVocPath);
	}
	mVocType = settings.value("vocType", defaultVoc.type()).toInt();
	if(mVocType > rdf::WriterVocabulary::WI_UNDEFINED)
		mVocType = rdf::WriterVocabulary::WI_UNDEFINED;
	mVocNumberOfClusters = settings.value("numberOfClusters", defaultVoc.numberOfCluster()).toInt();
	mVocNumberOfPCA = settings.value("numberOfPCA", defaultVoc.numberOfPCA()).toInt();
	mVocNumverOfPCAWhite = settings.value("numberOfPCAWhitening", defaultVoc.numberOfPCAWhiteningComponents()).toInt();
	mVocMaxSIFTSize = settings.value("maxSIFTSize", defaultVoc.maximumSIFTSize()).toInt();
	mVocMinSIFTSize = settings.value("minSIFTSize", defaultVoc.minimumSIFTSize()).toInt();
	mVocPowerNormalization = settings.value("powerNormalization", defaultVoc.powerNormalization()).toDouble();
	mFeatureDir = settings.value("featureDir", QString()).toString();
	mL2Before = settings.value("L2before", defaultVoc.l2before()).toBool();
	mEvalFile = settings.value("evalFile", QString()).toString();
	
	if (!mSettingsVocPath.isEmpty())
		qDebug() << "settings read: path: " << mSettingsVocPath << " type:" << mVocType << " numberOfClusters:" << mVocNumberOfClusters << " numberOfPCA: " << mVocNumberOfPCA;
	settings.endGroup();

}

void WriterIdentificationPlugin::saveSettings(QSettings & settings) const {
	settings.beginGroup("WriterIdentification");

	settings.endGroup();
}

QString WriterIdentificationPlugin::featureFilePath(QString imgPath, bool createDir) const {
	QString extension = ".yml";

	if(mFeatureDir.isEmpty()) {
		QString featureFilePath = imgPath;
		return featureFilePath.replace(featureFilePath.length() - 4, featureFilePath.length(), extension);
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
				QDir directory(fImgPath.absoluteDir());
				if(!directory.mkdir(mFeatureDir)) {
					qDebug() << "unable to create subdirectory";
				}
			}
			ffPath = combined.absoluteDir().path() + "/" + fImgPath.baseName() + extension;
		}
		qDebug() << "fFPath.isAbsolute():" << fFeatPath.isAbsolute() << " fFPath.isRelative():" << fFeatPath.isRelative();

		return ffPath;
	}
}

QString WriterIdentificationPlugin::extractWriterIDFromFilename(const QString fileName) const {
	int idxOfMinus = fileName.indexOf("-");
	int idxOfUScore = fileName.indexOf("_");
	int idx = -1;
	if(idxOfMinus == -1 && idxOfUScore > 0)
		idx = idxOfUScore;
	else if(idxOfUScore == -1 && idxOfMinus > 0)
		idx = idxOfMinus;
	else if(idxOfMinus > 0 && idxOfUScore > 0)
		idx = idxOfMinus < idxOfUScore ? idxOfMinus : idxOfUScore;
	QString label = fileName.left(idx);
	qDebug() << "label: " << label << "\t\tbaseName:" << fileName;
	return label;
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

void WIInfo::
setImageName(const QString & p) {
	mImageName = p;
}

QString WIInfo::imageName() const {
	return mImageName;
}

};





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
#include "PageParser.h"
#include "Elements.h"
#include "ElementsHelper.h"

#include "Utils.h"
#include "Algorithms.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#include <QImageWriter>
#include <opencv2/features2d.hpp>
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
	return "WriterIdentification";
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

		QImage qMask = QImage(imgC->image().size(), QImage::Format::Format_Grayscale8);
		qMask.fill(0);
		QPainter myPainter(&qMask);
		//myPainter.setPen(QPen(Qt::white, 3, Qt::SolidLine));
		//myPainter.setPen(QPen(Qt::white));
		rdf::RegionTypeConfig rtc = rdf::RegionTypeConfig(rdf::Region::Type::type_text_region);
		QPen pen = QPen(Qt::white);
		rtc.setPen(pen);
		rtc.setBrush(Qt::white);

		QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());
		if(QFileInfo(loadXmlPath).exists()) {
			rdf::PageXmlParser parser;
			parser.read(loadXmlPath);
			auto pe = parser.page();

			QVector<QSharedPointer<rdf::Region>> regs = pe->rootRegion()->allRegions();
			for(auto i : regs) {
				if(i->type() == i->type_text_line) {
					QSharedPointer<rdf::TextLine> text_region = i.dynamicCast<rdf::TextLine>();
					text_region->draw(myPainter, rtc);
					//rdf::Polygon poly = text_region->polygon();
				}
			}
			
			cv::Mat cMask = nmc::DkImage::qImage2Mat(qMask);
			cv::Mat cMaskC1 = cv::Mat();
			cv::cvtColor(cMask, cMaskC1, CV_RGB2GRAY);
			wi.setMask(cMaskC1);
		}
		wi.setImage(imgCv);
		wi.calculateFeatures();
		cv::cvtColor(imgCv, imgCv, CV_RGB2GRAY);
		qDebug() << "lenght:" << wi.keyPoints().size();
		QVector<cv::KeyPoint> kp = wi.keyPoints();
		////QVector<cv::KeyPoint>::iterator kpItr = kp.begin();
		////cv::Mat desc = wi.descriptors();
		////cv::Mat newDesc = cv::Mat(0, desc.cols, desc.type());
		////int r = 0;
		////rdf::Image::imageInfo(desc, "desc");
		////for(auto kpItr = kp.begin(); kpItr != kp.end(); r++) {
		//	//kpItr->size *= 1.5 * 4;
		////	if(kpItr->size > 70) {
		////		kpItr = kp.erase(kpItr);
		////	} else if(kpItr->size < 20) {
		////		kpItr = kp.erase(kpItr);
		////	} else {
		////		kpItr++;
		////		newDesc.push_back(desc.row(r).clone());
		////	}
		////}
		////rdf::Image::imageInfo(newDesc, "newDesc");
		////wi.setDescriptors(newDesc);
		////wi.setKeyPoints(kp);

		for(auto kpItr = kp.begin(); kpItr != kp.end(); kpItr++) 
			kpItr->size *= 1.5 * 4;
		cv::drawKeypoints(imgCv, kp.toStdVector(), imgCv, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);


		QString fFilePath = featureFilePath(imgC->filePath(), true);
		wi.saveFeatures(featureFilePath(imgC->filePath(), true));

		QImage img = nmc::DkImage::mat2QImage(imgCv);
		img = img.convertToFormat(QImage::Format_ARGB32);
		imgC->setImage(img, tr("SIFT keypoints"));

		//imgC->setImage(qMask, tr("Mask"));

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

		if(mVoc.isEmpty()) {
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

			if(mVoc.minimumSIFTSize() > 0 || mVoc.maximumSIFTSize() > 0) {
				cv::Mat filteredDesc = cv::Mat(0, descriptors.cols, descriptors.type());
				int r = 0;
				for(auto kpItr = kp.begin(); kpItr != kp.end(); r++) {
					if(kpItr->size*1.5 * 4 > mVoc.maximumSIFTSize() && mVoc.maximumSIFTSize() > 0) {
						kpItr = kp.erase(kpItr);
					}
					else if(kpItr->size * 1.5 * 4 < mVoc.minimumSIFTSize()) {
						kpItr = kp.erase(kpItr);
					}
					else {
						kpItr++;
						filteredDesc.push_back(descriptors.row(r).clone());
					}
				}
				qDebug() << "filtered " << descriptors.rows - filteredDesc.rows << " SIFT features (maxSize:" << mVoc.maximumSIFTSize() << " minSize:" << mVoc.minimumSIFTSize() << ")";
				descriptors = filteredDesc;
			}
			else
				qDebug() << "not filtering SIFT features, min or max size not set";

			cv::Mat feature = mVoc.generateHist(descriptors);

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
		if(mVoc.isEmpty()) {
			qWarning() << "batchProcess: vocabulary is empty ... not evaluating";
			return imgC;
		}


		QString label = extractWriterIDFromFilename(QFileInfo(imgC->filePath()).baseName());

		QSharedPointer<WIInfo> wInfo(new WIInfo(runID, imgC->filePath()));
		wInfo->setWriter(label);
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());

		rdf::WriterImage wi = rdf::WriterImage();

		QString fFilePath = featureFilePath(imgC->filePath());
		if(QFileInfo(fFilePath).exists()) { // check if feature file exists
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

			
			wi.setImage(imgCv);
			wi.setKeyPoints(QVector<cv::KeyPoint>::fromStdVector(kp));
			wi.setDescriptors(descriptors);
			wi.filterKeyPoints(mVoc.minimumSIFTSize(), mVoc.maximumSIFTSize());
			wInfo->setFeatureFilePath(fFilePath);
		}
		else { // calculate new features
			wi.setImage(imgCv);
			wi.calculateFeatures();
			wi.filterKeyPoints(mVoc.minimumSIFTSize(), mVoc.maximumSIFTSize());

			wInfo->setFeatureFilePath("");
		}
		cv::Mat feature = mVoc.generateHist(wi.descriptors());


		wInfo->setFeatureVector(feature);

		info = wInfo;

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
		if(mWriterVocConfig.type() != rdf::WriterVocabulary::WI_UNDEFINED) {
			voc.setType(mWriterVocConfig.type());
			voc.setNumberOfCluster(mWriterVocConfig.numberOfClusters());
			voc.setNumberOfPCA(mWriterVocConfig.numberOfPCA());
			voc.setNumOfPCAWhiteComp(mWriterVocConfig.numberOfPCAWhitening());
			voc.setMaximumSIFTSize(mWriterVocConfig.maxSIFTSize());
			voc.setMinimumSIFTSize(mWriterVocConfig.minSIFTSize());
			voc.setPowerNormalization(mWriterVocConfig.powerNormalization());
			voc.setL2Before(mWriterVocConfig.l2before());
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
		wiDatabase.saveVocabulary(voc.type() == rdf::WriterVocabulary::WI_UNDEFINED ? "C://tmp//voc-woSettings.yml" : mWriterRetrievalConfig.vocabularyPath());
		wiDatabase.evaluateDatabase(classLabels, featurePaths);
	}
	else if(runIdx == id_evaluate_database || runIdx == id_evaluate_database_transkribus) {
		rdf::WriterDatabase wiDatabase = rdf::WriterDatabase(); 
		wiDatabase.setVocabulary(mVoc);
		QStringList classLabels, featurePaths, imageNames;
		cv::Mat hists;
		for(auto bi : batchInfo) {
			WIInfo * wInfo = dynamic_cast<WIInfo*>(bi.data());
			featurePaths.append(wInfo->featureFilePath());
			classLabels.append(wInfo->writer());
			hists.push_back(wInfo->featureVector());
			imageNames.push_back(wInfo->imageName());
		}

		qDebug() << "vocabulary path:" << mWriterRetrievalConfig.vocabularyPath();
		
		QFileInfo fi = QFileInfo(mWriterRetrievalConfig.evalPath());
		QString evalFile = mWriterRetrievalConfig.evalPath();
		if(fi.isDir()) {
			// generate new filename for eval file
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

			if(mVoc.type() == rdf::WriterVocabulary::WI_BOW)
				evalFile += "-BOW";
			else if(mVoc.type() == rdf::WriterVocabulary::WI_GMM) {
				evalFile += "-GMM";
				evalFile += "-" + QString::number(mVoc.powerNormalization(),'f',2) + "pow";
			} 
			else
				evalFile += "-Unkown";

			evalFile += "-" + QString::number(mVoc.numberOfCluster()) + "c";
			evalFile += "-" + QString::number(mVoc.numberOfPCA()) + "PCA";
			evalFile += "-" + QString::number(mVoc.maximumSIFTSize()) + "SIFTmax";
			evalFile += "-" + QString::number(mVoc.minimumSIFTSize()) + "SIFTmin";
			if(mVoc.l2Mean().empty())
				evalFile += "-wol2Mean";
			if(mVoc.histL2Mean().empty())
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
		//qDebug() << "writing competition file to:" << "c:/tmp/comp.csv";
		//wiDatabase.writeCompetitionEvaluationFile(hists, imageNames, "c:/tmp/comp.csv");
		qDebug() << "evaluation written to " << evalFile;
	}
}

QString WriterIdentificationPlugin::settingsFilePath() const {
	return rdf::Config::instance().settingsFilePath();
}

void WriterIdentificationPlugin::init() {
	
	rdf::DefaultSettings s;
	s.beginGroup(name());
	mWriterRetrievalConfig.saveDefaultSettings(s);
	mWriterVocConfig.saveDefaultSettings(s);
	s.endGroup();
}

void WriterIdentificationPlugin::loadSettings(QSettings & settings) {
	settings.beginGroup(name());
	mWriterRetrievalConfig.loadSettings(settings);
	mWriterVocConfig.loadSettings(settings);
	settings.endGroup();

	QFileInfo fi = QFileInfo(mWriterRetrievalConfig.vocabularyPath());
	if(fi.exists()) {
		mVoc.loadVocabulary(mWriterRetrievalConfig.vocabularyPath());
	}
}

void WriterIdentificationPlugin::saveSettings(QSettings & settings) const {
	settings.beginGroup(name());
	mWriterRetrievalConfig.saveSettings(settings);
	mWriterVocConfig.saveSettings(settings);
	settings.endGroup();
}

QString WriterIdentificationPlugin::featureFilePath(QString imgPath, bool createDir) const {
	QString extension = ".yml";

	if(mWriterRetrievalConfig.featureDirectory().isEmpty()) {
		QString featureFilePath = imgPath;
		return featureFilePath.replace(featureFilePath.length() - 4, featureFilePath.length(), extension);
	}
	else {
		QString ffPath;
		QFileInfo fImgPath(imgPath);
		QFileInfo fFeatPath(mWriterRetrievalConfig.featureDirectory());
		if(fFeatPath.isAbsolute()) {
			ffPath = fFeatPath.absoluteDir().path() + fImgPath.baseName();
		}
		else {
			QFileInfo combined(fImgPath.absoluteDir().path() + "/" + mWriterRetrievalConfig.featureDirectory() + "/");
			if(!combined.exists() && createDir) {
				QDir directory(fImgPath.absoluteDir());
				if(!directory.mkdir(mWriterRetrievalConfig.featureDirectory())) {
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





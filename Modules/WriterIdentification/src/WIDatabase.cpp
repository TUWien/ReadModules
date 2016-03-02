/*******************************************************************************************************
 ReadFramework is the basis for modules developed at CVL/TU Wien for the EU project READ. 
  
 Copyright (C) 2016 Markus Diem <diem@caa.tuwien.ac.at>
 Copyright (C) 2016 Stefan Fiel <fiel@caa.tuwien.ac.at>
 Copyright (C) 2016 Florian Kleber <kleber@caa.tuwien.ac.at>

 This file is part of ReadFramework.

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

#include "WIDatabase.h"
#include "WriterIdentification.h"
#include "Image.h"

#pragma warning(push, 0)	// no warnings from includes
// Qt Includes
#include <QDebug>
#include "opencv2/ml.hpp"
#pragma warning(pop)

namespace rdm {
	WIDatabase::WIDatabase() {
		mVocabulary = WIVocabulary();
	}
	void WIDatabase::addFile(const QString filePath) {
		cv::FileStorage fs(filePath.toStdString(), cv::FileStorage::READ);
		if(!fs.isOpened()) {
			qWarning() << debugName() << " unable to read file " << filePath;
			return;
		}
		std::vector<cv::KeyPoint> kp;
		fs["keypoints"] >> kp;
		cv::Mat descriptors;
		fs["descriptors"] >> descriptors;
		fs.release();

		mDescriptors.append(descriptors);
		mKeyPoints.append(QVector<cv::KeyPoint>::fromStdVector(kp));
		qDebug() << "lenght of keypoint vector:" << mKeyPoints.length();
	}
	void WIDatabase::generateVocabulary() {
		if(mVocabulary.type() == WIVocabulary::WI_UNDEFINED || mVocabulary.numberOfCluster() <= 0 ) {
			qWarning() << " WIDatabase: vocabulary type and number of clusters have to be set before generating a new vocabulary";
			return;
		}
		if(mDescriptors.size() == 0) {
			qWarning() << " WIDatabase: at least one image has to be in the dataset before generating a new vocabulary";
			return;
		}

		//int numberOfRows;
		//int numberOfCols = mDescriptors[0].cols; // must exist because of if
		//for(int i = 0; i < mDescriptors.size(); i++) {
		//	numberOfRows += mDescriptors[i].rows;
		//}
		cv::Mat allDesc(0, 0, CV_32FC1);
		for(int i = 0; i < mDescriptors.size(); i++) {
			allDesc.push_back(mDescriptors[i]);
		}

		mVocabulary.setNumberOfPCA(64);
		if(mVocabulary.numberOfPCA() > 0) { 
			allDesc = calculatePCA(allDesc); // TODO ... currently also making a L2 normalization
		}		

		switch(mVocabulary.type()) {
		case WIVocabulary::WI_BOW:	generateBOW(allDesc); break;
		case WIVocabulary::WI_GMM:	generateGMM(allDesc); break;
		default: qWarning() << "WIVocabulary has unknown type"; // should not happen
			break;
		}

	}
	void WIDatabase::setVocabulary(const WIVocabulary voc) {
		mVocabulary = voc;
	}
	WIVocabulary WIDatabase::vocabulary() const {
		return mVocabulary;
	}
	QString WIDatabase::debugName() const {
		return QString("WriterIdentificationDatabase");
	}
	cv::Mat WIDatabase::calculatePCA(cv::Mat desc) {
		// calculate mean and stddev for L2 normalization
		cv::Mat means, stddev;
		for(int i = 0; i < desc.cols; i++) {
			cv::Scalar m, s;
			meanStdDev(desc.col(i), m, s);
			means.push_back(m.val[0]);
			stddev.push_back(s.val[0]);
		}
		stddev.convertTo(stddev, CV_32F);
		means.convertTo(means, CV_32F);
		mVocabulary.setL2Mean(means);
		mVocabulary.setL2Sigma(stddev);

		// L2 normalization 
		cv::Mat descResult = (desc - cv::Mat::ones(desc.rows, 1, CV_32F) * means.t());
		for(int i = 0; i < descResult.rows; i++) {
			descResult.row(i) = descResult.row(i) / stddev.t();	// caution - don't clone this line unless you know what you do (without an operator / the assignment does nothing)
		}

		qDebug() << "calculating PCA";
		cv::PCA pca = cv::PCA(descResult, cv::Mat(), CV_PCA_DATA_AS_ROW, mVocabulary.numberOfPCA());
		mVocabulary.setPcaEigenvectors(pca.eigenvectors);
		mVocabulary.setPcaEigenvalues(pca.eigenvalues);
		mVocabulary.setPcaMean(pca.mean);
		return descResult;
	}
	void WIDatabase::generateBOW(cv::Mat desc) {
		cv::BOWKMeansTrainer bow(mVocabulary.numberOfCluster(), cv::TermCriteria(), 10);
		mVocabulary.setVocabulary(bow.cluster(desc));
	}
	void WIDatabase::generateGMM(cv::Mat desc) {
		qDebug() << "start training GMM";
		cv::Ptr<cv::ml::EM> em = cv::ml::EM::create();
		em->setClustersNumber(mVocabulary.numberOfCluster());
		em->setCovarianceMatrixType(cv::ml::EM::COV_MAT_DIAGONAL);
		qDebug() << "start training GMM - again"; 
		if(!em->trainEM(desc)) {
			qWarning() << "unable to train GMM";
			return;
		} 
		qDebug() << "finished";
	
	}
	WIVocabulary::WIVocabulary() {
		mVocabulary = cv::Mat();
		mPcaMean = cv::Mat();
		mPcaSigma = cv::Mat();
		mPcaEigenvectors = cv::Mat();
		mPcaEigenvalues = cv::Mat();
		mL2Mean = cv::Mat();
		mL2Sigma = cv::Mat();
		mNumberOfClusters = 0;
		mNumberPCA = 0;
		mType = WI_UNDEFINED;
		mNote = QString();
	}
	void WIVocabulary::loadVocabulary(const QString filePath) {
		cv::FileStorage fs(filePath.toStdString(), cv::FileStorage::READ);
		if(!fs.isOpened()) {
			qWarning() << "WIVocabulary: unable to read file " << filePath;
			return;
		}
		fs["Vocabulary"] >> mVocabulary;
		fs["PcaMean"] >> mPcaMean;
		fs["PcaSigam"] >> mPcaSigma;
		fs["PcaEigenvectors"] >> mPcaEigenvectors;
		fs["PcaEigenvalues"] >> mPcaEigenvalues;
		fs["L2Mean"] >> mL2Mean;
		fs["L2Sigma"] >> mL2Sigma;
		fs["NumberOfClusters"] >> mNumberOfClusters;
		fs["NumberOfPCA"] >> mNumberPCA;
		fs["type"] >> mType;
		std::string note;
		fs["note"] >> note;
		mNote = QString::fromStdString(note);

		fs.release();
	}
	void WIVocabulary::saveVocabulary(const QString filePath) const {
		if(isEmpty()) {
			qWarning() << "WIVocabulary: isEmpty() is true ... unable to save to file";
			return;
		}
		cv::FileStorage fs(filePath.toStdString(), cv::FileStorage::WRITE);
		fs << "NumberOfClusters" << mNumberOfClusters;
		fs << "NumberOfPCA" << mNumberPCA;
		fs << "type" << mType;
		fs << "note" << mNote.toStdString();
		fs << "Vocabulary" << mVocabulary;
		fs << "PcaMean" << mPcaMean;
		fs << "PcaSigam" << mPcaSigma;
		fs << "PcaEigenvectors" << mPcaEigenvectors;
		fs << "PcaEigenvalues" << mPcaEigenvalues;
		fs << "L2Mean" << mL2Mean;
		fs << "L2Sigma" << mL2Sigma;
		fs.release();
	}
	bool WIVocabulary::isEmpty() const {
		if(mVocabulary.empty() || mNumberOfClusters <= 0 || mType == WI_UNDEFINED) {
			return true;
		}
		return false;
	}
	void WIVocabulary::setVocabulary(cv::Mat voc) {
		mVocabulary = voc;
	}
	cv::Mat WIVocabulary::vocabulary() {
		return mVocabulary;
	}
	void WIVocabulary::setPcaMean(cv::Mat mean) {
		mPcaMean = mean;
	}
	cv::Mat WIVocabulary::pcaMean() {
		return mPcaMean;
	}
	void WIVocabulary::setPcaSigma(cv::Mat pcaSigma) {
		mPcaSigma = pcaSigma;
	}
	cv::Mat WIVocabulary::pcaSigma() {
		return mPcaSigma;
	}
	void WIVocabulary::setPcaEigenvectors(cv::Mat ev) {
		mPcaEigenvectors = ev;
	}
	cv::Mat WIVocabulary::pcaEigenvectors() {
		return mPcaEigenvectors;
	}
	void WIVocabulary::setPcaEigenvalues(cv::Mat ev) {
		mPcaEigenvalues = ev;
	}
	cv::Mat WIVocabulary::pcaEigenvalues() {
		return mPcaEigenvalues;
	}
	void WIVocabulary::setL2Mean(cv::Mat l2mean) {
		mL2Mean = l2mean;
	}
	cv::Mat WIVocabulary::l2Mean() {
		return mL2Mean;
	}
	void WIVocabulary::setL2Sigma(cv::Mat l2sigma) {
		mL2Sigma = l2sigma;
	}
	cv::Mat WIVocabulary::l2Sigma() {
		return mL2Sigma;
	}
	void WIVocabulary::setNumberOfCluster(int number) {
		mNumberOfClusters = number;
	}
	int WIVocabulary::numberOfCluster() {
		return mNumberOfClusters;
	}
	void WIVocabulary::setNumberOfPCA(int number) {
		mNumberPCA = number;
	}
	int WIVocabulary::numberOfPCA() {
		return mNumberPCA;
	}
	void WIVocabulary::setType(int type) {
		mType = type;
	}
	int WIVocabulary::type() {
		return mType;
	}
	void WIVocabulary::setNote(QString note) {
		mNote = note;
	}
	QString WIVocabulary::note() {
		return mNote;
	}
}



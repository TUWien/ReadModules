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

#include "WriterIdentificationDatabase.h"
#include "WriterIdentification.h"

#pragma warning(push, 0)	// no warnings from includes
// Qt Includes
#include <QDebug>
#pragma warning(pop)

namespace rdm {
	WriterIdentificationDatabase::WriterIdentificationDatabase() {
		mVocabulary = WIVocabulary();
	}
	void WriterIdentificationDatabase::addFile(const QString filePath) {
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
	void WriterIdentificationDatabase::generateVocabulary() {
		//TODO
	}
	void WriterIdentificationDatabase::setVocabulary(const WIVocabulary voc) {
		mVocabulary = voc;
	}
	WIVocabulary WriterIdentificationDatabase::vocabulary() const {
		return mVocabulary;
	}
	QString WriterIdentificationDatabase::debugName() const {
		return QString("WriterIdentificationDatabase");
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



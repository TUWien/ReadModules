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

#include <iostream>
#include <fstream>

#pragma warning(push, 0)	// no warnings from includes
// Qt Includes
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include "opencv2/ml.hpp"
#include "opencv2/features2d/features2d.hpp"
#pragma warning(pop)

namespace rdm {
	/// <summary>
	/// Initializes a new instance of the <see cref="WIDatabase"/> class.
	/// </summary>
	WIDatabase::WIDatabase() {
		// do nothing
	}
	/// <summary>
	/// Adds a file with local features representing one image to the database
	/// </summary>
	/// <param name="filePath">The file path.</param>
	void WIDatabase::addFile(const QString filePath) {
		qDebug() << "adding file " << filePath;
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

		if(mVocabulary.minimumSIFTSize() > 0 || mVocabulary.maximumSIFTSize() > 0) {
			cv::Mat filteredDesc = cv::Mat(0, descriptors.cols, descriptors.type());
			int r = 0;
			for(auto kpItr = kp.begin(); kpItr != kp.end(); r++) {
				if(kpItr->size*1.5*4 > mVocabulary.maximumSIFTSize() && mVocabulary.maximumSIFTSize() > 0 ) {
					kpItr = kp.erase(kpItr);
				} else if(kpItr->size * 1.5*4 < mVocabulary.minimumSIFTSize()) {
					kpItr = kp.erase(kpItr);
				} else {
					kpItr++;
					filteredDesc.push_back(descriptors.row(r).clone());
				}
			}
			qDebug() << "filtered " << descriptors.rows - filteredDesc.rows << " SIFT features (maxSize:" << mVocabulary.maximumSIFTSize() << " minSize:" << mVocabulary.minimumSIFTSize() << ")";
			descriptors = filteredDesc;
		}
		else
			qDebug() << "not filtering SIFT features, vocabulary is emtpy, or min or max size not set";

		mDescriptors.append(descriptors);
		mKeyPoints.append(QVector<cv::KeyPoint>::fromStdVector(kp));
		qDebug() << "lenght of keypoint vector:" << mKeyPoints.length();

		//QString descFile = filePath;
		//writeMatToFile(descriptors, descFile.append("-desc.txt"));
	}
	/// <summary>
	/// Generates the vocabulary according to the type set in the vocabulary variable. If the number of PCA components is larger than 0 a PCA is applied beforehand.
	/// </summary>
	void WIDatabase::generateVocabulary() {
		if(mVocabulary.type() == WIVocabulary::WI_UNDEFINED || mVocabulary.numberOfCluster() <= 0 ) {
			qWarning() << " WIDatabase: vocabulary type and number of clusters have to be set before generating a new vocabulary";
			return;
		}
		if(mDescriptors.size() == 0) {
			qWarning() << " WIDatabase: at least one image has to be in the dataset before generating a new vocabulary";
			return;
		}
		qDebug() << "generating vocabulary:" << mVocabulary.toString();

		rdf::Image::instance().imageInfo(mDescriptors[0], "mDescriptors[0]");
		cv::Mat allDesc(0, 0, CV_32FC1);
		for(int i = 0; i < mDescriptors.size(); i++) {
			allDesc.push_back(mDescriptors[i]);
		}

		if(mVocabulary.numberOfPCA() > 0) { 
			rdf::Image::instance().imageInfo(allDesc, "allDesc vor aufruf calculatePCA");
			allDesc = calculatePCA(allDesc); // TODO ... currently also making a L2 normalization
		}		

		switch(mVocabulary.type()) {
		case WIVocabulary::WI_BOW:	generateBOW(allDesc); break;
		case WIVocabulary::WI_GMM:	generateGMM(allDesc); break;
		default: qWarning() << "WIVocabulary has unknown type"; // should not happen
			return;
		}

		// allDesc.cols is either PCA number or descriptor size
		qDebug() << "mVocabulary.numberOfCluster() * allDesc.cols:" << mVocabulary.numberOfCluster() * allDesc.cols;
		cv::Mat allHists = cv::Mat(0, mVocabulary.numberOfCluster() * allDesc.cols, CV_32F);
		qDebug() << "calculating histograms for all images";
		for(int i = 0; i < mDescriptors.length(); i++) {
			cv::Mat hist = generateHist(mDescriptors[i]);
			allHists.push_back(hist);
		}
		
		 //calculate mean and stddev for L2 normalization
		cv::Mat means, stddev;
		for(int i = 0; i < allHists.cols; i++) {
			cv::Scalar m, s;
			meanStdDev(allHists.col(i), m, s);
			means.push_back(m.val[0]);
			stddev.push_back(s.val[0]);
		}
		stddev.convertTo(stddev, CV_32F);
		means.convertTo(means, CV_32F);
		mVocabulary.setHistL2Mean(means);
		mVocabulary.setHistL2Sigma(stddev);
	}
	/// <summary>
	/// Sets the vocabulary for this database
	/// </summary>
	/// <param name="voc">The voc.</param>
	void WIDatabase::setVocabulary(const WIVocabulary voc) {
		mVocabulary = voc;
	}
	/// <summary>
	/// returns the current vocabulary
	/// </summary>
	/// <returns>the current vocabulary</returns>
	WIVocabulary WIDatabase::vocabulary() const {
		return mVocabulary;
	}
	/// <summary>
	/// Calls the saveVocabulary function of the current vocabulary
	/// </summary>
	/// <param name="filePath">The file path.</param>
	void WIDatabase::saveVocabulary(QString filePath) {
		mVocabulary.saveVocabulary(filePath);
	}
	/// <summary>
	/// Evaluates the database.
	/// </summary>
	/// <param name="classLabels">The class labels.</param>
	/// <param name="filePaths">The files paths of the images if needed in the evaluation output</param>
	/// <param name="evalFilePath">If set a csv file with the evaluation is written to the path.</param>
	void WIDatabase::evaluateDatabase(QStringList classLabels, QStringList filePaths, QString evalFilePath) const {
		qDebug() << "evaluating database";
		if(mVocabulary.histL2Mean().empty())
			qDebug() << "no l2 normalization of the histogram";
		if(abs(mVocabulary.powerNormalization() - 1.0f) > DBL_EPSILON)
			qDebug() << "power normalization of " << mVocabulary.powerNormalization() << " applied to the feature vector";

		QVector<cv::Mat> hists;
		qDebug() << "calculating histograms for all images";
		for(int i = 0; i < mDescriptors.length(); i++) {
			hists.push_back(generateHist(mDescriptors[i]));
		}
		qDebug() << "starting evaluating";
		int tp = 0; 
		int fp = 0;
		QVector<int> soft;
		QVector<int> hard;
		for(int i = 0; i <= 10; i++) {
			soft.push_back(0);
			hard.push_back(0);
		}
		for(int i = 0; i < mDescriptors.length(); i++) {
			cv::Mat distances(mDescriptors.length(), 1, CV_32FC1);
			distances.setTo(0);
			for(int j = 0; j < mDescriptors.length(); j++) {
				if(mVocabulary.type() == WIVocabulary::WI_GMM) {
					distances.at<float>(j) = (float)(1 - hists[i].dot(hists[j]) / (cv::norm(hists[i])*cv::norm(hists[j]) + DBL_EPSILON)); // 1-dist ... 0 is equal 2 is orthogonal
				} else if(mVocabulary.type() == WIVocabulary::WI_BOW) {
					cv::Mat tmp;
					pow(hists[i] - hists[j], 2, tmp);
					cv::Scalar scal = cv::sum(tmp);
					distances.at<float>(j) = (float)sqrt(scal[0]);
				}
			}
			cv::Mat idxs;
			//writeMatToFile(distances, "c:\\tmp\\distances-unsorted.txt");
			cv::sortIdx(distances, idxs, CV_SORT_EVERY_COLUMN| CV_SORT_ASCENDING);
			cv::sort(distances, distances, CV_SORT_EVERY_COLUMN | CV_SORT_ASCENDING);

			//writeMatToFile(distances, "c:\\tmp\\distances.txt");
			//writeMatToFile(idxs, "c:\\tmp\\distances-idxs.txt");
			//QFile file("c:\\tmp\\distances-idxs.txt");
			//file.open(QIODevice::ReadWrite);
			//QTextStream stream(&file);
			//QString out;
			//for(int i = 0; i < idxs.rows; i++) {
			//	out += QString::number(idxs.at<int>(i)) + "\n";
			//}
			//stream << out;
			//file.close();

			//QFile file2(filePaths[i].append("-hist.txt"));
			//file2.open(QIODevice::ReadWrite | QIODevice::Truncate);
			//QTextStream stream2(&file2);
			//QString out2;
			//for(int j = 0; j < hists[i].cols; j++) {
			//	out2 += QString::number(hists[i].at<float>(j));
			//	out2 += " ";
			//}
			//stream2 << out2 << "\n";
			//file2.close();


			if(!evalFilePath.isEmpty()) {
				QFile file(evalFilePath);
				if(file.open(QIODevice::ReadWrite | QIODevice::Append)) {
					QTextStream stream(&file);
					QFileInfo fi = QFileInfo(filePaths[i]);
					stream << "'" << fi.baseName() << "'," << classLabels[i] << ",";
					for(int k = 0; k < idxs.rows; k++) {
						QString out = classLabels[idxs.at<int>(k)] + "," + QString::number(distances.at<float>(k)) + "," + QString::number(idxs.at<int>(k)) + ",";
						stream << out;
					}
					stream << "\n";
				}
				file.close();
			}
			//qDebug() << "classLabels[i].toInt():" << classLabels[i].toInt() << " idxs.at<int>(1):" << idxs.at<int>(1) << " classLabels[idxs.at<int>(1)]:" << classLabels[idxs.at<int>(1)];
			if(classLabels[i] == classLabels[idxs.at<int>(1)])
				tp++;
			else
				fp++;
			if(idxs.rows > 11) {
				bool allCorrect = true;
				bool oneCorrect = false;
				for(int j = 1; j <= 11; j++) { // 1 because idx 0 is the original file
					if(classLabels[i] == classLabels[idxs.at<int>(j)]) 
						oneCorrect = true;
					else
						allCorrect = false;

					if(oneCorrect)
						soft[j-1] += 1;
					if(allCorrect)
						hard[j-1] += 1;
				}
			}
		}

		// begin evluation output
		qDebug() << "total:" << tp+fp << " tp:" << tp << " fp:" << fp;
		qDebug() << "precision:" << (float)tp / ((float)tp + fp);

		QVector<float> softPerc;
		QVector<float> hardPerc;
		for(int i = 0; i < soft.size(); i++) {
			softPerc.push_back(soft[i] / (float)(tp + fp));
			hardPerc.push_back(hard[i] / (float)(tp + fp));
		}

		QVector<int> softCriteria({ 1, 2, 5, 7 });
		QString softOutputHeader = "soft evaluation\n";
		QString softOutput = "";
		for(int i = 0; i < softCriteria.size(); i++) {
			if(softCriteria[i] > soft.size()) {
				qDebug() << "Database evaluation: criteria " << softCriteria[i] << " is larger than " << soft.size()-1 << " ... skipping";
				continue;
			}
			softOutputHeader += "Top " + QString::number(softCriteria[i]) + "\t";
			softOutput += QString::number(softPerc[softCriteria[i] - 1], 'f', 3) + "\t";
		}

		QVector<int> hardCriteria({ 2, 5, 7 });
		QString hardOutputHeader = "hard evaluation:\n";
		QString hardOutput = "";
		for(int i = 0; i < hardCriteria.size(); i++) {
			if(hardCriteria[i] > hard.size()) {
				qDebug() << "Database evaluation: criteria " << hardCriteria[i] << " is larger than " << hard.size()-1 << " ... skipping";
				continue;
			}
			hardOutputHeader += "Top " + QString::number(hardCriteria[i]) + "\t";
			hardOutput += QString::number(hardPerc[hardCriteria[i] - 1], 'f', 3) + "\t";
		}

		qDebug().noquote() << softOutputHeader;
		qDebug().noquote() << softOutput;
		qDebug().noquote() << hardOutputHeader;
		qDebug().noquote() << hardOutput;
		
	}
	/// <summary>
	/// Generates the histogram according to the vocabulary type
	/// </summary>
	/// <param name="desc">Descriptors of an image.</param>
	/// <returns>the generated histogram</returns>
	cv::Mat WIDatabase::generateHist(cv::Mat desc) const {
		if(mVocabulary.type() == WIVocabulary::WI_BOW)
			return generateHistBOW(desc);
		else if(mVocabulary.type() == WIVocabulary::WI_GMM) {
			return generateHistGMM(desc);
		} 
		else {
			qWarning() << "vocabulary type is undefined... not generating histograms";
			return cv::Mat();
		}
	}
	/// <summary>
	/// Debug name.
	/// </summary>
	/// <returns></returns>
	QString WIDatabase::debugName() const {
		return QString("WriterIdentificationDatabase");
	}
	/// <summary>
	/// Calculates a PCA according to the components set in the vocabulary.
	/// </summary>
	/// <param name="desc">The desc.</param>
	/// <returns>the projected descriptors</returns>
	cv::Mat WIDatabase::calculatePCA(const cv::Mat desc) {
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
		rdf::Image::instance().imageInfo(desc, "desc vor l2");
		cv::Mat descResult = (desc - cv::Mat::ones(desc.rows, 1, CV_32F) * means.t());
		
		//cv::Mat descResult = desc.clone();
		//qDebug() << "using modified L2";
		
		rdf::Image::instance().imageInfo(descResult, "descResults vor l2 after subtracting means");
		for(int i = 0; i < descResult.rows; i++) {
			descResult.row(i) = descResult.row(i) / stddev.t();	// caution - don't clone this line unless you know what you do (without an operator / the assignment does nothing)
		}
		rdf::Image::instance().imageInfo(descResult, "descResults nach l2");

		qDebug() << "calculating PCA";
		cv::PCA pca = cv::PCA(descResult, cv::Mat(), CV_PCA_DATA_AS_ROW, mVocabulary.numberOfPCA());
		mVocabulary.setPcaEigenvectors(pca.eigenvectors);
		mVocabulary.setPcaEigenvalues(pca.eigenvalues);
		mVocabulary.setPcaMean(pca.mean);
		

		descResult = applyPCA(descResult);
		rdf::Image::instance().imageInfo(descResult, "descResults nach pca");
		return descResult;
	}
	/// <summary>
	/// Generates the BagOfWords for the given descriptors.
	/// </summary>
	/// <param name="desc">The desc.</param>
	void WIDatabase::generateBOW(cv::Mat desc) {
		cv::BOWKMeansTrainer bow(mVocabulary.numberOfCluster(), cv::TermCriteria(), 10);
		mVocabulary.setVocabulary(bow.cluster(desc));
	}
	/// <summary>
	/// Generates the GMMs and the Fisher information for the given descriptors.
	/// </summary>
	/// <param name="desc">The desc.</param>
	void WIDatabase::generateGMM(cv::Mat desc) {
		qDebug() << "start training GMM";		
		cv::Ptr<cv::ml::EM> em = cv::ml::EM::create();
		em->setClustersNumber(mVocabulary.numberOfCluster());
		em->setCovarianceMatrixType(cv::ml::EM::COV_MAT_DIAGONAL);
		cv::TermCriteria tc = cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 5000, FLT_EPSILON);
		em->setTermCriteria(tc);

		qDebug() << "start training GMM - number of features:" << desc.rows; 
		rdf::Image::instance().imageInfo(desc, "all descriptors");
		if(!em->trainEM(desc)) {
			qWarning() << "unable to train GMM";
			return;
		} 
		
		mVocabulary.setEM(em);
		qDebug() << "finished";
	}
	/// <summary>
	/// Writes an opencv mat to txt file.
	/// </summary>
	/// <param name="mat">mat</param>
	/// <param name="filePath">The file path.</param>
	void WIDatabase::writeMatToFile(const cv::Mat mat, const QString filePath) const {
		std::ofstream fileStream;
		fileStream.open(filePath.toStdString());
		fileStream << mat.cols << "\n" << mat.rows << "\n" << std::flush;
		for(int i = 0; i < mat.rows; i++) {
			const float* row = mat.ptr<float>(i);
			for(int j = 0; j < mat.cols; j++)
				fileStream << row[j] << " ";
			fileStream << "\n" << std::flush;
		}
		fileStream.close();
	}
	/// <summary>
	/// Applies the PCA with the stored Eigenvalues and Eigenvectors of the vocabulary.
	/// </summary>
	/// <param name="desc">The desc.</param>
	/// <returns>the projected descriptors</returns>
	cv::Mat WIDatabase::applyPCA(cv::Mat desc) const {
		if(mVocabulary.pcaEigenvalues().empty() || mVocabulary.pcaEigenvectors().empty() || mVocabulary.pcaMean().empty()) {
			qWarning() << "applyPCA: vocabulary does not have a PCA ... not applying PCA";
			return desc;
		}
		cv::PCA pca;
		pca.eigenvalues = mVocabulary.pcaEigenvalues();
		pca.eigenvectors = mVocabulary.pcaEigenvectors();
		pca.mean = mVocabulary.pcaMean();
		pca.project(desc, desc);

		return desc;
	}
	/// <summary>
	/// Generates the histogram for a BOW vocabulary. 
	/// </summary>
	/// <param name="desc">The desc.</param>
	/// <returns>the histogram</returns>
	cv::Mat WIDatabase::generateHistBOW(cv::Mat desc) const {
		if(mVocabulary.isEmpty()) {
			qWarning() << "generateHistBOW: vocabulary is empty ... aborting";
			return cv::Mat();
		}
		if(desc.empty())
			return cv::Mat();

		cv::Mat d = desc.clone();
		if(!mVocabulary.l2Mean().empty())
			d = l2Norm(d, mVocabulary.l2Mean(), mVocabulary.l2Sigma());

		if(mVocabulary.numberOfPCA() > 0) {
			d = applyPCA(d);
		}
		cv::Mat dists, idx;
		
		cv::flann::Index flann_index(mVocabulary.vocabulary(), cv::flann::LinearIndexParams());
		cv::Mat hist = cv::Mat(1, (int)mVocabulary.vocabulary().rows, CV_32FC1);
		hist.setTo(0);

		flann_index.knnSearch(d, idx, dists, 1, cv::flann::SearchParams(64));

		float *ptrHist = hist.ptr<float>(0);
		int *ptrLabels = idx.ptr<int>(0);		//float?

		for(int i = 0; i < idx.rows; i++) {

			ptrHist[(int)*ptrLabels]++;
			ptrLabels++;
		}

		hist /= (float)d.rows;


		return hist;
	}
	/// <summary>
	/// Generates the Fisher vector for a GMM vocabulary.
	/// </summary>
	/// <param name="desc">The desc.</param>
	/// <returns>the Fisher vector</returns>
	cv::Mat WIDatabase::generateHistGMM(cv::Mat desc) const {
		if(mVocabulary.isEmpty()) {
			qWarning() << "generateHistGMM: vocabulary is empty ... aborting";
			return cv::Mat();
		}
		if(desc.empty())
			return cv::Mat();

		
		cv::Mat d = desc.clone();
		if(!mVocabulary.l2Mean().empty())
			d = l2Norm(d, mVocabulary.l2Mean(), mVocabulary.l2Sigma());
		else
			qDebug() << "gnerateHistGMM: mVocabulary.l2Mean() is empty ... no L2 normalization done";

		if(mVocabulary.numberOfPCA() > 0) {
			d = applyPCA(d);
		}
		cv::Mat fisher(mVocabulary.numberOfCluster(), d.cols, CV_32F);
		fisher.setTo(0);

		cv::Ptr<cv::ml::EM> em = mVocabulary.em();
		for(int i = 0; i < d.rows; i++) {
			cv::Mat feature = d.row(i);
			cv::Mat probs, means;
			std::vector<cv::Mat> covs;
			
			cv::Vec2d emOut = em->predict2(feature, probs);
			probs.convertTo(probs, CV_32F);
			em->getCovs(covs);
			means = em->getMeans();
			means.convertTo(means, CV_32F);
			for(int j = 0; j < em->getClustersNumber(); j++) {
				cv::Mat cov = covs[j];
				cv::Mat diag = cov.diag(0).t();
				diag.convertTo(diag, CV_32F);
				fisher.row(j) += probs.at<float>(j) * ((feature - means.row(j)) / diag);
			}
		}
		cv::Mat weights = em->getWeights();
		weights.convertTo(weights, CV_32F);
		for(int j = 0; j < em->getClustersNumber(); j++) {
			//fisher.row(j) *= 1.0f / (d.rows* sqrt(weights.at<float>(j)) + DBL_EPSILON);
			fisher.row(j) *= 1.0f / (sqrt(weights.at<float>(j)) + DBL_EPSILON);
		}
		cv::Mat hist = fisher.reshape(0, 1);

		cv::Mat tmp;
		cv::pow(abs(hist), mVocabulary.powerNormalization(), tmp);
		for(int i = 0; i < hist.cols; i++) {
			if(hist.at<float>(i) < 0)
				tmp.at<float>(i) *= -1;
		}
		hist = tmp;

		//qDebug() << "normalizing histogram with cv::norm";
		//hist = hist / cv::norm(hist);

		if(!mVocabulary.histL2Mean().empty())
			hist = l2Norm(hist, mVocabulary.histL2Mean(), mVocabulary.histL2Sigma());

		return hist;
	}
	/// <summary>
	/// Applies a L2 normalization with the values stored in the vocabulary.
	/// </summary>
	/// <param name="desc">The desc.</param>
	/// <returns>normalized descriptors</returns>
	cv::Mat WIDatabase::l2Norm(cv::Mat desc, cv::Mat mean, cv::Mat sigma) const {
		// L2 - normalization
		cv::Mat d = (desc - cv::Mat::ones(desc.rows, 1, CV_32F) * mean.t());
		
		//qDebug() << "modified L2";
		//cv::Mat d = desc;

		for(int i = 0; i < d.rows; i++) {
			d.row(i) = d.row(i) / sigma.t();
		}
		return d;
	}

	// WIVocabulary ----------------------------------------------------------------------------------
	/// <summary>
	/// Initializes a new instance of the <see cref="WIVocabulary"/> class.
	/// </summary>
	WIVocabulary::WIVocabulary() {
		// do nothing
	}
	/// <summary>
	/// Loads the vocabulary from the given file path.
	/// updates the mVocabulary path
	/// </summary>
	/// <param name="filePath">The file path.</param>
	void WIVocabulary::loadVocabulary(const QString filePath) {
		qDebug() << "loading vocabulary from " << filePath; 
		cv::FileStorage fs(filePath.toStdString(), cv::FileStorage::READ);
		if(!fs.isOpened()) {
			qWarning() << "WIVocabulary: unable to read file " << filePath;
			return;
		}
		fs["PcaMean"] >> mPcaMean;
		fs["PcaEigenvectors"] >> mPcaEigenvectors;
		fs["PcaEigenvalues"] >> mPcaEigenvalues;
		fs["L2Mean"] >> mL2Mean;
		fs["L2Sigma"] >> mL2Sigma;
		fs["histL2Mean"] >> mHistL2Mean;
		fs["histL2Sigma"] >> mHistL2Sigma;
		fs["NumberOfClusters"] >> mNumberOfClusters;
		fs["NumberOfPCA"] >> mNumberPCA;
		fs["type"] >> mType;
		fs["minimumSIFTSize"] >> mMinimumSIFTSize;
		fs["maximumSIFTSize"] >> mMaximumSIFTSize;
		fs["powerNormalization"] >> mPowerNormalization;
		std::string note;
		fs["note"] >> note;
		mNote = QString::fromStdString(note);
		if(mType == WI_BOW)
			fs["Vocabulary"] >> mVocabulary;
		else {
			std::string gmmPath;
			fs["GmmPath"] >> gmmPath;
			mEM = cv::ml::EM::load<cv::ml::EM>(gmmPath);

			//cv::FileNode fn = fs["StatModel.EM"];
			//mEM->read(fn);
		}

		fs.release();
		mVocabularyPath = filePath;
	}
	/// <summary>
	/// Saves the vocabulary to the given file path.
	/// updates the mVocabularyPath, thus this method is not const
	/// </summary>
	/// <param name="filePath">The file path.</param>
	void WIVocabulary::saveVocabulary(const QString filePath) {
		qDebug() << "saving vocabulary to " << filePath;
		if(isEmpty()) {
			qWarning() << "WIVocabulary: isEmpty() is true ... unable to save to file";
			return;
		}
		cv::FileStorage fs(filePath.toStdString(), cv::FileStorage::WRITE);

		fs << "description" << toString().toStdString();
		fs << "NumberOfClusters" << mNumberOfClusters;
		fs << "NumberOfPCA" << mNumberPCA;
		fs << "type" << mType;
		fs << "powerNormalization" << mPowerNormalization;
		fs << "minimumSIFTSize" << mMinimumSIFTSize;
		fs << "maximumSIFTSize" << mMaximumSIFTSize;
		//fs << "note" << mNote.toStdString();
		fs << "PcaMean" << mPcaMean;
		fs << "PcaEigenvectors" << mPcaEigenvectors;
		fs << "PcaEigenvalues" << mPcaEigenvalues;
		fs << "L2Mean" << mL2Mean;
		fs << "L2Sigma" << mL2Sigma;
		fs << "histL2Mean" << mHistL2Mean;
		fs << "histL2Sigma" << mHistL2Sigma;
		if(mType == WI_BOW)
			fs << "Vocabulary" << mVocabulary;
		else if(mType == WI_GMM) {
			QString gmmPath = filePath;
			gmmPath.insert(gmmPath.length() - 4, "-gmm");
			fs << "GmmPath" << gmmPath.toStdString();
			mEM->save(gmmPath.toStdString());
			
			//mEM->write(fs);
		}
		mVocabularyPath = filePath;
		fs.release();
	}
	/// <summary>
	/// Determines whether the vocabulary is empty respl. not trained.
	/// </summary>
	/// <returns></returns>
	bool WIVocabulary::isEmpty() const {
		if(mVocabulary.empty() && mType == WI_BOW || mEM.empty() && mType == WI_GMM || mNumberOfClusters <= 0 || mType == WI_UNDEFINED) {
			return true;
		}
		return false;
	}
	/// <summary>
	/// Sets the vocabulary for BOW.
	/// </summary>
	/// <param name="voc">The voc.</param>
	void WIVocabulary::setVocabulary(cv::Mat voc) {
		mVocabulary = voc;
	}
	/// <summary>
	/// BOW vocabulary of this instance
	/// </summary>
	/// <returns>the BOW vocabulary</returns>
	cv::Mat WIVocabulary::vocabulary() const {
		return mVocabulary;
	}
	/// <summary>
	/// Sets the em for GMM.
	/// </summary>
	/// <param name="em">The em.</param>
	void WIVocabulary::setEM(cv::Ptr<cv::ml::EM> em) {
		mEM = em;
	}
	/// <summary>
	/// the EM of this instance.
	/// </summary>
	/// <returns></returns>
	cv::Ptr<cv::ml::EM> WIVocabulary::em() const {
		return mEM;
	}
	/// <summary>
	/// Sets the mean Mat of the PCA.
	/// </summary>
	/// <param name="mean">The mean Mat.</param>
	void WIVocabulary::setPcaMean(const cv::Mat mean) {
		mPcaMean = mean;
	}
	/// <summary>
	/// Mean values of the PCA.
	/// </summary>
	/// <returns>mean Mat</returns>
	cv::Mat WIVocabulary::pcaMean() const {
		return mPcaMean;
	}
	/// <summary>
	/// Sets the pca eigenvectors.
	/// </summary>
	/// <param name="ev">The eigenvectors</param>
	void WIVocabulary::setPcaEigenvectors(const cv::Mat ev) {
		mPcaEigenvectors = ev;
	}
	/// <summary>
	/// Returns the Eigenvectors of the PCA.
	/// </summary>
	/// <returns>Mat of the Eigenvectors</returns>
	cv::Mat WIVocabulary::pcaEigenvectors() const {
		return mPcaEigenvectors;
	}
	/// <summary>
	/// Sets the PCA eigenvalues
	/// </summary>
	/// <param name="ev">The eigenvalues</param>
	void WIVocabulary::setPcaEigenvalues(const cv::Mat ev) {
		mPcaEigenvalues = ev;
	}
	/// <summary>
	/// Returns the Eigenvalues of the PCA
	/// </summary>
	/// <returns>Mat of the eigenvalues</returns>
	cv::Mat WIVocabulary::pcaEigenvalues() const {
		return mPcaEigenvalues;
	}
	/// <summary>
	/// sets the mean Mat of the L2 noramlization
	/// </summary>
	/// <param name="l2mean">Mean Mat.</param>
	void WIVocabulary::setL2Mean(const cv::Mat l2mean) {
		mL2Mean = l2mean;
	}
	/// <summary>
	/// Returns the mean Mat of the L2 normalization
	/// </summary>
	/// <returns>mean Mat</returns>
	cv::Mat WIVocabulary::l2Mean() const {
		return mL2Mean;
	}
	/// <summary>
	/// Sets the variance of the L2 normalization.
	/// </summary>
	/// <param name="l2sigma">variance Mat.</param>
	void WIVocabulary::setL2Sigma(const cv::Mat l2sigma) {
		mL2Sigma = l2sigma;
	}
	/// <summary>
	/// Returns the variance Mat of the L2 normalization
	/// </summary>
	/// <returns>variance Mat </returns>
	cv::Mat WIVocabulary::l2Sigma() const {
		return mL2Sigma;
	}
	void WIVocabulary::setHistL2Mean(const cv::Mat mean) {
		mHistL2Mean = mean;
	}
	cv::Mat WIVocabulary::histL2Mean() const {
		return mHistL2Mean;
	}
	void WIVocabulary::setHistL2Sigma(const cv::Mat sigma) {
		mHistL2Sigma = sigma;
	}
	cv::Mat WIVocabulary::histL2Sigma() const {
		return mHistL2Sigma;
	}
	/// <summary>
	/// Sets the number of cluster.
	/// </summary>
	/// <param name="number">number of clusters.</param>
	void WIVocabulary::setNumberOfCluster(const int number) {
		mNumberOfClusters = number;
	}
	/// <summary>
	/// Numbers of clusters.
	/// </summary>
	/// <returns>number of clusters</returns>
	int WIVocabulary::numberOfCluster() const {
		return mNumberOfClusters;
	}
	/// <summary>
	/// Sets the number of PCA components which should be used
	/// </summary>
	/// <param name="number">The number of PCA components.</param>
	void WIVocabulary::setNumberOfPCA(const int number) {
		mNumberPCA = number;
	}
	/// <summary>
	/// Numbers the of pca.
	/// </summary>
	/// <returns>The number of PCA components</returns>
	int WIVocabulary::numberOfPCA() const {
		return mNumberPCA;
	}
	/// <summary>
	/// Sets the type of the vocabulary.
	/// </summary>
	/// <param name="type">The type.</param>
	void WIVocabulary::setType(const int type) {
		mType = type;
	}
	/// <summary>
	/// Returns the vocabulary type.
	/// </summary>
	/// <returns>type of the vocabulary</returns>
	int WIVocabulary::type() const {
		return mType;
	}
	/// <summary>
	/// Sets a note to the vocabulary. 
	/// </summary>
	/// <param name="note">note.</param>
	void WIVocabulary::setNote(QString note) {
		mNote = note;
	}
	/// <summary>
	/// Sets the minimum size for sift features, all features smaller than this size are filtered out.
	/// </summary>
	/// <param name="size">The minimum size in pixels.</param>
	void WIVocabulary::setMinimumSIFTSize(const int size) {
		mMinimumSIFTSize = size;
	}
	/// <summary>
	/// Returns the value of the minimum SIFT features size
	/// </summary>
	/// <returns>minimum size of the SIFT features</returns>
	int WIVocabulary::minimumSIFTSize() const {
		return mMinimumSIFTSize;
	}
	/// <summary>
	/// Sets the maximum size for sift features, all features larger than this size are filtered out.
	/// </summary>
	/// <param name="size">The size.</param>
	void WIVocabulary::setMaximumSIFTSize(const int size) {
		mMaximumSIFTSize = size;
	}
	/// <summary>
	/// Returns the value of the maximum SIFT features size
	/// </summary>
	/// <returns>maximum size of the SIFT features</returns>
	int WIVocabulary::maximumSIFTSize() const {
		return mMaximumSIFTSize;
	}
	/// <summary>
	/// Sets the power normalization for the feature vector.
	/// </summary>
	/// <param name="power">The power normalization factor.</param>
	void WIVocabulary::setPowerNormalization(const double power) {
		mPowerNormalization = power;
	}
	/// <summary>
	/// Returns the factor of the power normalization used.
	/// </summary>
	/// <returns>the current power normalization factor</returns>
	double WIVocabulary::powerNormalization() const {
		return mPowerNormalization;
	}
	/// <summary>
	/// Returns the note of the vocabulary.
	/// </summary>
	/// <returns></returns>
	QString WIVocabulary::note() const {
		return mNote;
	}
	/// <summary>
	/// Creates a string with a description of the current vocabulary
	/// </summary>
	/// <returns>a short description</returns>
	QString WIVocabulary::toString() const {
		QString description = "";
		if(type() == WI_GMM)
			description.append("GMM ");
		else if(type() == WI_BOW)
			description.append("BOW ");
		description += " clusters:" + QString::number(mNumberOfClusters) + " pca:" + QString::number(mNumberPCA);
		return description;
	}
	/// <summary>
	/// Path of the vocabulary
	/// </summary>
	/// <returns></returns>
	QString WIVocabulary::vocabularyPath() const {
		return mVocabularyPath;
	}
}



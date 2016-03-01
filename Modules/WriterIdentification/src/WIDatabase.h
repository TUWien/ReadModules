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

#pragma once

#pragma warning(push, 0)	// no warnings from includes
// Qt Includes
#include <QString>
#include <QVector>
#include "opencv2\opencv.hpp"
#pragma warning(pop)

// TODO: add DllExport magic

// Qt defines

namespace rdm {
	class WIVocabulary {
	public:
		WIVocabulary();

		void loadVocabulary(const QString filePath);
		void saveVocabulary(const QString filePath) const;

		bool isEmpty() const;

		enum type {
			WI_GMM,
			WI_BOW,

			WI_UNDEFINED
		};

		void setVocabulary(cv::Mat voc);
		cv::Mat vocabulary();
		void setPcaMean(cv::Mat mean);
		cv::Mat pcaMean();
		void setPcaSigma(cv::Mat pcaSigma);
		cv::Mat pcaSigma();
		void setPcaEigenvectors(cv::Mat ev);
		cv::Mat pcaEigenvectors();
		void setPcaEigenvalues(cv::Mat ev);
		cv::Mat pcaEigenvalues();
		void setL2Mean(cv::Mat l2mean);
		cv::Mat l2Mean();
		void setL2Sigma(cv::Mat l2sigma);
		cv::Mat l2Sigma();
		void setNumberOfCluster(int number);
		int numberOfCluster();
		void setNumberOfPCA(int number);
		int numberOfPCA();
		void setType(int type);
		int type();
		void setNote(QString note);
		QString note();


	private:
		cv::Mat mVocabulary;
		cv::Mat mPcaMean;
		cv::Mat mPcaSigma;
		cv::Mat mPcaEigenvectors;
		cv::Mat mPcaEigenvalues;
		cv::Mat mL2Mean;
		cv::Mat mL2Sigma;

		int mNumberOfClusters;
		int mNumberPCA;
		int mType;

		QString mNote;
	};

// read defines
	class WIDatabase {
	public:
		WIDatabase();

		void addFile(const QString filePath);
		void generateVocabulary();

		void setVocabulary(const WIVocabulary voc);
		WIVocabulary vocabulary() const;
	private:
		QString debugName() const;
		cv::Mat calculatePCA(cv::Mat desc);
		void generateBOW(cv::Mat desc);
		void generateGMM(cv::Mat desc);

		QVector<QVector<cv::KeyPoint> > mKeyPoints;
		QVector<cv::Mat> mDescriptors;
		WIVocabulary mVocabulary;
	};
};
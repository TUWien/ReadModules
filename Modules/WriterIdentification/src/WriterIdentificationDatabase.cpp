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
		// do nothing atm
	}
	void WriterIdentificationDatabase::addFile(QString filePath) {
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
	void WriterIdentificationDatabase::saveVocabulary(QString filePath) {
		if(mVocabulary.empty()) {
			qWarning() << debugName() << " vocabulary is empty ... unable to save to file";
			return;
		}
		cv::FileStorage fs(filePath.toStdString(), cv::FileStorage::WRITE);
		fs << "vocabulary" << mVocabulary;
		fs.release();
	}
	void WriterIdentificationDatabase::loadVocabulary(QString filePath) {
		cv::FileStorage fs(filePath.toStdString(), cv::FileStorage::READ);
		if(!fs.isOpened()) {
			qWarning() << debugName() << " unable to read file " << filePath;
			return;
		}
		fs["vocabulary"] >> mVocabulary;
		fs.release();
	}
	QString WriterIdentificationDatabase::debugName() {
		return QString("WriterIdentificationDatabase");
	}
}



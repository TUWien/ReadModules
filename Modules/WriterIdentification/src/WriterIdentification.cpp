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

#include "WriterIdentification.h"

//nomacs
#include "DkImageContainer.h"

//opencv
#include "opencv2/opencv.hpp"
//#include "opencv2/xfeatures2d.hpp"

#pragma warning(push, 0)	// no warnings from includes
// Qt Includes
#include <QSharedPointer>
#include <QVector>
#pragma warning(pop)

namespace rdm {
	WriterIdentification::WriterIdentification() {
		// do nothing
	}
	WriterIdentification::~WriterIdentification() {
		// do nothing
	}
	void WriterIdentification::setImage(const cv::Mat img) {
		this->mImg = img;
	}
	void WriterIdentification::calculateFeatures() {
		if(mImg.empty())
			return;

		//cv::xfeatures2d::SIFT sift = cv::xfeatures2d::SIFT();
		//sift(mImg, Mat(), mKeyPoints, mDescriptors)

	}
	QString WriterIdentification::debugName() {
		return "WriterIdentification";
	}
}
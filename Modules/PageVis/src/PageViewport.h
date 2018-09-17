/*******************************************************************************************************
 ReadFramework is the basis for modules developed at CVL/TU Wien for the EU project READ. 
  
 Copyright (C) 2016 Markus Diem <diem@cvl.tuwien.ac.at>
 Copyright (C) 2016 Stefan Fiel <fiel@cvl.tuwien.ac.at>
 Copyright (C) 2016 Florian Kleber <kleber@cvl.tuwien.ac.at>

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
 [1] https://cvl.tuwien.ac.at/
 [2] https://transkribus.eu/Transkribus/
 [3] https://github.com/TUWien/
 [4] https://nomacs.org
 *******************************************************************************************************/

#pragma once

#include "DkPluginInterface.h"

#include "Elements.h"

#pragma warning(push, 0)	// no warnings from includes
// Qt Includes
#pragma warning(pop)

// Qt defines
class QPaintEvent;
class QSettings;

namespace nmc {
	class DkTransformRect;
}

namespace rdm {

// read defines
class PageDock;
class PageData;

class PageViewport : public nmc::DkPluginViewPort {
	Q_OBJECT

public:
	PageViewport(QWidget* parent = 0);
	~PageViewport();

	void updateImageContainer(QSharedPointer<nmc::DkImageContainerT> imgC) override;
	PageDock* dock() const;
	PageData* pageData() const;

signals:
	void selectRegionsSignal(const QVector<QSharedPointer<rdf::Region> >& regions) const;
	void addRegionModeSignal(bool) const;

public slots:
	void parseXml();
	void setAddRegionMode(bool add = true);

private:
	void init();
	void loadSettings(QSettings& settings);
	void saveSettings(QSettings& settings) const;

	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

	void selectRegion(QMouseEvent* event);
	void addRegion();

	PageDock* mPageDock = 0;
	PageData* mPageData = 0;
	QSharedPointer<nmc::DkImageContainerT> mImg;

	bool mAddRegion = false;
	QRectF mNewRegion;
};

};
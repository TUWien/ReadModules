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

#include "PageViewport.h"
#include "PageDock.h"

// nomacs includes
#include "DkSettings.h"

// read includes
#include "PageParser.h"
#include "Settings.h"
#include "ElementsHelper.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QPaintEvent>
#include <QSettings>
#pragma warning(pop)

namespace rdm {

// PageViewport --------------------------------------------------------------------
PageViewport::PageViewport(QWidget* parent) : DkPluginViewPort(parent) {

	init();

}

PageViewport::~PageViewport() {

	saveSettings(nmc::Settings::instance().getSettings());
	qDebug() << "destroying PAGE viewport";
}

void PageViewport::init() {
	
	DkPluginViewPort::init();
	setObjectName("PageViewport");
	
	loadSettings(nmc::Settings::instance().getSettings());

	mPageDock = new PageDock(tr("Page Visualization"), this);
	
	connect(mPageDock, SIGNAL(updateSignal()), this, SLOT(update()));
}

void PageViewport::saveSettings(QSettings& settings) const {

	settings.beginGroup(objectName());

	settings.endGroup();
}

void PageViewport::loadSettings(QSettings& settings) {

	settings.beginGroup(objectName());
	
	settings.endGroup();
}

void PageViewport::updateImageContainer(QSharedPointer<nmc::DkImageContainerT> imgC) {

	// something todo?
	if (!imgC)
		return;

	mImg = imgC;

	rdf::PageXmlParser parser;
	parser.read(parser.imagePathToXmlPath(imgC->filePath()));

	mPage = parser.page();

	qDebug() << "plugin receives new image: " << imgC->fileName();
}

PageDock * PageViewport::dock() const {
	return mPageDock;
}

void PageViewport::paintEvent(QPaintEvent* event) {

	if (mPageDock->drawRegions()) {
		QPainter painter(this);

		if (mWorldMatrix)
			painter.setWorldTransform((*mImgMatrix) * (*mWorldMatrix));	// >DIR: using both matrices allows for correct resizing [16.10.2013 markus]

		if (mPage)
			rdf::RegionManager::instance().drawRegion(painter, mPage->rootRegion(), mPageDock->config());
	}

	DkPluginViewPort::paintEvent(event);
}

}
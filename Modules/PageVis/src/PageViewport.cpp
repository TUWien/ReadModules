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
#include "PageData.h"

// nomacs includes
#include "DkSettings.h"

// read includes
#include "Settings.h"
#include "ElementsHelper.h"
#include "PageParser.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QPaintEvent>
#include <QSettings>
#pragma warning(pop)

namespace rdm {

// PageViewport --------------------------------------------------------------------
PageViewport::PageViewport(QWidget* parent) : DkPluginViewPort(parent) {

	init();
	setMouseTracking(true);
	setAttribute(Qt::WA_TransparentForMouseEvents);
}

PageViewport::~PageViewport() {

	// TODO: check this line!
	saveSettings(nmc::Settings::instance().getSettings());
	qDebug() << "destroying PAGE viewport";
}

void PageViewport::init() {
	
	//DkPluginViewPort::init();
	setObjectName("PageViewport");
	
	loadSettings(nmc::Settings::instance().getSettings());

	mPageData = new PageData(this);
	mPageDock = new PageDock(mPageData, tr("Page Visualization"), this);
	
	connect(mPageData, SIGNAL(updatePage(QSharedPointer<rdf::PageElement>)), this, SLOT(update()));
	connect(mPageData, SIGNAL(updateXml()), this, SLOT(parseXml()));
	connect(mPageDock, SIGNAL(updateSignal()), this, SLOT(update()));
	connect(mPageDock, SIGNAL(closeSignal()), this, SIGNAL(closePlugin()));
	connect(this, SIGNAL(selectRegionsSignal(const QVector<QSharedPointer<rdf::Region> >&)), mPageDock->regionWidget(), SLOT(setRegions(const QVector<QSharedPointer<rdf::Region> >&)));
}

void PageViewport::saveSettings(QSettings& settings) const {

	settings.beginGroup(objectName());

	settings.endGroup();
}

void PageViewport::mousePressEvent(QMouseEvent * event) {

	nmc::DkPluginViewPort::mousePressEvent(event);
}

void PageViewport::mouseReleaseEvent(QMouseEvent * event) {

	const rdf::RegionManager& rm = rdf::RegionManager::instance();

	if (event->button() == Qt::LeftButton && mPageData->page() && event->modifiers() == Qt::ControlModifier) {
		QPointF p = mapToImage(event->pos());
		QVector<QSharedPointer<rdf::Region> > sr = 
			rm.regionsAt(mPageData->page()->rootRegion(), p.toPoint(), mPageData->config());
		
		// select the region
		rm.selectRegions(sr, mPageData->page()->rootRegion());
		emit selectRegionsSignal(sr);
		update();

		qDebug() << "#regions:" << sr.size() << "point:" << p;
	}
	
	nmc::DkPluginViewPort::mouseReleaseEvent(event);
}

void PageViewport::loadSettings(QSettings& settings) {

	settings.beginGroup(objectName());
	
	settings.endGroup();
}

void PageViewport::updateImageContainer(QSharedPointer<nmc::DkImageContainerT> imgC) {

	mImg = imgC;

	// something todo?
	if (!imgC)
		return;

	parseXml();

	qDebug() << "plugin receives new image: " << imgC->fileName();
}

void PageViewport::parseXml() {

	if (!mImg)
		return;

	QString rawPath = mImg->filePath();

	// prefer the specified folder if it is not empty
	if (!mPageData->xmlPath().isEmpty())
		rawPath = QFileInfo(mPageData->xmlPath(), mImg->fileName()).absoluteFilePath();

	QString xmlPath = rdf::PageXmlParser::imagePathToXmlPath(rawPath);
	mPageData->parse(xmlPath);

	QFileInfo xmlImageInfo(mPageData->page()->imageFileName());
	if (!mPageData->page()->isEmpty() && xmlImageInfo.baseName() != mImg->fileInfo().baseName()) {
		emit showInfo(tr("PAGE file does not correspond with the image displayed..."));
		qDebug() << "NOTE" << xmlImageInfo.baseName() << "!=" << mImg->fileInfo().baseName();
	}
}

PageDock * PageViewport::dock() const {
	return mPageDock;
}

void PageViewport::paintEvent(QPaintEvent* event) {

	if (mImg.isNull()) {
		DkPluginViewPort::paintEvent(event);
		return;
	}

	if (mPageDock->drawRegions()) {
		QPainter painter(this);

		if (mWorldMatrix)
			painter.setWorldTransform((*mImgMatrix) * (*mWorldMatrix));	// >DIR: using both matrices allows for correct resizing [16.10.2013 markus]

		if (mPageData->page() && !mPageData->page()->isEmpty())
			rdf::RegionManager::instance().drawRegion(painter, mPageData->page()->rootRegion(), mPageData->config());
	}

	DkPluginViewPort::paintEvent(event);
}

}
/*******************************************************************************************************
 PageVisPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 Markus Diem

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

#include "PageVisPlugin.h"

#include "PageViewport.h"
#include "PageDock.h"
#include "PageData.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
PageVisPlugin::PageVisPlugin(QObject* parent) : QObject(parent) {

}
/**
*	Destructor
**/
PageVisPlugin::~PageVisPlugin() {

	//if (mViewport) {
	//	delete mViewport;
	//	mViewport = 0;
	//}

	qDebug() << "destroying PAGE plugin...";
}

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage PageVisPlugin::image() const {

	return QImage(":/PageVisPlugin/img/read.png");
}

bool PageVisPlugin::closesOnImageChange() const {
	return false;
}


/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> PageVisPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	if (!imgC)
		return imgC;

	PageViewport* vp = static_cast<PageViewport*>(mViewport);
	if (!vp)
		return imgC;

	if (vp->dock()->drawRegions()) {
		
		QImage img = imgC->image();
		QPainter painter(&img);

		const auto pd = vp->pageData();
		if (pd->page() && !pd->page()->isEmpty())
			rdf::RegionManager::instance().drawRegion(painter, pd->page()->rootRegion(), pd->config());

		imgC->setImage(img, tr("PAGE Attributes"));
	}

	// wrong runID? - do nothing
	return imgC;
}

nmc::DkPluginViewPort * PageVisPlugin::getViewPort() {
	
	if (!mViewport) {
		PageViewport* vp = new PageViewport();

		// open dock
		QMainWindow* win = getMainWindow();
		PageDock* dock = vp->dock();
		win->addDockWidget(dock->getDockLocationSettings(Qt::RightDockWidgetArea), dock);

		mViewport = vp;
	}

	return mViewport;
}

void PageVisPlugin::deleteViewPort() {

	//if (mViewport) {
	//	delete mViewport;
	//	mViewport = 0;
	//}
}

};


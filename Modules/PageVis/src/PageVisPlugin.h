/*******************************************************************************************************
 PageVisPlugin.h

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2011-2015 Markus Diem <markus@nomacs.org>

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

#pragma once

#include "DkPluginInterface.h"

namespace rdm {

class PageVisPlugin : public QObject, nmc::DkViewPortInterface {
	Q_OBJECT
	Q_INTERFACES(nmc::DkViewPortInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.PageVisPlugin/3.0" FILE "PageVisPlugin.json")

public:
	PageVisPlugin(QObject* parent = 0);
	~PageVisPlugin();

	QString id() const override;
	QImage image() const override;

	bool closesOnImageChange() const override;

	QSharedPointer<nmc::DkImageContainer> runPlugin(const QString &runID = QString(), QSharedPointer<nmc::DkImageContainer> imgC = QSharedPointer<nmc::DkImageContainer>()) const override;

	nmc::DkPluginViewPort* getViewPort();
	void deleteViewPort();

private:
	nmc::DkPluginViewPort* mViewport = 0;
};

};

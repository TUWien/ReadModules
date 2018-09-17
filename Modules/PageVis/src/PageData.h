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

#include "ElementsHelper.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QObject>
#pragma warning(pop)

// Qt defines

namespace rdm {

// read defines
class PageData : public QObject {
	Q_OBJECT

public:
	PageData(QObject* parent);
	~PageData();

	QVector<QSharedPointer<rdf::RegionTypeConfig> > config() const;
	QSharedPointer<rdf::PageElement> page() const;
	QString xmlPath() const;

public slots:
	void parse(const QString& xmlPath);
	void save(const QString& xmlPath = "");
	void loadConfig(const QString& name);
	void saveConfig(const QString& name) const;
	void setXmlPath(const QString& path);
	void deleteSelected();
	QSharedPointer<rdf::Region> addRegion(const QRectF& rect, const rdf::Region::Type& type = rdf::Region::Type::type_text_region);

signals:
	void updatePage(QSharedPointer<rdf::PageElement> page) const;
	void updateConfig() const;
	void updateXml();

private:
	QVector<QSharedPointer<rdf::RegionTypeConfig> > mConfig;
	QSharedPointer<rdf::PageElement> mPage;
	QString mXmlPath;

	void loadSettings(QSettings& settings, const QString& name);
	void saveSettings(QSettings& settings, const QString& name) const;
};


};
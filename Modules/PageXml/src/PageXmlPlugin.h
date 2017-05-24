/*******************************************************************************************************
ReadModules are plugins for nomacs developed at CVL/TU Wien for the EU project READ. 

Copyright (C) 2016 Markus Diem <diem@caa.tuwien.ac.at>
Copyright (C) 2016 Stefan Fiel <fiel@caa.tuwien.ac.at>
Copyright (C) 2016 Florian Kleber <kleber@caa.tuwien.ac.at>

This file is part of ReadModules.

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

#include "DkPluginInterface.h"
#include "BaseModule.h"

// opencv defines
namespace cv {
	class Mat;
}

namespace rdf {
	class PageXmlParser;
	class PageElement;
	class RegionTypeConfig;
}

namespace rdm {

class PageXmlConfig : public rdf::ModuleConfig {

public:
	PageXmlConfig();
	~PageXmlConfig() {};

	virtual QString toString() const override;

	QString validatorLog() const;
	QString filterName() const;
	QVector<QSharedPointer<rdf::RegionTypeConfig> > xmlConfig() const;

protected:

	QString mValidatorLog;					// filepath to the XML validator log file
	QString mFilterName = "TextRegion";
	QVector<QSharedPointer<rdf::RegionTypeConfig> > mXmlConfig;

	void load(const QSettings& settings) override;
	void save(QSettings& settings) const override;
};

class PageXmlInfo : public nmc::DkBatchInfo {

public:
	PageXmlInfo(const QString& id = QString(), const QString& filePath = QString());

	void setStatus(const QString& str);
	QString status() const;

private:
	QString mStatus;	// status report for validate XML
};


class PageXmlPlugin : public QObject, nmc::DkBatchPluginInterface {
	Q_OBJECT
	Q_INTERFACES(nmc::DkBatchPluginInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.PageXmlPlugin/3.0" FILE "PageXmlPlugin.json")

public:
	PageXmlPlugin(QObject* parent = 0);
	~PageXmlPlugin();

	QImage image() const override;

	QList<QAction*> createActions(QWidget* parent) override;
	QList<QAction*> pluginActions() const override;
	QSharedPointer<nmc::DkImageContainer> runPlugin(
		const QString &runID, 
		QSharedPointer<nmc::DkImageContainer> imgC, 
		const nmc::DkSaveInfo& saveInfo,
		QSharedPointer<nmc::DkBatchInfo>& batchInfo) const override;

	virtual void preLoadPlugin() const override {};
	virtual void postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> > &) const override;
	
	// settings
	virtual QSettings& settings() const override;
	virtual void saveSettings(QSettings& settings) const override;
	virtual void loadSettings(QSettings& settings) override;
	virtual QString name() const override;

	enum {
		id_page_filter,
		id_page_drawer,
		id_page_validator,
		// add actions here

		id_end
	};

protected:
	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;

	PageXmlConfig mConfig;

	// layout plugin functions
	void filterRegions(QSharedPointer<rdf::PageElement> & page) const;
};
};

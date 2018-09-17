/*******************************************************************************************************
ReadModules are plugins for nomacs developed at CVL/TU Wien for the EU project READ. 

Copyright (C) 2016 Markus Diem <diem@cvl.tuwien.ac.at>
Copyright (C) 2016 Stefan Fiel <fiel@cvl.tuwien.ac.at>
Copyright (C) 2016 Florian Kleber <kleber@cvl.tuwien.ac.at>

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

The READ project  has  received  funding  from  the European  Union�s  Horizon  2020  
research  and innovation programme under grant agreement No 674943

related links:
[1] https://cvl.tuwien.ac.at/
[2] https://transkribus.eu/Transkribus/
[3] https://github.com/TUWien/
[4] https://nomacs.org
*******************************************************************************************************/

#pragma once

#include "DkPluginInterface.h"
#include "LineTrace.h"
#include "SuperPixelTrainer.h"
#include "SuperPixelClassification.h"
#include "LayoutAnalysis.h"
#include "Evaluation.h"
#include "ScaleFactory.h"
#include "DkSettingsWidget.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QDialog>
#pragma warning(pop)		// no warnings from includes - end

// opencv defines
namespace cv {
	class Mat;
}

namespace rdf {
	class LineTrace;
	class PageXmlParser;
}

namespace rdm {

class LayoutConfig : public rdf::ModuleConfig {

public:
	LayoutConfig();
	~LayoutConfig() {};

	virtual QString toString() const override;

	bool drawResults() const;
	bool saveXml() const;
	bool useTextRegions() const;

protected:
	
	bool mDrawResults = false;
	bool mUseTextRegions = false;
	bool mSaveXml = true;

	void load(const QSettings& settings) override;
	void save(QSettings& settings) const override;
};

class FeatureCollectionInfo : public nmc::DkBatchInfo {

public:
	FeatureCollectionInfo(const QString& id = QString(), const QString& filePath = QString());

	void setFeatureCollectionManager(const rdf::FeatureCollectionManager& manager);
	rdf::FeatureCollectionManager featureCollectionManager() const;

private:
	rdf::FeatureCollectionManager mManager;
};

class StatsInfo : public nmc::DkBatchInfo {

public:
	StatsInfo(const QString& id = QString(), const QString& filePath = QString());

	void setEvalInfo(const rdf::EvalInfo& evalInfo);
	rdf::EvalInfo evalInfo() const;

private:
	rdf::EvalInfo mEvalInfo;
};

class SettingsDialog : public QDialog {
	Q_OBJECT

public:
	SettingsDialog(const QString& title = tr("Training Settings"), QWidget* parent = 0);

	void setGroupName(const QString& name);
	QString groupName() const;

public slots:
	void changeSetting(const QString& key, const QVariant& value, const QStringList& groups);
	void removeSetting(const QString& key, const QStringList& groups);

protected:
	nmc::DkSettingsWidget* mSettingsWidget = 0;
	void createLayout();

	QString mName;
};

class LayoutPlugin : public QObject, nmc::DkBatchPluginInterface {
	Q_OBJECT
		Q_INTERFACES(nmc::DkBatchPluginInterface)
		Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.LayoutPlugin/3.0" FILE "LayoutPlugin.json")

public:
	LayoutPlugin(QObject* parent = 0);
	~LayoutPlugin();

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
	virtual QString settingsFilePath() const override;
	virtual void saveSettings(QSettings& settings) const override;
	virtual void loadSettings(QSettings& settings) override;
	virtual QString name() const override;

	enum {
		id_layout,
		//id_text_block,
		id_lines,
		id_layout_collect_features,
		id_layout_train,
		id_layout_classify,
		// add actions here

		id_end
	};

protected:
	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;

	rdf::LineTraceConfig mLTRConfig;
	rdf::SuperPixelLabelerConfig mSplConfig;
	rdf::SuperPixelClassifierConfig mSpcConfig;
	rdf::SuperPixelTrainerConfig mSptConfig;
	rdf::LayoutAnalysisConfig mLAConfig;
	rdf::ScaleFactoryConfig mSfConfig;
	LayoutConfig mConfig;

	// layout plugin functions
	cv::Mat compute(const cv::Mat& src, rdf::PageXmlParser& parser) const;
	cv::Mat computePageSegmentation(const cv::Mat& src, const rdf::PageXmlParser& parser) const;
	cv::Mat collectFeatures(const cv::Mat& src, const rdf::PageXmlParser& parser, QSharedPointer<FeatureCollectionInfo>& layoutInfo) const;
	cv::Mat classifyRegions(const cv::Mat& src, const rdf::PageXmlParser& parser, QSharedPointer<StatsInfo>& statsInfo) const;
	rdf::LineTrace computeLines(QSharedPointer<nmc::DkImageContainer> imgC) const;
	bool train() const;
};
};

/*******************************************************************************************************
 BinarizationPlugin.h

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
#include "WriterDatabase.h"

class QSettings;
namespace rdm {
	
class WIInfo : public nmc::DkBatchInfo {

public:
	WIInfo(const QString& id = QString(), const QString& filePath = QString());

	void setWriter(const QString& writer);
	QString writer() const;

	void setFeatureFilePath(const QString& p);
	QString featureFilePath() const;

	void setFeatureVector(const cv::Mat featureVec);
	cv::Mat featureVector() const;
private:
	QString mWriter;
	QString mFeatureFilePath;
	cv::Mat mFeatureVec;

};

class WriterIdentificationPlugin : public QObject, nmc::DkBatchPluginInterface {
	Q_OBJECT
	Q_INTERFACES(nmc::DkBatchPluginInterface)
	Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.WriterIdentificationPlugin/3.3" FILE "WriterIdentificationPlugin.json")

public:
	WriterIdentificationPlugin(QObject* parent = 0);
	~WriterIdentificationPlugin();

	QString id() const override;
	QImage image() const override;
	QString name() const override;

	QList<QAction*> createActions(QWidget* parent) override;
	QList<QAction*> pluginActions() const override;
	QSharedPointer<nmc::DkImageContainer> runPlugin(
		const QString &runID, 
		QSharedPointer<nmc::DkImageContainer> imgC, 
		const nmc::DkSaveInfo& saveInfo,
		QSharedPointer<nmc::DkBatchInfo>& info) const;
	void preLoadPlugin() const override;
	void postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> >& batchInfo) const override;

	enum {
		id_calcuate_features,
		id_generate_vocabulary,
		id_identify_writer,
		id_evaluate_database,
		id_extract_patches,
		id_extract_patches_per_page,
		id_extract_random_patches,
		// add actions here

		id_end
	};

protected:
	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;
	rdf::WriterDatabase mWIDatabase;

private:
	void init();
	void loadSettings(QSettings& settings);
	void saveSettings(QSettings& settings) const;
	QString featureFilePath(QString imgPath, bool createDir=false) const;
	QString extractWriterIDFromFilename(const QString fileName) const;

	QString mSettingsVocPath = QString();
	int mVocType = rdf::WriterVocabulary::WI_UNDEFINED;
	int mVocNumberOfClusters = -1;
	int mVocNumberOfPCA = -1;
	int mVocMaxSIFTSize = -1;
	int mVocMinSIFTSize = -1;
	double mVocPowerNormalization;
	rdf::WriterVocabulary mVocabulary = rdf::WriterVocabulary();
	QString mFeatureDir = QString();
	QString mEvalFile = QString();
};

};

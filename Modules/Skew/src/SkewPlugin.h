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
#include "DkBatchInfo.h"

// RDF includes
#include "SkewEstimation.h"

class QSettings;


// opencv defines
namespace cv {
	class Mat;
}

namespace rdm {


class SkewInfo : public nmc::DkBatchInfo {

public:
	SkewInfo(const QString& id = QString(), const QString& filePath = QString());

	void setProperty(const QString& p);
	QString property() const;

	void setSkew(const double skew);
	double skew() const;

	void setSkewGt(const double skew);
	double skewGt() const;

private:
	QString mProp;
	double mSkew;
	double mSkewGt;

};

class SkewEstPlugin : public QObject, nmc::DkBatchPluginInterface {
	Q_OBJECT
		Q_INTERFACES(nmc::DkBatchPluginInterface)
		Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.SkewPlugin/5.1" FILE "SkewPlugin.json")

public:
	SkewEstPlugin(QObject* parent = 0);
	~SkewEstPlugin();

	QImage image() const override;
	QString name() const override;

	QList<QAction*> createActions(QWidget* parent) override;
	QList<QAction*> pluginActions() const override;
	QSharedPointer<nmc::DkImageContainer> runPlugin(
		const QString &runID, 
		QSharedPointer<nmc::DkImageContainer> imgC,
		const nmc::DkSaveInfo& saveInfo,
		QSharedPointer<nmc::DkBatchInfo>& info) const override;

	void preLoadPlugin() const override;
	void postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> >& batchInfo) const override;

	void setFilePath(QString fp);
	QString filePath() const;

	enum RunID {
		id_skew_native,
		id_skew_doc,
		id_skew_textline,
		id_skew_textline_draw,
		// add actions here

		id_end
	};

protected:
	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;

	QString mFilePath;
	rdf::BaseSkewEstimationConfig mBseConfig;

	double mMinAngle = -CV_PI/2.0;
	double mMaxAngle = CV_PI/2.0;

private:
	void init();
	void loadSettings(QSettings& settings);
	void saveSettings(QSettings& settings) const;

	void skewNative(QSharedPointer<nmc::DkImageContainer>& imgC, QSharedPointer<SkewInfo>& skewInfo) const;
	void skewDoc(QSharedPointer<nmc::DkImageContainer>& imgC, QSharedPointer<SkewInfo>& skewInfo) const;
	void skewTextLine(QSharedPointer<nmc::DkImageContainer>& imgC, QSharedPointer<SkewInfo>& skewInfo, const QString& runId) const;
	void parseGT(const QString& fileName, double skewAngle, QSharedPointer<SkewInfo>& skewInfo) const;
};
};

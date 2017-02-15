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
#include "DkBatchInfo.h"

#include "Shapes.h"
#include "Elements.h"
#include "FormAnalysis.h"

// opencv defines
namespace cv {
	class Mat;
}


namespace rdm {

class FormsInfo : public nmc::DkBatchInfo {

public:
	FormsInfo(const QString& id = QString(), const QString& filePath = QString());

	void setFormName(const QString& p);
	QString formName() const;

	void setMatchName(const QString& p);
	QString matchName() const;

	void setFormSize(const QSize& s);
	QSize formSize() const;

	void setTemplId(int id);
	int iDForm() const;

	void setXMLTemplate(QString t);
	QString xmlTemplate() const;

	void setLineImg(cv::Mat& img);
	cv::Mat lineImg() const;

	void setLines(QVector<rdf::Line> hL, QVector<rdf::Line> vL);
	QVector<rdf::Line> hLines();
	QVector<rdf::Line> vLines();

	void addCell(QSharedPointer<rdf::TableCell> c);
	void setCells(QVector<QSharedPointer<rdf::TableCell>> c);
	QVector<QSharedPointer<rdf::TableCell>> cells() const;
	
	void setRegion(QSharedPointer<rdf::TableRegion> r);
	QSharedPointer<rdf::TableRegion> region();

private:
	QString mProp; //form name
	QString mMatchName;
	QString mXMLTemplate;
	int mIdForm = 0;
	QSize mS;
	QVector<rdf::Line> mVerLines;
	QVector<rdf::Line> mHorLines;
	cv::Mat mLineImg;

	QSharedPointer<rdf::TableRegion> mRegion;
	QVector<QSharedPointer<rdf::TableCell>> mCells;

};

class FormsAnalysis : public QObject, nmc::DkBatchPluginInterface {
	Q_OBJECT
		Q_INTERFACES(nmc::DkBatchPluginInterface)
		Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.Forms/3.2" FILE "Forms.json")

public:
	FormsAnalysis(QObject* parent = 0);
	~FormsAnalysis();

	QString id() const override;
	QImage image() const override;
	QString name() const override;

	QList<QAction*> createActions(QWidget* parent) override;
	QList<QAction*> pluginActions() const override;
	QSharedPointer<nmc::DkImageContainer> runPlugin(
		const QString &runID, 
		QSharedPointer<nmc::DkImageContainer> imgC,
		const nmc::DkSaveInfo& saveInfo,
		QSharedPointer<nmc::DkBatchInfo>& info) const override;

	virtual QSettings& settings() const override;

	void preLoadPlugin() const override;
	void postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> >& batchInfo) const override;

	enum {
		id_train,
		id_show,
		id_classify,
		id_classifyxml,
		// add actions here

		id_end
	};

protected:
	QList<QAction*> mActions;
	QStringList mRunIDs;
	QStringList mMenuNames;
	QStringList mMenuStatusTips;

	QString mLineTemplPath;
	rdf::FormFeaturesConfig mFormConfig;

	void loadSettings(QSettings& settings);
	//void saveSettings(QSettings& settings) const;
};
};

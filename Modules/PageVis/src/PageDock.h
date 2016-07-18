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

#pragma once

// nomacs
#include "DkBaseWidgets.h"
#include "DkWidgets.h"

// framework
#include "Elements.h"


#pragma warning(push, 0)	// no warnings from includes
#include <QPushButton>
#include <QLineEdit>
#pragma warning(pop)

// Qt defines
class QPaintEvent;
class QSettings;
class QCheckBox;
class QSpinBox;
class QMimeData;

namespace rdf {
	class RegionTypeConfig;
}

namespace rdm {

// read defines
class PageData;

class ColorButton : public QWidget {
	Q_OBJECT

public:
	ColorButton(const QString& text = QString(), QWidget* parent = 0);

	void setColor(const QColor& col);

public slots:
	void on_colorButton_clicked();

signals:
	void newColor(const QColor& col) const;

private:
	void createLayout(const QString& title);

	QColor mColor;
};


class ConfigWidget : public QWidget {
	Q_OBJECT

public:
	ConfigWidget(QWidget* parent = 0);

	void setRegionConfig(QSharedPointer<rdf::RegionTypeConfig> config);

public slots:
	void on_outlineButton_newColor(const QColor& col);
	void on_brushButton_newColor(const QColor& col);
	void on_strokeBox_valueChanged(int val);
	void on_draw_clicked(bool toggled);
	void on_drawText_clicked(bool toggled);
	void on_drawPolygon_clicked(bool toggled);
	void on_drawBaseline_clicked(bool toggled);

	void loadConfig(const QString& name);
	void saveConfig(const QString& name);

signals:
	void updated() const;

private:
	void createLayout();
	void paintEvent(QPaintEvent* event) override;
	void updateElements();

	QSharedPointer<rdf::RegionTypeConfig> mConfig;

	ColorButton* mOutlineButton = 0;
	ColorButton* mBrushButton = 0;
	QSpinBox* mStrokeBox = 0;

	QCheckBox* mCbDraw = 0;
	QCheckBox* mCbDrawPoly = 0;
	QCheckBox* mCbDrawText = 0;
	QCheckBox* mCbDrawBaseline = 0;
};

class XmlLabel : public QLineEdit {
	Q_OBJECT

public:
	XmlLabel(QWidget* parent = 0);

public slots:
	void setPage(QSharedPointer<rdf::PageElement> page);
	void pathUpdated(const QString& path) const;


signals:
	void loadXml(const QString& path) const;

private:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	
	QString xmlPathFromMime(const QMimeData& mime) const;
	void createLayout();
};

class PageProfileWidget : public nmc::DkGenericProfileWidget {
	Q_OBJECT

public:
	PageProfileWidget(QWidget* parent);

public slots:
	void saveSettings(const QString& name) const override;
	void loadSettings(const QString& name) override;

signals:
	void savePageConfigSignal(const QString& name) const;
	void loadPageConfigSignal(const QString& name) const;

};

class TitledLabel : public QWidget {
	Q_OBJECT

public:
	TitledLabel(const QString& title = QString(), QWidget* parent = 0);

	QString text() const;

public slots:
	void setText(const QString& text);

protected:
	void createLayout(const QString& title);

	QLabel* mInfoLabel = 0;
};

class PolygonWidget : public QLabel {
	Q_OBJECT

public:
	PolygonWidget(QWidget* parent = 0);

	void setPolygon(const QPolygon & poly);
	void setConfig(const QSharedPointer<rdf::RegionTypeConfig>& config);

protected:
	void paintEvent(QPaintEvent* ev) override;

	QPolygonF mPoly;
	QSharedPointer<rdf::RegionTypeConfig> mConfig;
};

class PolygonInfoWidget : public QWidget {
	Q_OBJECT

public:
	PolygonInfoWidget(QWidget* parent = 0);

	void setConfig(const QSharedPointer<rdf::RegionTypeConfig>& config);

	void setPolygon(const QPolygon& poly);
	QPolygon polygon() const;

protected:
	void createLayout();

	QPolygon mPoly;

	PolygonWidget* mPolyWidget = 0;
	QLabel* mPolyText = 0;
};

class RegionWidget : public QWidget {
	Q_OBJECT

public:
	RegionWidget(QWidget* parent = 0);

	QSharedPointer<rdf::Region> currentRegion() const;

public slots:
	void setRegions(const QVector<QSharedPointer<rdf::Region> >& regions, int idx = -1);
	void setRegionTypes(const QVector<QSharedPointer<rdf::RegionTypeConfig> >& configs);
	void on_regionCombo_currentIndexChanged(int idx);
	void on_childCombo_currentIndexChanged(int idx);

signals:
	void updateSignal() const;

protected:
	void createLayout();
	void updateWidgets(QSharedPointer<rdf::Region> region);
	void clear();
	void showInfo(bool show = true);
	void paintEvent(QPaintEvent* event) override;

	QVector<QSharedPointer<rdf::Region> > mRegions;
	QVector<QSharedPointer<rdf::RegionTypeConfig> > mRegionTypes;
	QComboBox* mRegionCombo;
	QComboBox* mChildCombo;
	QLabel* mId;
	PolygonInfoWidget* mPolyWidget;
	PolygonInfoWidget* mBaselineWidget;
	QLabel* mTextTitle;
	TitledLabel* mText;
	TitledLabel* mCustom;
};

class PageDock : public nmc::DkDockWidget {
	Q_OBJECT

public:
	PageDock(PageData* pageData, const QString& title, QWidget* parent = 0);
	~PageDock();

	bool drawRegions() const;
	RegionWidget* regionWidget();

	// I don't really like this definition - but: it's hard to read an external stylesheet for these few widgets
	static QString widgetStyle;
	static QString largeComboStyle;
	static QString smallComboStyle;

public slots:
	void on_drawCheckbox_toggled(bool toggled) const;
	void on_configCombo_currentIndexChanged(int index);
	void on_infoWidget_updated();
	void on_infoWidget_updateSignal();
	void updateConfig();

signals:
	void updateSignal() const;
	void closeSignal() const;

private:
	void createLayout();
	void setConfigWidget(QSharedPointer<rdf::RegionTypeConfig> config);
	void closeEvent(QCloseEvent* ev) override;

	QCheckBox* mCbDraw = 0;
	ConfigWidget* mConfigWidget = 0;
	RegionWidget* mRegionWidget = 0;

	rdf::Region::Type mCurrentRegion = rdf::Region::type_text_region;
	
	PageData* mPageData = 0;
};

};
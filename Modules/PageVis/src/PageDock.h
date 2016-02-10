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

#include "DkBaseWidgets.h"

// framework
#include "Elements.h"


#pragma warning(push, 0)	// no warnings from includes
#include <QPushButton>
#pragma warning(pop)

// Qt defines
class QPaintEvent;
class QSettings;
class QCheckBox;
class QSpinBox;

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

signals:
	void updated() const;

private:
	void createLayout();
	void updateElements();
	void paintEvent(QPaintEvent* event) override;

	QSharedPointer<rdf::RegionTypeConfig> mConfig;

	ColorButton* mOutlineButton = 0;
	ColorButton* mBrushButton = 0;
	QSpinBox* mStrokeBox = 0;

	QCheckBox* mCbDraw = 0;
	QCheckBox* mCbDrawPoly = 0;
	QCheckBox* mCbDrawText = 0;
	QCheckBox* mCbDrawBaseline = 0;
};

class PageDock : public nmc::DkDockWidget {
	Q_OBJECT

public:
	PageDock(PageData* pageData, const QString& title, QWidget* parent = 0);
	~PageDock();

	bool drawRegions() const;
	QVector<rdf::RegionTypeConfig> config() const;

public slots:
	void on_drawCheckbox_toggled(bool toggled) const;
	void on_configCombo_currentIndexChanged(int index);
	void on_configWidget_updated();

signals:
	void updateSignal() const;

private:
	void createLayout();
	void setConfigWidget(QSharedPointer<rdf::RegionTypeConfig> config);

	QCheckBox* mCbDraw = 0;
	ConfigWidget* mConfigWidget = 0;

	rdf::Region::Type mCurrentRegion = rdf::Region::type_text_region;
	
	PageData* mPageData = 0;
};

};
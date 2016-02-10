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

#include "PageDock.h"
#include "Settings.h"
#include "ElementsHelper.h"

// nomacs 
#include "DkUtils.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QPaintEvent>
#include <QSettings>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDebug>
#include <QComboBox>
#include <QColorDialog>
#include <QStyleOption>
#include <QPainter>
#include <QSpinBox>
#pragma warning(pop)

namespace rdm {

// ColorButton --------------------------------------------------------------------
ColorButton::ColorButton(const QString& text, QWidget* parent) : QWidget(parent) {
	setObjectName("colorButton");

	createLayout(text);
	QMetaObject::connectSlotsByName(this);
}

void ColorButton::createLayout(const QString& title) {

	QPushButton* button = new QPushButton("", this);
	button->setFlat(true);
	button->setObjectName("colorButton");

	QLabel* titleLabel = new QLabel(title, this);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setAlignment(Qt::AlignLeft);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(button);
	layout->addWidget(titleLabel);

}

void ColorButton::on_colorButton_clicked() {

	QColorDialog* cd = new QColorDialog(mColor, this);
	cd->setOption(QColorDialog::ShowAlphaChannel, true);
	int answer = cd->exec();

	if (answer == QDialog::Accepted) {
		setColor(cd->currentColor());
		emit newColor(mColor);
	}

	cd->deleteLater();
}

void ColorButton::setColor(const QColor& col) {
	
	mColor = col;
	setStyleSheet("QPushButton#colorButton{background-color: " + nmc::DkUtils::colorToString(col) + "; border: 1px solid #888; min-height: 24px}");
}


// ConfigWidget --------------------------------------------------------------------
ConfigWidget::ConfigWidget(QWidget* parent) : QWidget(parent) {
	
	setObjectName("configWigdet");
	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void ConfigWidget::setRegionConfig(const rdf::RegionTypeConfig & config) {
	mConfig = config;
	updateElements();
}

rdf::RegionTypeConfig ConfigWidget::config() const {
	return mConfig;
}

void ConfigWidget::createLayout() {

	mOutlineButton = new ColorButton(tr("Outline"), this);
	mOutlineButton->setObjectName("outlineButton");

	mBrushButton = new ColorButton(tr("Brush"), this);
	mBrushButton->setObjectName("brushButton");

	mCbDraw = new QCheckBox(tr("Draw"), this);
	mCbDraw->setObjectName("draw");

	mCbDrawPoly = new QCheckBox(tr("Draw Polygon"), this);
	mCbDrawPoly->setObjectName("drawPolygon");

	mCbDrawBaseline = new QCheckBox(tr("Draw Baseline"), this);
	mCbDrawBaseline->setObjectName("drawBaseline");

	mCbDrawText = new QCheckBox(tr("Draw Text"), this);
	mCbDrawText->setObjectName("drawText");

	QWidget* cbWidget = new QWidget(this);
	QGridLayout* cbLayout = new QGridLayout(cbWidget);
	cbLayout->setContentsMargins(0, 0, 0, 0);
	cbLayout->addWidget(mCbDraw, 0, 0);
	cbLayout->addWidget(mCbDrawPoly, 0, 1);
	cbLayout->addWidget(mCbDrawText, 1, 0);
	cbLayout->addWidget(mCbDrawBaseline, 1, 1);

	// stroke
	QLabel* strokeLabel = new QLabel(tr("Stroke Width"), this);
	strokeLabel->setObjectName("titleLabel");

	mStrokeBox = new QSpinBox(this);
	mStrokeBox->setObjectName("strokeBox");
	mStrokeBox->setMaximumWidth(100);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mOutlineButton);
	layout->addWidget(mBrushButton);
	layout->addWidget(strokeLabel);
	layout->addWidget(mStrokeBox);
	layout->addWidget(cbWidget);

	setStyleSheet("QWidget#configWidget{background-color: #fff;} QLabel#titleLabel{font-size: 10pt; margin-top: 10pt;}");
}

void ConfigWidget::updateElements() {

	mOutlineButton->setColor(mConfig.pen().color());
	mBrushButton->setColor(mConfig.brush());
	mStrokeBox->setValue(mConfig.pen().width());

	mCbDraw->setChecked(mConfig.draw());
	mCbDrawPoly->setChecked(mConfig.drawPoly());
	mCbDrawBaseline->setChecked(mConfig.drawBaseline());
	mCbDrawText->setChecked(mConfig.drawText());
}

void ConfigWidget::on_outlineButton_newColor(const QColor& col) {
	
	QPen& p = mConfig.pen();
	p.setColor(col);
	mConfig.setPen(p);

	// also update brush
	QColor bc = col;
	bc.setAlpha(mConfig.brush().alpha());
	mConfig.setBrush(bc);

	mBrushButton->setColor(bc);
	qDebug() << "new OUTLINE color";

	emit updated();
}

void ConfigWidget::on_brushButton_newColor(const QColor& col) {
	
	mConfig.setBrush(col);

	emit updated();
}

void ConfigWidget::on_strokeBox_valueChanged(int val) {

	QPen p = mConfig.pen();
	p.setWidth(val);
	mConfig.setPen(p);

	emit updated();
}

void ConfigWidget::on_draw_clicked(bool toggled) {
	mConfig.setDraw(toggled);
	emit updated();
}

void ConfigWidget::on_drawPolygon_clicked(bool toggled) {
	mConfig.setDrawPoly(toggled);
	emit updated();
}

void ConfigWidget::on_drawBaseline_clicked(bool toggled) {
	mConfig.setDrawBaseline(toggled);
	emit updated();
}


void ConfigWidget::on_drawText_clicked(bool toggled) {
	mConfig.setDrawText(toggled);
	emit updated();
}

void ConfigWidget::paintEvent(QPaintEvent* event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}


// PageViewport --------------------------------------------------------------------
PageDock::PageDock(const QString& title, QWidget* parent) : nmc::DkDockWidget(title, parent) {

	setObjectName("PageDock");
	loadSettings(nmc::Settings::instance().getSettings());

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

PageDock::~PageDock() {

	saveSettings(nmc::Settings::instance().getSettings());
	qDebug() << "destroying PAGE viewport";
}

void PageDock::createLayout() {

	// draw checkbox
	mCbDraw = new QCheckBox(tr("Draw Elements"), this);
	mCbDraw->setObjectName("drawCheckbox");
	mCbDraw->setChecked(true);

	// config combo
	QComboBox* configCombo = new QComboBox(this);
	configCombo->setObjectName("configCombo");
	
	QString bg = "QComboBox{font-size: 14pt; min-height: 40px; color: #006699; font-weight: light; border: none; border-bottom: 1px solid #666;}";
	QString dd = "QComboBox::drop-down {border:none;}";
	configCombo->setStyleSheet(bg+dd);
	
	for (const rdf::RegionTypeConfig& c : mConfig)
		configCombo->addItem(QIcon(), rdf::RegionManager::instance().typeName(c.type()));

	// config widget
	mConfigWidget = new ConfigWidget(this);
	mConfigWidget->setObjectName("configWidget");

	QWidget* configDummy = new QWidget(this);
	QVBoxLayout* configLayout = new QVBoxLayout(configDummy);
	configLayout->setContentsMargins(0, 0, 0, 0);
	configLayout->setSpacing(0);
	configLayout->addWidget(configCombo);
	configLayout->addWidget(mConfigWidget);

	QWidget* base = new QWidget(this);
	QVBoxLayout* baseLayout = new QVBoxLayout(base);
	baseLayout->setAlignment(Qt::AlignTop);
	baseLayout->addWidget(mCbDraw);
	baseLayout->addWidget(configDummy);

	// add a scroll bar
	nmc::DkResizableScrollArea* scrollArea = new nmc::DkResizableScrollArea(this);
	scrollArea->setObjectName("pageDockScroll");
	scrollArea->setWidgetResizable(true);
	scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	scrollArea->setWidget(base);
	scrollArea->setStyleSheet("QScrollArea{border: none;}");

	QWidget* dummy = new QWidget(this);
	QVBoxLayout* l = new QVBoxLayout(dummy);
	l->setContentsMargins(0,0,0,0);
	l->addWidget(scrollArea);

	setWidget(dummy);

	// update
	configCombo->setCurrentIndex(mCurrentRegion);
	setConfigWidget(mConfig[mCurrentRegion]);
}

void PageDock::setConfigWidget(const rdf::RegionTypeConfig & config) {

	mConfigWidget->setRegionConfig(config);
}

void PageDock::saveSettings(QSettings& settings) const {

	settings.beginGroup(objectName());
	for (const rdf::RegionTypeConfig& c : mConfig)
		c.save(settings);
	settings.endGroup();
}

void PageDock::loadSettings(QSettings& settings) {

	settings.beginGroup(objectName());

	mConfig = rdf::RegionManager::instance().regionTypeConfig();

	// load from nomacs settings
	for (rdf::RegionTypeConfig& c : mConfig)
		c.load(settings);

	settings.endGroup();
}

QVector<rdf::RegionTypeConfig> PageDock::config() const {
	return mConfig;
}

void PageDock::on_drawCheckbox_toggled(bool toggled) const {

	emit updateSignal();
}

bool PageDock::drawRegions() const {
	return mCbDraw->isChecked();
}

void PageDock::on_configCombo_currentIndexChanged(int index) {
	
	mConfig[mCurrentRegion] = mConfigWidget->config();
	setConfigWidget(mConfig[index]);
	mCurrentRegion = (rdf::Region::Type)index;
}

void PageDock::on_configWidget_updated() {

	mConfig[mCurrentRegion] = mConfigWidget->config();
	emit updateSignal();
}

}
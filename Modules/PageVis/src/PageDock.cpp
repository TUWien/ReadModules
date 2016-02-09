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
	setStyleSheet("QPushButton {background-color: " + nmc::DkUtils::colorToString(col) + "; border: 1px solid #888; min-height: 24px}");
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

	mTitle = new QLabel("", this);
	mTitle->setObjectName("regionTitle");
	mTitle->setStyleSheet("QLabel{font-size: 14pt; color: #006699; font-weight: light; border-bottom: 2px solid #666;}");

	mOutlineButton = new ColorButton(tr("Outline"), this);
	mOutlineButton->setObjectName("outlineButton");

	mBrushButton = new ColorButton(tr("Brush"), this);
	mBrushButton->setObjectName("brushButton");

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mTitle);
	layout->addWidget(mOutlineButton);
	layout->addWidget(mBrushButton);

	setStyleSheet("QWidget#configWidget{background-color: #eee;}");
}

void ConfigWidget::updateElements() {

	mTitle->setText(rdf::RegionManager::instance().typeName(mConfig.type()));

	mOutlineButton->setColor(mConfig.pen().color());
	mBrushButton->setColor(mConfig.pen().brush().color());
}

void ConfigWidget::on_outlineButton_newColor(const QColor& col) {
	QPen& p = mConfig.pen();
	p.setColor(col);

	emit updated();
}

void ConfigWidget::on_brushButton_newColor(const QColor& col) {
	QPen& p = mConfig.pen();
	p.setBrush(col);

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
	
	for (const rdf::RegionTypeConfig& c : mConfig)
		configCombo->addItem(QIcon(), rdf::RegionManager::instance().typeName(c.type()));

	// config widget
	mConfigWidget = new ConfigWidget(this);
	mConfigWidget->setObjectName("configWidget");

	QWidget* base = new QWidget(this);
	QVBoxLayout* baseLayout = new QVBoxLayout(base);
	baseLayout->setAlignment(Qt::AlignTop);
	baseLayout->addWidget(mCbDraw);
	baseLayout->addWidget(configCombo);
	baseLayout->addWidget(mConfigWidget);

	setWidget(base);

	// update
	mCurrentRegion = rdf::Region::type_unknown;
	configCombo->setCurrentIndex(mCurrentRegion);
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
	setConfigWidget(mConfig[index]);
	mCurrentRegion = (rdf::Region::Type)index;
}

void PageDock::on_configWidget_updated() {

	mConfig[mCurrentRegion] = mConfigWidget->config();
	emit updateSignal();
}

}
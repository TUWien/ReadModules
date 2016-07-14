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
#include "PageData.h"

// framework
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
#include <QMimeData>
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

void ConfigWidget::setRegionConfig(QSharedPointer<rdf::RegionTypeConfig> config) {
	mConfig = config;
	updateElements();
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
	cbLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
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

	if (!mConfig)
		return;

	mOutlineButton->setColor(mConfig->pen().color());
	mBrushButton->setColor(mConfig->brush());
	mStrokeBox->setValue(mConfig->pen().width());

	mCbDraw->setChecked(mConfig->draw());
	mCbDrawPoly->setChecked(mConfig->drawPoly());
	mCbDrawBaseline->setChecked(mConfig->drawBaseline());
	mCbDrawText->setChecked(mConfig->drawText());
}

void ConfigWidget::on_outlineButton_newColor(const QColor& col) {
	
	if (!mConfig)
		return;

	QPen p = mConfig->pen();
	p.setColor(col);
	mConfig->setPen(p);

	// also update brush
	QColor bc = col;
	bc.setAlpha(mConfig->brush().alpha());
	mConfig->setBrush(bc);

	mBrushButton->setColor(bc);
	qDebug() << "new OUTLINE color";

	emit updated();
}

void ConfigWidget::on_brushButton_newColor(const QColor& col) {
	
	if (!mConfig)
		return;

	mConfig->setBrush(col);

	emit updated();
}

void ConfigWidget::on_strokeBox_valueChanged(int val) {

	if (!mConfig)
		return;

	QPen p = mConfig->pen();
	p.setWidth(val);
	mConfig->setPen(p);

	emit updated();
}

void ConfigWidget::on_draw_clicked(bool toggled) {
	
	if (!mConfig)
		return;
	
	mConfig->setDraw(toggled);
	emit updated();
}

void ConfigWidget::on_drawPolygon_clicked(bool toggled) {
	
	if (!mConfig)
		return;
	
	mConfig->setDrawPoly(toggled);
	emit updated();
}

void ConfigWidget::on_drawBaseline_clicked(bool toggled) {
	
	if (!mConfig)
		return;

	mConfig->setDrawBaseline(toggled);
	emit updated();
}

void ConfigWidget::loadConfig(const QString & name) {

	// use nomacs settings here - for it's in the GUI
	QSettings& settings = nmc::Settings::instance().getSettings();
	settings.beginGroup(name);
	auto config = QSharedPointer<rdf::RegionTypeConfig>::create();
	config->load(settings);
	settings.endGroup();

	setRegionConfig(config);
	emit updated();
}

void ConfigWidget::saveConfig(const QString & name) {

	if (!mConfig) {
		qWarning() << "Cannot save config - it's empty!";
		return;
	}

	// use nomacs settings here - for it's in the GUI
	QSettings& settings = nmc::Settings::instance().getSettings();
	settings.beginGroup(name);
	mConfig->save(settings);
	settings.endGroup();
}


void ConfigWidget::on_drawText_clicked(bool toggled) {
	
	if (!mConfig)
		return;
	
	mConfig->setDrawText(toggled);
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

// XmlLabel --------------------------------------------------------------------
XmlLabel::XmlLabel(QWidget* parent) : QLineEdit(parent) {

	setObjectName("XmlLabel");
	createLayout();
	setAcceptDrops(true);

	connect(this, SIGNAL(textEdited(const QString&)), this, SLOT(pathUpdated(const QString&)));
}

void XmlLabel::createLayout() {

	setMinimumHeight(100);
	setAlignment(Qt::AlignTop);

	setStyleSheet("QLineEdit#XmlLabel{background-color: #fff; border: none;}");
}

void XmlLabel::setPage(QSharedPointer<rdf::PageElement> page) {
	
	if (!page)
		return;

	setText(page->xmlPath());
}

void XmlLabel::pathUpdated(const QString& path) const {

	QFileInfo info(path);

	if (info.exists() && info.suffix() == "xml")
		emit loadXml(path);

}

void XmlLabel::dragEnterEvent(QDragEnterEvent* event) {

	if (event->mimeData() && !xmlPathFromMime(*event->mimeData()).isEmpty()) {
		event->acceptProposedAction();
		qDebug() << "event accepted...";
	}

	QWidget::dragEnterEvent(event);
}

void XmlLabel::dropEvent(QDropEvent * event) {

	if (event->mimeData())
		emit loadXml(xmlPathFromMime(*event->mimeData()));

	QLineEdit::dropEvent(event);
}

void XmlLabel::paintEvent(QPaintEvent* event) {

	QLineEdit::paintEvent(event);

	QColor linCol(200, 200, 200);
	QRect dropArea(QPoint(0, 20), QSize(width(), height()-20));

	// The view is empty.
	QPainter p(this);
	p.setPen(Qt::NoPen);
	p.setBrush(QBrush(linCol, Qt::BDiagPattern));
	p.drawRect(dropArea);

	p.setPen(QPen(linCol));
	p.drawLine(dropArea.topLeft(), dropArea.topRight());

	p.setPen(QPen(QColor(100,100,100)));
	p.drawText(dropArea, Qt::AlignCenter, tr("Drag XMLs Here"));
}

QString XmlLabel::xmlPathFromMime(const QMimeData & mime) const {

	QString path;
	if (mime.hasUrls()) {
		path = mime.urls()[0].toLocalFile();
	}
	else if (mime.hasText()) {
		path = mime.text();
	}
	
	QFileInfo info(path);

	if (info.exists() && info.suffix() == "xml")
		return info.absoluteFilePath();
	else
		qDebug() << info.absoluteFilePath() << "is not valid - rejecting...";

	return QString();
}

// PageProfileWidget --------------------------------------------------------------------
PageProfileWidget::PageProfileWidget(QWidget* parent) : nmc::DkGenericProfileWidget(tr("PageVis Profiles"), parent) {

	// NOTE: hast to be the object name of PageData
	mSettingsGroup = "PageDataProfiles";
	init();
	activate();
}

void PageProfileWidget::saveSettings(const QString& name) const {

	emit savePageConfigSignal(name);

	DkGenericProfileWidget::saveSettings(name);
	setDefaultModel();
}

void PageProfileWidget::loadSettings(const QString& name) {

	emit loadPageConfigSignal(name);
	setDefaultModel();
}


// PageDock --------------------------------------------------------------------
PageDock::PageDock(PageData* pageData, const QString& title, QWidget* parent) : nmc::DkDockWidget(title, parent) {

	setObjectName("PageDock");
	mPageData = pageData;

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

PageDock::~PageDock() {

	qDebug() << "destroying PAGE viewport";
}

void PageDock::createLayout() {

	// draw checkbox
	mCbDraw = new QCheckBox(tr("Draw Elements"), this);
	mCbDraw->setObjectName("drawCheckbox");
	mCbDraw->setChecked(true);

	// profiles
	PageProfileWidget* profileWidget = new PageProfileWidget(this);

	// config combo
	QComboBox* configCombo = new QComboBox(this);
	configCombo->setObjectName("configCombo");
	
	QString bg = "QComboBox{font-size: 14pt; min-height: 40px; color: #006699; font-weight: light; border: none; border-bottom: 1px solid #666;}";
	QString dd = "QComboBox::drop-down {border:none;}";
	configCombo->setStyleSheet(bg+dd);
	
	for (auto c : mPageData->config())
		configCombo->addItem(QIcon(), rdf::RegionManager::instance().typeName(c->type()));

	// config widget
	mConfigWidget = new ConfigWidget(this);
	mConfigWidget->setObjectName("configWidget");

	QWidget* configDummy = new QWidget(this);
	QVBoxLayout* configLayout = new QVBoxLayout(configDummy);
	configLayout->setContentsMargins(0, 0, 0, 0);
	configLayout->setSpacing(0);
	configLayout->addWidget(configCombo);
	configLayout->addWidget(mConfigWidget);

	// xml drop widget
	XmlLabel* xmlWidget = new XmlLabel(this);
	xmlWidget->setPage(mPageData->page());

	// create our widget
	QWidget* base = new QWidget(this);
	QVBoxLayout* baseLayout = new QVBoxLayout(base);
	baseLayout->setAlignment(Qt::AlignTop);
	baseLayout->addWidget(mCbDraw);
	baseLayout->addWidget(profileWidget);
	baseLayout->addWidget(configDummy);
	baseLayout->addWidget(xmlWidget);

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

	// connects
	connect(mPageData, SIGNAL(updatePage(QSharedPointer<rdf::PageElement>)), xmlWidget, SLOT(setPage(QSharedPointer<rdf::PageElement>)));
	connect(xmlWidget, SIGNAL(loadXml(const QString&)), mPageData, SLOT(parse(const QString&)));
	connect(profileWidget, SIGNAL(loadPageConfigSignal(const QString&)), mPageData, SLOT(loadConfig(const QString&)));
	connect(profileWidget, SIGNAL(savePageConfigSignal(const QString&)), mPageData, SLOT(saveConfig(const QString&)));
	connect(mPageData, SIGNAL(updateConfig()), this, SLOT(updateConfig()));

	// update
	configCombo->setCurrentIndex(mCurrentRegion);
	setConfigWidget(mPageData->config()[mCurrentRegion]);

	profileWidget->show();	// nasty: but it is derived from dk widget

}

void PageDock::setConfigWidget(QSharedPointer<rdf::RegionTypeConfig> config) {

	mConfigWidget->setRegionConfig(config);
}

void PageDock::on_drawCheckbox_toggled(bool toggled) const {

	emit updateSignal();
}

bool PageDock::drawRegions() const {
	return mCbDraw->isChecked();
}

void PageDock::updateConfig() {

	setConfigWidget(mPageData->config()[mCurrentRegion]);
	emit updateSignal();
}

void PageDock::on_configCombo_currentIndexChanged(int index) {
	
	setConfigWidget(mPageData->config()[index]);
	mCurrentRegion = (rdf::Region::Type)index;

	emit updateSignal();
}

void PageDock::on_configWidget_updated() {

	//// correctly udpate even if we changed the current region already
	//rdf::RegionTypeConfig c = mConfigWidget->config();
	//mConfig[c.type()] = c;
	emit updateSignal();
}

}
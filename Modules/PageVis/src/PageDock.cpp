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
#include <QCompleter>
#include <QDirModel>
#pragma warning(pop)

namespace rdm {

	QString PageDock::widgetStyle = QString("QWidget#infoWidget{background-color: #fff; border: none;}") +
		"QLabel#titleLabel{font-size: 12pt; margin-top: 10pt;}" +
		"QLabel#infoLabel{font-size: 10pt; color: #666;}";

QString PageDock::largeComboStyle = QString("QComboBox{font-size: 14pt; min-height: 40px; color: #006699; font-weight: light; border: none; border-bottom: 1px solid #666;}") +
		"QComboBox::drop-down {border:none;}";

QString PageDock::smallComboStyle = QString("QComboBox{font-size: 11pt; min-height: 25px; color: #666; font-weight: light; border: none; border-bottom: 1px solid #666;}") +
"QComboBox::drop-down {border:none;}";

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
	
	setObjectName("infoWidget");
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

	mCbDrawText = new QCheckBox(tr("Draw Text"), this);
	mCbDrawText->setObjectName("drawText");

	mCbDrawPoly = new QCheckBox(tr("Draw Polygon"), this);
	mCbDrawPoly->setObjectName("drawPolygon");

	mCbDrawBaseline = new QCheckBox(tr("Draw Baseline"), this);
	mCbDrawBaseline->setObjectName("drawBaseline");

	QWidget* cbWidget = new QWidget(this);
	QGridLayout* cbLayout = new QGridLayout(cbWidget);
	cbLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	cbLayout->setContentsMargins(0, 0, 0, 0);
	cbLayout->addWidget(mCbDraw, 0, 0);
	cbLayout->addWidget(mCbDrawText, 1, 0);
	cbLayout->addWidget(mCbDrawPoly, 0, 1);
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

	setStyleSheet(PageDock::widgetStyle);
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

	QSettings& settings = nmc::DkSettingsManager::instance().qSettings();
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

	QSettings& settings = nmc::DkSettingsManager::instance().qSettings();
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

// DirLabel --------------------------------------------------------------------
DirLabel::DirLabel(QWidget* parent) : QLineEdit(parent) {

	setObjectName("dirWidget");
	createLayout();
	setAcceptDrops(true);

	connect(this, SIGNAL(textEdited(const QString&)), this, SIGNAL(xmlPathChanged(const QString&)));
}

void DirLabel::createLayout() {

	setAlignment(Qt::AlignTop);

	//"QLineEdit#XmlLabel{background-color: #fff; border: none;}"
	setStyleSheet(PageDock::widgetStyle);
	setPlaceholderText(tr("XML Directory"));

	// this is a directory edit
	QCompleter *completer = new QCompleter(this);
	QDirModel* model = new QDirModel(completer);
	model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	completer->setModel(model);
	setCompleter(completer);
}

void DirLabel::dragEnterEvent(QDragEnterEvent* event) {

	if (event->mimeData() && !dirPathFromMime(*event->mimeData()).isEmpty()) {
		event->acceptProposedAction();
		qDebug() << "event accepted...";
	}

	QWidget::dragEnterEvent(event);
}

void DirLabel::dropEvent(QDropEvent * event) {

	if (event->mimeData()) {
		QString path = dirPathFromMime(*event->mimeData());
		setText(path);
		emit xmlPathChanged(path);
	}

	//QLineEdit::dropEvent(event);
}

QString DirLabel::dirPathFromMime(const QMimeData & mime) const {

	QString path;
	if (mime.hasUrls()) {
		path = mime.urls()[0].toLocalFile();
	}
	else if (mime.hasText()) {
		path = mime.text();
	}

	QDir dir(path);

	if (dir.exists())
		return dir.absolutePath();
	else
		qDebug() << dir.absolutePath() << "is not valid - rejecting...";

	return QString();
}


// XmlLabel --------------------------------------------------------------------
XmlLabel::XmlLabel(QWidget* parent) : QLineEdit(parent) {

	setObjectName("infoWidget");
	createLayout();
	setAcceptDrops(true);

	connect(this, SIGNAL(textEdited(const QString&)), this, SLOT(pathUpdated(const QString&)));
}

void XmlLabel::createLayout() {

	setMinimumHeight(100);
	setAlignment(Qt::AlignTop);

	//"QLineEdit#XmlLabel{background-color: #fff; border: none;}"
	setStyleSheet(PageDock::widgetStyle);
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

// TitledLabel --------------------------------------------------------------------
TitledLabel::TitledLabel(const QString& title, QWidget* parent) : QWidget(parent) {
	createLayout(title);
}

QString TitledLabel::text() const {
	return mInfoLabel->text();
}

void TitledLabel::createLayout(const QString& title) {

	QLabel* titleLabel = new QLabel(title, this);
	titleLabel->setObjectName("titleLabel");

	mInfoLabel = new QLabel("", this);
	mInfoLabel->setObjectName("infoLabel");
	mInfoLabel->setWordWrap(true);
	mInfoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(titleLabel);
	layout->addWidget(mInfoLabel);
}

void TitledLabel::setText(const QString& msg) {
	mInfoLabel->setText(msg);
}

// PolygonWidget --------------------------------------------------------------------
PolygonWidget::PolygonWidget(QWidget* parent) {
}

void PolygonWidget::setPolygon(const QPolygon & poly) {
	
	// norm polygon
	QPolygonF pn;
	QRect r = poly.boundingRect();
	double scale = qMax(r.width(), r.height());

	for (const QPoint& p : poly) {

		QPointF pf(p);
		pf.setX((pf.x()-r.left()) / scale);
		pf.setY((pf.y()-r.top()) / scale);
		pn << pf;
	}

	mPoly = pn;
	update();
}

void PolygonWidget::setConfig(const QSharedPointer<rdf::RegionTypeConfig>& config) {
	mConfig = config;
}

void PolygonWidget::paintEvent(QPaintEvent * ev) {

	// transform
	QRectF br = mPoly.boundingRect();
	double s = qMin(width(), height());
	int dx = qRound((1.0-br.width()) * s/2.0);
	int dy = qRound((1.0-br.height()) * s/2.0);

	QTransform wt;
	wt = wt.translate(dx, dy);
	wt = wt.scale(s, s);

	// style
	QPen pen;
	if (mConfig)
		pen = mConfig->pen();
	pen.setWidthF(.01);

	// draw
	QPainter p(this);
	p.setWorldTransform(wt);
	p.setPen(pen);

	// draw the polygon
	QPainterPath path;
	path.addPolygon(mPoly);
	p.drawPath(path);

	if (mPoly.isClosed() && mConfig)
		p.fillPath(path, mConfig->brush());

	QLabel::paintEvent(ev);
}

// PolygonInfoWidget --------------------------------------------------------------------
PolygonInfoWidget::PolygonInfoWidget(QWidget* parent) : QWidget(parent) {
	createLayout();
	setMaximumHeight(50);
}

void PolygonInfoWidget::createLayout() {

	mPolyWidget = new PolygonWidget(this);
	mPolyWidget->setFixedSize(50, 50);

	mPolyText = new QLabel(this);
	mPolyText->setObjectName("infoLabel");
	mPolyText->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mPolyText->setWordWrap(true);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(mPolyWidget);
	layout->addWidget(mPolyText);
}

void PolygonInfoWidget::setConfig(const QSharedPointer<rdf::RegionTypeConfig>& config) {
	mPolyWidget->setConfig(config);
}

void PolygonInfoWidget::setPolygon(const QPolygon & poly) {
	
	mPoly = poly;
	mPolyWidget->setPolygon(poly);

	if (poly.empty()) {
		mPolyText->setText("");
		return;
	}

	QString pt = "[";
	for (const QPoint& p : mPoly) {
		pt += QString::number(p.x()) + " " + QString::number(p.y()) + ", ";
	}

	pt += "]";

	mPolyText->setText(pt);
}

QPolygon PolygonInfoWidget::polygon() const {
	return mPoly;
}


// RegionWidget --------------------------------------------------------------------
RegionWidget::RegionWidget(QWidget* parent) : QWidget(parent) {

	setObjectName("infoWidget");
	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void RegionWidget::createLayout() {

	// Region Selection
	mRegionCombo = new QComboBox(this);
	mRegionCombo->setObjectName("regionCombo");
	mRegionCombo->setStyleSheet(PageDock::largeComboStyle);

	mId = new QLabel(this);
	mId->setObjectName("infoLabel");
	mId->setTextInteractionFlags(Qt::TextSelectableByMouse);

	// polygons
	mPolyWidget = new PolygonInfoWidget(this);

	mBaselineWidget = new PolygonInfoWidget(this);

	// text tag
	mText = new TitledLabel(tr("Text"), this);

	// custom tag
	mCustom = new TitledLabel(tr("Custom"), this);

	// children selection
	mChildCombo = new QComboBox(this);
	mChildCombo->setObjectName("childCombo");
	mChildCombo->setStyleSheet(PageDock::smallComboStyle);

	QWidget* infoWidget = new QWidget(this);
	QVBoxLayout* infoLayout = new QVBoxLayout(infoWidget);
	infoLayout->addWidget(mId);
	infoLayout->addWidget(mPolyWidget);
	infoLayout->addWidget(mBaselineWidget);
	infoLayout->addWidget(mText);
	infoLayout->addWidget(mCustom);
	infoLayout->addWidget(mChildCombo);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(mRegionCombo);
	layout->addWidget(infoWidget);

	setStyleSheet(PageDock::widgetStyle);
	showInfo(false);
	hide();
}

void RegionWidget::setRegions(const QVector<QSharedPointer<rdf::Region> >& regions, int idx) {
	
	mRegions = regions;
	mRegionCombo->clear();
	
	setVisible(!mRegions.empty());

	for (const QSharedPointer<rdf::Region> r : regions)
		mRegionCombo->addItem(rdf::RegionManager::instance().typeName(r->type()));

	if (!mRegions.isEmpty()) {
		if (idx)
			idx = mRegions.size() - 1;

		// reset index if necessary
		if (idx < 0 || idx > mRegions.size())
			idx = 0;

		mRegionCombo->setCurrentIndex(idx);
	}
}

void RegionWidget::setRegionTypes(const QVector<QSharedPointer<rdf::RegionTypeConfig> >& configs) {
	mRegionTypes = configs;
	updateWidgets(currentRegion());
}

void RegionWidget::on_regionCombo_currentIndexChanged(int idx) {
	updateWidgets(currentRegion());
}

void RegionWidget::on_childCombo_currentIndexChanged(int idx) {

	idx = idx - 1;	// first entry is <no child>

	if (!currentRegion() || idx < 0 || idx >= currentRegion()->children().size()) {
		qWarning() << "illegal child index" << idx;
		return;
	}

	for (auto r : mRegions)
		r->setSelected(false);

	QVector<QSharedPointer<rdf::Region> > r;
	r << currentRegion()->children()[idx];
	rdf::RegionManager::instance().selectRegions(r);
	setRegions(r);

	emit updateSignal();
}

QSharedPointer<rdf::Region> RegionWidget::currentRegion() const {
	
	int idx = mRegionCombo->currentIndex();
	if (mRegions.isEmpty() || idx < 0 || idx >= mRegions.size())
		return QSharedPointer<rdf::Region>();

	return mRegions[idx];
}

void RegionWidget::updateWidgets(QSharedPointer<rdf::Region> region) {

	if (region.isNull()) {
		clear();
		return;
	}

	QSharedPointer<rdf::RegionTypeConfig> config = rdf::RegionManager::instance().getConfig(region, mRegionTypes);
	
	mId->setText(tr("ID: %1").arg(region->id()));

	// polygons
	mPolyWidget->setPolygon(region->polygon().closedPolygon().toPolygon());
	mPolyWidget->setConfig(config);

	// child combo
	QString nChildrenStr;
	switch (region->children().size()) {
	case 0: nChildrenStr = tr("no children");	break;
	case 1: nChildrenStr = tr("1 child");		break;
	default:
		nChildrenStr = tr("%1 children").arg(region->children().size());
	}

	mChildCombo->blockSignals(true);
	mChildCombo->clear();
	mChildCombo->addItem(nChildrenStr);
	for (QSharedPointer<rdf::Region> r : region->children()) {
		mChildCombo->addItem(r->id());
	}
	mChildCombo->blockSignals(false);
	mChildCombo->setEnabled(!region->children().empty());

	// text
	if (auto tl = qSharedPointerDynamicCast<rdf::TextLine>(region)) {
		mBaselineWidget->setPolygon(tl->baseLine().toPolygon());
		mBaselineWidget->setConfig(config);
		mText->setText(tl->text());
	}
	else {
		mText->setText("");
		mBaselineWidget->setPolygon(QPolygon());
	}

	// custom tag
	mCustom->setText(region->custom());

	showInfo(true);
}

void RegionWidget::showInfo(bool show) {

	mId->setVisible(show);

	if (mPolyWidget->polygon().empty())
		mPolyWidget->hide();
	else
		mPolyWidget->setVisible(show);

	if (mBaselineWidget->polygon().empty())
		mBaselineWidget->hide();
	else
		mBaselineWidget->setVisible(show);


	//if (show) {
	//	mText->setVisible(!mText->text().isEmpty());
	//	mCustom->setVisible(!mCustom->text().isEmpty());
	//	mChildCombo->setVisible(mChildCombo->count() > 1);
	//}
	//else {
		mText->setVisible(show);
		mCustom->setVisible(show);
		mChildCombo->setVisible(show);
	//}
}

void RegionWidget::clear() {

	mId->setText("");

	mRegionCombo->clear();
	mChildCombo->clear();

	mPolyWidget->setPolygon(QPolygon());
	mBaselineWidget->setPolygon(QPolygon());

	mCustom->setText("");
	mText->setText("");

	showInfo(false);
}

void RegionWidget::paintEvent(QPaintEvent* event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// PageDock --------------------------------------------------------------------
PageDock::PageDock(PageData* pageData, const QString& title, QWidget* parent) : nmc::DkDockWidget(title, parent) {

	setObjectName("PageDock");
	mPageData = pageData;

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

PageDock::~PageDock() {

	qDebug() << "destroying PAGE dock";
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

	QWidget* configDummy = new QWidget(this);
	QVBoxLayout* configLayout = new QVBoxLayout(configDummy);
	configLayout->setContentsMargins(0, 0, 0, 0);
	configLayout->setSpacing(0);
	configLayout->addWidget(configCombo);
	configLayout->addWidget(mConfigWidget);

	// dir drop widget
	DirLabel* dirWidget = new DirLabel(this);

	// xml drop widget
	XmlLabel* xmlWidget = new XmlLabel(this);
	xmlWidget->setPage(mPageData->page());

	// region selection widget
	mRegionWidget = new RegionWidget(this);

	// create our widget
	QWidget* base = new QWidget(this);
	QVBoxLayout* baseLayout = new QVBoxLayout(base);
	baseLayout->setAlignment(Qt::AlignTop);
	baseLayout->addWidget(mCbDraw);
	baseLayout->addWidget(profileWidget);
	baseLayout->addWidget(configDummy);
	baseLayout->addWidget(dirWidget);
	baseLayout->addWidget(xmlWidget);
	baseLayout->addWidget(mRegionWidget);

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
	connect(dirWidget, SIGNAL(xmlPathChanged(const QString&)), mPageData, SLOT(setXmlPath(const QString&)));
	connect(dirWidget, SIGNAL(xmlPathChanged(const QString&)), this, SIGNAL(updateSignal()));
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

void PageDock::closeEvent(QCloseEvent * ev) {
	emit closeSignal();
}

void PageDock::on_drawCheckbox_toggled(bool toggled) const {

	emit updateSignal();
}

bool PageDock::drawRegions() const {
	return mCbDraw->isChecked();
}

RegionWidget * PageDock::regionWidget() {
	return mRegionWidget;
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

void PageDock::on_infoWidget_updated() {

	//// correctly udpate even if we changed the current region already
	//rdf::RegionTypeConfig c = mConfigWidget->config();
	//mConfig[c.type()] = c;
	emit updateSignal();
	mRegionWidget->setRegionTypes(mPageData->config());
}

// that's weird: we cannot connect two objects with the same names
// so I just changed the signal name...
void PageDock::on_infoWidget_updateSignal() {

	emit updateSignal();
}

}
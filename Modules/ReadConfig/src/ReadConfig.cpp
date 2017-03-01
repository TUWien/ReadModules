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

#include "ReadConfig.h"

// nomacs
#include "DkImageStorage.h"

#include "Settings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#include <QUuid>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
ReadConfig::ReadConfig(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	for (QString& rid : runIds)
		rid = QUuid::createUuid().toString();

	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_settings] = tr("Settings");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_settings] = tr("READ Settings");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
ReadConfig::~ReadConfig() {

	qDebug() << "destroying config plugin...";
}

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage ReadConfig::image() const {

	return QImage(":/ReadConfig/img/read.png");
}
QString ReadConfig::name() const {
	return "READ Config";
}

QList<QAction*> ReadConfig::createActions(QWidget* parent) {

	return QList<QAction*>();

	if (mActions.empty()) {

		for (int idx = 0; idx < id_end; idx++) {
			QAction* ca = new QAction(mMenuNames[idx], parent);
			ca->setObjectName(mMenuNames[idx]);
			ca->setStatusTip(mMenuStatusTips[idx]);
			ca->setData(mRunIDs[idx]);	// runID needed for calling function runPlugin()
			mActions.append(ca);
		}
	}

	return mActions;
}

QList<QAction*> ReadConfig::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> ReadConfig::runPlugin(
	const QString &runID,
	QSharedPointer<nmc::DkImageContainer> imgC,
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& info) const {

	SettingsDialog* sd = new SettingsDialog(tr("READ Settings"), nmc::DkUtils::getMainWindow());
	sd->setMinimumSize(480, 600);
	sd->exec();
	qDebug() << "showing settings...";

	// wrong runID? - do nothing
	return imgC;
}

void ReadConfig::preLoadPlugin() const {
}

void ReadConfig::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo>>& batchInfo) const {
}

// DkTestInfo --------------------------------------------------------------------
DkTestInfo::DkTestInfo(const QString& id, const QString & filePath) : nmc::DkBatchInfo(id, filePath) {
}

void DkTestInfo::setProperty(const QString & p) {
	mProp = p;
}

QString DkTestInfo::property() const {
	return mProp;
}

// DkSettingsDialog --------------------------------------------------------------------
SettingsDialog::SettingsDialog(const QString& title, QWidget* parent) : QDialog(parent) {
	setWindowTitle(title);
	createLayout();
	nmc::DkSettingsGroup g = nmc::DkSettingsGroup::fromSettings(rdf::Config::instance().settings());
	mSettingsWidget->addSettingsGroup(g);
}

void SettingsDialog::changeSetting(const QString& key, const QVariant& value, const QStringList& groups) {

	QSettings& s = rdf::Config::instance().settings();
	nmc::DkSettingsWidget::changeSetting(s, key, value, groups);
}

void SettingsDialog::removeSetting(const QString & key, const QStringList & groups) {

	QSettings& s = rdf::Config::instance().settings();
	nmc::DkSettingsWidget::removeSetting(s, key, groups);
}

void SettingsDialog::createLayout() {

	mSettingsWidget = new nmc::DkSettingsWidget(this);

	QLabel* titleLabel = new QLabel(rdf::Config::instance().settingsFilePath(), this);
	titleLabel->setStyleSheet("QLabel {color: #666; font-style: italic;}");
	titleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mSettingsWidget);
	layout->addWidget(titleLabel);
	layout->addWidget(buttons);

	connect(mSettingsWidget, SIGNAL(changeSettingSignal(const QString&, const QVariant&, const QStringList&)), 
		this, SLOT(changeSetting(const QString&, const QVariant&, const QStringList&)));
	connect(mSettingsWidget, SIGNAL(removeSettingSignal(const QString&, const QStringList&)), 
		this, SLOT(removeSetting(const QString&, const QStringList&)));

}

};


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

#include "Forms.h"

// nomacs
#include "DkImageStorage.h"
#include "DkSettings.h"

#include "Settings.h"
#include "Image.h"
#include "FormAnalysis.h"
#include "LineTrace.h"
#include "Algorithms.h"
#include "PageParser.h"
#include "Elements.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
FormsAnalysis::FormsAnalysis(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);
	
	runIds[id_train] = "871fe4d1de79497388292ef534ff25d8";
	runIds[id_show] = "d3359f4a4ec943e3abd4860e68152769";
	runIds[id_classify] = "91fd4f094fc34085a7a8715142f2292d";
	runIds[id_classifyxml] = "5f34992f7494400d9beb298e0800a571";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_train] = tr("Train forms");
	menuNames[id_show] = tr("Shows the line information");
	menuNames[id_classify] = tr("Classify");
	menuNames[id_classifyxml] = tr("Classify based on xml separators");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_train] = tr("Train forms");
	statusTips[id_show] = tr("id_show");
	statusTips[id_classify] = tr("Classify");
	statusTips[id_classifyxml] = tr("Classify based on xml separators");
	mMenuStatusTips = statusTips.toList();

	loadSettings(nmc::DkSettingsManager::instance().qSettings());
}
/**
*	Destructor
**/
FormsAnalysis::~FormsAnalysis() {

	qDebug() << "destroying binarization plugin...";

}


/**
* Returns unique ID for the generated dll
**/
QString FormsAnalysis::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage FormsAnalysis::image() const {

	return QImage(":/ReadConfig/img/read.png");
};

QList<QAction*> FormsAnalysis::createActions(QWidget* parent) {

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



QList<QAction*> FormsAnalysis::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> FormsAnalysis::runPlugin(
	const QString &runID,
	QSharedPointer<nmc::DkImageContainer> imgC,
	const nmc::DkSaveInfo& saveInfo,
	QSharedPointer<nmc::DkBatchInfo>& info) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_train] || runID == mRunIDs[id_classify]) {

		QImage img = imgC->image();
		//imgC->setImage(img.mirrored(), "Mirrored");

		QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));
		
		cv::Mat imgTempl = rdf::Image::qImage2Mat(img);
		cv::Mat imgTemplG;
		if (imgTempl.channels() != 1) cv::cvtColor(imgTempl, imgTemplG, CV_RGB2GRAY);
		//cv::Mat maskTempl = rdf::Algorithms::estimateMask(imgTemplG);
		rdf::FormFeatures formTempl(imgTemplG);
		
		if (!formTempl.compute()) {
			qWarning() << "could not compute form template " << imgC->filePath();
		}
		
		
		testInfo->setFormName(imgC->filePath());
		testInfo->setFormSize(QSize(imgTemplG.cols, imgTemplG.rows));
		testInfo->setLines(formTempl.horLines(), formTempl.verLines());
				
		qDebug() << "template calculated...";

		info = testInfo;
	}
	else if(runID == mRunIDs[id_show]) {

		//test LSD here
		QImage img = imgC->image();
		cv::Mat imgIn = rdf::Image::qImage2Mat(img);
		cv::Mat imgInG;
		if (imgIn.channels() != 1) cv::cvtColor(imgIn, imgInG, CV_RGB2GRAY);

		rdf::ReadLSD lsd(imgInG);
		//ReadLSD(inputG, mask);

		lsd.compute();
		QVector<rdf::LineSegment> detLines = lsd.lines();

		cv::Mat outImg = lsd.magImg();
		outImg = lsd.radImg();
		//cv::Mat outImg = imgIn.clone();
		

		if (outImg.channels() == 1) {
			outImg.convertTo(outImg, CV_32F);
			cv::normalize(outImg, outImg, 1, 0, cv::NORM_MINMAX);
			rdf::Image::imageInfo(outImg, "outimg");
			cv::cvtColor(outImg, outImg, CV_GRAY2BGRA);
		}
				
				
		QImage result = rdf::Image::mat2QImage(outImg);
		
		QPainter myPainter(&result);
		myPainter.setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::RoundCap));
		//myPainter.drawLine(QPoint(0,0), QPoint(500,500));

		qDebug() << "LSD lines detected: " << detLines.size();;

		for (int i = 0; i < detLines.size(); i++) {

			rdf::LineSegment lineTmp = detLines[i];
			myPainter.drawLine(lineTmp.line().p1().toQPoint(), lineTmp.line().p2().toQPoint());
			//qDebug() << "Point 1: " << lineTmp.line().p1().toQPoint() << " Point 2: " << lineTmp.line().p2().toQPoint();
		}
		myPainter.end();


		qDebug() << "LSD calculated...";
		imgC->setImage(result, "LSD Image");

		////only test version
		////not yet implemented
		//QImage img = imgC->image();
		//img = img.convertToFormat(QImage::Format_Grayscale8);
		//imgC->setImage(img, "Grayscale");

		//QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));
		//testInfo->setFormName(imgC->filePath());
		//qDebug() << "id_show... (not implemented, shows only grayscale img)";

		//info = testInfo;
	}
	else if (runID == mRunIDs[id_classifyxml]) {


		qDebug() << "classify based on xml separators...";
		
		QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));

		QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());
		//QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());

		rdf::PageXmlParser parser;
		parser.read(loadXmlPath);
		auto pe = parser.page();

		//read xml separators and store them to testinfo
		QVector<rdf::Line> hLines;
		QVector<rdf::Line> vLines;

		QVector<QSharedPointer<rdf::Region>> test = rdf::Region::allRegions(pe->rootRegion());// pe->rootRegion()->children();
		for (auto i : test) {
			if (i->type() == i->type_separator) {
				rdf::SeparatorRegion* tSep = dynamic_cast<rdf::SeparatorRegion*>(i.data());
				if (tSep) {
					if (tSep->line().isHorizontal(5.0))
						hLines.push_back(tSep->line());

					if (tSep->line().isVertical(5.0))
						vLines.push_back(tSep->line());
				}
					
			}
		}
		
		testInfo->setFormName(imgC->filePath());
		testInfo->setFormSize(pe->imageSize());
		testInfo->setLines(hLines, vLines);

		qDebug() << "separators read from xml...";


		info = testInfo;
	}

	// wrong runID? - do nothing
	//imgC->setImage(QImage(), "empty");
	return imgC;
}

void FormsAnalysis::preLoadPlugin() const {

	qDebug() << "[PRE LOADING] form classification/training";

}

void FormsAnalysis::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo>>& batchInfo) const {
	int runIdx = mRunIDs.indexOf(batchInfo.first()->id());

	if (runIdx == id_train) {

		for (auto bi : batchInfo) {
			qDebug() << bi->filePath() << "computed...";
			FormsInfo* tInfo = dynamic_cast<FormsInfo*>(bi.data());

			////TODO: how to save vector formTemplates for classification
			//QVector<QSharedPointer<FormsInfo> > formTemplates;
			//formTemplates.push_back(QSharedPointer<FormsInfo>(tInfo));

			if (tInfo)
				qDebug() << "form template: " << tInfo->iDForm();

			QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(tInfo->filePath());
			QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(tInfo->filePath());

			rdf::PageXmlParser parser;
			parser.read(loadXmlPath);
			auto pe = parser.page();

			pe->setImageSize(tInfo->formSize());
			pe->setImageFileName(tInfo->formName());
			//pe->setCreator(QString("CVL"));
			//horizontal lines
			QVector<rdf::Line> tmp = tInfo->hLines();
			for (int i = 0; i < tmp.size(); i++) {

				QSharedPointer<rdf::SeparatorRegion> pSepR(new rdf::SeparatorRegion());
				pSepR->setLine(tmp[i].line());

				pe->rootRegion()->addUniqueChild(pSepR);
			}
			//vertical lines
			tmp = tInfo->vLines();
			for (int i = 0; i < tmp.size(); i++) {

				QSharedPointer<rdf::SeparatorRegion> pSepR(new rdf::SeparatorRegion());
				pSepR->setLine(tmp[i].line());

				pe->rootRegion()->addUniqueChild(pSepR);
			}
			//save pageXml
			parser.write(saveXmlPath, pe);

		}
	}

	if (runIdx == id_classify || runIdx == id_classifyxml) {
		qDebug() << "[POST LOADING] classify";

		//Read training xml data
		rdf::FormFeatures templateLoader;
		templateLoader.loadTemplateDatabase(mLineTemplPath);
		QVector<rdf::FormFeatures> templates;
		templates = templateLoader.templatesDb();
		double minErr = std::numeric_limits<double>::max();

		for (auto bi : batchInfo) {
			qDebug() << bi->filePath() << "computed...";
			FormsInfo* tInfo = dynamic_cast<FormsInfo*>(bi.data());
			bool match = false;
			minErr = std::numeric_limits<double>::max();

			if (tInfo) {
				qDebug() << "testing form: " << tInfo->formName();
				qDebug() << "-------------";
			}

			for (int jT = 0; jT < templates.size(); jT++) {
				qDebug() << "match against template: " << templates[jT].formName();
				rdf::FormFeatures currentForm;
				currentForm.setHorLines(tInfo->hLines());
				currentForm.setVerLines(tInfo->vLines());
				currentForm.setSize(cv::Size(tInfo->formSize().width(), tInfo->formSize().height()));
				currentForm.setFormName(tInfo->formName());
				if (templates[jT].compareWithTemplate(currentForm)) {
					if (templates[jT].error() < minErr) {
						minErr = templates[jT].error();
						qDebug() << "match with: " << templates[jT].formName() << " error is " << minErr;
						tInfo->setMatchName(templates[jT].formName());
						match = true;
					}
				}
				
			}
			if (!match)
				qDebug() << "no match found for " << tInfo->formName();

		}

		//QString outf;
		QString outf = "D:\\tmp\\evalForms.txt";
		//QString outf = "F:\\flo\\evalSkew.txt";
		//outf = mFilePath;
		QFile file(outf);
		if (file.open(QIODevice::WriteOnly)) {

			for (auto bi : batchInfo) {
				FormsInfo* tInfo = dynamic_cast<FormsInfo*>(bi.data());
				QString tmpStr = tInfo->filePath();
				tmpStr += " matches with: ";
				tmpStr += tInfo->matchName();
				tmpStr += "\n";
				QTextStream out(&file);
				out << tmpStr;
			}

			file.close();
		}

	}
	else
		qDebug() << "[POST LOADING] train/add training";

	//not tested....
	saveSettings(rdf::Config::instance().settings());
}

void FormsAnalysis::loadSettings(QSettings & settings) {
	settings.beginGroup("FormAnalysis");
	mLineTemplPath = settings.value("lineTemplPath", mLineTemplPath).toString();
	settings.endGroup();
}

void FormsAnalysis::saveSettings(QSettings & settings) const {
	settings.beginGroup("FormAnalysis");
	settings.setValue("lineTemplPath", mLineTemplPath);
	settings.endGroup();
}

// DkTestInfo --------------------------------------------------------------------
FormsInfo::FormsInfo(const QString& id, const QString & filePath) : nmc::DkBatchInfo(id, filePath) {
}

void FormsInfo::setFormName(const QString & p) {
	mProp = p;
}

QString FormsInfo::formName() const {
	return mProp;
}

void FormsInfo::setMatchName(const QString & p){
	mMatchName = p;
}

QString FormsInfo::matchName() const{
	return mMatchName;
}

void FormsInfo::setFormSize(const QSize & s){
	mS = s;
}

QSize FormsInfo::formSize() const{
	return mS;
}

void FormsInfo::setTemplId(int id) {
	mIdForm = id;
}

int FormsInfo::iDForm() const {
	return mIdForm;
}

void FormsInfo::setLines(QVector<rdf::Line> hL, QVector<rdf::Line> vL) {
	mHorLines = hL;
	mVerLines = vL;
}

QVector<rdf::Line> FormsInfo::hLines() {
	return mHorLines;
}

QVector<rdf::Line> FormsInfo::vLines() {
	return mVerLines;
}

};


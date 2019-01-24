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
#include <QUuid>
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

	for (QString& rid : runIds)
		rid = QUuid::createUuid().toString();

	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_train] = tr("Train forms - not implemented");
	menuNames[id_show] = tr("Shows form information based on XML");
	menuNames[id_apply] = tr("Just apply template to document (overlay)");
	menuNames[id_match] = tr("Apply template (Match)");
	menuNames[id_evaluate] = tr("Apply template and evaluate");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_train] = tr("Train forms");
	statusTips[id_show] = tr("Show form (Page XML)");
	statusTips[id_apply] = tr("Just apply template to document (overlay)");
	statusTips[id_match] = tr("Apply template (Match)");
	statusTips[id_evaluate] = tr("Apply template and evaluate");
	mMenuStatusTips = statusTips.toList();

	
	//old settings version
	//loadSettings(rdf::Config::instance().settings());
	//saveSettings(rdf::Config::instance().settings());
	//mFormConfig.loadSettings();

	rdf::DefaultSettings s;
	s.beginGroup(name());
	mFormConfig.saveDefaultSettings(s);
	s.endGroup();
}
/**
*	Destructor
**/
FormsAnalysis::~FormsAnalysis() {
	//qDebug() << "destroying forms plugin...";
}

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage FormsAnalysis::image() const {

	return QImage(":/ReadConfig/img/read.png");
}
QString FormsAnalysis::name() const {
	return "FormAnalysis";
}

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

	if(runID == mRunIDs[id_train]) {

		QImage img = imgC->image();

		QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));
		
		testInfo->setFormName(imgC->filePath());
		testInfo->setFormSize(img.size());
			
		//qDebug() << "template calculated...";
		qDebug() << "nothing implemented here...";
		qWarning() << "nothing implemented here...";

		info = testInfo;
	}
	else if(runID == mRunIDs[id_show]) {

		QImage img = imgC->image();
		cv::Mat imgIn = rdf::Image::qImage2Mat(img);
		cv::Mat imgInG;
		if (imgIn.channels() != 1) cv::cvtColor(imgIn, imgInG, CV_RGB2GRAY);

		qDebug() << imgC->filePath() << "reading XML...";
		
		QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());
		//QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());

		rdf::PageXmlParser parser;
		parser.read(loadXmlPath);
		auto pe = parser.page();

		//read xml separators and store them to testinfo
		QVector<rdf::Line> hLines;
		QVector<rdf::Line> vLines;

		QVector<QSharedPointer<rdf::Region>> test = pe->rootRegion()->allRegions();
		//QVector<rdf::TableCell> cells;
		QVector<QSharedPointer<rdf::TableCell>> cells;


		for (auto i : test) {
			if (i->type() == i->type_table_cell) {
				//rdf::TableCell* tCell = dynamic_cast<rdf::TableCell*>(i.data());
				QSharedPointer<rdf::TableCell> tCell = i.dynamicCast<rdf::TableCell>();
				cells.push_back(tCell);

				//check if tCell has a Textline as child, if yes, mark as table header;
				if (!tCell->children().empty()) {
					QVector<QSharedPointer<rdf::Region>> childs = tCell->children();
					for (auto child : childs) {
						if (child->type() == child->type_text_line) {
							tCell->setHeader(true);
							//qDebug() << imgC->filePath() << "detected header...";
							qDebug() << "detected header...";
							break;
						}
					}
				}


				if (tCell) {
					
					if (tCell->topBorderVisible()) {
						hLines.push_back(tCell->topBorder());
					}
					if (tCell->bottomBorderVisible()) {
						hLines.push_back(tCell->bottomBorder());
					}
					if (tCell->leftBorderVisible()) {
						vLines.push_back(tCell->leftBorder());
					}
					if (tCell->rightBorderVisible()) {
						vLines.push_back(tCell->rightBorder());
					}
				}

			}
		}

		std::sort(cells.begin(), cells.end());


		QImage result = rdf::Image::mat2QImage(imgIn);

		QPainter myPainter(&result);
		myPainter.setPen(QPen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap));
		//myPainter.drawLine(QPoint(0,0), QPoint(500,500));

		for (int i = 0; i < hLines.size(); i++) {
			rdf::Line lineTmp = hLines[i];
			myPainter.drawLine(lineTmp.p1().toQPoint(), lineTmp.p2().toQPoint());
			//qDebug() << "Point 1: " << lineTmp.line().p1().toQPoint() << " Point 2: " << lineTmp.line().p2().toQPoint();
		}
		myPainter.setPen(QPen(Qt::blue, 3, Qt::SolidLine, Qt::RoundCap));
		for (int i = 0; i < vLines.size(); i++) {
			rdf::Line lineTmp = vLines[i];
			myPainter.drawLine(lineTmp.p1().toQPoint(), lineTmp.p2().toQPoint());
			//qDebug() << "Point 1: " << lineTmp.line().p1().toQPoint() << " Point 2: " << lineTmp.line().p2().toQPoint();
		}


		myPainter.end();
		
		qDebug() << "Drawing form...";
		imgC->setImage(result, "Form Image");

	}
	else if (runID == mRunIDs[id_apply]) {

		//use for debugging - apply template
		QImage img = imgC->image();
		QImage result;
		//imgC->setImage(img.mirrored(), "Mirrored");

		QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));

		cv::Mat imgForm = rdf::Image::qImage2Mat(img);

		cv::Mat imgFormG = imgForm;
		if (imgForm.channels() != 1)
			cv::cvtColor(imgForm, imgFormG, CV_RGB2GRAY);
		else {
			cv::cvtColor(imgForm, imgForm, CV_GRAY2RGB);
		}
		//cv::Mat maskTempl = rdf::Algorithms::estimateMask(imgTemplG);
		rdf::FormFeatures formF(imgFormG);
		formF.setFormName(imgC->fileName());
		formF.setSize(imgFormG.size());

		//formF.setTemplateName(mLineTemplPath);
		QString templateN = mFormConfig.templDatabase();
		//contains full path to template xml, or csv specifying the template xml
		if (!formF.setTemplateName(templateN)) {
			qWarning() << "template not found - aborting";
			info = testInfo;
			return imgC;
		}

		QSharedPointer<rdf::FormFeaturesConfig> tmpConfig(new rdf::FormFeaturesConfig());
		(*tmpConfig) = mFormConfig;
		formF.setConfig(tmpConfig);

		//rdf::FormFeatures formTemplate;
		QSharedPointer<rdf::FormFeatures> formTemplate(new rdf::FormFeatures());
		//no scaling -> set calcScaling to false
		if (!formF.readTemplate(formTemplate, false)) {
			qWarning() << "not template set - aborting...";
			qInfo() << "please provide a template Plugins > Read Config > Form Analysis > lineTemplPath";
			info = testInfo;
			return imgC;
		}



		if (!formF.applyTemplate()) {
			qWarning() << "could not apply template " << imgC->filePath();
			qInfo() << "could not apply template";
			return imgC;
		}

		cv::Mat drawImg = imgForm.clone();
		cv::cvtColor(drawImg, drawImg, CV_RGBA2BGR);
		cv::Mat resultImg;// = imgForm;

		resultImg = drawImg.clone();

		if (!resultImg.empty()) {

			resultImg = formF.drawMatchedForm(drawImg, 20);
			cv::cvtColor(resultImg, resultImg, CV_BGR2RGBA);
			result = rdf::Image::mat2QImage(resultImg);
			imgC->setImage(result, "Matched form");
		}

		//test - save output to xml...

		QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());
		QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());

		rdf::PageXmlParser parser;
		bool newXML = parser.read(loadXmlPath);
		auto pe = parser.page();

		if (!newXML) {
			//xml is newly created
			pe->setImageFileName(imgC->fileName());
			pe->setImageSize(img.size());
			pe->setCreator("CVL");
			pe->setDateCreated(QDateTime::currentDateTime());
		}

		//clear children first...
		QSharedPointer<rdf::TableRegion> t = formF.tableRegion();
		pe->rootRegion()->addUniqueChild(t);

		formF.setSeparators(pe->rootRegion());

		//save pageXml
		parser.write(saveXmlPath, pe);

		//qDebug() << "Align form...";
		//imgC->setImage(result, "Form Image");
		info = testInfo;


	}
	else if (runID == mRunIDs[id_match]) {

		//use for debugging - apply template
		QImage img = imgC->image();
		QImage result;
		//imgC->setImage(img.mirrored(), "Mirrored");

		QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));

		//QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());
		//QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());
		//rdf::PageXmlParser parser;
		//parser.read(loadXmlPath);
		//auto pe = parser.page();

		cv::Mat imgForm = rdf::Image::qImage2Mat(img);

		cv::Mat imgFormG = imgForm;
		if (imgForm.channels() != 1) 
			cv::cvtColor(imgForm, imgFormG, CV_RGB2GRAY);
		else {
			cv::cvtColor(imgForm, imgForm, CV_GRAY2RGB);
		}
		//cv::Mat maskTempl = rdf::Algorithms::estimateMask(imgTemplG);
		rdf::FormFeatures formF(imgFormG);
		formF.setFormName(imgC->fileName());
		formF.setSize(imgFormG.size());
		
		//formF.setTemplateName(mLineTemplPath);
		QString templateN = mFormConfig.templDatabase();
		//contains full path to template xml, or csv specifying the template xml
		if (!formF.setTemplateName(templateN)) {
			qWarning() << "template not found - aborting";
			info = testInfo;
			return imgC;
		}


		//check if is meant that way!!!
		//mFormConfig.distThreshold();
		QSharedPointer<rdf::FormFeaturesConfig> tmpConfig(new rdf::FormFeaturesConfig());
		(*tmpConfig) = mFormConfig;
		formF.setConfig(tmpConfig);

		//rdf::FormFeatures formTemplate;
		QSharedPointer<rdf::FormFeatures> formTemplate(new rdf::FormFeatures());
		if (!formF.readTemplate(formTemplate)) {
			qWarning() << "not template set - aborting...";
			qInfo() << "please provide a template Plugins > Read Config > Form Analysis > lineTemplPath";
			info = testInfo;
			return imgC;
		}

		
		if (!formF.compute()) {
			qWarning() << "could not compute form template " << imgC->filePath();
			qInfo() << "could not compute form template";
			return imgC;
		}

		qDebug() << "Compute rough alignment...";
		formF.estimateRoughAlignment();

		cv::Mat drawImg = imgForm.clone();
		cv::cvtColor(drawImg, drawImg, CV_RGBA2BGR);
		cv::Mat resultImg;// = imgForm;

		resultImg = formF.drawAlignment(drawImg);
		
		if (!resultImg.empty()) {
			cv::cvtColor(resultImg, resultImg, CV_BGR2RGBA);
			result = rdf::Image::mat2QImage(resultImg);
			imgC->setImage(result, "Rough Alignment...");
			//rdf::Image::save(resultImg, "D:\\tmp\\alignedImg.png");

			qDebug() << "Match template...";
			formF.matchTemplate();

			resultImg = formF.drawLinesNotUsedForm(drawImg);
			cv::cvtColor(resultImg, resultImg, CV_BGR2RGBA);
			result = rdf::Image::mat2QImage(resultImg);
			imgC->setImage(result, "lines not used");

			resultImg = formF.drawLines(drawImg);
			cv::cvtColor(resultImg, resultImg, CV_BGR2RGBA);
			result = rdf::Image::mat2QImage(resultImg);
			imgC->setImage(result, "all detected lines");

			//resultImg = formF.drawMaxCliqueNeighbours(7, rdf::AssociationGraphNode::LinePosition::pos_right, 2, drawImg);
			//cv::cvtColor(resultImg, resultImg, CV_BGR2RGBA);
			//result = rdf::Image::mat2QImage(resultImg);
			//imgC->setImage(result, "draw neighbours");


			//resultImg = formF.drawMaxClique(drawImg,10, 2);
			//cv::cvtColor(resultImg, resultImg, CV_BGR2RGBA);
			//result = rdf::Image::mat2QImage(resultImg); 
			//imgC->setImage(result, "maxClique 2");

			//resultImg = formF.drawMaxClique(drawImg, 10, 1);
			//cv::cvtColor(resultImg, resultImg, CV_BGR2RGBA);
			//result = rdf::Image::mat2QImage(resultImg);
			//imgC->setImage(result, "maxClique 1");

			resultImg = formF.drawMaxClique(drawImg);
			cv::cvtColor(resultImg, resultImg, CV_BGR2RGBA);
			result = rdf::Image::mat2QImage(resultImg);
			imgC->setImage(result, "maxClique 0");

			resultImg = formF.drawMatchedForm(drawImg, 20);
			cv::cvtColor(resultImg, resultImg, CV_BGR2RGBA);
			result = rdf::Image::mat2QImage(resultImg);
			imgC->setImage(result, "Matched form");
		}
				
		//cv::Mat resultImg = imgForm;
		//if (resultImg.channels() != 3)
		//	cv::cvtColor(resultImg, resultImg, CV_GRAY2RGB);
		
		//test - save output to xml...

		QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());
		QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());

		rdf::PageXmlParser parser;
		bool newXML = parser.read(loadXmlPath);
		auto pe = parser.page();

		if (!newXML) {
			//xml is newly created
			pe->setImageFileName(imgC->fileName());
			pe->setImageSize(img.size());
			pe->setCreator("CVL");
			pe->setDateCreated(QDateTime::currentDateTime());
		}

		QSharedPointer<rdf::TableRegion> t = formF.tableRegion();
		pe->rootRegion()->addUniqueChild(t);

		formF.setSeparators(pe->rootRegion());

		//save pageXml
		parser.write(saveXmlPath, pe);


		//// ----------- use this one for batch processing-----------------------------------------
		////set batchinfo for further processing
		//testInfo->setFormName(imgC->filePath());
		//testInfo->setFormSize(img.size());
		//testInfo->setLines(formF.horLines(), formF.verLines());
		//qDebug() << "Form img calculated...";
		//info = testInfo;
		//// --------------------------------------------------------------------------------------

		//result = rdf::Image::mat2QImage(resultImg);

		//qDebug() << "Align form...";
		//imgC->setImage(result, "Form Image");
		info = testInfo;
	}
	else if (runID == mRunIDs[id_evaluate]) {

		//calculate match the same way as for table
		//no debug images are created

		QImage img = imgC->image();
		QImage result;

		//testInfo contains debug information
		QSharedPointer<FormsInfo> testInfo(new FormsInfo(runID, imgC->filePath()));
		info = testInfo;

		cv::Mat imgForm = rdf::Image::qImage2Mat(img);
		cv::Mat imgFormG = imgForm;
		if (imgForm.channels() != 1)
			cv::cvtColor(imgForm, imgFormG, CV_RGB2GRAY);

		rdf::FormFeatures formF(imgFormG);
		formF.setFormName(imgC->fileName());
		formF.setSize(imgFormG.size());

		if (!formF.setTemplateName(mFormConfig.templDatabase())) {
			qWarning() << "template not found - aborting";
			info = testInfo;
			return imgC;
		}

		QSharedPointer<rdf::FormFeaturesConfig> tmpConfig(new rdf::FormFeaturesConfig());
		(*tmpConfig) = mFormConfig;
		formF.setConfig(tmpConfig);

		//rdf::FormFeatures formTemplate;
		QSharedPointer<rdf::FormFeatures> formTemplate(new rdf::FormFeatures());
		if (!formF.readTemplate(formTemplate)) {
			qWarning() << "not template set - aborting";
			qInfo() << "please provide a template Plugins > Read Config > Form Analysis > lineTemplPath";
			
			return imgC;
		}


		if (!formF.compute()) {
			qWarning() << "could not compute form template " << imgC->filePath();
			qInfo() << "could not compute form template";
			
			return imgC;
		}

		qDebug() << "Compute rough alignment...";
		if (!formF.estimateRoughAlignment()) {
			qWarning() << "could not compute rough alignment " << imgC->filePath();
			qInfo() << "could not compute rough alignment ";
			
			return imgC;
		}

		cv::Mat drawImg = imgForm.clone();
		cv::cvtColor(drawImg, drawImg, CV_RGBA2BGR);

		qDebug() << "Match template...";
		formF.matchTemplate();

		drawImg = formF.drawMatchedForm(drawImg);
		cv::cvtColor(drawImg, drawImg, CV_BGR2RGBA);
		QImage finalImg = rdf::Image::mat2QImage(drawImg);
		imgC->setImage(finalImg, "matched table");

		rdf::FormEvaluation formEval;
		formEval.setSize(formF.sizeImg());
		QString templateXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());
		if (!formEval.setTemplate(templateXmlPath)) {
			qWarning() << "could not find template for evaluation " << imgC->filePath();
			qInfo() << "could not find template for evaluation";

			return imgC;
		}

		formEval.setTable(formF.tableRegion());


		formEval.computeEvalTableRegion();
		formEval.computeEvalCells();
				
		double tableJI = formEval.tableJaccard();
		double tableM = formEval.tableMatch();

		QVector<double> cellJI = formEval.cellJaccards();
		double meanCellJI = formEval.meanCellJaccard();

		QVector<double> cellM = formEval.cellMatches();
		double meanCellM = formEval.meanCellMatch();

		double missedCells = formEval.missedCells();
		double underSeg = formEval.underSegmented();
		QVector<double> underSegCells = formEval.underSegmentedC();

		// ----------- use this one for batch processing-----------------------------------------
		//set batchinfo for further processing
		testInfo->setFormName(imgC->filePath());
		testInfo->setFormSize(img.size());
		testInfo->setLines(formF.horLines(), formF.verLines());
		
		testInfo->setJaccardTable(tableJI);
		testInfo->setMatchTable(tableM);

		testInfo->setJaccardCell(cellJI);
		testInfo->setJaccardMeanCell(meanCellJI);
		testInfo->setCellMatch(cellM);
		testInfo->setmatchMeanCell(meanCellM);

		testInfo->setUnderSegmented(underSeg);
		testInfo->setUnderSegmentedC(underSegCells);
		testInfo->setMissedCells(missedCells);

		qDebug() << "table match calculated...";
		info = testInfo;

		
		//test - save output to different xml...
		//in current xml related to the file is the GT
		QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(imgC->filePath());
		QFileInfo fi(loadXmlPath);
		QString finalName = fi.baseName() + "_matched." + fi.suffix();
		QFileInfo finalXmlPath;
		finalXmlPath.setFile(fi.absolutePath() , finalName);
		
		QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(finalXmlPath.absoluteFilePath());
		
		rdf::PageXmlParser parser;
		bool newXML = parser.read(loadXmlPath);
		auto pe = parser.page();

		if (!newXML) {
			//xml is newly created
			pe->setImageFileName(imgC->fileName());
			pe->setImageSize(img.size());
			pe->setCreator("CVL");
			pe->setDateCreated(QDateTime::currentDateTime());
		}

		QSharedPointer<rdf::TableRegion> t = formF.tableRegion();
		pe->rootRegion()->addUniqueChild(t);

		formF.setSeparators(pe->rootRegion());

		//save pageXml
		parser.write(saveXmlPath, pe);
	}

	// wrong runID? - do nothing
	//imgC->setImage(QImage(), "empty");

	return imgC;
}

QString FormsAnalysis::settingsFilePath() const {

	return rdf::Config::instance().settingsFilePath();
}

void FormsAnalysis::preLoadPlugin() const {

	qDebug() << "[PRE LOADING] form classification/training";

}

void FormsAnalysis::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo>>& batchInfo) const {
	int runIdx = mRunIDs.indexOf(batchInfo.first()->id());



	if (runIdx == id_evaluate) {
		//save final results to yml file
		QDir evalDir(mFormConfig.evalPath());
		QString filename = "eval.yml";
		QFileInfo evalFile(evalDir, filename);
		
		std::string saveFile = evalFile.absoluteFilePath().toStdString();
		cv::FileStorage fs(saveFile, cv::FileStorage::WRITE);

		int nForms = batchInfo.size();
		int nCnt = 0;
		double meanTableJI = 0;
		double meanTableMatch = 0;
		double meanCellJI = 0;
		double meanCellMatch = 0;
		double underSeg = 0;
		double missedCells = 0;


		for (auto bi : batchInfo) {
			qDebug() << bi->filePath() << " computed...";
			FormsInfo* tInfo = dynamic_cast<FormsInfo*>(bi.data());
			QString tmpInfo = tInfo->filePath();
			QFileInfo tmpInfoF(tInfo->filePath());
			QString baseN = tmpInfoF.baseName();

			//workaround: some filenames start with a number which causes an error...
			//so all filenamens have as prefix A-
			//maybe an error can be caused here in the future due to filestorage: wrong element name
			baseN = "A-" + baseN;
			

			//fs << tmpInfo.toStdString() << "[";
			fs << baseN.toStdString() << "{";

			//values for each table with mean cell measures
			fs << "jaccardTable" << tInfo->jaccardTable(); 
			fs << "matchTable" << tInfo->matchTable(); 
			fs << "meanJaccardCell" << tInfo->jaccardMeanCell();
			fs << "meanMatchCell" << tInfo->matchMeanCell(); 
			fs << "underSegmented" << tInfo->underSegmented(); 
			fs << "missedCells" << tInfo->missedCells(); 
			fs << "}";

			if (tInfo->jaccardTable() >= 0) {
				//otherwise table is not calculated...
				meanTableJI += tInfo->jaccardTable();
				meanTableMatch += tInfo->matchTable();
				meanCellJI += tInfo->jaccardMeanCell();
				meanCellMatch += tInfo->matchMeanCell();
				underSeg += tInfo->underSegmented();
				missedCells += tInfo->missedCells();

				nCnt++;
			}
		}

		//mean values for all calculated tables
		fs << "meanJaccardTable" << meanTableJI / (double)nCnt;
		fs << "meanMatchTable" << meanTableMatch / (double)nCnt;
		fs << "overallMeanJaccardCell" << meanCellJI / (double)nCnt;
		fs << "overallMeanMatchCell" << meanCellMatch / (double)nCnt;
		fs << "overallUnderSegmented" << underSeg / (double)nCnt;
		fs << "overallMissedCells" << missedCells / (double)nCnt;

		fs.release();

	}

	if (runIdx == id_train) {

		qDebug() << "currently not implemented ...";

		//for (auto bi : batchInfo) {
		//	qDebug() << bi->filePath() << "computed...";
		//	FormsInfo* tInfo = dynamic_cast<FormsInfo*>(bi.data());

		//	////TODO: how to save vector formTemplates for classification
		//	//QVector<QSharedPointer<FormsInfo> > formTemplates;
		//	//formTemplates.push_back(QSharedPointer<FormsInfo>(tInfo));

		//	if (tInfo)
		//		qDebug() << "form template: " << tInfo->iDForm();

		//	QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(tInfo->filePath());
		//	QString saveXmlPath = rdf::PageXmlParser::imagePathToXmlPath(tInfo->filePath());

		//	rdf::PageXmlParser parser;
		//	parser.read(loadXmlPath);
		//	auto pe = parser.page();

		//	pe->setImageSize(tInfo->formSize());
		//	pe->setImageFileName(tInfo->formName());
		//	//pe->setCreator(QString("CVL"));
		//	//horizontal lines
		//	QVector<rdf::Line> tmp = tInfo->hLines();
		//	for (int i = 0; i < tmp.size(); i++) {

		//		QSharedPointer<rdf::SeparatorRegion> pSepR(new rdf::SeparatorRegion());
		//		pSepR->setLine(tmp[i].line());

		//		pe->rootRegion()->addUniqueChild(pSepR);
		//	}
		//	//vertical lines
		//	tmp = tInfo->vLines();
		//	for (int i = 0; i < tmp.size(); i++) {

		//		QSharedPointer<rdf::SeparatorRegion> pSepR(new rdf::SeparatorRegion());
		//		pSepR->setLine(tmp[i].line());

				//pe->rootRegion()->addUniqueChild(pSepR);
		//	}
		//	//save pageXml
		//	parser.write(saveXmlPath, pe);

		//}
	}

	if (runIdx == id_apply) {
		qDebug() << "[POST LOADING] apply";

		qDebug() << "currently not implemented ...";
	
		// --------- currently not used/implemented -----------------------------------------------------------------------------
		// ----------------------------------------------------------------------------------------------------------------------
		// ---------- uncomment until the end -----------------------------------------------------------------------------------
		//////old part
		//////Read template data
		//////rdf::FormFeatures templateLoader;
		//////templateLoader.loadTemplateDatabase(mLineTemplPath);
		//////QVector<rdf::FormFeatures> templates;
		//////templates = templateLoader.templatesDb();
		//////double minErr = std::numeric_limits<double>::max();

		////Read template data
		//rdf::FormFeatures templateForm;
		//
		//
		//QSharedPointer<FormsInfo> templateInfo(new FormsInfo());
		//templateInfo->setXMLTemplate(mLineTemplPath);
		//QString loadXmlPath = rdf::PageXmlParser::imagePathToXmlPath(mLineTemplPath);
		////QString loadXmlPath = templateInfo->xmlTemplate();
		////QString loadXmlPath = mLineTemplPath;

		//rdf::PageXmlParser parser;
		//parser.read(loadXmlPath);
		//auto pe = parser.page();

		//templateInfo->setFormSize(pe->imageSize());

		////read xml separators and store them to testinfo
		//QVector<rdf::Line> hLines;
		//QVector<rdf::Line> vLines;

		//QVector<QSharedPointer<rdf::Region>> test = rdf::Region::allRegions(pe->rootRegion());// pe->rootRegion()->children();
		//																					  //QVector<rdf::TableCell> cells;
		//QVector<QSharedPointer<rdf::TableCell>> cells;
		//QSharedPointer<rdf::TableRegion> region;


		//for (auto i : test) {

		//	if (i->type() == i->type_table_region) {
		//		region = i.dynamicCast<rdf::TableRegion>();

		//	}
		//	else if (i->type() == i->type_table_cell) {
		//		//rdf::TableCell* tCell = dynamic_cast<rdf::TableCell*>(i.data());
		//		QSharedPointer<rdf::TableCell> tCell = i.dynamicCast<rdf::TableCell>();
		//		cells.push_back(tCell);

		//		//check if tCell has a Textline as child, if yes, mark as table header;
		//		if (!tCell->children().empty()) {
		//			QVector<QSharedPointer<rdf::Region>> childs = tCell->children();
		//			for (auto child : childs) {
		//				if (child->type() == child->type_text_line) {
		//					tCell->setHeader(true);
		//					//qDebug() << imgC->filePath() << "detected header...";
		//					qDebug() << "detected header...";
		//					break;
		//				}
		//			}
		//		}

		//		float thickness = 30.0;
		//		//if (tCell && tCell->header()) {
		//		if (tCell) {

		//			if (tCell->topBorderVisible()) {
		//				rdf::Line tmpL = tCell->topBorder();
		//				tmpL.setThickness(thickness);
		//				hLines.push_back(tmpL);
		//			}
		//			if (tCell->bottomBorderVisible()) {
		//				rdf::Line tmpL = tCell->bottomBorder();
		//				tmpL.setThickness(thickness);
		//				hLines.push_back(tmpL);
		//			}
		//			if (tCell->leftBorderVisible()) {
		//				rdf::Line tmpL = tCell->leftBorder();
		//				tmpL.setThickness(thickness);
		//				vLines.push_back(tmpL);
		//			}
		//			if (tCell->rightBorderVisible()) {
		//				rdf::Line tmpL = tCell->rightBorder();
		//				tmpL.setThickness(thickness);
		//				vLines.push_back(tmpL);
		//			}
		//		}

		//	}
		//}

		//std::sort(cells.begin(), cells.end());
		//templateInfo->setLines(hLines, vLines);
		//templateInfo->setRegion(region);
		//templateInfo->setCells(cells);

		//templateForm.setFormName(mLineTemplPath);
		////templateForm.setSize()
		//templateForm.setHorLines(hLines);
		//templateForm.setVerLines(vLines);
		//templateForm.setSize(cv::Size(templateInfo->formSize().width(), templateInfo->formSize().height()));
		//
		//for (auto bi : batchInfo) {
		//	qDebug() << bi->filePath() << "computed...";
		//	FormsInfo* tInfo = dynamic_cast<FormsInfo*>(bi.data());
		//	//bool match = false;
		//	//minErr = std::numeric_limits<double>::max();


		//	if (tInfo) {
		//		qDebug() << "testing form: " << tInfo->formName();
		//		qDebug() << "-------------";
		//	}

		//	qDebug() << "match against template form";

		//	cv::Mat lineImg = tInfo->lineImg();
		//	//lineImg = 255 - lineImg;
		//	lineImg.convertTo(lineImg, CV_32FC1, 1.0 / 255.0);

		//	QPointF sizeTemplate = region->rightDownCorner() - region->leftUpperCorner();
		//	//use 10 pixel as offset
		//	QPointF offsetSize = QPointF(60, 60);
		//	sizeTemplate += offsetSize;
		//	cv::Point2d lU((int)region->leftUpperCorner().x(), (int)region->leftUpperCorner().y());
		//	cv::Point2d offSetLines = cv::Point2d(offsetSize.x()/2, offsetSize.y()/2);
		//	lU -= offSetLines;
		//	
		//	//cv::Size templSize = templateForm.sizeImg();
		//	cv::Size templSize = cv::Size((int)sizeTemplate.x(), (int)sizeTemplate.y());
		//	cv::Mat tmplImg(templSize, CV_32FC1);
		//	tmplImg.setTo(0.0);
		//	

		//	rdf::LineTrace::generateLineImage(templateForm.horLines(), templateForm.verLines(), tmplImg, cv::Scalar(1.0), cv::Scalar(1.0), lU);

		//	rdf::Image::save(tmplImg, "D:\\tmp\\templateImg.png");
		//	rdf::Image::save(lineImg, "D:\\tmp\\lineImg.png");

		//	//cv::Mat outTmp = lineImg.clone();
		//	////cv::matchTemplate(lineImg, tmplImg, outTmp, cv::TM_SQDIFF);
		//	//cv::matchTemplate(lineImg, tmplImg, outTmp, cv::TM_CCOEFF);
		//	////cv::Point alignment = cv::phaseCorrelate(tmplImg, lineImg);
		//	//double minV, maxV;
		//	//cv::Point minLoc, maxLoc;
		//	//cv::minMaxLoc(outTmp, &minV, &maxV, &minLoc, &maxLoc);
		//	//qDebug() << "Shift: " << minLoc.x << "  " << minLoc.y;
		//	//qDebug() << "Shift: " << maxLoc.x << "  " << maxLoc.y;
		//	//outTmp /= maxV;
		//	//rdf::Image::save(outTmp, "D:\\tmp\\corrImg.png");

		//	cv::Mat tmplRowSum, tmplColSum;
		//	cv::Mat formRowSum, formColSum;

		//	//cv::resize(tmplImg, tmplImg, cv::Size(), 0.25, 0.25, cv::INTER_NEAREST);
		//	//cv::resize(lineImg, lineImg, cv::Size(), 0.25, 0.25, cv::INTER_NEAREST);

		//	cv::reduce(tmplImg, tmplRowSum, 1, cv::REDUCE_SUM);
		//	cv::reduce(tmplImg, tmplColSum, 0, cv::REDUCE_SUM);
		//	cv::reduce(lineImg, formRowSum, 1, cv::REDUCE_SUM);
		//	cv::reduce(lineImg, formColSum, 0, cv::REDUCE_SUM);

		//	cv::Mat outIndex;
		//	cv::matchTemplate(formRowSum, tmplRowSum, outIndex, cv::TM_CCOEFF);
		//	double minV, maxV;
		//	cv::Point minLoc, maxLoc;
		//	cv::minMaxLoc(outIndex, &minV, &maxV, &minLoc, &maxLoc);
		//	//maxLoc *= 4.0;
		//	qDebug() << "Shift y: "  << "  " << maxLoc.y;

		//	cv::matchTemplate(formColSum, tmplColSum, outIndex, cv::TM_CCOEFF);
		//	cv::minMaxLoc(outIndex, &minV, &maxV, &minLoc, &maxLoc);
		//	//maxLoc *= 4.0;
		//	qDebug() << "Shift x: " << maxLoc.x << "  ";

		//	qDebug() << rdf::Image::printImage(tmplRowSum, "tmpRow");
		//	qDebug() << rdf::Image::printImage(tmplColSum, "tmpCol");
		//	qDebug() << rdf::Image::printImage(formRowSum, "formRow");
		//	qDebug() << rdf::Image::printImage(formColSum, "formCol");
		//	

		//	
		//	//rdf::FormFeatures currentForm;
		//	//currentForm.setHorLines(tInfo->hLines());
		//	//currentForm.setVerLines(tInfo->vLines());
		//	//currentForm.setSize(cv::Size(tInfo->formSize().width(), tInfo->formSize().height()));
		//	//currentForm.setFormName(tInfo->formName());

		//	//
		//	////TODO adapt compareWithTemplate
		//	//templateForm.compareWithTemplate(currentForm);

		//}

		//////QString outf;
		////QString outf = "D:\\tmp\\evalForms.txt";
		//////QString outf = "F:\\flo\\evalSkew.txt";
		//////outf = mFilePath;
		////QFile file(outf);
		////if (file.open(QIODevice::WriteOnly)) {

		////	for (auto bi : batchInfo) {
		////		FormsInfo* tInfo = dynamic_cast<FormsInfo*>(bi.data());
		////		QString tmpStr = tInfo->filePath();
		////		tmpStr += " matches with: ";
		////		tmpStr += tInfo->matchName();
		////		tmpStr += "\n";
		////		QTextStream out(&file);
		////		out << tmpStr;
		////	}

		////	file.close();
		////}

	}
	else
		qDebug() << "[POST LOADING] train/add training";

	//not tested....
	//saveSettings(rdf::Config::instance().settings());
}

void FormsAnalysis::loadSettings(QSettings & settings) {
	settings.beginGroup(name());
	//mLineTemplPath = settings.value("lineTemplPath", mLineTemplPath).toString();
	mFormConfig.loadSettings(settings);
	settings.endGroup();
}

void FormsAnalysis::saveSettings(QSettings & settings) const {
	settings.beginGroup(name());
	mFormConfig.saveSettings(settings);
	//settings.setValue("lineTemplPath", mLineTemplPath);
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

void FormsInfo::setXMLTemplate(QString t) {
	mXMLTemplate = t;
}

QString FormsInfo::xmlTemplate() const {
	return mXMLTemplate;
}

void FormsInfo::setLineImg(cv::Mat & img) {
	mLineImg = img;
}

cv::Mat FormsInfo::lineImg() const {
	return mLineImg;
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

void FormsInfo::addCell(QSharedPointer<rdf::TableCell> c) {
	mCells.push_back(c);
}

void FormsInfo::setCells(QVector<QSharedPointer<rdf::TableCell>> c) {
	mCells = c;
}

QVector<QSharedPointer<rdf::TableCell>> FormsInfo::cells() const {
	return mCells;
}

void FormsInfo::setRegion(QSharedPointer<rdf::TableRegion> r) {
	mRegion = r;
}

QSharedPointer<rdf::TableRegion> FormsInfo::region() {
	return mRegion;
}


double FormsInfo::jaccardTable() const {
	return mJaccardTable;
}

void FormsInfo::setJaccardTable(double d) {
	mJaccardTable = d;
}

double FormsInfo::matchTable() const {
	return mMatchTable;
}

void FormsInfo::setMatchTable(double d) {
	mMatchTable = d;
}

QVector<double> FormsInfo::jaccardCell() const {
	return mJaccardCell;
}

void FormsInfo::setJaccardCell(QVector<double> v) {
	mJaccardCell = v;
}

double FormsInfo::jaccardMeanCell() const {
	return mMeanJICells;
}

void FormsInfo::setJaccardMeanCell(double d) {
	mMeanJICells = d;
}

QVector<double> FormsInfo::cellMatch() const {
	return mCellMatch;
}

void FormsInfo::setCellMatch(QVector<double> v) {
	mCellMatch = v;
}

double FormsInfo::matchMeanCell() const {
	return mMeanMatchCells;
}

void FormsInfo::setmatchMeanCell(double d) {
	mMeanMatchCells = d;
}

QVector<double> FormsInfo::underSegmentedC() const {
	return mUnderSegmentedC;
}

void FormsInfo::setUnderSegmentedC(QVector<double> v) {
	mUnderSegmentedC = v;
}

double FormsInfo::underSegmented() const {
	return mUnderSegmented;
}

void FormsInfo::setUnderSegmented(double d) {
	mUnderSegmented = d;
}

double FormsInfo::missedCells() const {
	return mMissedCells;
}

void FormsInfo::setMissedCells(double d) {
	mMissedCells = d;
}

};


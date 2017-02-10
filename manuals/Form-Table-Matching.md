# Table/Form Matching Description
09.02.2017 created - kleber@caa.tuwien.ac.at

## Description

The table/form module can be used to align a template to a given document. Currently documents with two table regions are not supported. A form classification to select the correct template and a scale independency is planned for the next release. In the following section the workflow for the nomacs plugin is described.

## Table/Form Template

A table/form template must be defined using the table editor in Transkribus. The template must comprise all fixed lines (printed). If cells have e.g. a varying height, the lines should not be labelled in the template. The following picture shows a template for the collection READ GT (Table_Template_M_Freyung). Note that the horizontal lines are not marked.

![TemplateImage](ftp://scruffy.caa.tuwien.ac.at/staff/read/manuals/form/M_Aigen_am_Inn_003-01_0001-template.jpg)

## Workflow in nomacs

*Open Nomacs Settings (Edit - Settings) and define the template for the table module using the editor. Choose FormAnalysis and set lineTemplPath to the template page xml

*Open an image and select Pugins - Forms Analysis - Apply Template (Single). As an result you will see the aligned template. In the page xml also additional separators are stored. You can visualize the rough alignment, the matching and the additional line separators if you open Panels - Edit History. See the following image:

![Result](ftp://scruffy.caa.tuwien.ac.at/staff/read/manuals/form/nomacs-screenshot-form1.png)

*Alternatively you can use Plugins - Page Visualization to visualize the result.

* You can also use the batch processing to calculate multiple images by choosing the same Plugin (Forms Analysis - Apply Template (Single))

## Remarks

* Form classification is currently not supported. Correct template most be selected
* Not scale independent
* Two table regions are not supported
* Template must be set as page xml (defined in the table editor)
    additional detected separators are stored in the xml

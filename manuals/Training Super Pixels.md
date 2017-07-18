# SuperPixel Training Guide
- Markus Diem _23.10.2016_
- edit _18.07.2017_

# Collect Features
- Create a label config file and name it e.g. ``config-labels.json``
- a simple two-class config would look like this:

```json
{
  "Labels": [
  {
  "Class" : {
    "id": 4,
    "name": "handwriting",
    "alias": ["TextLine", "Word"],
    "color": "#006699"
  }}, {
  "Class" : {
    "id": 8,
    "name": "noise",
    "alias": ["NoiseRegion", "SeparatorRegion", "TextRegion"],
    "color": "#009966"
  }}
  ]
}
```

- The ``id`` must be unique and greater 1 (id 0 is reserved for unknown)
- Each label is represented by ``Class``
- The ``name`` attribute is the internal label name and must be unique
- The ``alias`` allows you to define different (e.g PAGE XML) tags
that are grouped together to one class

### Configure the I/O paths
- Open ``nomacs READ`` _Plugins > READ Config_
  - _Input_: set _SuperPixelTrainer > labelConfigFilePath_ to the config file which was created previously
  - _Output_: _SuperPixelTrainer > featureFilePath_ choose a file e.g. ``features-labels.json``
- Open ``nomacs READ`` _Plugins > Layout Analysis > Collect Features_
  - Open _Tools > Batch Processing_
  - INPUT: Choose all training data
  TIP: if you drop a folder to the file list holding CTRL, it will be added recursively
  - PLUGINS: _Select Layout Analysis | Collect Features_
  - OUTPUT: check _Do not Save Output Images_
  TIP: you can save the profile...

  ...run the batch...

### Background
You can configure how background regions should be trained.
- Open `nomacs READ` Plugins > _READ Config_
  - In _LayoutPlugin > Super Pixel Labeler > backgroundLabelName_ specify the class name (e.g. _noise_)
  - The entry is empty by default which indicates that background regions are treated as `unknown` and therefore not trained

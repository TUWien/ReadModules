# READ Modules
READ Modules are document analysis modules developed at CVL/TU Wien for the EU project READ. The READ project  has  received  funding  from  the European  Union’s  Horizon  2020 research  and innovation programme under grant agreement No 674943.
 
[![Build Status](https://travis-ci.org/TUWien/ReadModules.svg?branch=master)](https://travis-ci.org/TUWien/ReadModules)

## Build on Windows

todo

## Build on Ubuntu
note that Qt 5.5 is needed, thus Ubuntu version must be >= 16.04 or backports of Qt 5.5 have to be used (see .travis.yml for an ppa repository and names packages which need to be installed)

Get required packages:

``` console
sudo apt-get install qt5-qmake qttools5-dev-tools qt5-default libqt5svg5-dev qt5-image-formats-plugins libopencv-dev cmake git libexiv2-dev libraw-dev
```

Get nomacs sources from github:
``` console
git clone https://github.com/nomacs/nomacs
```
This will by default place the source into ./nomacs
Go to the nomacs directory and run `cmake` to get the Makefiles:
``` console
cmake ImageLounge/.
```
Compile nomacs: 
``` console
make
```

Get the READ Framework sources from github:
``` console
git clone https://github.com/TUWien/ReadFramework
```
This will by default place the source into ./ReadFramework

Go to the ReadFramework directory and run `cmake` to get the Makefiles:
``` console
cmake . 
```

Compile READ Framework: 
``` console
make
```

Get the READ Modules sources from github:
``` console
git clone https://github.com/TUWien/ReadModules
```
This will by default place the source into ./ReadModules

Go to the ReadModules directory and run `cmake` to get the Makefiles:
you have to set the correct nomacs and ReadFramework build directories
``` console
cmake -Dnomacs_DIR=../nomacs/ -DReadFramework_DIR=../ReadFramework . 
```

Compile READ Modules: 
``` console
make
```

The READ modules are nomacs plugins which will be automatically copied to the nomacs build directory after compiling. Go to the nomacs build directory and start nomacs
``` console
./nomacs
```
The READ modules are now available in the Menu -> Plugins (you have to execute the PluginManager once from the menu)


### authors
Markus Diem
Stefan Fiel
Florian Kleber

### related links:
[1] http://www.caa.tuwien.ac.at/cvl/
[2] https://transkribus.eu/Transkribus/
[3] https://github.com/TUWien/
[4] http://nomacs.org
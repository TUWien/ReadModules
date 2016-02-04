# READ Modules
READ Modules are document analysis modules developed at CVL/TU Wien for the EU project READ. The READ project  has  received  funding  from  the European  Union’s  Horizon  2020 research  and innovation programme under grant agreement No 674943.
 
[![Build Status](https://travis-ci.org/TUWien/ReadFramework.svg?branch=master)](https://travis-ci.org/TUWien/ReadFramework)

## Build on Windows
### Compile dependencies
- `Qt` SDK or the compiled sources (>= 5.4.0)
- `OpenCV` (>= 3.0.0)

### Compile ReadFramework
1. Clone the repository from `git@github.com:TUWien/ReadFramework.git`
2. Open CMake GUI
3. set your ReadFramework folder to `where is the source code`
4. choose a build folder
5. Hit `Configure`
6. Set `QT_QMAKE_EXECUTABLE` by locating the qmake.exe
7. Set `OpenCV_DIR` to your OpenCV build folder
8. Hit `Configure` then `Generate`
9. Open the `ReadFramework.sln` which is in your new build directory
10. Right-click the ReadFramework project and choose `Set as StartUp Project`
11. Compile the Solution
12. enjoy

### If anything did not work
- check if you have setup opencv
- check if your Qt is set correctly (otherwise set the path to `qt_install_dir/qtbase/bin/qmake.exe`)
- check if your builds proceeded correctly

## Build on Ubuntu
note that Qt 5.5 is needed, thus Ubuntu version must be >= 16.04 or backports of Qt 5.5 have to be used (see .travis.yml for an ppa repository and names packages which need to be installed)

Get required packages:

``` console
sudo apt-get install qt5-qmake qt5tools-dev qttools5-dev-tools qt5-default libqt5svg5 qt5-image-formats-plugins libopencv-dev cmake git
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

You will now have a binary (ReadFramework), which you can test (or use directly). Also the build libraries are in this directory. To install it to /usr/local/bin, use: 
``` console
sudo make install
```

### authors
Markus Diem
Stefan Fiel
Florian Kleber

### related links:
[1] http://www.caa.tuwien.ac.at/cvl/
[2] https://transkribus.eu/Transkribus/
[3] https://github.com/TUWien/
[4] http://nomacs.org
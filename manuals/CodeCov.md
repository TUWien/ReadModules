# Adding Code Coverage Tests
Markus Diem
_diem@caa.tuwien.ac.at_
_21.07.2017_

## General
Code Coverage tests give a comprehensive overview of the functioning of our software [1]. For simplicity, we now only test `ReadFramework`.

## Create Test in CMake
- Tests are added via CMake and will run on Travis (gcc/Ubuntu)
- Open `cmake/BuildTargets.cmake`
- Search # tests
- Add a line like:
````
add_test(NAME YourTestName COMMAND ${RDF_TEST_NAME} "--your-cmd-argument")
````
## Adding Test in your Code
- Open `ReadFrameworkTest`
- add your cmd argument in `main.cpp`
````cpp
// your test
QCommandLineOption yourOpt(QStringList() << "your-cmd-argument", QObject::tr("Short Description."));
parser.addOption(yourOpt);
````
- if necessary, add your test files to `./src/UnitTests`



## Links
[1] https://codecov.io/gh/TUWien/ReadFramework

# gneis-daq

[![Project Stats](https://www.openhub.net/p/gneis-daq/widgets/project_thin_badge?format=gif)](https://www.openhub.net/p/gneis-daq)

Collection of data-acquisition applications for [GNEIS](http://accelconf.web.cern.ch/AccelConf/rupac2016/talks/wezmh01_talk.pdf) facility. Based on [MIDAS](https://midas.triumf.ca/MidasWiki/index.php/Main_Page) data acquisition system.

## Content

* `fe-v1720`: MIDAS front-end application for [CAEN V1720E](https://www.caen.it/products/v1720/) digitizer.
* `fe-v1724`: MIDAS front-end application for [CAEN V1724](https://www.caen.it/products/v1724/) digitizer.
* `fe-sinus`: MIDAS front-end application that generates sinusoidal signal for testing purposes.
* `an-basic-waveform`: MIDAS basic wave form analyzer application.
* `gd-convert`: MIDAS data file conversion utility.
* `caen-lib`: library with light-weight wrappers around CAEN digitizer API.
* `midas-util-lib`: library with light-weight wrappers around MIDAS API.
* `util-lib`: library with commonly used definitions.
* `util-lib-test`: unit tests for `util-lib`.
* `analyzer-lib`: library for analyzer applications.
* `math-lib`: general purpose math library.
* `math-lib-test`: unit tests for `math-lib`.
* `r-packages`: R packages with data parsing and hadling functions.
* `doc`: documentation.

## Requirements

* CMake 3.1+
* GCC 4.8+
* GoogleTest
* Latex

## Dependencies

* [ROOT](https://github.com/root-project/root)
* [MIDAS](https://bitbucket.org/tmidas/midas)
* [MXML](https://bitbucket.org/tmidas/mxml)
* [ROOTANA](https://bitbucket.org/tmidas/rootana)
* [CAEN Digitizer Library](https://www.caen.it/products/caendigitizer-library/) and its dependencies

## Prerequisites

* All the dependencies above installed
* ROOT, MIDAS, and ROOTANA environment variables initialized


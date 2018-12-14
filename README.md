# gneis-daq

[![Project Stats](https://www.openhub.net/p/gneis-daq/widgets/project_thin_badge?format=gif)](https://www.openhub.net/p/gneis-daq)

Collection of data-acquisition applications for [GNEIS](http://hepd.pnpi.spb.ru/~gneis/) facility. Based on [MIDAS](https://midas.triumf.ca/MidasWiki/index.php/Main_Page) data acquisition system.

## Content

* `fe-v1720`: MIDAS front-end application for [CAEN V1720E](https://www.caen.it/products/v1720/) digitizer.
* `fe-sinus`: MIDAS front-end application that generates sinusoidal signal for testing purposes.
* `an-basic-wave-form`: MIDAS basic wave form analyzer application.
* `caen-lib`: library with light-weight wrappers around CAEN digitizer API.
* `midas-util-lib`: library with light-weight wrappers around MIDAS API.
* `util-lib`: library with commonly used definitions.
* `doc`: documentation.

## Requirements

* CMake 3.1+
* GCC 4.8+
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


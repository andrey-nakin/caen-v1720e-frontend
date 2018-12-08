# gneis-daq

Collection of data-acquisition applications used in [GNEIS][http://hepd.pnpi.spb.ru/~gneis/] experiment. Based on [MIDAS](https://midas.triumf.ca/MidasWiki/index.php/Main_Page) toolkit.

## Content

* `fe-v1720`: MIDAS front-end application for [CAEN V1720E](https://www.caen.it/products/v1720/) digitizer.
* `fe-sinus`: MIDAS front-end application that generates sinusoidal signal for testing purposes.
* `caen-lib`: library with light-weight wrappers around CAEN digitizer API.
* `midas-util-lib`: library with light-weight wrappers around MIDAS API.
* `frontend-util-lib`: library with commonly used definitions.

## Requirements

* Linux GCC 4.8+
* CMake 3.1+

## Dependencies

* [ROOT][https://github.com/root-project/root]
* [MIDAS][https://bitbucket.org/tmidas/midas]
* [MXML][https://bitbucket.org/tmidas/mxml]
* [ROOTANA][https://bitbucket.org/tmidas/rootana]
* [CAEN Digitizer Library][https://www.caen.it/products/caendigitizer-library/] and its dependencies

## Prerequisites

* All the dependencies above installed
* ROOT, MIDAS, and ROOTANA environment variables initialized


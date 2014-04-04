ADPvCam Releases
====================

The latest untagged master branch can be obtained at
https://github.com/areaDetector/ADPvCam.

Tagged source code and pre-built binary releases prior to R2-0 are included
in the areaDetector releases available via links at
http://cars.uchicago.edu/software/epics/areaDetector.html.

Tagged source code releases from R2-0 onward can be obtained at 
https://github.com/areaDetector/ADPvCam/releases.

Tagged prebuilt binaries from R2-0 onward can be obtained at
http://cars.uchicago.edu/software/pub/ADPvCam.

The versions of EPICS base, asyn, and other synApps modules used for each release can be obtained from 
the EXAMPLE_RELEASE_PATHS.local, EXAMPLE_RELEASE_LIBS.local, and EXAMPLE_RELEASE_PRODS.local
files respectively, in the configure/ directory of the appropriate release of the 
[top-level areaDetector](https://github.com/areaDetector/areaDetector) repository.


Release Notes
=============

R2-0
----
* Moved the repository to [Github](https://github.com/areaDetector/ADPvCam).
* Re-organized the directory structure to separate the driver library from the example IOC application.


R1-9-1 and earlier
------------------
Release notes are part of the
[areaDetector Release Notes](http://cars.uchicago.edu/software/epics/areaDetectorReleaseNotes.html).

Future Releases
===============
* Add 64-bit SDK files for PvCAM
* Break into 2 respositories, one based on Photometrics PvCam library, 
  the other based on Princeton Instruments PvCam library.

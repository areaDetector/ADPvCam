TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

USR_CFLAGS +=   -DWIN32
USR_CXXFLAGS += -DWIN32

LIBRARY_IOC_WIN32    = pvCam
LIBRARY_IOC_cygwin32 = pvCam
LIB_SRCS += pvCam.cpp
LIB_SRCS += drvPVCamEpics.c
LIB_LIBS += pvcam32 pv_icl32

PROD_IOC_WIN32    += pvCamApp
PROD_IOC_cygwin32 += pvCamApp

# <name>.dbd will be created from <name>Include.dbd
DBD += pvCamApp.dbd
DBD += pvCamSupport.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
PROD_SRCS += pvCamApp_registerRecordDeviceDriver.cpp pvCamAppMain.cpp

# Add locally compiled object code
PROD_LIBS += pvCam
PROD_LIBS += pvcam32 pv_icl32

include $(TOP)/ADApp/commonDriverMakefile

#=============================


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


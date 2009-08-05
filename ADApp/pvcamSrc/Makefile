TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

USR_CFLAGS += /DHDF5 /DWIN32
USR_CXXFLAGS += /DHDF5 /DWIN32

LIBRARY_IOC = pvCam
LIB_SRCS += pvCam.cpp
LIB_SRCS += drvPVCamEpics.c
LIB_LIBS += NDPlugin ADBase netCDF
#LIB_LIBS += nexusPlugin
#LIB_LIBS += socketConPlugin
LIB_LIBS += pvcam32 pv_icl32
LIB_LIBS += calc busy sscan autosave
LIB_LIBS += asyn
#LIB_LIBS += hdf5
LIB_LIBS += $(EPICS_BASE_IOC_LIBS)

#==================================
PROD_IOC += pvCamApp
# <name>.dbd will be created from <name>Include.dbd
DBD += pvCamApp.dbd
DBD += pvCamSupport.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
pvCamApp_SRCS_DEFAULT += pvCamApp_registerRecordDeviceDriver.cpp pvCamAppMain.cpp
pvCamApp_SRCS_vxWorks += pvCamApp_registerRecordDeviceDriver.cpp

# Add locally compiled object code
pvCamApp_LIBS += pvCam
pvCamApp_LIBS += NDPlugin ADBase netCDF
#pvCamApp_LIBS += nexusPlugin
#pvCamApp_LIBS += socketconPlugin
pvCamApp_LIBS += pvcam32 pv_icl32
pvCamApp_LIBS += calc busy sscan autosave 
pvCamApp_LIBS += asyn
#pvCamApp_LIBS += hdf5 libc zdll szlib
#pvCamApp_LIBS += ACEd ACE_QoSd
pvCamApp_LIBS += $(EPICS_BASE_IOC_LIBS)

pvCamApp_SYS_LIBS_cygwin32 += rpc
pvCamApp_LIBS_WIN32        += libtiff jpeg
pvCamApp_LIBS_cygwin32     += libtiff jpeg
pvCamApp_SYS_LIBS_Linux    += tiff jpeg z

#=============================


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


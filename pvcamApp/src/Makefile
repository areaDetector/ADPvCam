TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
LIBRARY_NAME = pvCam 
PROD_NAME    = pvCamApp

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
USR_CFLAGS +=   -DWIN32
USR_CXXFLAGS += -DWIN32

else ifeq (cygwin-x86, $(findstring cygwin-x86, $(T_A)))
USR_CFLAGS +=   -DWIN32
USR_CXXFLAGS += -DWIN32

endif



#Only build for 32bit versions of Windows, cygwin, and Linux

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
LIBRARY_IOC = $(LIBRARY_NAME)
PROD_IOC += $(PROD_NAME)

else ifeq (cygwin-x86, $(findstring cygwin-x86, $(T_A)))
LIBRARY_IOC = $(LIBRARY_NAME)
PROD_IOC += $(PROD_NAME)

else ifeq (linux-x86, $(T_A))
LIBRARY_IOC = $(LIBRARY_NAME)
PROD_IOC += $(PROD_NAME)

endif



#Include driver code
LIB_SRCS += pvCam.cpp
LIB_SRCS += drvPVCamEpics.c


#Add required libraries
ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
LIB_LIBS += pvcam32 pv_icl32

else ifeq (cygwin-x86, $(findstring cygwin-x86, $(T_A)))
LIB_LIBS += pvcam32 pv_icl32

else ifeq (linux-x86, $(T_A))

SHRLIB_SEARCH_DIRS += lib/linux-x86
LIB_SYS_LIBS += pvcam

#Current setup of RHEL at APS includes this library, but not the symbolic link
#to build against, so we'll specify the exact version.
LIB_SYS_LIBS += :libraw1394.so.11

endif



# <name>.dbd will be created from <name>Include.dbd
DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += pvCamSupport.dbd
DBD += pvCamSupport.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
PROD_SRCS += pvCamApp_registerRecordDeviceDriver.cpp pvCamAppMain.cpp

# Add locally compiled object code
PROD_LIBS += pvCam

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
PROD_LIBS += pvcam32 pv_icl32

else ifeq (cygwin-x86, $(findstring cygwin-x86, $(T_A)))
PROD_LIBS += pvcam32 pv_icl32

else ifeq (linux-x86, $(T_A))
PROD_SYS_LIBS += :libpvcam.so.2.7.4.2
PROD_SYS_LIBS += :libraw1394.so.11
endif

include $(TOP)/ADApp/commonDriverMakefile

#=============================


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


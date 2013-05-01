TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#======== DETECTOR LIBRARY ==============

USR_CFLAGS +=   -DWIN32
USR_CXXFLAGS += -DWIN32

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
LIBRARY_IOC = pvCam
else ifeq (cygwin-x86, $(findstring cygwin-x86, $(T_A)))
LIBRARY_IOC = pvCam
endif

LIB_SRCS += pvCam.cpp
LIB_SRCS += drvPVCamEpics.c
LIB_LIBS += pvcam32 pv_icl32

DBD += pvCamSupport.dbd

#====== DETECTOR APPLICATION ============

ifeq ($(BUILD_APPS), YES)
PROD_NAME = pvCamApp
ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
PROD_IOC += $(PROD_NAME)
else ifeq (cygwin-x86, $(findstring cygwin-x86, $(T_A)))
PROD_IOC += $(PROD_NAME)
endif

# <name>.dbd will be created from <name>Include.dbd
DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += pvCamSupport.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
PROD_SRCS += $(PROD_NAME)_registerRecordDeviceDriver.cpp $(PROD_NAME)Main.cpp

# Add locally compiled object code
PROD_LIBS += pvCam
PROD_LIBS += pvcam32 pv_icl32

include $(TOP)/ADApp/commonDriverMakefile
endif

#=============================


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
ifeq ($(STATIC_BUILD), YES)
  USR_LDFLAGS_Linux += -Wl,-rpath,$(abspath $(ADPVCAM)/lib/linux-x86)
endif
PROD_NAME    = pvCamApp

PROD_IOC_Linux += $(PROD_NAME)
PROD_IOC_WIN32 += $(PROD_NAME)


# <name>.dbd will be created from <name>Include.dbd
DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += pvCamSupport.dbd


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
$(PROD_NAME)_SRCS += $(PROD_NAME)_registerRecordDeviceDriver.cpp $(PROD_NAME)Main.cpp


# Add locally compiled object code
$(PROD_NAME)_LIBS += ADPvCam

# 64 bit windows support
$(PROD_NAME)_LIBS_WIN32 += pvcam64

# For linux we only have a shared library, so use SYS_LIBS
$(PROD_NAME)_SYS_LIBS_Linux += pvcam

include $(ADCORE)/ADApp/commonDriverMakefile

#=============================


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


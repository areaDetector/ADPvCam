TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
PROD_NAME    = pvCamApp

#Only build for 32bit versions of Windows, cygwin, and Linux

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
PROD_IOC += $(PROD_NAME)

else ifeq (cygwin-x86, $(findstring cygwin-x86, $(T_A)))
PROD_IOC += $(PROD_NAME)

else ifeq (linux-x86, $(T_A))
PROD_IOC += $(PROD_NAME)

endif

# <name>.dbd will be created from <name>Include.dbd
DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += pvCamSupport.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
PROD_SRCS += pvCamApp_registerRecordDeviceDriver.cpp pvCamAppMain.cpp

# Add locally compiled object code
PROD_LIBS += pvCam

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
PROD_LIBS += pvcam32 pv_icl32

else ifeq (cygwin-x86, $(findstring cygwin-x86, $(T_A)))
PROD_LIBS += pvcam32 pv_icl32

else ifeq (linux-x86, $(T_A))
PROD_LIBS += pvcam
PROD_SYS_LIBS += raw1394

endif

include $(ADCORE)/ADApp/commonDriverMakefile

#=============================


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


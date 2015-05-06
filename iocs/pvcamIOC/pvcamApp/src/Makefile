TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
PROD_NAME    = pvCamApp

# Only build for 32bit versions of Windows and Linux
# Cygwin no longer recognizes the .lib files

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
PROD_IOC += $(PROD_NAME)

#else ifeq (cygwin-x86, $(findstring cygwin-x86, $(T_A)))
#PROD_IOC += $(PROD_NAME)

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
# The pvcam library is only provided as a .so file, not .a.
# When building statically we must declare it to be a SYS_LIB so it finds the .so file.
# Do this even when building dynamically for simplicity.
SHRLIB_SEARCH_DIRS += lib/linux-x86
PROD_SYS_LIBS += pvcam
PROD_SYS_LIBS += raw1394

endif

include $(ADCORE)/ADApp/commonDriverMakefile

#=============================


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


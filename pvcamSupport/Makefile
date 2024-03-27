TOP=..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

# The files in this directory are installed first because they are
# used by PVCam driver

INC += pvcam.h master.h #pv_icl.h

# Note, the following files were manually copied from the PVCam distribution
# to this directory after building

LIB_INSTALLS_Linux += ../os/linux-x86_64/libpvcam.so
LIB_INSTALLS_WIN32 += ../os/WIN32/pvcam64.lib

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


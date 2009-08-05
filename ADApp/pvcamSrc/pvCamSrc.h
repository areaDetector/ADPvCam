/* pvCamSrc.h
 *
 * This is a driver for a PVCam (PI/Acton) detector.
 *
 * Author: Brian Tieman
 *
 * Created:  06/14/2008
 *
 */

#ifndef PVCAMSRC_H
#define PVCAMSRC_H

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsMutex.h>
#include <epicsString.h>
#include <epicsStdio.h>
#include <epicsMutex.h>
#include <cantProceed.h>

#include "ADDriver.h"

#include "drvPVCam.h"

/* PM FILES */
#include "master.h"
#include "pvcam.h"

//______________________________________________________________________________________________

static const char *driverName = "drvPVCam";

//______________________________________________________________________________________________

/** The polling interval when checking to see if acquisition is complete */
#define POLL_TIME 						0.01

#define NUM_PV_CAM_PARAMS (sizeof(PVCamParamString)/sizeof(PVCamParamString[0]))

#define MAX_DETECTORS_SUPPORTED			3
#define MAX_SPEEDS_SUPPORTED			5

//______________________________________________________________________________________________

/* If we have any private driver parameters they begin with ADFirstDriverParam and should end
   with ADLastDriverParam, which is used for setting the size of the parameter library table */
typedef enum {
    PVCamInitDetector
        = ADLastStdParam,
    PVCamSlot1Cam,
    PVCamSlot2Cam,
    PVCamSlot3Cam,
    PVCamDetectorSelected,
    PVCamChipNameRBV,
    PVCamNumParallelPixelsRBV,
    PVCamNumSerialPixelsRBV,
    PVCamPixelParallelSizeRBV,
    PVCamPixelSerialSizeRBV,
    PVCamChipHeightMMRBV,
    PVCamChipWidthMMRBV,
    PVCamPixelParallelDistRBV,
    PVCamPixelSerialDistRBV,
    PVCamPostMaskRBV,
    PVCamPreMaskRBV,
    PVCamPostScanRBV,
    PVCamPreScanRBV,
    PVCamNumPortsRBV,
    PVCamFullWellCapacityRBV,
    PVCamFrameTransferCapableRBV,
    PVCamNumSpeedTableEntriesRBV,
    PVCamSpeedTableIndex,
    PVCamSpeedTableIndexRBV,
    PVCamBitDepthRBV,
    PVCamPixelTimeRBV,
    PVCamGainIndex,
    PVCamGainIndexRBV,
    PVCamMaxGainIndexRBV,
    PVCamMinShutterOpenDelayRBV,
    PVCamMaxShutterOpenDelayRBV,
    PVCamMinShutterCloseDelayRBV,
    PVCamMaxShutterCloseDelayRBV,
    PVCamShutterOpenDelay,
    PVCamShutterOpenDelayRBV,
    PVCamShutterCloseDelay,
    PVCamShutterCloseDelayRBV,
    PVCamMeasuredTemperatureRBV,
    PVCamMinTemperatureRBV,
    PVCamMaxTemperatureRBV,
    PVCamSetTemperature,
    PVCamSetTemperatureRBV,
    PVCamDetectorMode,
    PVCamDetectorModeRBV,
    PVCamTriggerMode,
    PVCamTriggerModeRBV,
    PVCamTriggerEdge,
    PVCamTriggerEdgeRBV,
    ADLastDriverParam
} PVCamParam_t;

//______________________________________________________________________________________________

static asynParamString_t PVCamParamString[] = {
    {PVCamInitDetector,    			"PVCAM_INITIALIZE_DETECTOR"},
    {PVCamSlot1Cam,    				"PVCAM_SLOT1"},
    {PVCamSlot2Cam,    				"PVCAM_SLOT2"},
    {PVCamSlot3Cam,    				"PVCAM_SLOT3"},
    {PVCamDetectorSelected,    		"PVCAM_DETECTORSELECTED"},
    {PVCamChipNameRBV,	    		"PVCAM_CHIPNAME"},
    {PVCamNumParallelPixelsRBV,		"PVCAM_NUMPARALLELPIXELS"},
    {PVCamNumSerialPixelsRBV,		"PVCAM_NUMSERIALPIXELS"},
    {PVCamPixelParallelSizeRBV,		"PVCAM_PIXELPARALLELSIZE"},
    {PVCamPixelSerialSizeRBV,		"PVCAM_PIXELSERIALSIZE"},
    {PVCamChipHeightMMRBV,			"PVCAM_CHIPHEIGHT"},
    {PVCamChipWidthMMRBV,			"PVCAM_CHIPWIDTH"},
    {PVCamPixelParallelDistRBV,		"PVCAM_PIXELPARALLELDIST"},
    {PVCamPixelSerialDistRBV,		"PVCAM_PIXELSERIALDIST"},
    {PVCamPostMaskRBV,				"PVCAM_POSTMASK"},
    {PVCamPreMaskRBV,				"PVCAM_PREMASK"},
    {PVCamPostScanRBV,				"PVCAM_POSTSCAN"},
    {PVCamPreScanRBV,				"PVCAM_PRESCAN"},
    {PVCamNumPortsRBV,				"PVCAM_NUMPORTS"},
    {PVCamFullWellCapacityRBV,		"PVCAM_FULLWELLCAPACITY"},
    {PVCamFrameTransferCapableRBV,	"PVCAM_FRAMETRANSFERCAPABLE"},
    {PVCamNumSpeedTableEntriesRBV,	"PVCAM_NUMSPEEDTABLEENTRIES"},
    {PVCamSpeedTableIndex,			"PVCAM_SPEEDTABLEINDEX"},
    {PVCamSpeedTableIndexRBV,		"PVCAM_SPEEDTABLEINDEX_RBV"},
    {PVCamBitDepthRBV,				"PVCAM_BITDEPTH"},
    {PVCamPixelTimeRBV,				"PVCAM_PIXELTIME"},
    {PVCamGainIndex,				"PVCAM_GAININDEX"},
    {PVCamGainIndexRBV,				"PVCAM_GAININDEX_RBV"},
    {PVCamMaxGainIndexRBV,			"PVCAM_MAXGAININDEX"},
    {PVCamMinShutterOpenDelayRBV,	"PVCAM_MINSHUTTEROPENDELAY"},
    {PVCamMaxShutterOpenDelayRBV,	"PVCAM_MAXSHUTTEROPENDELAY"},
    {PVCamMinShutterCloseDelayRBV,	"PVCAM_MINSHUTTERCLOSEDELAY"},
    {PVCamMaxShutterCloseDelayRBV,	"PVCAM_MAXSHUTTERCLOSEDELAY"},
    {PVCamShutterOpenDelay,			"PVCAM_SHUTTEROPENDELAY"},
    {PVCamShutterOpenDelayRBV,		"PVCAM_SHUTTEROPENDELAY_RBV"},
    {PVCamShutterCloseDelay,		"PVCAM_SHUTTERCLOSEDELAY"},
    {PVCamShutterCloseDelayRBV,		"PVCAM_SHUTTERCLOSEDELAY_RBV"},
    {PVCamMeasuredTemperatureRBV,	"PVCAM_MEASUREDTEMPERATURE"},
    {PVCamMinTemperatureRBV,		"PVCAM_MINTEMPERATURE"},
    {PVCamMaxTemperatureRBV,		"PVCAM_MAXTEMPERATURE"},
    {PVCamSetTemperature,			"PVCAM_SETTEMPERATURE"},
    {PVCamSetTemperatureRBV,		"PVCAM_SETTEMPERATURE_RBV"},
    {PVCamDetectorMode,				"PVCAM_DETECTORMODE"},
    {PVCamDetectorModeRBV,			"PVCAM_DETECTORMODE_RBV"},
    {PVCamTriggerMode,				"PVCAM_TRIGGERMODE"},
    {PVCamTriggerModeRBV,			"PVCAM_TRIGGERMODE_RBV"},
    {PVCamTriggerEdge,				"PVCAM_TRIGGEREDGE"},
    {PVCamTriggerEdgeRBV,			"PVCAM_TRIGGEREDGE_RBV"},
};

//______________________________________________________________________________________________

class pvCam : public ADDriver
{
public:
int 				imagesRemaining;
epicsEventId 		startEventId,
					stopEventId;
NDArray 			*pRaw;

    pvCam(const char *portName, int maxSizeX, int maxSizeY, NDDataType_t dataType,
                int maxBuffers, size_t maxMemory, int priority, int stackSize);

    /* These are the methods that we override from ADDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    virtual asynStatus drvUserCreate(asynUser *pasynUser, const char *drvInfo, const char **pptypeName, size_t *psize);
    void report(FILE *fp, int details);

    /* These are the methods that are new to this class */
    template <typename epicsType> int computeArray(int maxSizeX, int maxSizeY);

    int allocateBuffer();
    int computeImage();

    void pvCamAcquisitionTask();
    void pvCamMonitorTask();

    ~pvCam ();

private:
int16			numDetectorsInstalled,
				detectorSelected,
				detectorHandle;

char			*detectorList[5];

unsigned short  *rawData;

	void outputErrorMessage (const char *functionName, char *appMessage);

	void initializeDetectorInterface (void);
	void selectDetector (int selectedDetector);

	void queryCurrentSettings (void);

	void initializeDetector (void);

	int getAcquireStatus (void);

};

//______________________________________________________________________________________________

#endif

/* pvCam.cpp
 *
 * This is a driver for a PVCam (PI/Acton) detector.
 *
 * Author: Brian Tieman
 *
 * Created:  06/14/2009
 *
 */

#include "ADPvCam.h"



// Error message formatters
#define ERR(msg) asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s::%s: %s\n", \
    driverName, functionName, msg)

#define ERR_ARGS(fmt,...) asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, \
    "%s::%s: " fmt "\n", driverName, functionName, __VA_ARGS__);

// Flow message formatters
#define LOG(msg) asynPrint(pasynUserSelf, ASYN_TRACE_FLOW, "%s::%s: %s\n", \
    driverName, functionName, msg)

#define LOG_ARGS(fmt,...) asynPrint(pasynUserSelf, ASYN_TRACE_FLOW, \
    "%s::%s: " fmt "\n", driverName, functionName, __VA_ARGS__);



//_____________________________________________________________________________________________

extern "C" int ADPvCamConfig(const char *portName, int maxSizeX, int maxSizeY, int dataType,
                                 int maxBuffers, size_t maxMemory, int priority, int stackSize)
{
    new ADPvCam(portName, maxSizeX, maxSizeY, (NDDataType_t)dataType, maxBuffers, maxMemory, priority, stackSize);
    return(asynSuccess);
}

//_____________________________________________________________________________________________

static void pvCamAcquisitionTaskC(void *drvPvt)
{
    ADPvCam *pPvt = (ADPvCam *)drvPvt;

    pPvt->pvCamAcquisitionTask();
}

//_____________________________________________________________________________________________

static void pvCamMonitorTaskC(void *drvPvt)
{
    ADPvCam *pPvt = (ADPvCam *)drvPvt;

    pPvt->pvCamMonitorTask();
}

//_____________________________________________________________________________________________
//_____________________________________________________________________________________________
//Public methods
//_____________________________________________________________________________________________
//_____________________________________________________________________________________________

ADPvCam::ADPvCam(const char *portName, int maxSizeX, int maxSizeY, NDDataType_t dataType, int maxBuffers, size_t maxMemory, int priority, int stackSize)
    : ADDriver(portName, 1, NUM_PVCAM_PARAMS, maxBuffers, maxMemory, 0, 0, 0, 1, priority, stackSize), imagesRemaining(0), pRaw(NULL)
{
    const char *functionName = "ADPvCam";
    int status = asynSuccess;
    size_t dims[2];

    //init some variables
    detectorSelected = 0;
    numDetectorsInstalled = 0;
    detectorHandle = 0;
    tempAvailable = false;
    rawData = NULL;

    createParam(PVCamInitDetectorString,             asynParamInt32,   &PVCamInitDetector);
    createParam(PVCamSlot1CamString,                 asynParamOctet,   &PVCamSlot1Cam);
    createParam(PVCamSlot2CamString,                 asynParamOctet,   &PVCamSlot2Cam);
    createParam(PVCamSlot3CamString,                 asynParamOctet,   &PVCamSlot3Cam);
    createParam(PVCamDetectorSelectedString,         asynParamInt32,   &PVCamDetectorSelected);
    createParam(PVCamChipNameRBVString,              asynParamOctet,   &PVCamChipNameRBV);
    createParam(PVCamNumParallelPixelsRBVString,     asynParamInt32,   &PVCamNumParallelPixelsRBV);
    createParam(PVCamNumSerialPixelsRBVString,       asynParamInt32,   &PVCamNumSerialPixelsRBV);
    createParam(PVCamPixelParallelSizeRBVString,     asynParamInt32,   &PVCamPixelParallelSizeRBV);
    createParam(PVCamPixelSerialSizeRBVString,       asynParamInt32,   &PVCamPixelSerialSizeRBV);
    createParam(PVCamChipHeightMMRBVString,          asynParamFloat64, &PVCamChipHeightMMRBV);
    createParam(PVCamChipWidthMMRBVString,           asynParamFloat64, &PVCamChipWidthMMRBV);
    createParam(PVCamPixelParallelDistRBVString,     asynParamInt32,   &PVCamPixelParallelDistRBV);
    createParam(PVCamPixelSerialDistRBVString,       asynParamInt32,   &PVCamPixelSerialDistRBV);
    createParam(PVCamPostMaskRBVString,              asynParamInt32,   &PVCamPostMaskRBV);
    createParam(PVCamPreMaskRBVString,               asynParamInt32,   &PVCamPreMaskRBV);
    createParam(PVCamPostScanRBVString,              asynParamInt32,   &PVCamPostScanRBV);
    createParam(PVCamPreScanRBVString,               asynParamInt32,   &PVCamPreScanRBV);
    createParam(PVCamNumPortsRBVString,              asynParamInt32,   &PVCamNumPortsRBV);
    createParam(PVCamFullWellCapacityRBVString,      asynParamInt32,   &PVCamFullWellCapacityRBV);
    createParam(PVCamFrameTransferCapableRBVString,  asynParamInt32,   &PVCamFrameTransferCapableRBV);
    createParam(PVCamNumSpeedTableEntriesRBVString,  asynParamInt32,   &PVCamNumSpeedTableEntriesRBV);
    createParam(PVCamSpeedTableIndexString,          asynParamInt32,   &PVCamSpeedTableIndex);
    createParam(PVCamSpeedTableIndexRBVString,       asynParamInt32,   &PVCamSpeedTableIndexRBV);
    createParam(PVCamBitDepthRBVString,              asynParamInt32,   &PVCamBitDepthRBV);
    createParam(PVCamPixelTimeRBVString,             asynParamInt32,   &PVCamPixelTimeRBV);
    createParam(PVCamGainIndexString,                asynParamInt32,   &PVCamGainIndex);
    createParam(PVCamGainIndexRBVString,             asynParamInt32,   &PVCamGainIndexRBV);
    createParam(PVCamMaxGainIndexRBVString,          asynParamInt32,   &PVCamMaxGainIndexRBV);
    createParam(PVCamMinShutterOpenDelayRBVString,   asynParamInt32,   &PVCamMinShutterOpenDelayRBV);
    createParam(PVCamMaxShutterOpenDelayRBVString,   asynParamInt32,   &PVCamMaxShutterOpenDelayRBV);
    createParam(PVCamMinShutterCloseDelayRBVString,  asynParamInt32,   &PVCamMinShutterCloseDelayRBV);
    createParam(PVCamMaxShutterCloseDelayRBVString,  asynParamInt32,   &PVCamMaxShutterCloseDelayRBV);
    createParam(PVCamShutterOpenDelayString,         asynParamInt32,   &PVCamShutterOpenDelay);
    createParam(PVCamShutterOpenDelayRBVString,      asynParamInt32,   &PVCamShutterOpenDelayRBV);
    createParam(PVCamShutterCloseDelayString,        asynParamInt32,   &PVCamShutterCloseDelay);
    createParam(PVCamShutterCloseDelayRBVString,     asynParamInt32,   &PVCamShutterCloseDelayRBV);
    createParam(PVCamMeasuredTemperatureRBVString,   asynParamFloat64, &PVCamMeasuredTemperatureRBV);
    createParam(PVCamMinTemperatureRBVString,        asynParamFloat64, &PVCamMinTemperatureRBV);
    createParam(PVCamMaxTemperatureRBVString,        asynParamFloat64, &PVCamMaxTemperatureRBV);
    createParam(PVCamSetTemperatureString,           asynParamFloat64, &PVCamSetTemperature);
    createParam(PVCamSetTemperatureRBVString,        asynParamFloat64, &PVCamSetTemperatureRBV);
    createParam(PVCamDetectorModeString,             asynParamInt32,   &PVCamDetectorMode);
    createParam(PVCamDetectorModeRBVString,          asynParamInt32,   &PVCamDetectorModeRBV);
    createParam(PVCamTriggerModeString,              asynParamInt32,   &PVCamTriggerMode);
    createParam(PVCamTriggerModeRBVString,           asynParamInt32,   &PVCamTriggerModeRBV);
    createParam(PVCamTriggerEdgeString,              asynParamInt32,   &PVCamTriggerEdge);
    createParam(PVCamTriggerEdgeRBVString,           asynParamInt32,   &PVCamTriggerEdgeRBV);
    createParam(PVCamCamFirmwareVersRBVString,       asynParamOctet,   &PVCamCamFirmwareVersRBV);
    createParam(PVCamPCIFWVersRBVString,             asynParamInt32,   &PVCamPCIFWVersRBV);
    createParam(PVCamHeadSerialNumRBVString,         asynParamOctet,   &PVCamHeadSerialNumRBV);
    createParam(PVCamSerialNumRBVString,             asynParamInt32,   &PVCamSerialNumRBV);
    createParam(PVCamPVCamVersRBVString,             asynParamOctet,   &PVCamPVCamVersRBV);
    createParam(PVCamDevDrvVersRBVString,            asynParamOctet,   &PVCamDevDrvVersRBV);

    /* Create the epicsEvents for signaling to the simulate task when acquisition starts and stops */
    this->startEventId = epicsEventCreate(epicsEventEmpty);
    if (!this->startEventId)
    {
        ERR("epicsEventCreate failure for start event");
        return;
    }
    
    this->stopEventId = epicsEventCreate(epicsEventEmpty);
    if (!this->stopEventId)
    {
        ERR("epicsEventCreate failure for stop event");
        return;
    }

    /* Allocate the raw buffer we use to compute images.  Only do this once */
    dims[0] = maxSizeX;
    dims[1] = maxSizeY;
    this->pRaw = (NDArray *) this->pNDArrayPool->alloc(2, dims, dataType, 0, NULL);

    if (!pl_pvcam_init ())
    {
        reportPvCamError(functionName, "pl_pvcam_init");
        exit (1);
    }

    /* Set some default values for parameters */
    status =  setStringParam (ADManufacturer, "PI/Acton");
    status |= setStringParam (ADModel, "PVCam Cameras");
    status |= setIntegerParam(ADMaxSizeX, maxSizeX);
    status |= setIntegerParam(ADMaxSizeY, maxSizeY);
    status |= setIntegerParam(ADSizeX, maxSizeX);
    status |= setIntegerParam(ADSizeX, maxSizeX);
    status |= setIntegerParam(ADSizeY, maxSizeY);
    status |= setIntegerParam(NDArraySizeX, maxSizeX);
    status |= setIntegerParam(NDArraySizeY, maxSizeY);
    status |= setIntegerParam(NDArraySize, 0);
    status |= setIntegerParam(NDDataType, dataType);
    status |= setIntegerParam(ADImageMode, ADImageContinuous);
    status |= setDoubleParam (ADAcquireTime, .001);
    status |= setDoubleParam (ADAcquirePeriod, .005);
    status |= setIntegerParam(ADNumImages, 100);

//    status |= setIntegerParam(PVCamGainIndex, 11);
//    status |= setIntegerParam(PVCamGainIndexRBV, 11);

    status |= setStringParam(PVCamCamFirmwareVersRBV, "Unknown");
    status |= setStringParam(PVCamHeadSerialNumRBV, "Unknown");
    status |= setStringParam(PVCamPVCamVersRBV, "Unknown");
    status |= setStringParam(PVCamDevDrvVersRBV, "Unknown");
    status |= setIntegerParam(PVCamPCIFWVersRBV, -1);
    status |= setIntegerParam(PVCamSerialNumRBV, -1);

    if (status)
    {
        ERR("unable to set camera parameters");
        return;
    }

    callParamCallbacks();

    initializeDetectorInterface ();
    selectDetector (1);
    queryCurrentSettings ();
    initializeDetector ();

    /* Create the thread that updates the images */
    status = (epicsThreadCreate("PvCamAcquisitionTask",
                                epicsThreadPriorityMedium,
                                epicsThreadGetStackSize(epicsThreadStackMedium),
                                (EPICSTHREADFUNC)pvCamAcquisitionTaskC,
                                this) == NULL);
    
    if (status)
    {
        ERR("epicsThreadCreate failure for acquisition task");
        return;
    }

    /* Create the thread that monitors the temperature, etc...*/
    status = (epicsThreadCreate("PvCamMonitorTask",
                                epicsThreadPriorityMedium,
                                epicsThreadGetStackSize(epicsThreadStackMedium),
                                (EPICSTHREADFUNC)pvCamMonitorTaskC,
                                this) == NULL);
    
    if (status)
    {
        ERR("epicsThreadCreate failure for monitor task");
        return;
    }

}

//_____________________________________________________________________________________________

template <typename epicsType> int ADPvCam::computeArray(int maxSizeX, int maxSizeY)
{
    epicsType *pData = (epicsType *)this->pRaw->pData;
    int status = asynSuccess;
    int sizeX, sizeY;

    status |= getIntegerParam(ADSizeX, &sizeX);
    status |= getIntegerParam(ADSizeY, &sizeY);

    for (int loopy=0; loopy<sizeY; loopy++)
    {
        for (int loopx=0; loopx<sizeX; loopx++)
        {
            (*pData++) = (epicsType) rawData[(loopy*sizeX)+loopx];
        }
    }
        

    return status;
}

//_____________________________________________________________________________________________

int ADPvCam::allocateBuffer()
{
    int status = asynSuccess;
    NDArrayInfo_t arrayInfo;

    /* Make sure the raw array we have allocated is large enough.
     * We are allowed to change its size because we have exclusive use of it */
    this->pRaw->getInfo(&arrayInfo);
    
    if (arrayInfo.totalBytes > this->pRaw->dataSize)
    {
        free(this->pRaw->pData);
        this->pRaw->pData  = malloc(arrayInfo.totalBytes);
        this->pRaw->dataSize = arrayInfo.totalBytes;
        if (!this->pRaw->pData) status = asynError;
    }
    
    return status;
}

//_____________________________________________________________________________________________

int ADPvCam::computeImage()
{
    int status = asynSuccess;
    NDDataType_t dataType;
    int binX, binY, minX, minY, sizeX, sizeY, reverseX, reverseY;
    int maxSizeX, maxSizeY;
    NDDimension_t dimsOut[2];
    NDArrayInfo_t arrayInfo;
    NDArray *pImage;
    const char* functionName = "computeImage";

    /* NOTE: The caller of this function must have taken the mutex */

    status |= getIntegerParam(ADBinX,         &binX);
    status |= getIntegerParam(ADBinY,         &binY);
    status |= getIntegerParam(ADMinX,         &minX);
    status |= getIntegerParam(ADMinY,         &minY);
    status |= getIntegerParam(ADSizeX,        &sizeX);
    status |= getIntegerParam(ADSizeY,        &sizeY);
    status |= getIntegerParam(ADReverseX,     &reverseX);
    status |= getIntegerParam(ADReverseY,     &reverseY);
    status |= getIntegerParam(ADMaxSizeX,     &maxSizeX);
    status |= getIntegerParam(ADMaxSizeY,     &maxSizeY);
    status |= getIntegerParam(NDDataType,     (int *)&dataType);
    
    if (status) ERR("getting parameters");

    /* Make sure parameters are consistent, fix them if they are not */
    if (binX < 1)
    {
        binX = 1;
        status |= setIntegerParam(ADBinX, binX);
    }
    if (binY < 1)
    {
        binY = 1;
        status |= setIntegerParam(ADBinY, binY);
    }
    if (minX < 0)
    {
        minX = 0;
        status |= setIntegerParam(ADMinX, minX);
    }
    if (minY < 0)
    {
        minY = 0;
        status |= setIntegerParam(ADMinY, minY);
    }
    if (minX > maxSizeX-1)
    {
        minX = maxSizeX-1;
        status |= setIntegerParam(ADMinX, minX);
    }
    if (minY > maxSizeY-1)
    {
        minY = maxSizeY-1;
        status |= setIntegerParam(ADMinY, minY);
    }
    if (minX+sizeX > maxSizeX)
    {
        sizeX = maxSizeX-minX;
        status |= setIntegerParam(ADSizeX, sizeX);
    }
    if (minY+sizeY > maxSizeY)
    {
        sizeY = maxSizeY-minY;
        status |= setIntegerParam(ADSizeY, sizeY);
    }

    /* Make sure the buffer we have allocated is large enough. */
    this->pRaw->dataType = dataType;
    status = allocateBuffer();
    if (status)
    {
        ERR("error allocating raw buffer");
        return status;
    }
    
    switch (dataType)
    {
        case NDInt8:
            status |= computeArray<epicsInt8>(maxSizeX, maxSizeY);
            break;
        case NDUInt8:
            status |= computeArray<epicsUInt8>(maxSizeX, maxSizeY);
            break;
        case NDInt16:
            status |= computeArray<epicsInt16>(maxSizeX, maxSizeY);
            break;
        case NDUInt16:
            status |= computeArray<epicsUInt16>(maxSizeX, maxSizeY);
            break;
        case NDInt32:
            status |= computeArray<epicsInt32>(maxSizeX, maxSizeY);
            break;
        case NDUInt32:
            status |= computeArray<epicsUInt32>(maxSizeX, maxSizeY);
            break;
        case NDInt64:
            status |= computeArray<epicsInt64>(maxSizeX, maxSizeY);
            break;
        case NDUInt64:
            status |= computeArray<epicsUInt64>(maxSizeX, maxSizeY);
            break;
        case NDFloat32:
            status |= computeArray<epicsFloat32>(maxSizeX, maxSizeY);
            break;
        case NDFloat64:
            status |= computeArray<epicsFloat64>(maxSizeX, maxSizeY);
            break;
    }

    /* Extract the region of interest with binning.
     * If the entire image is being used (no ROI or binning) that's OK because
     * convertImage detects that case and is very efficient */
    this->pRaw->initDimension(&dimsOut[0], sizeX);
    dimsOut[0].binning = binX;
    dimsOut[0].offset = minX;
    dimsOut[0].reverse = reverseX;
    
    this->pRaw->initDimension(&dimsOut[1], sizeY);
    dimsOut[1].binning = binY;
    dimsOut[1].offset = minY;
    dimsOut[1].reverse = reverseY;
    
    /* We save the most recent image buffer so it can be used in the read() function.
     * Now release it before getting a new version. */
    if (this->pArrays[0])
        this->pArrays[0]->release();
    
    status = this->pNDArrayPool->convert(this->pRaw,
                                         &this->pArrays[0],
                                         dataType,
                                         dimsOut);
    if (status)
    {
        ERR("error allocating buffer in convert()");
        return(status);
    }
    
    pImage = this->pArrays[0];
    pImage->getInfo(&arrayInfo);
    
    status = asynSuccess;
    status |= setIntegerParam(NDArraySize,  (int)arrayInfo.totalBytes);
    status |= setIntegerParam(NDArraySizeX, (int)pImage->dims[0].size);
    status |= setIntegerParam(NDArraySizeY, (int)pImage->dims[1].size);

    if (status) ERR("error setting parameters");
    return status;
}

//_____________________________________________________________________________________________

/* This thread computes new image data and does the callbacks to send it to higher layers */
void ADPvCam::pvCamAcquisitionTask()
{
    const char *functionName = "pvCamAcquisitionTask()";
    int status = asynSuccess;
    int dataType;
    int imageSizeX, imageSizeY, imageSize;
    int imageCounter;
    int acquire, autoSave;
    NDArray *pImage;
    double acquireTime, acquirePeriod, delay;
    epicsTimeStamp startTime, endTime;
    double elapsedTime;
    int abort;

    this->lock();

    /* Loop forever */
    while (1)
    {
        abort = 0;
        /* Is acquisition active? */
        getIntegerParam(ADAcquire, &acquire);

        /* If we are not acquiring then wait for a semaphore that is given when acquisition is started */
        if (!acquire)
        {
            setIntegerParam(ADStatus, ADStatusIdle);
            callParamCallbacks();
            /* Release the lock while we wait for an event that says acquire has started, then lock again */
            this->unlock();
            LOG("waiting for acquire to start");
            status = epicsEventWait(this->startEventId);
            this->lock();
        }

        /* We are acquiring. */
        /* Get the current time */
        epicsTimeGetCurrent(&startTime);

        /* Get the exposure parameters */
        getDoubleParam(ADAcquireTime, &acquireTime);
        getDoubleParam(ADAcquirePeriod, &acquirePeriod);

        setIntegerParam(ADStatus, ADStatusAcquire);

        /* Call the callbacks to update any changes */
        callParamCallbacks();

        //Acquire Image Start
        if (!pl_exp_start_seq (detectorHandle, rawData))
            reportPvCamError(functionName, "pl_exp_start_seq");

        /* Wait for acquisition to complete, but allow acquire stop events to be handled */
        while (1)
        {
            this->unlock();
            status = epicsEventWaitWithTimeout(this->stopEventId, POLL_TIME);
            this->lock();

            if (status == epicsEventWaitOK)
            {
                /* We got a stop event, abort acquisition */
                printf("Got a stop event from somewhere...\n");

                if (!pl_exp_abort (detectorHandle, CCS_HALT))
                    reportPvCamError(functionName, "pl_exp_abort");

                acquire = 0;
                abort = 1;
                setIntegerParam(ADStatus, ADStatusReadout);
                callParamCallbacks();
                break;
            }
            else
                acquire = this->getAcquireStatus();

            if (acquire)
            {
                //printf ("Got 1!!!\n");
                break;
            }
        }
        //Acquire Image End

        if (abort) continue;
        
        /* Update the image */
        status = computeImage();
        if (status) continue;

        pImage = this->pArrays[0];

        epicsTimeGetCurrent(&endTime);
        elapsedTime = epicsTimeDiffInSeconds(&endTime, &startTime);

        /* Get the current parameters */
        getIntegerParam(NDArraySizeX, &imageSizeX);
        getIntegerParam(NDArraySizeY, &imageSizeY);
        getIntegerParam(NDArraySize,  &imageSize);
        getIntegerParam(NDDataType,   &dataType);
        getIntegerParam(NDAutoSave,   &autoSave);
        getIntegerParam(NDArrayCounter, &imageCounter);
        imageCounter++;
        setIntegerParam(NDArrayCounter, imageCounter);

        /* Put the frame number and time stamp into the buffer */
        pImage->uniqueId = imageCounter;
        pImage->timeStamp = startTime.secPastEpoch + startTime.nsec / 1.e9;
        updateTimeStamp(&pImage->epicsTS);

        /* Call the NDArray callback */
        LOG("calling imageData callback");
        doCallbacksGenericPointer(pImage, NDArrayData, 0);

        /* See if acquisition is done */
        if (this->imagesRemaining > 0)
            this->imagesRemaining--;

        if (this->imagesRemaining == 0)
        {
            setIntegerParam(ADAcquire, ADStatusIdle);
            LOG("acquisition completed");
        }

        /* Call the callbacks to update any changes */
        callParamCallbacks();

        /* If we are acquiring then sleep for the acquire period minus elapsed time. */
        if (acquire)
        {
            /* We set the status to readOut to indicate we are in the period delay */
            setIntegerParam(ADStatus, ADStatusReadout);
            callParamCallbacks();
            delay = acquirePeriod - elapsedTime;
            LOG_ARGS("delay=%f", delay);
            if (delay >= epicsThreadSleepQuantum())
                status = epicsEventWaitWithTimeout(this->stopEventId, delay);

        }
    }
}

//_____________________________________________________________________________________________

/* This thread computes new image data and does the callbacks to send it to higher layers */
void ADPvCam::pvCamMonitorTask()
{
    const char *functionName = "pvCamMonitorTask";
    int status = asynSuccess;
    int acquire=0;
    int16 i16Value;
    double measuredTemperature;

    /* Loop forever */
    this->lock();
    while (1)
    {
        this->unlock();
        epicsThreadSleep(1.0);
        this->lock();

        /* Are we idle? */
        getIntegerParam(ADAcquire, &acquire);

        /* If we are not acquiring then check the temperature */
        if (acquire == ADStatusIdle)
        {
            if (tempAvailable ) {
                if (!pl_get_param (detectorHandle, PARAM_TEMP, ATTR_CURRENT, (void *) &i16Value))
                    reportPvCamError (functionName, "pl_get_param (PARAM_TEMP, ATTR_CURRENT)");

                measuredTemperature = (double) i16Value / 100.0;
                status |= setDoubleParam(PVCamMeasuredTemperatureRBV, measuredTemperature);
            }
            callParamCallbacks();
        }
    }
}

//_____________________________________________________________________________________________

asynStatus ADPvCam::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    const char *functionName = "writeInt32";
    int function = pasynUser->reason;
    int adstatus;
    asynStatus status = asynSuccess;

    /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
     * status at the end, but that's OK */
    status = setIntegerParam(function, value);

    /* For a real detector this is where the parameter is sent to the hardware */
    if (function == ADAcquire)
    {
        getIntegerParam(ADStatus, &adstatus);
        if (value && (adstatus == ADStatusIdle))
        {
            
            /* We need to set the number of images we expect to collect, so the image callback function
               can know when acquisition is complete.  We need to find out what mode we are in and how
               many images have been requested.  If we are in continuous mode then set the number of
               remaining images to -1. */
            int imageMode, numImages;
            status = getIntegerParam(ADImageMode, &imageMode);
            status = getIntegerParam(ADNumImages, &numImages);
            switch(imageMode)
            {
                case ADImageSingle:
                    this->imagesRemaining = 1;
                    break;
                case ADImageMultiple:
                    this->imagesRemaining = numImages;
                    break;
                case ADImageContinuous:
                    this->imagesRemaining = -1;
                    break;
            }
            
            /* Send an event to wake up the simulation task.
             * It won't actually start generating new images until we release the lock below */
            epicsEventSignal(this->startEventId);
        }
        
        if (!value && (adstatus != ADStatusIdle))
        {
            /* This was a command to stop acquisition */
            /* Send the stop event */
            epicsEventSignal(this->stopEventId);
        }
    } else if (function == ADImageMode) {
        /* The image mode may have changed while we are acquiring,
         * set the images remaining appropriately. */
        switch (value)
        {
            case ADImageSingle: 
                this->imagesRemaining = 1;
                break;
            case ADImageMultiple:
                int numImages;
                getIntegerParam(ADNumImages, &numImages);
                this->imagesRemaining = numImages;
                break;
            case ADImageContinuous:
                this->imagesRemaining = -1;
                break;
        }
    } else if (function == PVCamInitDetector) {
        initializeDetector();
    }

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();

    if (status)
    {
        ERR_ARGS("error, status=%d function=%d, value=%d", status, function, value);
    }
    else
    {
        LOG_ARGS("function=%d, value=%d", function, value);
    }

    return status;
}

//_____________________________________________________________________________________________

asynStatus ADPvCam::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    const char *functionName = "writeFloat64";
    int function = pasynUser->reason;
    int status = asynSuccess;

    /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
     * status at the end, but that's OK */
    status = setDoubleParam(function, value);

    /* Changing any of the following parameters requires recomputing the base image */
    if (function == ADAcquireTime)
    {
        LOG_ARGS("Setting proposed exposure time to %e", value);
        status |= setDoubleParam(ADAcquireTime, value);
    }

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();
    
    if (status)
    {
        ERR_ARGS("error, status=%d function=%d, value=%f", status, function, value);
    }
    else
    {
        LOG_ARGS("function=%d, value=%f", function, value);
    }
    
    return (asynStatus) status;
}

//_____________________________________________________________________________________________

void ADPvCam::report(FILE *fp, int details)
{

    //const char* functionName = "report";

    fprintf(fp, "PVCam %s\n", this->portName);
    if (details > 0)
    {
        int nx, ny, dataType;
        getIntegerParam(ADSizeX, &nx);
        getIntegerParam(ADSizeY, &ny);
        getIntegerParam(NDDataType, &dataType);
        fprintf(fp, "  NX, NY:            %d  %d\n", nx, ny);
        fprintf(fp, "  Data type:         %d\n", dataType);
    }

    /* Invoke the base class method */
    ADDriver::report(fp, details);
}

//_____________________________________________________________________________________________

ADPvCam::~ADPvCam()
{
    const char *functionName = "~ADPvCam";

    if (rawData != NULL)
        free (rawData);

    //if (!pl_exp_uninit_seq())
    //    reportPvCamError (functionName, "pl_exp_uninit_seq ()");

    if (!pl_cam_close (detectorHandle))
        reportPvCamError(functionName, "pl_cam_close");

    if (!pl_pvcam_uninit ())
        reportPvCamError(functionName, "pl_pvcam_uninit");

}

//_____________________________________________________________________________________________
//_____________________________________________________________________________________________
//Private methods
//_____________________________________________________________________________________________
//_____________________________________________________________________________________________


void ADPvCam::reportPvCamError(const char *functionName, const char *appMessage)
{
    int16 errorCode;
    char errorMessage[256];

    errorCode = pl_error_code();
    pl_error_message(errorCode, errorMessage);
    printf("ERROR in %s->%s: errorCode %d -- %s\n", functionName, appMessage, errorCode, errorMessage);
}

//_____________________________________________________________________________________________

void ADPvCam::initializeDetectorInterface(void)
{
    const char *functionName = "initializeDetectorInterface";
    int status = asynSuccess;
    printf("\n\n\nInitialize detector interface...\n");

    for (int loop = 0;loop<MAX_DETECTORS_SUPPORTED;loop++)
        detectorList[loop] = NULL;

    for (int loop=0;loop<MAX_DETECTORS_SUPPORTED;loop++)
    {
        if (detectorList[loop] != NULL)
        {
            free (detectorList[loop]);
            detectorList[loop] = NULL;
        }
        detectorList[loop] = (char *) malloc(50);

        strcpy (detectorList[loop], "Empty");
    }

    if (!pl_cam_get_total (&numDetectorsInstalled))
        reportPvCamError (functionName, "pl_cam_get_total");

    if (numDetectorsInstalled > MAX_DETECTORS_SUPPORTED)
        numDetectorsInstalled = MAX_DETECTORS_SUPPORTED;

    printf("%d detectors installed...\n", numDetectorsInstalled);
    for (int loop=0;loop<numDetectorsInstalled;loop++)
    {
        if (detectorList[loop] != NULL)
        {
            free (detectorList[loop]);
            detectorList[loop] = NULL;
        }
        
        detectorList[loop] = (char *) malloc(50);

        if (!pl_cam_get_name (loop, detectorList[loop]))
            reportPvCamError (functionName, "pl_cam_get_name");
        
        printf ("Detector[%d] = %s\n", loop, detectorList[loop]);
    }


    status |= setStringParam(PVCamSlot1Cam, detectorList[0]);
    status |= setStringParam(PVCamSlot2Cam, detectorList[1]);
    status |= setStringParam(PVCamSlot3Cam, detectorList[2]);

    if (status)
    {
        ERR("unable to set camera parameters");
        return;
    }

    /* Call the callbacks to update any changes */
    callParamCallbacks();

    printf("...interface initialized.\n\n\n\n");

}

//_____________________________________________________________________________________________

void ADPvCam::selectDetector(int selectedDetector)
{
    const char *functionName = "selectDetector";
    int status = asynSuccess;

    printf("Selecting detector %d\n", selectedDetector);

    if ((selectedDetector <= numDetectorsInstalled) && (selectedDetector > 0))
    {
        detectorSelected = selectedDetector;

        if (detectorHandle != 0)
        {
            //if (!pl_exp_uninit_seq())
            //    reportPvCamError (functionName, "pl_exp_uninit_seq ()");

            if (!pl_cam_close (detectorHandle))
                reportPvCamError (functionName, "pl_cam_close ()");
        }

        //Open camera...
        printf("Opening camera %s\n", detectorList[detectorSelected-1]);
        if (!pl_cam_open (detectorList[detectorSelected-1], &detectorHandle, OPEN_EXCLUSIVE))
            reportPvCamError (functionName, "pl_cam_open");
        
        // Depracated in newer SDK versions
        //if (!pl_cam_get_diags (detectorHandle))
        //    reportPvCamError (functionName, "pl_cam_get_diags");

        //if (!pl_exp_init_seq())
        //    reportPvCamError (functionName, "pl_cam_init_seq");

        status |= setIntegerParam(PVCamDetectorSelected, detectorSelected);

        queryCurrentSettings();
    }
}

//_____________________________________________________________________________________________

void ADPvCam::queryCurrentSettings (void)
{
    const char *functionName = "queryCurrentSettings";
    int status = asynSuccess;
    uns16 ui16Value;
    uns32 ui32Value;
    int16 i16Value,
          parallelSize,
          pixelParallelSize,
          serialSize,
          pixelSerialSize;
    double dValue;
    char cValue[CCD_NAME_LEN];
    rs_bool paramAvail;
    const char *availStr[] = {"NO", "YES"};

    printf("\n\n\nBegin detector ...\n");


    status |= setIntegerParam(PVCamInitDetector, 0);

    //Query open camera parameters
    if (!pl_get_param (detectorHandle, PARAM_CHIP_NAME, ATTR_COUNT, (void *) &ui32Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_CHIP_NAME, ATTR_COUNT)");
    else
    {
        if ( ui32Value <= CCD_NAME_LEN )
        {
            if (!pl_get_param (detectorHandle, PARAM_CHIP_NAME, ATTR_CURRENT, (void *) cValue))
                reportPvCamError (functionName, "pl_get_param (PARAM_CHIP_NAME, ATTR_CURRENT)");
        
            printf("Chip name: %s\n", cValue);
            status |= setStringParam(PVCamChipNameRBV, cValue);
        }
        else
        {
            sprintf(cValue, "%s", "unknown");
            status |= setStringParam(PVCamChipNameRBV, cValue);
            reportPvCamError (functionName, "pl_get_param (PARAM_CHIP_NAME, ATTR_CURRENT)\n");
            reportPvCamError (functionName, "Name is too long for storage allotted\n");
            printf("PARAM_CHIP_NAME ATTR_COUNT = %d, CCD_NAME_LEN = %d\n", (int)ui32Value, CCD_NAME_LEN);
        }
    }

    //Num pixels
    if (!pl_get_param (detectorHandle, PARAM_PAR_SIZE, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_PAR_SIZE, ATTR_CURRENT)");
    
    printf("Parallel size: %d\n", ui16Value);
    status |= setIntegerParam(PVCamNumParallelPixelsRBV, ui16Value);
    parallelSize = ui16Value;

    if (!pl_get_param (detectorHandle, PARAM_SER_SIZE, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_SER_SIZE, ATTR_CURRENT)");
    
    printf("Serial size: %d\n", ui16Value);
    status |= setIntegerParam(PVCamNumSerialPixelsRBV, ui16Value);
    serialSize = ui16Value;


    //Pixel size
    if (!pl_get_param (detectorHandle, PARAM_PIX_PAR_SIZE, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_PIX_PAR_SIZE, ATTR_CURRENT)");
    
    printf("Parallel pixel size: %d\n", ui16Value);
    status |= setIntegerParam(PVCamPixelParallelSizeRBV, ui16Value);
    pixelParallelSize = ui16Value;

    if (!pl_get_param (detectorHandle, PARAM_PIX_SER_SIZE, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_PIX_SER_SIZE, ATTR_CURRENT)");
    
    printf("Serial pixel size: %d\n", ui16Value);
    status |= setIntegerParam(PVCamPixelSerialSizeRBV, ui16Value);
    pixelSerialSize = ui16Value;


    //Calculated chip dims in mm
    dValue = parallelSize * (pixelParallelSize / 1000.0 / 1000.0);
    printf("width: %f\n", dValue);
    status |= setDoubleParam(PVCamChipWidthMMRBV, dValue);

    dValue = serialSize * (pixelSerialSize / 1000.0 / 1000.0);
    printf("height: %f\n", dValue);
    status |= setDoubleParam(PVCamChipHeightMMRBV, dValue);


    //Pixel distance
    if (!pl_get_param (detectorHandle, PARAM_PIX_PAR_DIST, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_PIX_PAR_DIST, ATTR_CURRENT)");
    
    printf("Parallel pixel dist: %d\n", ui16Value);
    status |= setIntegerParam(PVCamPixelParallelDistRBV, ui16Value);

    if (!pl_get_param (detectorHandle, PARAM_PIX_SER_DIST, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_PIX_SER_DIST, ATTR_CURRENT)");
    
    printf("Serial pixel dist: %d\n", ui16Value);
    status |= setIntegerParam(PVCamPixelSerialDistRBV, ui16Value);


    //Pre/PostMask
    if (!pl_get_param (detectorHandle, PARAM_POSTMASK, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_POSTMASK, ATTR_CURRENT)");
    
    printf("postmask: %d\n", ui16Value);
    status |= setIntegerParam(PVCamPostMaskRBV, ui16Value);

    if (!pl_get_param (detectorHandle, PARAM_PREMASK, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_PREMASK, ATTR_CURRENT)");
    
    printf("premask: %d\n", ui16Value);
    status |= setIntegerParam(PVCamPreMaskRBV, ui16Value);


    //Pre/PostScan
    if (!pl_get_param (detectorHandle, PARAM_POSTSCAN, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_POSTSCAN, ATTR_CURRENT)");
    
    printf("postscan: %d\n", ui16Value);
    status |= setIntegerParam(PVCamPostScanRBV, ui16Value);

    if (!pl_get_param (detectorHandle, PARAM_PRESCAN, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError (functionName, "pl_get_param (PARAM_PRESCAN, ATTR_CURRENT)");
    
    printf("prescan: %d\n", ui16Value);
    status |= setIntegerParam(PVCamPreScanRBV, ui16Value);


    //pre/post shutter compensation
    if (!pl_get_param(detectorHandle, PARAM_SHTR_OPEN_DELAY, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError (functionName, "pl_get_param(PARAM_SHTR_OPEN_DELAY, ATTR_AVAIL)");
    
    printf("Open Shutter delay available: %s\n", availStr[paramAvail]);
    
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_SHTR_OPEN_DELAY, ATTR_MIN, (void *) &ui16Value))
            reportPvCamError (functionName, "pl_get_param (PARAM_SHTR_OPEN_DELAY, ATTR_MIN)");
    
        printf("Min shutter open delay: %d\n", ui16Value);
        status |= setIntegerParam(PVCamMinShutterOpenDelayRBV, ui16Value);

        if (!pl_get_param (detectorHandle, PARAM_SHTR_OPEN_DELAY, ATTR_MAX, (void *) &ui16Value))
            reportPvCamError (functionName, "pl_get_param (PARAM_SHTR_OPEN_DELAY, ATTR_MAX)");
    
        printf("Max shutter open delay: %d\n", ui16Value);
        status |= setIntegerParam(PVCamMaxShutterOpenDelayRBV, ui16Value);

        if (!pl_get_param (detectorHandle, PARAM_SHTR_OPEN_DELAY, ATTR_CURRENT, (void *) &ui16Value))
            reportPvCamError (functionName, "pl_get_param (PARAM_SHTR_OPEN_DELAY, ATTR_CURRENT)");
        printf("Current shutter open delay: %d\n", ui16Value);
        status |= setIntegerParam(PVCamShutterOpenDelay, ui16Value);
        status |= setIntegerParam(PVCamShutterOpenDelayRBV, ui16Value);
    }
    else {
        ui16Value = (uns16)0;
        status |= setIntegerParam(PVCamMinShutterOpenDelayRBV, ui16Value);
        status |= setIntegerParam(PVCamMaxShutterOpenDelayRBV, ui16Value);
        status |= setIntegerParam(PVCamShutterOpenDelay, ui16Value);
        status |= setIntegerParam(PVCamShutterOpenDelayRBV, ui16Value);
    }

    if (!pl_get_param(detectorHandle, PARAM_SHTR_CLOSE_DELAY, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError (functionName, "pl_get_param(PARAM_SHTR_CLOSE_DELAY, ATTR_AVAIL)");
    
    printf("Close Shutter delay available: %s\n", availStr[paramAvail]);
    
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_SHTR_CLOSE_DELAY, ATTR_MIN, (void *) &ui16Value))
            reportPvCamError (functionName, "pl_get_param (PARAM_SHTR_CLOSE_DELAY, ATTR_MIN)");
        
        printf("Min shutter close delay: %d\n", ui16Value);
        status |= setIntegerParam(PVCamMinShutterCloseDelayRBV, ui16Value);

        if (!pl_get_param (detectorHandle, PARAM_SHTR_CLOSE_DELAY, ATTR_MAX, (void *) &ui16Value))
            reportPvCamError (functionName, "pl_get_param (PARAM_SHTR_OPEN_DELAY, ATTR_MAX)");
        
        printf("Max shutter close delay: %d\n", ui16Value);
        status |= setIntegerParam(PVCamMaxShutterCloseDelayRBV, ui16Value);

        if (!pl_get_param (detectorHandle, PARAM_SHTR_CLOSE_DELAY, ATTR_CURRENT, (void *) &ui16Value))
            reportPvCamError (functionName, "pl_get_param (PARAM_SHTR_CLOSE_DELAY, ATTR_CURRENT)");
        
        printf("Current shutter close delay: %d\n", ui16Value);
        status |= setIntegerParam(PVCamShutterCloseDelay, ui16Value);
        status |= setIntegerParam(PVCamShutterCloseDelayRBV, ui16Value);
    }
    else
    {
        ui16Value = (uns16)0;
        status |= setIntegerParam(PVCamMinShutterCloseDelayRBV, ui16Value);
        status |= setIntegerParam(PVCamMaxShutterCloseDelayRBV, ui16Value);
        status |= setIntegerParam(PVCamShutterCloseDelay, ui16Value);
        status |= setIntegerParam(PVCamShutterCloseDelayRBV, ui16Value);
    }


    //Full well capacity
    if (!pl_get_param(detectorHandle, PARAM_FWELL_CAPACITY, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError (functionName, "pl_get_param(PARAM_FWELL_CAPACITY, ATTR_AVAIL)");
    
    printf("Full Well Capacity available: %s\n", availStr[paramAvail]);
    
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_FWELL_CAPACITY, ATTR_MAX, (void *) &ui32Value))
            reportPvCamError(functionName, "pl_get_param (PARAM_FWELL_CAPACITY,  ATTR_MAX)");
        
        printf("Full well capacity: %d\n", (int)ui32Value);
        status |= setIntegerParam(PVCamFullWellCapacityRBV, ui32Value);
    }
    else
    {
        ui32Value = (uns32)0;
        status |= setIntegerParam(PVCamFullWellCapacityRBV, ui32Value);
    }


    //Number of ports
    if (!pl_get_param (detectorHandle, PARAM_SPDTAB_INDEX, ATTR_MAX, (void *) &ui16Value))
        reportPvCamError(functionName, "pl_get_param (PARAM_SPDTAB_INDEX, ATTR_MAX)");
    
    printf("Total ports: %d\n", ui16Value);
    status |= setIntegerParam(PVCamNumPortsRBV, ui16Value);


    //Get transfer capable
    if (!pl_get_param(detectorHandle, PARAM_FRAME_CAPABLE, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError(functionName, "pl_get_param(PARAM_FRAME_CAPABLE, ATTR_AVAIL)");
    
    printf("Frame Capable available: %s\n", availStr[paramAvail]);
    
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_FRAME_CAPABLE, ATTR_AVAIL, (void *) &ui16Value))
            reportPvCamError(functionName, "pl_get_param (PARAM_FRAME_CAPABLE, ATTR_AVAIL)");
        
        printf("Frame capable: %d\n", ui16Value);
        status |= setIntegerParam(PVCamFrameTransferCapableRBV, ui16Value);
    }
    else
    {
        ui16Value = (uns16)0;
        status |= setIntegerParam(PVCamFullWellCapacityRBV, ui16Value);
    }

    //Get speed table entries
    if (!pl_get_param (detectorHandle, PARAM_SPDTAB_INDEX, ATTR_MAX, (void *) &ui16Value))
        reportPvCamError(functionName, "pl_get_param (PARAM_SPDTAB_INDEX, ATTR_MAX)");
    
    printf("Speed table entries: %d\n", ui16Value);
    status |= setIntegerParam(PVCamNumSpeedTableEntriesRBV, ui16Value);

    if (!pl_get_param (detectorHandle, PARAM_SPDTAB_INDEX, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError(functionName, "pl_get_param (PARAM_SPDTAB_INDEX, ATTR_CURRENT)");
    
    printf("Speed table index: %d\n", ui16Value);
    status |= setIntegerParam(PVCamSpeedTableIndex, ui16Value);
    status |= setIntegerParam(PVCamSpeedTableIndexRBV, ui16Value);


    //Get max gain
    if (!pl_get_param (detectorHandle, PARAM_GAIN_INDEX, ATTR_MAX, (void *) &ui16Value))
        reportPvCamError(functionName, "pl_get_param (PARAM_GAIN_INDEX, ATTR_MAX)");
    
    printf("Max gain index: %d\n", ui16Value);
    status |= setIntegerParam(PVCamMaxGainIndexRBV, ui16Value);


    //Get gain index
    if (!pl_get_param (detectorHandle, PARAM_GAIN_INDEX, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError(functionName, "pl_get_param (PARAM_SPDTAB_INDEX)");
    
    printf("Current gain index: %d\n", ui16Value);
    status |= setIntegerParam(PVCamGainIndex, ui16Value);
    status |= setIntegerParam(PVCamGainIndexRBV, ui16Value);


    //Get bits
    if (!pl_get_param (detectorHandle, PARAM_BIT_DEPTH, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError(functionName, "pl_get_param (PARAM_BIT_DEPTH, ATTR_CURRENT)");
    
    printf("Bit depth: %d\n", ui16Value);
    status |= setIntegerParam(PVCamBitDepthRBV, ui16Value);


    //Get pixel time
    if (!pl_get_param (detectorHandle, PARAM_PIX_TIME, ATTR_CURRENT, (void *) &ui16Value))
        reportPvCamError(functionName, "pl_get_param (PARAM_PIX_TIME, ATTR_CURRENT)");
    
    printf("Pixel time: %d\n", ui16Value);
    status |= setIntegerParam(PVCamPixelTimeRBV, ui16Value);


    //temperature
    if (!pl_get_param(detectorHandle, PARAM_TEMP, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError(functionName, "pl_get_param(PARAM_TEMP, ATTR_AVAIL)");
    
    printf("Temperature available: %s\n", availStr[paramAvail]);
    
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_TEMP, ATTR_CURRENT, (void *) &i16Value))
            reportPvCamError(functionName, "pl_get_param (PARAM_TEMP, ATTR_CURRENT)");
        
        dValue = (double) i16Value / 100.0;
        printf("Measured temperature: %f\n", dValue);
        status |= setDoubleParam(PVCamMeasuredTemperatureRBV, dValue);

        if (!pl_get_param (detectorHandle, PARAM_TEMP, ATTR_MIN, (void *) &i16Value))
            reportPvCamError(functionName, "pl_get_param (PARAM_TEMP, ATTR_CURRENT)");
        
        dValue = (double) i16Value / 100.0;
        printf("Min temperature: %f\n", dValue);
        status |= setDoubleParam(PVCamMinTemperatureRBV, dValue);

        if (!pl_get_param (detectorHandle, PARAM_TEMP, ATTR_MAX, (void *) &i16Value))
            reportPvCamError(functionName, "pl_get_param (PARAM_TEMP, ATTR_CURRENT)");
        
        dValue = (double) i16Value / 100.0;
        printf("Max temperature: %f\n", dValue);
        status |= setDoubleParam(PVCamMaxTemperatureRBV, dValue);
    }
    else
    {
        dValue = 0.0;
        status |= setDoubleParam(PVCamMeasuredTemperatureRBV, dValue);
        status |= setDoubleParam(PVCamMinTemperatureRBV, dValue);
        status |= setDoubleParam(PVCamMaxTemperatureRBV, dValue);
    }

    if (!pl_get_param(detectorHandle, PARAM_TEMP_SETPOINT, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError(functionName, "pl_get_param(PARAM_TEMP_SETPOINT, ATTR_AVAIL)");
    
    printf("Temperature Setpoint available: %s\n", availStr[paramAvail]);
    
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_TEMP_SETPOINT, ATTR_CURRENT, (void *) &i16Value))
            reportPvCamError(functionName, "pl_get_param (PARAM_TEMP_SETPOINT, ATTR_CURRENT)");
        
        dValue = (double) i16Value / 100.0;
        printf("Set temperature: %f\n", dValue);
        status |= setDoubleParam(PVCamSetTemperature, dValue);
        status |= setDoubleParam(PVCamSetTemperatureRBV, dValue);
    }
    else
    {
        dValue = 0.0;
        status |= setDoubleParam(PVCamSetTemperature, dValue);
        status |= setDoubleParam(PVCamSetTemperatureRBV, dValue);
    }

    //Detector Mode
    if (!pl_get_param (detectorHandle, PARAM_PMODE, ATTR_CURRENT, (void *) &ui32Value))
        reportPvCamError(functionName, "pl_get_param (PARAM_PMODE, ATTR_CURRENT)");
    
    printf("Detector Mode: %d\n", ui16Value);
    status |= setIntegerParam(PVCamDetectorMode, ui32Value);
    status |= setIntegerParam(PVCamDetectorModeRBV, ui32Value);



    /*
    //Trigger Edge
    if (!pl_get_param(detectorHandle, PARAM_EDGE_TRIGGER, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError (functionName, "pl_get_param(PARAM_EDGE_TRIGGER, ATTR_AVAIL)");
    
    printf ("Trigger avail: %s\n", availStr[paramAvail]);
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_EDGE_TRIGGER, ATTR_CURRENT, (void *) &ui32Value))
            reportPvCamError (functionName, "pl_get_param (PARAM_EDGE_TRIGGER, ATTR_CURRENT)");
        printf ("Trigger edge: %d\n", ui16Value);

        status |= setIntegerParam(PVCamTriggerEdge, ui32Value);
        status |= setIntegerParam(PVCamTriggerEdgeRBV, ui32Value);
    }
    else
    {
        printf ("Trigger edge status is not available\n");

        status |= setIntegerParam(PVCamTriggerEdge, 0);
        status |= setIntegerParam(PVCamTriggerEdgeRBV, 0);
    }
    */


    uns16 fwVersion;
    //device firmware version
    if (PV_OK != pl_get_param(detectorHandle, PARAM_HEAD_SER_NUM_ALPHA, ATTR_CURRENT, (void *)&fwVersion))
        reportPvCamError(functionName, "pl_get_param PARAM_HEAD_SER_NUM_ALPHA");
    
    sprintf(cValue, "%d.%d", (fwVersion >> 8) & 0xFF, (fwVersion >> 0) & 0xFF );
    printf("Device Driver Version %s\n", cValue);
    status |= setStringParam(PVCamDevDrvVersRBV, cValue);

    //PV Cam version
    if (!pl_pvcam_get_ver(&ui16Value))
        reportPvCamError(functionName, "pl_pvcam_get_ver");

    sprintf(cValue, "%d.%d.%d", (0xFF00&ui16Value)>>8, (0x00F0&ui16Value)>>4, (0x000F&ui16Value) );
    printf("PVCam Version %s\n", cValue);
    status |= setStringParam(PVCamPVCamVersRBV, cValue);



    //Camera Firmware revision
    if (!pl_get_param(detectorHandle, PARAM_CAM_FW_VERSION, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError(functionName, "pl_get_param(PARAM_CAM_FW_VERSION, ATTR_AVAIL)");
    
    printf("Camera Firmware version available: %s\n", availStr[paramAvail]);
    
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_CAM_FW_VERSION, ATTR_CURRENT, (void *) &ui16Value))
            reportPvCamError(functionName, "pl_get_param (PARAM_CAM_FW_VERSION, ATTR_CURRENT)");

        sprintf(cValue, "%d.%d", (0xFF00&ui16Value)>>8, (0x00FF&ui16Value) );
    }
    else
        sprintf(cValue, "%s", "unknown");

    printf("Camera Firmware Version %s\n", cValue);
    status |= setStringParam(PVCamCamFirmwareVersRBV, cValue);

    //Head Serial Number
    if (!pl_get_param(detectorHandle, PARAM_HEAD_SER_NUM_ALPHA, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError(functionName, "pl_get_param(PARAM_HEAD_SER_NUM_ALPHA, ATTR_AVAIL)");
    
    printf("Head Serial Number available: %s\n", availStr[paramAvail]);
    
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_HEAD_SER_NUM_ALPHA, ATTR_CURRENT, (void *) &cValue))
            reportPvCamError(functionName, "pl_get_param (PARAM_HEAD_SER_NUM_ALPHA, ATTR_CURRENT)");
    }
    else
        sprintf(cValue, "%s", "unknown");

    printf("Head Serial Number %s\n", cValue);
    status |= setStringParam(PVCamHeadSerialNumRBV, cValue);

    /*/ Serial Number
    if (!pl_get_param(detectorHandle, PARAM_SERIAL_NUM, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError (functionName, "pl_get_param(PARAM_SERIAL_NUM, ATTR_AVAIL)");
    printf ("Serial Number available: %s\n", availStr[paramAvail]);
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_SERIAL_NUM, ATTR_CURRENT, (void *) &ui16Value)) {
            reportPvCamError (functionName, "pl_get_param (PARAM_SERIAL_NUM, ATTR_CURRENT)");
        }
        printf("Serial Number %d\n", ui16Value);

    }
    
    else {
        ui16Value = (uns16)0;
    }
    */
    // Serial num param doesnt exist in new SDK version
    ui16Value = (uns16) 0;
    status |= setIntegerParam(PVCamSerialNumRBV, ui16Value);

    // PCI FirmwareVersion
    if (!pl_get_param(detectorHandle, PARAM_PCI_FW_VERSION, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError(functionName, "pl_get_param(PARAM_PCI_FW_AVAIL, ATTR_AVAIL)");

    printf("PCI Firmware version available: %s\n", availStr[paramAvail]);
    
    if (paramAvail)
    {
        if (!pl_get_param (detectorHandle, PARAM_PCI_FW_VERSION, ATTR_CURRENT, (void *) &ui16Value))
            reportPvCamError(functionName, "pl_get_param (PARAM_PCI_FW_VERSION, ATTR_CURRENT)");

        printf("PARAM_PCI_FW_VERSION %d\n", ui16Value);
    }
    else
        ui16Value = (uns16) 0;

    status |= setIntegerParam(PVCamPCIFWVersRBV, ui16Value);



    /* Call the callbacks to update any changes */
    callParamCallbacks();

    printf("...all current values retrieved.\n\n\n\n");

}

//_____________________________________________________________________________________________

void ADPvCam::initializeDetector (void)
{
    const char *functionName = "initializeDetector";

    int status = asynSuccess;

    rgn_type roi;
    uns32           rawDataSize;

    int32 int16Parm,
          int16Parm2;

    int binX,
        binY,
        minX,
        minY,
        sizeX,
        sizeY,
        width,
        height,
        iValue;

    double dValue;
    rs_bool paramAvail;

    printf("Initilizing hardware...\n");

    status |= getIntegerParam(PVCamNumSerialPixelsRBV, &width);
    status |= getIntegerParam(PVCamNumParallelPixelsRBV, &height);

    printf("width: %d, height: %d\n", width, height);


    //Camera Mode
    status |= getIntegerParam(PVCamDetectorMode, &iValue);
    printf("Proposed detector mode: %d\n", iValue);
    
    if (!pl_set_param(detectorHandle, PARAM_PMODE, (void *) &iValue))
        reportPvCamError(functionName, "pl_set_param(PARAM_PMODE)");
    
    status |= setIntegerParam(PVCamDetectorModeRBV, iValue);


    //Get num speed table entries
    status |= getIntegerParam(PVCamSpeedTableIndex, &iValue);
    printf("Proposed speed table index: %d\n", iValue);
    
    if (!pl_set_param (detectorHandle, PARAM_SPDTAB_INDEX, (void *) &iValue))
        reportPvCamError(functionName, "pl_set_param (PARAM_SPDTAB_INDEX)");
    
    status |= setIntegerParam(PVCamSpeedTableIndexRBV, iValue);


    //Gain
    status |= getIntegerParam(PVCamGainIndex, &iValue);
    printf("Proposed gain: %d\n", iValue);
    
    if (!pl_set_param (detectorHandle, PARAM_GAIN_INDEX, (void *) &iValue))
        reportPvCamError(functionName, "pl_set_param(PARAM_GAIN_INDEX)");
    
    status |= setIntegerParam(PVCamGainIndexRBV, iValue);


    //Temperature
    if (!pl_get_param(detectorHandle, PARAM_TEMP_SETPOINT, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError(functionName, "pl_get_param(PARAM_TEMP_SETPOINT, ATTR_AVAIL)");
    
    printf("Temperature Setpoint available: %d\n", paramAvail);
    
    if (paramAvail)
    {
        status |= getDoubleParam(PVCamSetTemperature, &dValue);
        int16Parm = (int32)(dValue * 100);
        printf("Proposed temperature: %f\n", dValue);
        
        if (!pl_set_param (detectorHandle, PARAM_TEMP_SETPOINT, (void *) &int16Parm))
            reportPvCamError(functionName, "pl_set_param(PARAM_TEMP_SETPOINT)");
        
        status |= setDoubleParam(PVCamSetTemperatureRBV, dValue);
        tempAvailable = true;
    }

/*
    //Trigger
    if (!pl_get_param(detectorHandle, PARAM_EDGE_TRIGGER, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError (functionName, "pl_get_param(PARAM_EDGE_TRIGGER, ATTR_AVAIL)");
    if (paramAvail) {
        //Edge
        status |= getIntegerParam(PVCamTriggerEdge, &iValue);
        printf ("Proposed trigger edge: %d\n", iValue);

        if (iValue == 1)
            int16Parm = EDGE_TRIG_POS;
        else
            int16Parm = EDGE_TRIG_NEG;

        if (!pl_set_param (detectorHandle, PARAM_EDGE_TRIGGER, (void *) &int16Parm))
            reportPvCamError (functionName, "pl_set_param(PARAM_EDGE_TRIGGER)");

        status |= setIntegerParam(PVCamTriggerEdge, iValue);

        //TTL output logic
//        int16Parm = detectorParms.proposedTTLLogic;
//        if (!pl_set_param(camera_handle, PARAM_LOGIC_OUTPUT, (void *) &int16Parm))
//            reportPvCamError (functionName, "pl_set_param(PARAM_LOGIC_OUTPUT)");
    }
*/

    //pre/post shutter compensation
    if (!pl_get_param(detectorHandle, PARAM_SHTR_OPEN_DELAY, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError(functionName, "pl_get_param(PARAM_SHTR_OPEN_DELAY, ATTR_AVAIL)");
    printf("Open Shutter delay available: %d\n", paramAvail);
    
    if (paramAvail)
    {
        status |= getIntegerParam(PVCamShutterOpenDelay, &iValue);
        printf("Proposed shutter open delay: %d\n", iValue);
        
        if (!pl_set_param (detectorHandle, PARAM_SHTR_OPEN_DELAY, (void *) &iValue))
            reportPvCamError(functionName, "pl_get_param (PARAM_SHTR_OPEN_DELAY)");
        
        status |= setIntegerParam(PVCamShutterOpenDelayRBV, iValue);
    }
    
    if (!pl_get_param(detectorHandle, PARAM_SHTR_CLOSE_DELAY, ATTR_AVAIL, (void *) &paramAvail))
        reportPvCamError(functionName, "pl_get_param(PARAM_SHTR_CLOSE_DELAY, ATTR_AVAIL)");
    
    printf("Close Shutter delay available: %d\n", paramAvail);
    
    if (paramAvail)
    {
        status |= getIntegerParam(PVCamShutterCloseDelay, &iValue);
        printf("Proposed shutter close delay: %d\n", iValue);
        
        if (!pl_set_param (detectorHandle, PARAM_SHTR_CLOSE_DELAY, (void *) &iValue))
            reportPvCamError(functionName, "pl_get_param (PARAM_SHTR_CLOSE_DELAY)");
        
        status |= setIntegerParam(PVCamShutterCloseDelayRBV, iValue);
    }


    //ROI
    status |= getIntegerParam(ADBinX, &binX);
    status |= getIntegerParam(ADBinY, &binY);
    status |= getIntegerParam(ADMinX, &minX);
    status |= getIntegerParam(ADMinY, &minY);
    status |= getIntegerParam(ADSizeX, &sizeX);
    status |= getIntegerParam(ADSizeY, &sizeY);

    roi.sbin = binX;
    roi.s1 = minX;
    roi.s2 = sizeX-1;
    roi.pbin = binY;
    roi.p1 = minY;
    roi.p2 = sizeY-1;


    //Exposure Time
    status |= getDoubleParam(ADAcquireTime,  &dValue);
    int16Parm = (int32) (dValue * 1000);
    status |= getIntegerParam(PVCamTriggerMode, &iValue);
    int16Parm2 = iValue;
    printf("binX: %d, binY: %d, minx: %d, miny: %d, sizex: %d, sizey: %d, triggerMode: %d, exposureTime: %d\n",
             binX, binY, minX, minY, sizeX, sizeY, (int)int16Parm2, (int)int16Parm);
    
    if (!pl_exp_setup_seq (detectorHandle, 1, 1, &roi, (int16)int16Parm2, int16Parm, &rawDataSize))
        reportPvCamError(functionName, "pl_exp_setup_seq");
    
    status |= setIntegerParam(PVCamTriggerModeRBV, iValue);

    //Register callbacks ???does this really exist???
    //if (!pl_cam_register_callback (detectorHandle, PL_CALLBACK_EOF, OnEndFrameCallback))
    //    reportPvCamError (functionName, "pl_cam_register_callback");

    if (rawData != NULL)
      free(rawData);
    
    rawData = (unsigned short *) malloc(sizeof (unsigned short)*width*height);

    //Put back the values we actually used

    /* Call the callbacks to update any changes */
    callParamCallbacks();

    queryCurrentSettings();
}

//_____________________________________________________________________________________________

int ADPvCam::getAcquireStatus (void)
{

    const char *functionName = "getAcquireStatus";
    int16 status;
    uns32 byteCount;
    int collectionStatus = 0;

    if (!pl_exp_check_status (detectorHandle, &status, &byteCount))
        reportPvCamError(functionName, "pl_exp_check_status");

    if ((status == READOUT_COMPLETE) || (status == READOUT_NOT_ACTIVE))
        collectionStatus = 1;

    return collectionStatus;

}

//_____________________________________________________________________________________________



/* Code for iocsh registration */

/* pvCamConfig */
static const iocshArg ADPvCamConfigArg0 = {"Port name", iocshArgString};
static const iocshArg ADPvCamConfigArg1 = {"Max X size", iocshArgInt};
static const iocshArg ADPvCamConfigArg2 = {"Max Y size", iocshArgInt};
static const iocshArg ADPvCamConfigArg3 = {"Data type", iocshArgInt};
static const iocshArg ADPvCamConfigArg4 = {"maxBuffers", iocshArgInt};
static const iocshArg ADPvCamConfigArg5 = {"maxMemory", iocshArgInt};
static const iocshArg ADPvCamConfigArg6 = {"priority", iocshArgInt};
static const iocshArg ADPvCamConfigArg7 = {"stackSize", iocshArgInt};
static const iocshArg * const ADPvCamConfigArgs[] =  {&ADPvCamConfigArg0,
                                                          &ADPvCamConfigArg1,
                                                          &ADPvCamConfigArg2,
                                                          &ADPvCamConfigArg3,
                                                          &ADPvCamConfigArg4,
                                                          &ADPvCamConfigArg5,
                                                          &ADPvCamConfigArg6,
                                                          &ADPvCamConfigArg7};


static const iocshFuncDef configPVCam = {"ADPvCamConfig", 8, ADPvCamConfigArgs};

static void configPVCamCallFunc(const iocshArgBuf *args)
{
    ADPvCamConfig(args[0].sval, args[1].ival, args[2].ival, args[3].ival,
                      args[4].ival, args[5].ival, args[6].ival, args[7].ival);
}


static void pvCamRegister(void)
{
    iocshRegister(&configPVCam, configPVCamCallFunc);
}

extern "C" {
    epicsExportRegistrar(pvCamRegister);
}


// CEtAPI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CEtAPI.h"

#import "EtApi.tlb" no_namespace raw_interfaces_only

//Size of ringbuffer
#define BUFFERSIZE		100		//50Hz: Buffering 100 Samples = Buffering 1 second 

#define EVENTUNDEFINED	-1
#define FIXATIONEVENT	0
#define SACCADEEVENT	1
#define BLINKEVENT		2
#define	NOEVENT			3

void ReadFromRingBuffer(ALEADATA* data);
void WriteToRingBuffer();

//Handles for Thread synchronization
HANDLE hMutexBuffer = NULL;
HANDLE hEventDataInBuffer = NULL;

//Handle for CalibrationDone Event
HANDLE hEventCalibrationDone = NULL;

//Intelligaze Callbacks
void RawDataCB(RawData& r, void* userData);
void FixationCB(Fixation& f, void* userData);
void SaccadeCB(Saccade& s, void* userData);
void BlinkCB(Blink& b, void* userData);
void NoEventCB(NoEventData& n, void* userData);
void CalibrationDoneCB(int& status, bool improve, void* userData);

// Create the interface pointer.
IEtApiPtr pIEtApi = NULL;

//Ring Buffer
ALEADATA buffer[BUFFERSIZE];

RawData _currentRawData;
RawData* _pCurrentRawData = NULL;
Fixation _currentFixationData;
Fixation* _pCurrentFixationData = NULL;
Saccade _currentSaccadeData;
Saccade* _pCurrentSaccadeData = NULL;
Blink _currentBlinkData;
Blink* _pCurrentBlinkData = NULL;
NoEventData _currentNoEventData;
NoEventData* _pCurrentNoEventData = NULL;
int	_currentMode = -1;
int _currentCalibrationStatus = -1;
BOOL _currentCalibrationImproveValue = -1;

int idxWrite = -1;
int idxRead = -1;


CETAPI_API int __stdcall Open(const char* aleakey)
{
	hMutexBuffer = CreateMutex(NULL, FALSE, NULL);

	hEventDataInBuffer = CreateEvent(NULL, TRUE, FALSE, NULL);

	hEventCalibrationDone = CreateEvent(NULL, FALSE, FALSE, NULL);

	if(hMutexBuffer == NULL || hEventDataInBuffer == NULL || hEventCalibrationDone == NULL)
		return -10;

	//COM
	HRESULT hr = CoInitialize(NULL);

	hr = pIEtApi.CreateInstance(__uuidof(EtApi));

	if (FAILED(hr)) 
	{		
		return -1;
    }

	ApiError result;	

	BSTR ip = SysAllocString( L"127.0.0.1");
//	BSTR key = SysAllocString((LPTSTR)aleakey);
	int ws_len = MultiByteToWideChar(CP_ACP, 0, aleakey, strlen(aleakey), 0, 0);
	BSTR key = SysAllocStringLen(0, ws_len);
	MultiByteToWideChar(CP_ACP, 0, aleakey, strlen(aleakey), key, ws_len);

	pIEtApi->Open(key,ip, 27412, ip, 27413, &result);

	int returnValue = 0;

	if( result != ApiError_NoError )
		returnValue = -2;

	SysFreeString(ip);
	SysFreeString(key);

	if(returnValue != 0)
		return returnValue;		

	pIEtApi->SetRawDataCB64((__int64)(&RawDataCB),0,&result);
	if( result != ApiError_NoError)
		return -3;

	pIEtApi->SetFixationCB64((__int64)(&FixationCB),0,&result);
	if( result != ApiError_NoError)
		return -3;

	pIEtApi->SetSaccadeCB64((__int64)(&SaccadeCB),0,&result);
	if( result != ApiError_NoError)
		return -3;

	pIEtApi->SetBlinkCB64((__int64)(&BlinkCB),0,&result);
	if( result != ApiError_NoError)
		return -3;

	pIEtApi->SetNoEventCB64((__int64)(&NoEventCB),0,&result);
	if( result != ApiError_NoError)
		return -3;

	pIEtApi->SetCalibrationDoneCB64((__int64)(&CalibrationDoneCB), 0, &result);
	if( result != ApiError_NoError)
		return -3;
	
	return 0;
}

CETAPI_API void __stdcall Close()
{
	ApiError result;

	if(pIEtApi != NULL)
		pIEtApi->Close(&result);

	// Uninitialize COM.
	CoUninitialize();

	//Close Mutex
	if(hMutexBuffer != NULL)
	{
		CloseHandle(hMutexBuffer);
		hMutexBuffer = NULL;
	}

	//Close Event Handle
	if(hEventDataInBuffer != NULL)
	{
		CloseHandle(hEventDataInBuffer);
		hEventDataInBuffer = NULL;
	}

	if(hEventCalibrationDone != NULL)
	{
		CloseHandle(hEventCalibrationDone);
		hEventCalibrationDone = NULL;
	}

	//Reset Data
	idxWrite = -1;
	idxRead = -1;
	_pCurrentRawData = NULL;
	_currentMode = EVENTUNDEFINED;
	_pCurrentFixationData = NULL;
	_pCurrentSaccadeData = NULL;
	_pCurrentBlinkData = NULL;
	_pCurrentNoEventData = NULL;
}

CETAPI_API int __stdcall Version(long* major, long* minor, long* build, long* device)
{	
	if(pIEtApi == NULL)
	{
		return -100;
	}
	else
	{
		ApiError result;
		pIEtApi->Version(major,minor,build,device,&result);
		return result;
	}		
}

CETAPI_API int __stdcall IsOpen(VARIANT_BOOL* open)
{
	if(pIEtApi == NULL)
	{
		return -100;
	}
	else
	{
		ApiError result;
		pIEtApi->IsOpen(open, &result);
		//VARIANT_BOOL TO BOOL
		*open *= -1;
		
		return result;
	}
}

CETAPI_API int __stdcall PerformCalibration(long noPoints, int location, BOOL randomizedPoints, BOOL slowMode, BOOL audioFeedback, int eye, BOOL calibrationImprovement, BOOL skipBadPoints, BOOL autoCalibration, long backColor, long pointColor, const char* imageName)
{
	if(pIEtApi == NULL)
	{
		return -100;
	}
	else
	{		
		//reset event ... maybe WaitForCalibrationDone wasn't call on last Calibration
		ResetEvent(hEventCalibrationDone);

		ApiError result;
		//Convert VARIANT_BOOL to BOOL
		pIEtApi->PerformCalibration(noPoints, (PointLocationEnum)location, randomizedPoints * -1, slowMode * -1, audioFeedback * -1,(EyeTypeEnum) eye, calibrationImprovement * -1, skipBadPoints * -1, autoCalibration * -1, backColor, pointColor, _bstr_t(imageName), &result);
		return result;
	}
}

CETAPI_API int __stdcall WaitForCalibrationResult(int* status, BOOL* improve, DWORD dwMilliseconds)
{
	//wait for calibration done
	DWORD dwWaitResult = WaitForSingleObject(hEventCalibrationDone, dwMilliseconds);
	
	switch (dwWaitResult) 
    {
        // Event object was signaled
        case WAIT_OBJECT_0: 
            //Set Status
			*status = _currentCalibrationStatus;
			*improve = _currentCalibrationImproveValue;
			return 0;            

        // An error occurred
        default: return -1; 
    }
}

CETAPI_API int __stdcall ShowStatusWindow(long posX, long posY, long size, long opacity)
{
	if(pIEtApi == NULL)
	{
		return -100;
	}
	else
	{
		ApiError result;
		pIEtApi->ShowStatusWindow(posX,posY,size,opacity, &result);
		return result;
	}
}

CETAPI_API int __stdcall HideStatusWindow()
{
	if(pIEtApi == NULL)
	{
		return -100;
	}
	else
	{
		ApiError result;
		pIEtApi->HideStatusWindow(&result);
		return result;
	}
}

CETAPI_API int __stdcall DataStreaming(BOOL enable)
{
	if(pIEtApi == NULL)
	{
		return -100;
	}
	else
	{
		ApiError result;
		if(enable == 1)
			pIEtApi->DataStreaming(3, &result);
		else
			pIEtApi->DataStreaming(0, &result);

		return result;
	}
}

CETAPI_API int __stdcall WaitForData(ALEADATA* data, DWORD dwMilliseconds)
{
	//wait for data in buffer
	DWORD dwWaitResult = WaitForSingleObject(hEventDataInBuffer, dwMilliseconds);
	switch (dwWaitResult) 
    {
        // Event object was signaled
        case WAIT_OBJECT_0: 
            ReadFromRingBuffer(data);
            break; 

        // An error occurred
        default: return -1; 
    }

	return 0;
}

CETAPI_API int __stdcall ClearDataBuffer()
{
	if(hEventDataInBuffer == NULL || hMutexBuffer == NULL)
		return -1;

	//Lock
	WaitForSingleObject(hMutexBuffer, INFINITE);
	//Clear Ringbuffer
	idxWrite = -1;
	idxRead = -1;
	ResetEvent(hEventDataInBuffer);
	//Unlock
	ReleaseMutex(hMutexBuffer);

	return 0;
}

CETAPI_API int __stdcall ExitServer()
{
	if (pIEtApi == NULL)
	{
		return -100;
	}
	else
	{
		ApiError result;
		pIEtApi->ExitServer(&result);				
		return result;
	}
}

void WriteToRingBuffer()
{
	//Lock
	WaitForSingleObject(hMutexBuffer, INFINITE);

	if((_pCurrentRawData != NULL ) && (_currentMode != EVENTUNDEFINED) && 
		(((_pCurrentFixationData != NULL) && (_currentMode == FIXATIONEVENT)) || 
		((_pCurrentSaccadeData != NULL) && (_currentMode == SACCADEEVENT)) || 
		((_pCurrentBlinkData != NULL) && (_currentMode == BLINKEVENT)) || 
		((_pCurrentNoEventData != NULL) && (_currentMode == NOEVENT))))
	{
		//Save Data
		ALEADATA data;
		//raw data
		data.rawDataTimestamp = _currentRawData.timeStamp;
		data.intelligazeX = _currentRawData.intelliGazeX;
		data.intelligazeY = _currentRawData.intelliGazeY;
		data.gazePositionXLeftEye = _currentRawData.leftEye.gazePositionX;
		data.gazePositionYLeftEye = _currentRawData.leftEye.gazePositionY;
		data.gazePositionConfidenceLeftEye = _currentRawData.leftEye.gazePositionConfidence;
		data.pupilDiameterLeftEye = _currentRawData.leftEye.pupilDiameter;
		data.gazePositionXRightEye = _currentRawData.rightEye.gazePositionX;
		data.gazePositionYRightEye = _currentRawData.rightEye.gazePositionY;
		data.gazePositionConfidenceRightEye = _currentRawData.rightEye.gazePositionConfidence;
		data.pupilDiameterRightEye = _currentRawData.rightEye.pupilDiameter;
		//event data
		data.eventID = _currentMode;
		switch(_currentMode)
		{
		case FIXATIONEVENT:
			data.eventDataTimestamp = _currentFixationData.timeStamp;
			data.duration = _currentFixationData.duration;
			data.positionX = _currentFixationData.positionX;
			data.positionY = _currentFixationData.positionY;
			data.dispersionX = _currentFixationData.dispersionX;
			data.dispersionY = _currentFixationData.dispersionY;
			data.confidence = _currentFixationData.Confidence;
			break;
		case SACCADEEVENT:
			data.eventDataTimestamp = _currentSaccadeData.timeStamp;
			data.duration = _currentSaccadeData.duration;
			data.positionX = -1;
			data.positionY = -1;
			data.dispersionX = -1;
			data.dispersionY = -1;
			data.confidence =  -1;
			break;
		case BLINKEVENT:
			data.eventDataTimestamp = _currentBlinkData.timeStamp;
			data.duration = _currentBlinkData.duration;
			data.positionX = _currentBlinkData.positionX;
			data.positionY = _currentBlinkData.positionY;
			data.dispersionX = -1;
			data.dispersionY = -1;
			data.confidence =  -1;
			break;
		case NOEVENT:
			data.eventDataTimestamp = _currentNoEventData.timeStamp;
			data.duration = -1;
			data.positionX = -1;
			data.positionY = -1;
			data.dispersionX = -1;
			data.dispersionY = -1;
			data.confidence =  -1;
			break;
		}	

		//Reset Data
		_pCurrentRawData = NULL;
		_currentMode = EVENTUNDEFINED;
		_pCurrentFixationData = NULL;
		_pCurrentSaccadeData = NULL;
		_pCurrentBlinkData = NULL;
		_pCurrentNoEventData = NULL;

		//Backup writeposition
		int idxWriteBak = idxWrite;

		idxWrite++;
		if(idxWrite == BUFFERSIZE)
		{
			idxWrite = 0;
		}

		if(idxWrite == idxRead)
		{
			//Buffer full, don't write to buffer
			idxWrite = idxWriteBak;			
		}
		else
		{
			buffer[idxWrite] = data;
		}

		//Raise Signal
		SetEvent(hEventDataInBuffer);
	}

	//Unlock
	ReleaseMutex(hMutexBuffer);
}

void ReadFromRingBuffer(ALEADATA* data)
{
	//Lock
	WaitForSingleObject(hMutexBuffer, INFINITE);

	if(idxRead != idxWrite)
	{
		idxRead ++;

		if(idxRead == BUFFERSIZE)
		{
			idxRead = 0;
		}
			
		//Copy Data
		data->rawDataTimestamp = buffer[idxRead].rawDataTimestamp;
		data->intelligazeX = buffer[idxRead].intelligazeX;
		data->intelligazeY = buffer[idxRead].intelligazeY;
		data->gazePositionXLeftEye = buffer[idxRead].gazePositionXLeftEye;
		data->gazePositionYLeftEye = buffer[idxRead].gazePositionYLeftEye;
		data->gazePositionConfidenceLeftEye = buffer[idxRead].gazePositionConfidenceLeftEye;
		data->pupilDiameterLeftEye = buffer[idxRead].pupilDiameterLeftEye;
		data->gazePositionXRightEye = buffer[idxRead].gazePositionXRightEye;
		data->gazePositionYRightEye = buffer[idxRead].gazePositionYRightEye;
		data->gazePositionConfidenceRightEye = buffer[idxRead].gazePositionConfidenceRightEye;
		data->pupilDiameterRightEye = buffer[idxRead].pupilDiameterRightEye;
		data->eventID = buffer[idxRead].eventID;
		data->eventDataTimestamp = buffer[idxRead].eventDataTimestamp;
		data->duration = buffer[idxRead].duration;
		data->positionX = buffer[idxRead].positionX;
		data->positionY = buffer[idxRead].positionY;
		data->dispersionX = buffer[idxRead].dispersionX;
		data->dispersionY = buffer[idxRead].dispersionY;
		data->confidence = buffer[idxRead].confidence;

		if(idxRead == idxWrite)
		{
			//Reset Signal
			ResetEvent(hEventDataInBuffer);
		}	
	}
	//Unlock
	ReleaseMutex(hMutexBuffer);
}

//Callbacks
void RawDataCB(RawData& r, void* userData)
{
	_currentRawData = r;
	_pCurrentRawData = &_currentRawData;

	WriteToRingBuffer();
}

void FixationCB(Fixation& f, void* userData)
{
	_currentFixationData = f;
	_pCurrentFixationData = &_currentFixationData;
	_currentMode = FIXATIONEVENT;	

	WriteToRingBuffer();
}

void SaccadeCB(Saccade& s, void* userData)
{
	_currentSaccadeData = s;
	_pCurrentSaccadeData = &_currentSaccadeData;
	_currentMode = SACCADEEVENT;	

	WriteToRingBuffer();
}

void BlinkCB(Blink& b, void* userData)
{
	_currentBlinkData = b;
	_pCurrentBlinkData = &_currentBlinkData;
	_currentMode = BLINKEVENT;	

	WriteToRingBuffer();
}

void NoEventCB(NoEventData& n, void* userData)
{
	_currentNoEventData = n;
	_pCurrentNoEventData = &_currentNoEventData;
	_currentMode = NOEVENT;	

	WriteToRingBuffer();
}

void CalibrationDoneCB(int& status, bool improve, void* userData)
{
	_currentCalibrationStatus = status;
	_currentCalibrationImproveValue = improve;
	SetEvent(hEventCalibrationDone);
}
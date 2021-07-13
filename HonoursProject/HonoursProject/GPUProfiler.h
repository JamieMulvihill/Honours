#pragma once
#include "DXF.h"
//#define TIMER_INIT \
//	LARGE_INTEGER time;
//
//#define TIMER_START QueryPerformanceCounter(&time);
//	


enum TIMESTAMP {

	TIMESTAMP_BEGIN,
	TIMESTAMP_DEPTH,
	TIMESTAMP_COMPUTE_READ,
	TIMESTAMP_COMPUTE_PENETRATION,
	TIMESTAMP_COMPUTE_JUMPFLOOD,
	TIMESTAMP_COMPUTE_DISPLACE,
	TIMESTAMP_COMPUTE_VOLUMEPRESERVE,
	TIMESTAMP_COMPUTE_WRITE,
	TIMESTAMP_SCENE,
	TIMESTAMP_END,

	TIMESTAMP_TOTAL
};


class GPUProfiler
{
public:
	GPUProfiler();

	void Init(ID3D11DeviceContext* deviceContext_, ID3D11Device* device_);
	void Stop();

	void BeginFrame();
	void CheckTimestamp(TIMESTAMP timestamp_);
	void EndFrame();

	void DataWaitandUpdate();

	float GetTimings(TIMESTAMP ts){ return frameTiming[ts]; }
	
	float GetAvgTimings(TIMESTAMP timestamp) { return avgFrameTiming[timestamp]; }
protected:

	ID3D11DeviceContext* deviceContext;
	ID3D11Device* device;
	ID3D11Query* timestampQueryDisjoint[2];
	ID3D11Query* timeStampQuerys[TIMESTAMP_TOTAL][2];

	int frameQuery, frameCollect;

	float frameTiming[TIMESTAMP_TOTAL];
	float avgFrameTiming[TIMESTAMP_TOTAL];
	float avgTotalTimes[TIMESTAMP_TOTAL];
	
	int avgFrameCount;						 
	double avgBeginTime;	

	LARGE_INTEGER time, frequency;
};


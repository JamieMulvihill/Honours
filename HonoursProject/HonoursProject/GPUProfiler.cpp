#include "GPUProfiler.h"

GPUProfiler::GPUProfiler():
	frameQuery(0),
	frameCollect(-1),
	avgFrameCount(0),
	avgBeginTime(0.0f)
{
	memset(timestampQueryDisjoint, 0, sizeof(timestampQueryDisjoint));
	memset(timeStampQuerys, 0, sizeof(timeStampQuerys));
	memset(frameTiming, 0, sizeof(frameTiming));
	memset(avgFrameTiming, 0, sizeof(frameTiming));
	memset(avgTotalTimes, 0, sizeof(frameTiming));
}

void GPUProfiler::Init(ID3D11DeviceContext* deviceContext_, ID3D11Device* device_ )
{
	
	deviceContext = deviceContext_;
	device = device_;

	D3D11_QUERY_DESC queryDesc = { D3D11_QUERY_TIMESTAMP_DISJOINT, 0 };

	// create timestamp disjoint query for frame 0 and 1
	device->CreateQuery(&queryDesc, &timestampQueryDisjoint[0]); 
	device->CreateQuery(&queryDesc, &timestampQueryDisjoint[1]);
	
	queryDesc.Query = D3D11_QUERY_TIMESTAMP;

	for (TIMESTAMP timestamp = TIMESTAMP_BEGIN; timestamp < TIMESTAMP_TOTAL; timestamp = TIMESTAMP(timestamp + 1))
	{
		//create start-frame timestamp query at frame 0 amd 1
		device->CreateQuery(&queryDesc, &timeStampQuerys[timestamp][0]); 
		device->CreateQuery(&queryDesc, &timeStampQuerys[timestamp][1]);
	}
	QueryPerformanceCounter(&time);
}

void GPUProfiler::Stop()
{
	if (timestampQueryDisjoint[0])
		timestampQueryDisjoint[0]->Release();

	if (timestampQueryDisjoint[1])
		timestampQueryDisjoint[1]->Release();

	for (TIMESTAMP timestamp = TIMESTAMP_BEGIN; timestamp < TIMESTAMP_TOTAL; timestamp = TIMESTAMP(timestamp + 1))
	{
		if (timeStampQuerys[timestamp][0])
			timeStampQuerys[timestamp][0]->Release();

		if (timeStampQuerys[timestamp][1])
			timeStampQuerys[timestamp][1]->Release();
	}

}

void GPUProfiler::BeginFrame()
{
	deviceContext->Begin(timestampQueryDisjoint[frameQuery]);
	CheckTimestamp(TIMESTAMP_BEGIN);
}

void GPUProfiler::CheckTimestamp(TIMESTAMP timestamp_)
{
	deviceContext->End(timeStampQuerys[timestamp_][frameQuery]);
}

void GPUProfiler::EndFrame()
{
	CheckTimestamp(TIMESTAMP_END);
	deviceContext->End(timestampQueryDisjoint[frameQuery]);

	++frameQuery &= 1;
}

void GPUProfiler::DataWaitandUpdate()
{

	QueryPerformanceCounter(&time);
	QueryPerformanceFrequency(&frequency);
	double timeInMilli = (double)time.QuadPart * 1000.0 / (double)frequency.QuadPart;

	if (frameCollect < 0)
	{
		// Haven't run enough frames yet to have data
		frameCollect = 0;
		return;
	}

	// Wait for data
	while (deviceContext->GetData(timestampQueryDisjoint[frameCollect], NULL, 0, 0) == S_FALSE)
	{
		Sleep(1);
	}

	int iFrame = frameCollect;
	++frameCollect &= 1;

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timestampDisjoint;
	deviceContext->GetData(timestampQueryDisjoint[iFrame], &timestampDisjoint, sizeof(timestampDisjoint), 0) != S_OK;

	if (timestampDisjoint.Disjoint)
	{
		// lose this frame's data
		return;
	}

	UINT64 timestampPrev;
	deviceContext->GetData(timeStampQuerys[TIMESTAMP_BEGIN][iFrame], &timestampPrev, sizeof(UINT64), 0) != S_OK;
	
	for (TIMESTAMP timestamp = TIMESTAMP(TIMESTAMP_BEGIN + 1); timestamp < TIMESTAMP_TOTAL; timestamp = TIMESTAMP(timestamp + 1))
	{
		UINT64 timestamp_;
		deviceContext->GetData(timeStampQuerys[timestamp][iFrame], &timestamp_, sizeof(UINT64), 0) != S_OK;
		
		frameTiming[timestamp] = float(timestamp_ - timestampPrev) / float(timestampDisjoint.Frequency);
		timestampPrev = timestamp_;

		avgTotalTimes[timestamp] += frameTiming[timestamp];
	}

	++avgFrameCount;

	if (timeInMilli > avgBeginTime + 0.5)
	{
		for (TIMESTAMP timestamp = TIMESTAMP_BEGIN; timestamp < TIMESTAMP_TOTAL; timestamp = TIMESTAMP(timestamp + 1))
		{
			avgFrameTiming[timestamp] = avgTotalTimes[timestamp] / avgFrameCount;
			avgTotalTimes[timestamp] = 0.0f;
			
		}

		avgFrameCount = 0;
		avgBeginTime = timeInMilli;
	}


}

//--------------------------------------------------------------------------------------
// https://github.com/walbourn/directx-sdk-samples/blob/master/BasicCompute11/BasicCompute11.hlsl
//--------------------------------------------------------------------------------------

struct Data
{
	float4 texel;
};

Texture2D heightMap : register(t0);
RWStructuredBuffer<Data> gOutput;

#define cacheSize 1024
groupshared float4 gCache[cacheSize];

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID, int3 groupID : SV_GroupID)
{
	int groupIDPosition = groupThreadID.y * 32 + groupThreadID.x;
	int groupPosition = groupID.y * 32 + groupID.x;
	gCache[groupIDPosition] = heightMap[dispatchThreadID.xy];

	// Wait for all threads in group to finish.
	GroupMemoryBarrierWithGroupSync();

	float currentVolume = 0.f;
	
	// Increment value of current volume by every value in the cache
	for (int i = 0; i < cacheSize; i++) {
		currentVolume += gCache[i].x;
	}
	
	// Last thread in each group will overwrite to group position in buffer
	gOutput[groupPosition].texel = float4(currentVolume, 0, 0, 1);
}

Texture2D displacementMap : register(t0);
RWTexture2D<float4> resultHeightMap: register(u0);

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID, int3 groupID : SV_GroupID)
{
	float displacementValue = displacementMap[dispatchThreadID.xy].x;
	// Wait for all threads in group to finish.
	GroupMemoryBarrierWithGroupSync();

	resultHeightMap[dispatchThreadID.xy] = float4(max(0.f, displacementValue), max(0.f, displacementValue), max(0.f, displacementValue), 1.f);
}
Texture2D penetrationMap : register(t0);
Texture2D jumpFloodingMap : register(t1);
Texture2D heightMap : register(t2);
RWTexture2D<float4> displacementMap: register(u0);

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{

	float4 penetrationValue = penetrationMap[dispatchThreadID.xy];
	float2 closestSeed = jumpFloodingMap[dispatchThreadID.xy].xy;

	GroupMemoryBarrierWithGroupSync();

	if (penetrationValue.z != -1) {
		displacementMap[dispatchThreadID.xy] = float4(heightMap[dispatchThreadID.xy].x, penetrationValue.z, 0, 1);
	}

	else {
		float currentPeneration = penetrationValue.w;

		float2 direction = float2(0, 0);
		direction = normalize(closestSeed - float2(dispatchThreadID.xy));
	
		// set result map value for the deformning/penetrating pixel
		displacementMap[dispatchThreadID.xy] = float4(heightMap[dispatchThreadID.xy].x + currentPeneration, -1, direction.x, direction.y);

		// Displace all material to the closest seed pixel
		displacementMap[closestSeed] = float4(displacementMap[closestSeed].x - currentPeneration, -3, direction.x, direction.y);
	}
}
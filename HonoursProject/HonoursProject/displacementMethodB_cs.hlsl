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

	// if not a penertrating pixel set value as is, nothing to displace
	if (penetrationValue.z != -1) {
		displacementMap[dispatchThreadID.xy] = float4(heightMap[dispatchThreadID.xy].x, penetrationValue.z, 0, 1);
	}

	else {

		float currentPeneration = penetrationValue.w;

		float2 direction = float2(0, 0);
		direction = normalize(closestSeed - float2(dispatchThreadID.xy));
		
		// set result map value for the deformning/penetrating pixel
		displacementMap[dispatchThreadID.xy] = float4(heightMap[dispatchThreadID.xy].x + currentPeneration, penetrationValue.z, direction.x, direction.y);
		
		int range = 16; // This value would be controlled by surface type or calculated using the Deforemer object like weight or size
		float amount = currentPeneration / range;
		
		// Iterate along the direction from the closest seed pixel to the range, displacing material as iterating
		for (int i = 0; i < range; i++) {
			float2 seed = float2(ceil(closestSeed.x + (direction.x * i)), ceil(closestSeed.y + (direction.y * i)));
			displacementMap[seed] += float4(-amount, 0, 0, 0);
			displacementMap[seed] = float4(displacementMap[seed].x, -3, direction.x, direction.y);
		}

	}
}
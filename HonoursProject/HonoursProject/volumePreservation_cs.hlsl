Texture2D displacementMap : register(t0);
Texture2D heightMap : register(t1);
RWTexture2D<float4> volumeMap: register(u0);

cbuffer PreservationBuffer : register(b0)
{
	float treshold;
};

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{

	float4 penetrationValue = displacementMap[dispatchThreadID.xy];
	float myHeight = penetrationValue.x;
	float heightTreshold = myHeight * treshold; // set the treshold value for where a height value needs to be reduced

	volumeMap[dispatchThreadID.xy] = float4(myHeight, penetrationValue.y, myHeight, 1); // initialize values of result texture
	GroupMemoryBarrierWithGroupSync();

	// Calculate neighbors
	float2 topLeft = float2(dispatchThreadID.x - 1, dispatchThreadID.y + 1);
	float2 topCentre = float2(dispatchThreadID.x, dispatchThreadID.y + 1);
	float2 topRight = float2(dispatchThreadID.x + 1, dispatchThreadID.y + 1);

	float2 centreLeft = float2(dispatchThreadID.x - 1, dispatchThreadID.y);
	float2 centreRight = float2(dispatchThreadID.x + 1, dispatchThreadID.y);

	float2 bottomLeft = float2(dispatchThreadID.x - 1, dispatchThreadID.y - 1);
	float2 bottomCentre = float2(dispatchThreadID.x, dispatchThreadID.y - 1);
	float2 bottomRight = float2(dispatchThreadID.x + 1, dispatchThreadID.y - 1);

	float2 neighbors[8] = { topLeft, topCentre, topRight, centreLeft, centreRight, bottomLeft, bottomCentre, bottomRight };

	for (int i = 0; i < 8; i++) {

		float3 neighborValue = displacementMap[neighbors[i]].xyz;
		float neighborHeight = neighborValue.x;

		// if neighbor is not a seed, skip to next neighbor
		if (neighborValue.y != -3) {
			continue;
		}

		// if neighrbor height is greater than current pixel, skip to next neighbor
		if (neighborHeight >= myHeight) {
			continue;
		}
		// If the neighbor value is less than treshold, claculate the heightDifference, adjust the pixel data of the neghbor and current pixel, recalculate threshold
		if (volumeMap[neighbors[i]].x < heightTreshold) {
			float difference = heightTreshold - displacementMap[neighbors[i]].x;
			volumeMap[neighbors[i]] = float4(displacementMap[neighbors[i]].x + (difference), neighborValue.y, neighborHeight + difference, 1);
			myHeight -= (difference);
			volumeMap[dispatchThreadID.xy] = float4(myHeight, penetrationValue.y, myHeight, 1);
			heightTreshold = myHeight * treshold;
		}
	}
}
Texture2D heightMap : register(t0);
Texture2D deformationMap : register(t1);
RWTexture2D<float4> penetrationMap: register(u0);

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float4 heightMapValue = heightMap[dispatchThreadID.xy];
	float4 depthMapValue = deformationMap[dispatchThreadID.xy];
	float depthValue = depthMapValue.x;
	float obstacleValue = .024f;
	float penetration = 0;

	// penetration is the value of material that is displaced from a pixel/column
	// if ther is no collision, the depthValue is 1 and penetration will be 0, if there is a collison the value is calculated using the return depth of a collision
	penetration = (heightMapValue.x + 1) - min(heightMapValue.x + 1, heightMapValue.x + depthValue);

	int pixelType;
	float2 seedPosition;

	// Penetrating pixel.
	if (penetration > obstacleValue) {
		penetration =  1 - heightMapValue.x; // pentration is the matertial to be moved, 1 being full, therfore 1 minus current height is the amount to displace
		pixelType = -1;
		seedPosition = float2(0, 0);
	}

	// Seeds pixel - capable of recieving displaced material
	else if (penetration <= obstacleValue && depthValue == 1) {
		pixelType = -3;
		seedPosition = dispatchThreadID.xy;
		penetration = 0;
	}

	// Obstacle pixel - not penetrated but has a depth value close to penetrating so cant be displaceed to
	else
	{
		pixelType = -2;
		seedPosition = dispatchThreadID.xy;
		penetration = 0;
	}

	penetrationMap[dispatchThreadID.xy] = float4(seedPosition.x, seedPosition.y, pixelType, penetration);
}

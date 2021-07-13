
// Texture and sampler registers
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
SamplerState Sampler0 : register(s0);


cbuffer PentrationBuffer : register(b0)
{
	float cameraNear;
	float cameraFar;
	float heightMapScale;
	float padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

// Pixek Type -1 for penetrating point ("obstacle"), -2 for close to penetrating, -3 for not penetrating ("seed")
float4 main(InputType input) : SV_TARGET
{
	float frustumHeight = cameraFar - cameraNear;

	float4 heightMapValue = texture0.Sample(Sampler0, input.tex);
	float4 depthMapValue = texture1.Sample(Sampler0, input.tex);
	float depthValue = depthMapValue.x * heightMapScale * frustumHeight;

	// might need to change this to be the sum of the pixels for height map value
	int penetration = heightMapValue.x - min(heightMapValue.x, depthValue);


	int pixelType;
	float2 seedPosition;
	float closeValue = 0;

	// Penetrating point.
	if (penetration > 0) {
		pixelType = -1;
	}
	// Not penetrating but close. "Obstacles"
	
	else if (penetration <= 0 && abs(int(depthValue) - int(heightMapValue.x)) < closeValue) {
		pixelType = -2;
	}

	// Not penetatring. "Seeds"
	else
	{
		pixelType = -3;
		seedPosition = input.tex;
	}

	// if depth value is < 1
	// is collided, leave it alone
	//else
	//	check its 8 neibhbors
	// if they are collided
	//	add displacement to me til full or equal to differnece between standard pixel sum and their pixel sum.


	return float4(seedPosition.x, seedPosition.y, pixelType, penetration);
}

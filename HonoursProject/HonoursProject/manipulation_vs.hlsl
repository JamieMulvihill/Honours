
// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer TimeBuffer : register(b1)
{
	float4 scale;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

float4 ApplyHeightMap(float4 position, float3 normal, float2 tex, int height) {

	float textureColor = texture0.SampleLevel(sampler0, tex, 0);

	float offset = (textureColor * height);
	position.y -= normal.y * offset;
	position.y = max(position.y, -5); // clamp the y so that the deformation "hits the ground"
	position.x -= normal.x * offset;
	position.z -= normal.z * offset;

	return position;
}

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(ApplyHeightMap(input.position, input.normal, input.tex, scale.x), worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	////sobel calculation for normals
	float2 pixelSize = float2(1.f / 1024, 1.f / 1024);
	
	// computing pixel offset values
	float2 topLeft = output.tex + float2(-pixelSize.x, -pixelSize.y);
	float2 topCentre = output.tex + float2(0, -pixelSize.y);
	float2 topRight = output.tex + float2(pixelSize.x, -pixelSize.y);
	
	float2 centreLeft = output.tex + float2(-pixelSize.x, 0.f);
	float2 centreRight = output.tex + float2(pixelSize.x, 0.f);
	
	float2 bottomLeft = output.tex + float2(-pixelSize.x, pixelSize.y);
	float2 bottomCentre = output.tex + float2(0, pixelSize.y);
	float2 bottomRight = output.tex + float2(pixelSize.x, pixelSize.y);
	
	float scale = 3;
	
	// apply sobel filter to surrounding current pixel
	float resTopLeft = scale * texture0.SampleLevel(sampler0, topLeft, 0).r;
	float resTopCentre = scale * texture0.SampleLevel(sampler0, topCentre, 0).r;
	float resTopRight = scale * texture0.SampleLevel(sampler0, topRight, 0).r;
	
	float resCentreLeft = scale * texture0.SampleLevel(sampler0, centreLeft, 0).r;
	float resCentreRight = scale * texture0.SampleLevel(sampler0, centreRight, 0).r;
	
	float resBottomLeft = scale * texture0.SampleLevel(sampler0, bottomLeft, 0).r;
	float resBottomCentre = scale * texture0.SampleLevel(sampler0, bottomCentre, 0).r;
	float resBottomRight = scale * texture0.SampleLevel(sampler0, bottomRight, 0).r;
	
	//execute sobel filters
	float Gx = resTopLeft - resTopRight + 2.f * resCentreLeft - 2.f * resCentreRight + resBottomLeft - resBottomRight;
	float Gy = resTopLeft + 2.f * resTopCentre + resTopRight - resBottomLeft - 2.f * resBottomCentre - resBottomRight;
	
	//Generate the so far missing Z values
	float Gz = 0.5f * sqrt(max(0.f, 1.f - Gx * Gx - Gy * Gy));
	
	output.normal = normalize(float3(2.f * Gx, Gz, 2.f * Gy));
	output.normal = mul(output.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	return output;
}
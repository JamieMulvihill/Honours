Texture2D heightMap : register(t0);
Texture2D deformationMap : register(t1);
Texture2D previousHeightMap : register(t2);
RWTexture2D<float4> normalMap: register(u0);

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	
	////sobel calculation for normals
	// put screen width and heaight as thats the render texture size
	float2 pixelSize = float2(1.f / 1024, 1.f / 1024);

	// computing pixel offset values
	float2 topLeft = dispatchThreadID.xy + float2(-pixelSize.x, -pixelSize.y);
	float2 topCentre = dispatchThreadID.xy + float2(0, -pixelSize.y);
	float2 topRight = dispatchThreadID.xy + float2(pixelSize.x, -pixelSize.y);

	float2 centreLeft = dispatchThreadID.xy + float2(-pixelSize.x, 0.f);
	float2 centreRight = dispatchThreadID.xy + float2(pixelSize.x, 0.f);

	float2 bottomLeft = dispatchThreadID.xy + float2(-pixelSize.x, pixelSize.y);
	float2 bottomCentre = dispatchThreadID.xy + float2(0, pixelSize.y);
	float2 bottomRight = dispatchThreadID.xy + float2(pixelSize.x, pixelSize.y);

	float scale = 1;

	// apply sobel filter to surrounding current pixel
	float resTopLeft = scale * heightMap[topLeft].x;
	float resTopCentre = scale * heightMap[topCentre].x;
	float resTopRight = scale * heightMap[topRight].x;

	float resCentreLeft = scale * heightMap[centreLeft].x;
	float resCentreRight = scale * heightMap[centreRight].x;

	float resBottomLeft = scale * heightMap[bottomLeft].x;
	float resBottomCentre = scale * heightMap[bottomCentre].x;
	float resBottomRight = scale * heightMap[bottomRight].x;

	//execute sobel filters
	float Gx = resTopLeft - resTopRight + 2.f * resCentreLeft - 2.f * resCentreRight + resBottomLeft - resBottomRight;
	float Gy = resTopLeft + 2.f * resTopCentre + resTopRight - resBottomLeft - 2.f * resBottomCentre - resBottomRight;

	//Generate the so far missing Z values
	float Gz = 0.5f * sqrt(max(0.f, 1.f - Gx * Gx - Gy * Gy));

	float3 testNormal;
	testNormal.x = scale * -(resBottomRight - resBottomLeft + 2 * (resCentreRight - resCentreLeft) + resTopRight - resTopLeft);
	testNormal.y = scale * -(resTopLeft - resBottomLeft + 2 * (resTopCentre - resBottomCentre) + resTopRight - resBottomRight);
	testNormal.z = 1.0 / 4;
	testNormal = normalize(testNormal);

	float3 result = normalize(float3(2.f * Gx, 2.f * Gy, Gz));
	normalMap[dispatchThreadID.xy] = float4(testNormal.y, testNormal.x, testNormal.z, 1);
}

Texture2D heightMap : register(t0);
RWTexture2D<float4> resultHeightMap: register(u0);

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	resultHeightMap[dispatchThreadID.xy] = heightMap[dispatchThreadID.xy];
}

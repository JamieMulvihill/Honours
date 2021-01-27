// Compute shader to add two textures
// values of deformation map are inverted to create a value closer to white 
// that can be added to the black image heightmap 

Texture2D heightMap : register(t0);
Texture2D deformationMap : register(t1);
RWTexture2D<float4> resultHeightMap: register(u0);

[numthreads(256, 4, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{	
	// to increse the strength of the deforamtions increse scale
	float deformationScale = 1.f;
	resultHeightMap[dispatchThreadID.xy] = (heightMap[dispatchThreadID.xy]) + (( 1 - deformationMap[dispatchThreadID.xy]) * deformationScale);
}

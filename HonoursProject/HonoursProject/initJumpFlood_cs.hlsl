Texture2D penetrationMap : register(t0);
RWTexture2D<float4> jumpFloodTexture: register(u0);

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    float4 penetrationValue = penetrationMap[dispatchThreadID.xy];

    int maxDistance = int(length(float2(1024, 1024)));

    int distance = 0;
    float2 coOrd = float2(0, 0);

    // Check for Seed pixel
    if (penetrationValue.z != -3)
    {
        distance = maxDistance;
        coOrd = float2(-1, -1);
    }
    else
    {
        distance = 0;
        coOrd = float2(dispatchThreadID.xy);
    }

    jumpFloodTexture[dispatchThreadID.xy] = float4(coOrd.x, coOrd.y, penetrationValue.z, distance);

}
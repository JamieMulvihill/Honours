
Texture2D penetrationMap : register(t0);
RWTexture2D<float4> distanceMap: register(u0);

cbuffer JumpBuffer : register(b0)
{
    int stepLength;
};

[numthreads(32, 32, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{	
    float4 currentValue = penetrationMap[dispatchThreadID.xy];

    // If seed pixel, set value to be self
    if (currentValue.z == -3)
    {
        distanceMap[dispatchThreadID.xy] = float4(currentValue.x, currentValue.y, currentValue.z, currentValue.w);
    }
    else {
        
        // Calculate neighbor locations
        float2 topLeft = float2(dispatchThreadID.x - (stepLength), dispatchThreadID.y + (stepLength));
        float2 topCentre = float2(dispatchThreadID.x, dispatchThreadID.y + (stepLength));
        float2 topRight = float2(dispatchThreadID.x + (stepLength), dispatchThreadID.y + (stepLength));

        float2 centreLeft = float2(dispatchThreadID.x - (stepLength), dispatchThreadID.y);
        float2 centreRight = float2(dispatchThreadID.x + (stepLength), dispatchThreadID.y);

        float2 bottomLeft = float2(dispatchThreadID.x - (stepLength), dispatchThreadID.y - (stepLength));
        float2 bottomCentre = float2(dispatchThreadID.x, dispatchThreadID.y - (stepLength));
        float2 bottomRight = float2(dispatchThreadID.x + (stepLength), dispatchThreadID.y - (stepLength));

        float2 neighbors[8] = { topLeft, topCentre, topRight, centreLeft, centreRight, bottomLeft, bottomCentre, bottomRight};

        int minDistance = currentValue.w;
        float2 neighborSeed = currentValue.xy;
        
        // Loop through neighbors
        for (int i = 0; i < 8; i++)
        {
            float3 neighborValue = penetrationMap[neighbors[i]].xyz;

            // if neighbor is seed, check its distance from my current closest seed andif closer set as closet seed
            if (neighborValue.z == -3)
            {
                int distance = int(length(dispatchThreadID.xy - neighborValue.xy));

                if (distance < length(minDistance))
                {
                    minDistance = distance;
                    neighborSeed = neighborValue.xy;
                }
            }
        } 
        distanceMap[dispatchThreadID.xy] = float4(neighborSeed.x, neighborSeed.y, currentValue.z, minDistance);     
    }

}

//--------------------------------------------------------------------------------------
// https://github.com/walbourn/directx-sdk-samples/blob/master/BasicCompute11/BasicCompute11.hlsl
// this is not the final shader, this the code to read from two buffers and add the values
// before writing to a structured buffer, I am trying to read and sum the values from a 
// texture and write to a buffer, using this as guide, attepmting to implement this first
// and adapt and change for programs needs after
//--------------------------------------------------------------------------------------

struct BufType
{
    int i;
    float f; 
};

StructuredBuffer<BufType> Buffer0 : register(t0);
StructuredBuffer<BufType> Buffer1 : register(t1);
RWStructuredBuffer<BufType> BufferOut : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    BufferOut[DTid.x].i = Buffer0[DTid.x].i + Buffer1[DTid.x].i;
    BufferOut[DTid.x].f = Buffer0[DTid.x].f + Buffer1[DTid.x].f;
}

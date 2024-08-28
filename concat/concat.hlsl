
cbuffer Constants : register(b0)
{
    int StartIndex; // Corresponds to the value loaded at cbuffer index 6
    int EndIndex; // Corresponds to the value loaded at cbuffer index 7
    int Stride; // This is inferred as it's used to calculate the offset
}

// Define the UAV structures for reading and writing
RWStructuredBuffer<uint16_t> InputBuffer : register(u0);
RWStructuredBuffer<uint16_t> OutputBuffer : register(u1);

// The main compute shader function
[numthreads(256, 1, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
    // Calculate the global index based on the thread ID and stride
    int globalIndex = StartIndex + threadID.x * Stride;
    
    // Bounds check to ensure we don't read/write out of bounds
    if (globalIndex < EndIndex)
    {
        // Read from the input buffer
        uint16_t value = InputBuffer[globalIndex];
        //value = value | StartIndex;

        // Write to the output buffer
        OutputBuffer[globalIndex] = value;
    }
}

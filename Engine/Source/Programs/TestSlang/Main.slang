import "Interface.slang";

RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain<T: IInterface, TInput: IInterfaceInput>(uint3 threadId : SV_DispatchThreadID)
{
    TInput input = TInput();
    T interface = T(input);
    result[threadId.x] = interface.test();
}
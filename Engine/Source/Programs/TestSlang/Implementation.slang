import "Interface.slang";

uniform int Value0;

struct ImplementationInput : IInterfaceInput
{
    int GetValue()
    {
        return Value0;
    }
}

struct Implementation : IInterface
{
    IInterfaceInput input;
    __init(IInterfaceInput input)
    {
        this.input = input;
    }
    int test()
    {
        return input.GetValue();
    }
}
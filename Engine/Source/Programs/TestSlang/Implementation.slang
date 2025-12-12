import "Interface.slang";

struct ImplementationInput : IInterfaceInput
{
    int value = 123456;
    int GetValue()
    {
        return this.value;
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
implementing RenderCore;

public interface IVertexFactoryInput
{
    float3 GetPosition();
    float3 GetNormal();
    float4 GetTangent();
    float4 GetColor();
    float2 GetTexCoords(int index);
}

public interface IVertexFactory
{
    __init(IVertexFactoryInput Input);
    float3 GetWorldPosition();
    float3 GetWorldNormal();
    float4 GetWorldTangent();
    float4 GetColor();
    float2 GetTexCoords(int index);
};

public float3 RotateLocalToWorld(float3 LocalDirection, float3x3 LocalToWorld, float3 InvScale)
{
	return
		InvScale.x * LocalToWorld[0] * LocalDirection.xxx +
		InvScale.y * LocalToWorld[1] * LocalDirection.yyy +
		InvScale.z * LocalToWorld[2] * LocalDirection.zzz;
}

public float3 RotateLocalToWorld(float3 LocalDirection, float4x4 LocalToWorld, float3 InvScale)
{
	return RotateLocalToWorld(LocalDirection, float3x3(LocalToWorld), InvScale);
}

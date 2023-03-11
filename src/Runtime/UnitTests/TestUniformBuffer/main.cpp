#include "main.h"
#include <array>

// IMPLEMENT_UNIFORM_BUFFER_STRUCT(Light)

// IMPLEMENT_UNIFORM_BUFFER_STRUCT(FPrimitiveShaderParameters)
#define OFFSET(obj, field) \
    ((unsigned long long)&obj.field - (unsigned long long)&obj)
    

BEGIN_UNIFORM_BUFFER_STRUCT(FViewShaderParameters)
    SHADER_PARAMETER(mat4, WorldToView)
    SHADER_PARAMETER(mat4, ViewToClip)
    SHADER_PARAMETER(mat4, WorldToClip) // WorldToClip = ViewToClip * WorldToView
    SHADER_PARAMETER(vec3, CameraPosition)
    SHADER_PARAMETER(vec3, CameraDirection)
END_UNIFORM_BUFFER_STRUCT()
int main()
{
    FViewShaderParameters test_obj;
    uint64 offset = OFFSET(test_obj, CameraPosition);

    // FUniformBufferBuilder builder;
    // FUniformBufferBuilder::BeginUniformBufferStruct("FStruct");
    // FUniformBufferBuilder::DeclareUniformBufferStructMember(EUniformBufferMemberType::UBMT_int, "d");
    // FUniformBufferBuilder::DeclareUniformBufferStructMember(EUniformBufferMemberType::UBMT_bvec2, "e");
    // auto FStructDeclaration = FUniformBufferBuilder::EndUniformBufferStruct();

    // BEGIN_UNIFORM_BUFFER_STRUCT(Light)
    //     SHADER_PARAMETER_ARRAY(float, 5, lightDistAttenCurveParams)
    //     SHADER_PARAMETER_ARRAY(float, 5, lightAngleAttenCurveParams)
    //     SHADER_PARAMETER(mat4, lightVP)
    //     SHADER_PARAMETER(vec4, lightIntensity)
    //     SHADER_PARAMETER(vec3, lightPosition)
    //     SHADER_PARAMETER(vec3, lightDirection)
    //     SHADER_PARAMETER(int, lightType) 
    //     SHADER_PARAMETER(float, lightIntensity)
    //     SHADER_PARAMETER(int, lightDistAttenCurveType)
    //     SHADER_PARAMETER(int, lightAngleAttenCurveType)
    //     SHADER_PARAMETER(bool, lightCastShadow)
    //     SHADER_PARAMETER(int, lightShadowMapLayerIndex)
    // END_UNIFORM_BUFFER_STRUCT()

    // BEGIN_UNIFORM_BUFFER_STRUCT(FPrimitiveShaderParameters)
    //     SHADER_PARAMETER_NESTED_STRUCT_ARRAY(Light, 10, Lights)
    //     SHADER_PARAMETER(mat4, ViewMatrix)
    //     SHADER_PARAMETER(mat4, ProjectionMatrix)
    //     SHADER_PARAMETER(mat4, VP)
    // END_UNIFORM_BUFFER_STRUCT()

    // auto LightDeclaration = FUniformBuffer::GetStructDeclarationByName("Light");
    // auto FPrimitiveShaderParametersDeclaration = FUniformBuffer::GetStructDeclarationByName("FPrimitiveShaderParameters");
    // TUniformBufferRef<FPrimitiveShaderParameters> ref = std::make_shared<TUniformBuffer<FPrimitiveShaderParameters>>();
    // TUniformBuffer<FPrimitiveShaderParameters> buffer;

    // ref->Set("Lights[5].lightVP", mat4(10.f));
    // mat4 lightVP;
    // ref->Get("Lights[5].lightVP", &lightVP);
    // buffer.Set("vector", glm::vec3(1.f, 2.f, 3.f));
    // buffer.Set("matrix", glm::mat4(10.f));
    // buffer.Set("values", 0, 0.f);
    // buffer.Set("values", 1, 1.f);
    // buffer.Set("values", 2, 2.f);
    // buffer.Set("boolean", true);
    // buffer.Set("integer", 5);

    // float value;
    // buffer.Get("value", &value);
    // glm::vec3 vector;
    // buffer.Get("vector", &vector);
    // glm::mat4 matrix;
    // buffer.Get("matrix", &matrix);
    // float values[3];
    // buffer.Get("values", 0, &values[0]);
    // buffer.Get("values", 1, &values[1]);
    // buffer.Get("values", 2, &values[2]);
    // bool boolean;
    // buffer.Get("boolean", &boolean);
    // int integer;
    // buffer.Get("integer", &integer);


    return 0;
}
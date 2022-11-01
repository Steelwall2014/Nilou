#include "RHIDefinitions.h"
#include "DynamicRHI.h"
#include "OpenGL/OpenGLUtils.h"
#include "Common/SceneManager.h"
#include "Common/DrawPass/SeabedSurfacePass.h"
#include "RHIResources.h"

namespace und {
    und::DrawBatchContext und::CreateDBCFromMesh(const und::SceneObjectMesh &mesh)
    {
        //unsigned int vao;
        DrawBatchContext context;
        for (auto &prim : mesh.GetPrimitives())
        {
            EPrimitiveMode primitive_mode;
            switch (prim.mode) 
            {
                case TINYGLTF_MODE_LINE: primitive_mode = EPrimitiveMode::PM_Lines; break;
                case TINYGLTF_MODE_LINE_LOOP: primitive_mode = EPrimitiveMode::PM_Line_Loop; break;
                case TINYGLTF_MODE_LINE_STRIP: primitive_mode = EPrimitiveMode::PM_Line_Strip; break;
                case TINYGLTF_MODE_POINTS: primitive_mode = EPrimitiveMode::PM_Points; break;
                case TINYGLTF_MODE_TRIANGLES: primitive_mode = EPrimitiveMode::PM_Triangles; break;
                case TINYGLTF_MODE_TRIANGLE_STRIP: primitive_mode = EPrimitiveMode::PM_Triangle_Strip; break;
                case TINYGLTF_MODE_TRIANGLE_FAN: primitive_mode = EPrimitiveMode::PM_Triangle_Fan; break;
            }
            uint32 index_stride;
            switch (prim.type) 
            {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: index_stride = 1; break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: index_stride = 2; break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: index_stride = 4; break;
            }
            RHIVertexArrayObjectRef VAO = _GM->RHICreateVertexArrayObject(primitive_mode);
            RHIBufferRef IndexBuffer = _GM->RHICreateBuffer(
                index_stride, prim.IndexArraySize, 
                EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::Static, 
                prim.pIndexArray
            );
            VAO->SetIndexBuffer(IndexBuffer);

            for (int i = 0; i < prim.Attributes.size(); i++)
            {
                auto &attr = prim.Attributes[i];
                RHIShaderResourceView srv;
                if (attr.name == "POSITION")
                    srv.AttributeIndex = 0;
                else if (attr.name == "NORMAL")
                    srv.AttributeIndex = 1;
                else if (attr.name == "TANGENT")
                    srv.AttributeIndex = 2;
                else if (attr.name == "TEXCOORD_0")
                    srv.AttributeIndex = 3;
                srv.Offset = attr.offset;
                srv.Stride = attr.stride;
                switch (attr.type) {
                    case TINYGLTF_COMPONENT_TYPE_BYTE: srv.Type = EVertexElementTypeFlags::VET_Byte; break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: srv.Type = EVertexElementTypeFlags::VET_UByte; break;
                    case TINYGLTF_COMPONENT_TYPE_SHORT: srv.Type = EVertexElementTypeFlags::VET_Short; break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: srv.Type = EVertexElementTypeFlags::VET_UShort; break;
                    case TINYGLTF_COMPONENT_TYPE_INT: srv.Type = EVertexElementTypeFlags::VET_Int; break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: srv.Type = EVertexElementTypeFlags::VET_UInt; break;
                    case TINYGLTF_COMPONENT_TYPE_FLOAT: srv.Type = EVertexElementTypeFlags::VET_Float; break;
                }
                srv.Type = srv.Type | (EVertexElementTypeFlags)(1 << attr.size);
                RHIBufferRef VBO = _GM->RHICreateBuffer(
                    attr.stride,
                    attr.VertexAttriArraySize,
                    EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static,
                    attr.pVertexAttriArray);
                VAO->AddVertexAttribBuffer(srv, VBO);
                //glGenBuffers(1, &vbo);
                //glBindBuffer(GL_ARRAY_BUFFER, vbo);
                //glBufferData(GL_ARRAY_BUFFER, attr.VertexAttriArraySize, attr.pVertexAttriArray, GL_STATIC_DRAW);
                //glVertexAttribPointer(index, attr.size, attr.type, attr.normalized, attr.stride, (void *)attr.offset);
                //glEnableVertexAttribArray(index);
                //glBindBuffer(GL_ARRAY_BUFFER, 0);
                //m_Buffers.push_back(vbo);

#ifdef _DEBUG_SHOWNORMAL
                if (attr.name == "NORMAL")
                {
                    float *data = (float *)attr.pVertexAttriArray;
                    for (int j = 0; j < attr.VertexAttriArraySize / 4; j += attr.size)
                    {
                        vertexnormal.push_back(glm::vec3(data[j], data[j + 1], data[j + 2]));
                    }
                }
                else if (attr.name == "TANGENT")
                {
                    float *data = (float *)attr.pVertexAttriArray;
                    for (int j = 0; j < attr.VertexAttriArraySize / 4; j += attr.size)
                    {
                        vertextangent.push_back(glm::vec3(data[j], data[j + 1], data[j + 2]));
                    }
                }
                else if (attr.name == "POSITION")
                {
                    float *data = (float *)attr.pVertexAttriArray;
                    for (int j = 0; j < attr.VertexAttriArraySize / 4; j += attr.size)
                    {
                        vertexposition.push_back(glm::vec3(data[j], data[j + 1], data[j + 2]));
                    }
                }
#endif // _DEBUG_SHOWNORMAL

            }

#ifdef _DEBUG_SHOWNORMAL
            for (int j = 0; j < vertexnormal.size(); j++)
            {
                glm::vec3 position = vertexposition[j];
                glm::vec3 normal = vertexnormal[j];
                glm::vec3 tangent = vertextangent[j];
                glm::vec3 bitangent = glm::cross(normal, tangent);
                vertexattr_modelmatrix.push_back(glm::mat4(
                    glm::vec4(bitangent, 0),
                    glm::vec4(normal, 0),
                    glm::vec4(tangent, 0),
                    glm::vec4(position, 1)
                ));
            }
#endif // _DEBUG_SHOWNORMAL
            _GM->RHIInitializeVertexArrayObject(VAO);

            //auto upload_texture = [](std::shared_ptr<SceneObjectTexture> tex) {
            //    auto img = tex->GetTextureImage();
            //    auto texture = _GM->RHICreateTexture2D("",
            //        img->GetPixelFormat(), img->Width, img->Height, img->data/*,
            //        tex->Mag_Filter, tex->Min_Filter,
            //        tex->Wrap_S, tex->Wrap_T*/);
            //    return texture;
            //};
            MaterialTextures material_ids;
            auto material = prim.Material;
            if (material)
            {
                auto normal = material->GetNormal();
                if (normal.ValueMap)
                    material_ids.normalTexture = UploadTexture(normal.ValueMap);

                auto baseColor = material->GetBaseColor();
                if (baseColor.ValueMap)
                    material_ids.baseColorTexture = UploadTexture(baseColor.ValueMap);

                auto emissive = material->GetEmissive();
                if (emissive.ValueMap)
                    material_ids.emissiveTexture = UploadTexture(emissive.ValueMap);

                auto roughnessMetallic = material->GetRoughnessMetallic();
                if (roughnessMetallic.ValueMap)
                    material_ids.roughnessMetallicTexture = UploadTexture(roughnessMetallic.ValueMap);

                auto occlusion = material->GetOcclusion();
                if (occlusion.ValueMap)
                    material_ids.occlusionTexture = UploadTexture(occlusion.ValueMap);
            }

            context.vao = VAO;
            //context.ebo = IndexBuffer;
            //context.count = prim.count;
            //context.type = prim.type;
            context.material = material_ids;
        }
        return context;
    }

    void SetPerFrameShaderParameters(const DrawFrameContext &context)
    {
        _GM->GLDEBUG();
        char uniformName[256];
        for (int i = 0; i < context.lights.size(); i++)
        {
            const Light &light = context.lights[i];

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightType");
            _GM->RHISetShaderParameter(uniformName, (int)light.lightType);
            _GM->GLDEBUG();

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightCastShadow");
            _GM->RHISetShaderParameter(uniformName, light.lightCastShadow);
            _GM->GLDEBUG();

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightColor");
            _GM->RHISetShaderParameter(uniformName, light.lightColor);
            _GM->GLDEBUG();

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightDirection");
            _GM->RHISetShaderParameter(uniformName, light.lightDirection);
            _GM->GLDEBUG();

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightIntensity");
            _GM->RHISetShaderParameter(uniformName, light.lightIntensity);
            _GM->GLDEBUG();

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightDistAttenCurveType");
            _GM->RHISetShaderParameter(uniformName, (int)light.lightDistAttenCurveType);
            _GM->GLDEBUG();
            for (int j = 0; j < 5; j++)
            {
                sprintf(uniformName, "%s[%d].%s[%d]", "lights", i, "lightDistAttenCurveParams", j);
                _GM->RHISetShaderParameter(uniformName, light.lightDistAttenCurveParams[j]);
            }
            _GM->GLDEBUG();

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightAngleAttenCurveType");
            _GM->RHISetShaderParameter(uniformName, (int)light.lightAngleAttenCurveType);
            for (int j = 0; j < 5; j++)
            {
                sprintf(uniformName, "%s[%d].%s[%d]", "lights", i, "lightAngleAttenCurveParams", j);
                _GM->RHISetShaderParameter(uniformName, light.lightAngleAttenCurveParams[j]);
            }
            _GM->GLDEBUG();

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightPosition");
            _GM->RHISetShaderParameter(uniformName, light.lightPosition);

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightShadowMapLayerIndex");
            _GM->RHISetShaderParameter(uniformName, light.lightShadowMapLayerIndex);

            sprintf(uniformName, "%s[%d].%s", "lights", i, "lightVP");
            _GM->RHISetShaderParameter(uniformName, light.lightVP);
        }
        _GM->RHISetShaderParameter("lightCount", (int)context.lights.size());

        _GM->RHISetShaderParameter("VP", context.projectionMatrix * context.viewMatrix);
        _GM->RHISetShaderParameter("cameraPos", context.cameraPosition);
        _GM->GLDEBUG();
        //RHISetShaderParameter("lightPos", context.lights[0].lightPosition);
    }

    void SetAtmosphereParameters(std::shared_ptr<SceneObjectAtmosphere> atmosphere)
    {
        auto set_density_profile = [](const DensityProfile &profile, const char *param_name) {
            char uniform[128];
            for (int i = 0; i < 2; i++)
            {
                sprintf(uniform, "ATMOSPHERE.%s.layers[%d].constant_term", param_name, i);
                _GM->RHISetShaderParameter(uniform, profile.layers[i].constant_term);
                sprintf(uniform, "ATMOSPHERE.%s.layers[%d].exp_scale", param_name, i);
                _GM->RHISetShaderParameter(uniform, profile.layers[i].exp_scale);
                sprintf(uniform, "ATMOSPHERE.%s.layers[%d].exp_term", param_name, i);
                _GM->RHISetShaderParameter(uniform, profile.layers[i].exp_term);
                sprintf(uniform, "ATMOSPHERE.%s.layers[%d].linear_term", param_name, i);
                _GM->RHISetShaderParameter(uniform, profile.layers[i].linear_term);
                sprintf(uniform, "ATMOSPHERE.%s.layers[%d].width", param_name, i);
                _GM->RHISetShaderParameter(uniform, profile.layers[i].width);
            }
        };
        _GM->RHISetShaderParameter("ATMOSPHERE.solar_irradiance", atmosphere->solar_irradiance);
        _GM->RHISetShaderParameter("ATMOSPHERE.sun_angular_radius", atmosphere->sun_angular_radius);
        _GM->RHISetShaderParameter("ATMOSPHERE.bottom_radius", atmosphere->bottom_radius);
        _GM->RHISetShaderParameter("ATMOSPHERE.top_radius", atmosphere->top_radius);
        set_density_profile(atmosphere->rayleigh_density, "rayleigh_density");
        _GM->RHISetShaderParameter("ATMOSPHERE.rayleigh_scattering", atmosphere->rayleigh_scattering);
        set_density_profile(atmosphere->mie_density, "mie_density");
        _GM->RHISetShaderParameter("ATMOSPHERE.mie_scattering", atmosphere->mie_scattering);
        _GM->RHISetShaderParameter("ATMOSPHERE.mie_extinction", atmosphere->mie_extinction);
        _GM->RHISetShaderParameter("ATMOSPHERE.mie_phase_function_g", atmosphere->mie_phase_function_g);
        set_density_profile(atmosphere->absorption_density, "absorption_density");
        _GM->RHISetShaderParameter("ATMOSPHERE.absorption_extinction", atmosphere->absorption_extinction);
        _GM->RHISetShaderParameter("ATMOSPHERE.ground_albedo", atmosphere->ground_albedo);
        _GM->RHISetShaderParameter("ATMOSPHERE.mu_s_min", atmosphere->mu_s_min);
    }

    void SetWaterbodyParameters(std::shared_ptr<SceneObjectWaterbody> waterbody)
    {
        _GM->RHISetShaderParameter("WATERBODY.water_depth", waterbody->water_depth);
        _GM->RHISetShaderParameter("WATERBODY.fog_density", waterbody->fog_density);
        _GM->RHISetShaderParameter("WATERBODY.zeta", waterbody->zeta);
        _GM->RHISetShaderParameter("WATERBODY.gf", waterbody->gf);
        _GM->RHISetShaderParameter("WATERBODY.gb", waterbody->gb);
        _GM->RHISetShaderParameter("WATERBODY.hdr_exposure", waterbody->hdr_exposure);
        _GM->RHISetShaderParameter("WATERBODY.pure_water_scattering", waterbody->pure_water_scattering);
        _GM->RHISetShaderParameter("WATERBODY.minerals_scattering", waterbody->minerals_scattering);
        _GM->RHISetShaderParameter("WATERBODY.phytoplankton_scattering", waterbody->phytoplankton_scattering);
        _GM->RHISetShaderParameter("WATERBODY.pure_water_absorbtion", waterbody->pure_water_absorbtion);
        _GM->RHISetShaderParameter("WATERBODY.minerals_absorbtion", waterbody->minerals_absorbtion);
        _GM->RHISetShaderParameter("WATERBODY.phytoplankton_absorbtion", waterbody->phytoplankton_absorbtion);
        _GM->RHISetShaderParameter("WATERBODY.CDOM_absorbtion", waterbody->CDOM_absorbtion);
        _GM->RHISetShaderParameter("WATERBODY.total_scattering", waterbody->total_scattering);
        _GM->RHISetShaderParameter("WATERBODY.total_absorbtion", waterbody->total_absorbtion);
        _GM->RHISetShaderParameter("WATERBODY.total_extinction", waterbody->total_extinction);
    }

    RHITexture2DRef UploadTexture(std::shared_ptr<SceneObjectTexture> tex, bool EnableMipmap)
    {
        auto img = tex->GetTextureImage();
        int nummips = 1;
        if (EnableMipmap)
            nummips = std::min(log2(img->Width), log2(img->Height));
        auto texture = _GM->RHICreateTexture2D("",
            img->GetPixelFormat(), nummips, img->Width, img->Height, img->data);
        return texture;
    }

}

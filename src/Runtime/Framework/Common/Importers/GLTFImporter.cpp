#include "Common/AssetImporter.h"
#include "Material.h"
#include "Texture2D.h"
#include "StaticMeshResources.h"
#include <regex>

namespace fs = std::filesystem;

namespace nilou {

    static bool _LoadGLTFModel(const std::string& InFilePath, tinygltf::Model& model)
    {
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;
        bool ret;
        if (GameStatics::EndsWith(InFilePath, ".glb"))
        {
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, InFilePath);
        }
        else if (GameStatics::EndsWith(InFilePath, ".gltf"))
        {
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, InFilePath);
        }
        else
        {
            NILOU_LOG(Warning, "FGLTFImporter::Import: Unsupported file format: {}", InFilePath);
            return false;
        }

        if (!warn.empty()) {
            NILOU_LOG(Warning, "FGLTFImporter::Import warning from tinygltf: {}", warn)
        }
        if (!err.empty()) {
            NILOU_LOG(Error, "FGLTFImporter::Import error from tinygltf: {}", err)
        }
        if (!ret) {
            NILOU_LOG(Error, "FGLTFImporter::Import failed to load glTF: {}", InFilePath)
        }
        return ret;
    }

    static ESamplerFilter _GLTFFilterToETextureFilters(uint32 GLTFFilter)
    {
        ESamplerFilter filter;
        switch (GLTFFilter) {
            case TINYGLTF_TEXTURE_FILTER_NEAREST: filter = ESamplerFilter::SF_Point; break;
            case TINYGLTF_TEXTURE_FILTER_LINEAR: filter = ESamplerFilter::SF_Bilinear; break;
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: filter = ESamplerFilter::SF_Trilinear; break;
        }
        return filter;
    }
    
    static ESamplerAddressMode _GLTFFilterToETextureWrapModes(uint32 GLTFWrapMode)
    {
        ESamplerAddressMode mode;
        switch (GLTFWrapMode) {
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: mode = ESamplerAddressMode::AM_Clamp; break;
            case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: mode = ESamplerAddressMode::AM_Mirror; break;
            case TINYGLTF_TEXTURE_WRAP_REPEAT: mode = ESamplerAddressMode::AM_Wrap; break;
        }
        return mode;
    }

    static void _ImportMaterials(const fs::path& FilePath, const std::string& OutDirectory, tinygltf::Model& model, std::vector<UMaterial*>& OutMaterials, std::vector<UTexture2D*>& OutTextures)
    {
        for (int TextureIndex = 0; TextureIndex < model.textures.size(); TextureIndex++)
        {
            tinygltf::Texture &gltf_texture = model.textures[TextureIndex];
            tinygltf::Image gltf_image = model.images[gltf_texture.source];
            std::string texture_name = gltf_texture.name;
            if (texture_name == "")
                texture_name = gltf_image.name;
            if (texture_name == "")
                texture_name = fs::path(gltf_image.uri).filename().replace_extension().generic_string();
            if (texture_name == "")
                texture_name = "Texture_" + std::to_string(TextureIndex)+ "_"  + FilePath.filename().replace_extension().generic_string();
            UTexture2D* Texture = GetContentManager()->CreateAsset<UTexture2D>(texture_name, OutDirectory);

            // Currently only support 8-bit per channel
            EPixelFormat Format;
            if (gltf_image.component == 3)
            {
                Format = EPixelFormat::PF_R8G8B8;
            }
            else if (gltf_image.component == 4)
            {
                Format = EPixelFormat::PF_R8G8B8A8;
            }
            else
            {
                Format = EPixelFormat::PF_R8;
            }

            FImage image = FImage(
                gltf_image.width, gltf_image.height, 
                Format,
                EImageType::IT_Image2D, 1);
            image.AllocateSpace();
            memcpy(image.GetData(), gltf_image.image.data(), image.GetDataSize());
            Texture->ImageData = image;

            int NumMips = std::min(std::log2(gltf_image.width), std::log2(gltf_image.height)) + 1;
            
            FSamplerStateInitializer& SamplerState = Texture->SamplerState;
            if (gltf_texture.sampler != -1)
            {
                tinygltf::Sampler &sampler = model.samplers[gltf_texture.sampler];

                // if (sampler.minFilter != -1)
                //     SamplerState.Filter = _GLTFFilterToETextureFilters(sampler.minFilter);
                // if (sampler.magFilter != -1)
                //     SamplerState.Mag_Filter = _GLTFFilterToETextureFilters(sampler.magFilter);
                SamplerState.AddressU = _GLTFFilterToETextureWrapModes(sampler.wrapS);
                SamplerState.AddressV = _GLTFFilterToETextureWrapModes(sampler.wrapT);
            }
            Texture->UpdateResource();
            Texture->MarkAssetDirty();
            OutTextures.push_back(Texture);
        }

        for (int MaterialIndex = 0; MaterialIndex < model.materials.size(); MaterialIndex++)
        {
            tinygltf::Material &gltf_material = model.materials[MaterialIndex];
            if (gltf_material.name == "")
                gltf_material.name = "Material_" + std::to_string(MaterialIndex) + "_" + FilePath.filename().replace_extension().generic_string();
            UMaterial *Material = GetContentManager()->CreateAsset<UMaterial>(gltf_material.name, OutDirectory);
            Material->SetShadingModel(EShadingModel::SM_DefaultLit);

            {   // Generate and save the body of the shader
                std::string uniform_block_code =    "layout (std140, binding=0) uniform MAT_UNIFORM_BLOCK {\n"
                                                    "   vec4 baseColorFactor;\n"
                                                    "   vec3 emissiveFactor;\n"
                                                    "   float metallicFactor;\n"
                                                    "   float roughnessFactor;\n";
                std::string samplers_code = "";
                std::string material_interface_code =   "vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)\n"
                                                        "{\n"
                                                        "    return vec3(0);\n"
                                                        "}\n";

                if (gltf_material.pbrMetallicRoughness.baseColorTexture.index != -1)
                {
                    samplers_code += "layout (binding=1) uniform sampler2D baseColorTexture;\n";
                    material_interface_code += "vec4 MaterialGetBaseColor(VS_Out vs_out)\n"
                                            "{\n"
                                            "   return texture(baseColorTexture, vs_out.TexCoords) * baseColorFactor;\n"
                                            "}\n";
                }
                else 
                {
                    uniform_block_code += "   vec4 baseColor;\n";
                    material_interface_code += "vec4 MaterialGetBaseColor(VS_Out vs_out)\n"
                                            "{\n"
                                            "   return baseColor * baseColorFactor;\n"
                                            "}\n";
                }
                
                if (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
                {
                    samplers_code += "layout (binding=2) uniform sampler2D metallicRoughnessTexture;\n";
                    material_interface_code += "float MaterialGetMetallic(VS_Out vs_out)\n"
                                            "{\n"
                                            "   return texture(metallicRoughnessTexture, vs_out.TexCoords).b * metallicFactor;\n"
                                            "}\n"
                                            "float MaterialGetRoughness(VS_Out vs_out)\n"
                                            "{\n"
                                            "   return texture(metallicRoughnessTexture, vs_out.TexCoords).g * roughnessFactor;\n"
                                            "}\n";
                }
                else 
                {
                    uniform_block_code += "   float metallic;\n"
                                        "   float roughness;\n";
                    material_interface_code += "float MaterialGetMetallic(VS_Out vs_out)\n"
                                            "{\n"
                                            "   return metallic * metallicFactor;\n"
                                            "}\n"
                                            "float MaterialGetRoughness(VS_Out vs_out)\n"
                                            "{\n"
                                            "   return roughness * roughnessFactor;\n"
                                            "}\n";
                }
                
                if (gltf_material.normalTexture.index != -1)
                {
                    samplers_code += "layout (binding=3) uniform sampler2D normalTexture;\n";
                    material_interface_code +=  "vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)\n"
                                                "{\n"
                                                "    vec3 tangent_normal = texture(normalTexture, vs_out.TexCoords).rgb;\n"
                                                "    tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);\n"
                                                "    return normalize(vs_out.TBN * tangent_normal);\n"
                                                "}\n";
                }
                else
                {
                    material_interface_code +=  "vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)\n"
                                                "{\n"
                                                "    vec3 tangent_normal = vec3(0, 0, 1);\n"
                                                "    return normalize(vs_out.TBN * tangent_normal);\n"
                                                "}\n";
                }
                
                if (gltf_material.emissiveTexture.index != -1)
                {
                    samplers_code += "layout (binding=4) uniform sampler2D emissiveTexture;\n";
                    material_interface_code += "vec3 MaterialGetEmissive(VS_Out vs_out)\n"
                                            "{\n"
                                            "   return texture(emissiveTexture, vs_out.TexCoords).rgb * emissiveFactor;\n"
                                            "}\n";
                }
                else
                {
                    uniform_block_code += "   vec3 emissive;\n";
                    material_interface_code += "vec3 MaterialGetEmissive(VS_Out vs_out)\n"
                                            "{\n"
                                            "   return emissive * emissiveFactor;\n"
                                            "}\n";
                }
                uniform_block_code += "};\n";
                std::string glsl_code = "#version 460\n"
                                        "#include \"../include/Macros.glsl\"\n"
                                        "#include \"../include/BasePassCommon.glsl\"\n"+
                                        uniform_block_code +
                                        samplers_code +
                                        material_interface_code;
                std::string ShaderAbsPath = FPath::VirtualPathToAbsPath(OutDirectory + "/" + Material->GetName() + ".glsl").generic_string();
                std::ofstream out_stream(ShaderAbsPath);
                out_stream << glsl_code;
                out_stream.close();
                Material->SetShaderFileVirtualPath(OutDirectory + "/" + Material->GetName() + ".glsl");
            }

            
            {   // Set parameter values of the material
                if (gltf_material.pbrMetallicRoughness.baseColorTexture.index != -1)
                {
                    Material->SetTextureParameterValue("baseColorTexture", OutTextures[gltf_material.pbrMetallicRoughness.baseColorTexture.index]);
                }
                else
                {
                    Material->SetVectorParameterValue("baseColor", vec4(0));
                }
                if (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
                {
                    Material->SetTextureParameterValue("metallicRoughnessTexture", OutTextures[gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index]);
                }
                else
                {
                    Material->SetScalarParameterValue("metallic", 0.0f);
                    Material->SetScalarParameterValue("roughness", 0.8f);
                }
                if (gltf_material.normalTexture.index != -1)
                {
                    Material->SetTextureParameterValue("normalTexture", OutTextures[gltf_material.normalTexture.index]);
                }
                if (gltf_material.emissiveTexture.index != -1)
                {
                    Material->SetTextureParameterValue("emissiveTexture", OutTextures[gltf_material.emissiveTexture.index]);
                }
                else
                {
                    Material->SetVectorParameterValue("emissive", vec4(0));
                }
                Material->SetVectorParameterValue("baseColorFactor", vec4(1.0f, 1.0f, 1.0f, 1.0f));
                Material->SetVectorParameterValue("emissiveFactor", vec4(1.0f, 1.0f, 1.0f, 1.0f));
                Material->SetScalarParameterValue("metallicFactor", 1.0f);
                Material->SetScalarParameterValue("roughnessFactor", 1.0f);
            }

            Material->MarkAssetDirty();
            OutMaterials.push_back(Material);
        }
    }

    template<class T>
    static std::vector<T> BufferToVector(uint8 *pos, int count, size_t stride)
    {
        stride = stride == 0 ? sizeof(T) : stride;
        std::vector<T> Vertices;
        for (int i = 0; i < count; i++)
        {
            T *data = reinterpret_cast<T*>(pos);
            Vertices.push_back(*data);
            pos = pos + stride;
        }
        return Vertices;
    }
    static void _CalcNodeTransform(tinygltf::Model& model, int node_idx, const FTransform& parent_transform, std::vector<FTransform>& node_transforms)
    {
        auto& node = model.nodes[node_idx];
        FTransform local_transform;
        if (!node.matrix.empty())
        {
            mat4 transform = mat4(
                node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
                node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
                node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
                node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]);
            local_transform.SetFromMatrix(transform);            
        }
        else
        {
            if (!node.scale.empty())
            {
                local_transform.SetScale3D(vec3(node.scale[0], node.scale[1], node.scale[2]));
            }
            if (!node.rotation.empty())
            {
                quat rotation = quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
                local_transform.SetRotation(rotation);
            }
            if (!node.translation.empty())
            {
                local_transform.SetTranslation(vec3(node.translation[0], node.translation[1], node.translation[2]));
            }
        }
        FTransform global_transform = parent_transform * local_transform;
        node_transforms[node_idx] = global_transform;
        for (int child : node.children)
        {
            _CalcNodeTransform(model, child, global_transform, node_transforms);
        }
    }
    static void _ImportMesh(const fs::path& FilePath, const std::string& OutDirectory, const std::vector<UMaterial*>& Materials, tinygltf::Model& model, std::vector<UStaticMesh*>& OutMeshes)
    {
        std::vector<FTransform> node_transforms(model.nodes.size());
        for (auto& Scene : model.scenes)
        {
            for (int root : Scene.nodes)
            {
                // gltf may have multiple root nodes
                _CalcNodeTransform(model, root, FTransform::Identity, node_transforms);
            }
        }
        std::vector<FTransform> mesh_transforms(model.meshes.size());
        for (auto& node : model.nodes)
        {
            if (node.mesh != -1)
            {
                mesh_transforms[node.mesh] = node_transforms[node.mesh];
            }
        }
        // simply swap Y and Z axis
        FTransform axis_transform;
        axis_transform.SetFromMatrix(dmat4(1.0, 0.0, 0.0, 0.0,
                                                0.0, 0.0, 1.0, 0.0,
                                                0.0, 1.0, 0.0, 0.0,
                                                0.0, 0.0, 0.0, 1.0));
        for (int MeshIndex = 0; MeshIndex < model.meshes.size(); MeshIndex++)
        {
            tinygltf::Mesh &gltf_mesh = model.meshes[MeshIndex];
            const FTransform& transform = mesh_transforms[MeshIndex] * axis_transform;
            if (gltf_mesh.name == "")
                gltf_mesh.name = "Mesh_" + std::to_string(MeshIndex) + "_" + FilePath.filename().replace_extension().generic_string();
            UStaticMesh *StaticMesh = GetContentManager()->CreateAsset<UStaticMesh>(gltf_mesh.name, OutDirectory);

            FMeshDescription MeshDesc;
            for (int prim_index = 0; prim_index < gltf_mesh.primitives.size(); prim_index++)
            {
                tinygltf::Primitive &gltf_prim = gltf_mesh.primitives[prim_index];
                FMeshDescription::Primitive& primitive = MeshDesc.Primitives.emplace_back();
                
                {   // Material
                    if (gltf_prim.material != -1)
                    {
                        primitive.MaterialIndex = StaticMesh->MaterialSlots.size();
                        StaticMesh->MaterialSlots.push_back(Materials[gltf_prim.material]);
                    }
                }

                {   // Index
                    tinygltf::Accessor &indices_accessor = model.accessors[gltf_prim.indices];
                    tinygltf::BufferView &indices_bufferview = model.bufferViews[indices_accessor.bufferView];
                    tinygltf::Buffer &indices_buffer = model.buffers[indices_bufferview.buffer];
                    uint8 *idx_ptr = indices_buffer.data.data() + indices_bufferview.byteOffset + indices_accessor.byteOffset;
                    Ncheck(indices_accessor.type == TINYGLTF_TYPE_SCALAR);
                    int stride = indices_accessor.ByteStride(indices_bufferview);
                    for (int i = 0; i < indices_accessor.count; i++)
                    {
                        if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            uint8 *data = reinterpret_cast<uint8*>(idx_ptr);
                            primitive.Indices.push_back(*data);
                        }
                        else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            uint16 *data = reinterpret_cast<uint16*>(idx_ptr);
                            primitive.Indices.push_back(*data);
                        }
                        else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                        {
                            uint32 *data = reinterpret_cast<uint32*>(idx_ptr);
                            primitive.Indices.push_back(*data);
                        }
                        idx_ptr += stride;
                    }
                    for (int i = 0; i < indices_accessor.count; i+=3)
                    {
                        // Since Y and Z axis are swapped, we need to swap the winding order
                        std::swap(primitive.Indices[i], primitive.Indices[i + 2]);
                    }
                }

                {   // Vertex
                    for (auto &[attr_name, attr_index] : gltf_prim.attributes)
                    {
                        tinygltf::Accessor &attr_accessor = model.accessors[attr_index];
                        tinygltf::BufferView &attr_bufferview = model.bufferViews[attr_accessor.bufferView];
                        tinygltf::Buffer &attr_buffer = model.buffers[attr_bufferview.buffer];
                        std::regex re("TEXCOORD_([0-9]+)");
                        std::smatch match;
                        if (attr_name == "POSITION")
                        {
                            Ncheck(attr_accessor.type == TINYGLTF_TYPE_VEC3 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                            uint8 *pos_ptr = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                            primitive.Positions = BufferToVector<vec3>(pos_ptr, attr_accessor.count, attr_bufferview.byteStride);
                            // GLTF defines +Y as up, +Z as forward and -X as right
                            // While Nilou defines +Z as up, +X as forward and +Y as right
                            // So we need to convert the position
                            for (vec3 &pos : primitive.Positions)
                            {
                                pos = transform.TransformPosition(pos);
                            }
                        }
                        else if (attr_name == "NORMAL")
                        {
                            Ncheck(attr_accessor.type == TINYGLTF_TYPE_VEC3 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                            uint8 *nrm_ptr = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                            primitive.Normals = BufferToVector<vec3>(nrm_ptr, attr_accessor.count, attr_bufferview.byteStride);
                            for (vec3 &nrm : primitive.Normals)
                            {
                                nrm = transform.TransformVector(nrm);
                            }
                        }
                        else if (attr_name == "TANGENT")
                        {
                            Ncheck(attr_accessor.type == TINYGLTF_TYPE_VEC4 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                            uint8 *tng_ptr = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                            primitive.Tangents = BufferToVector<vec4>(tng_ptr, attr_accessor.count, attr_bufferview.byteStride);
                            for (vec4 &tng : primitive.Tangents)
                            {
                                tng = vec4(transform.TransformVector(vec3(tng)), tng.w);
                            }
                        }
                        else if (attr_name == "COLOR") 
                        {
                            Ncheck(attr_accessor.type == TINYGLTF_TYPE_VEC4 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                            uint8 *color_ptr = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                            primitive.Colors = BufferToVector<vec4>(color_ptr, attr_accessor.count, attr_bufferview.byteStride);
                        }
                        else if (std::regex_match(attr_name, match, re))
                        {
                            int texcoord_index = std::stoi(match[1].str());
                            Ncheck(0 <= texcoord_index && texcoord_index < MAX_STATIC_TEXCOORDS);
                            Ncheck(attr_accessor.type == TINYGLTF_TYPE_VEC2 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                            uint8 *uv_ptr = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                            primitive.TexCoords[texcoord_index] = BufferToVector<vec2>(uv_ptr, attr_accessor.count, attr_bufferview.byteStride);
                        }
                        else 
                        {
                            NILOU_LOG(Warning, "FGLTFImporter::Import: Unsupported attribute name: {}", attr_name);
                        }
                    }
                }
            }

            StaticMesh->Build(MeshDesc);
            StaticMesh->MarkAssetDirty();
            OutMeshes.push_back(StaticMesh);
        }
    }

    void FGLTFImporter::Import(const std::string& InFilePath, const std::string& OutDirectory, std::vector<UTexture2D*>& Textures, std::vector<UMaterial*>& Materials, std::vector<UStaticMesh*>& Meshes)
    {
        tinygltf::Model model;
        if (!_LoadGLTFModel(InFilePath, model))
        {
            return;
        }
        fs::path FilePath = InFilePath;

        _ImportMaterials(FilePath, OutDirectory, model, Materials, Textures);
        
        _ImportMesh(FilePath, OutDirectory, Materials, model, Meshes);
    }

}
#include <fstream>
#include <sstream>
#include <regex>

#include "BaseApplication.h"
#include "StaticMeshResources.h"
#include "Common/ContentManager.h"
#include "Material.h"
#include "Common/World.h"
#include "Cesium3DTilesetComponent.h"

#include "PrimitiveUtils.h"
#include "Common/Asset/AssetLoader.h"
#include "Common/InputManager.h"
#include "Texture2D.h"
#include "Common/Actor/CameraActor.h"


namespace nilou {


    static ETextureFilters GLTFFilterToETextureFilters(uint32 GLTFFilter)
    {
        ETextureFilters filter;
        switch (GLTFFilter) {
            case TINYGLTF_TEXTURE_FILTER_NEAREST: filter = ETextureFilters::TF_Nearest; break;
            case TINYGLTF_TEXTURE_FILTER_LINEAR: filter = ETextureFilters::TF_Linear; break;
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: filter = ETextureFilters::TF_Nearest_Mipmap_Nearest; break;
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST: filter = ETextureFilters::TF_Linear_Mipmap_Nearest; break;
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR: filter = ETextureFilters::TF_Nearest_Mipmap_Linear; break;
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: filter = ETextureFilters::TF_Linear_Mipmap_Linear; break;
        }
        return filter;
    }
    static ETextureWrapModes GLTFFilterToETextureWrapModes(uint32 GLTFWrapMode)
    {
        ETextureWrapModes mode;
        switch (GLTFWrapMode) {
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: mode = ETextureWrapModes::TW_Clamp; break;
            case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: mode = ETextureWrapModes::TW_Mirrored_Repeat; break;
            case TINYGLTF_TEXTURE_WRAP_REPEAT: mode = ETextureWrapModes::TW_Repeat; break;
        }
        return mode;
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

    static void ParseToMaterials(tinygltf::Model &model, 
        std::vector<std::shared_ptr<UMaterialInstance>> &OutMaterials, 
        std::vector<std::shared_ptr<UTexture>> &OutTextures,
        TUniformBufferRef<FGLTFMaterialBlock> &OutUniformBuffer)
    {
        OutMaterials.clear();
        OutTextures.clear();
        for (int TextureIndex = 0; TextureIndex < model.textures.size(); TextureIndex++)
        {
            tinygltf::Texture &gltf_texture = model.textures[TextureIndex];
            tinygltf::Image gltf_image = model.images[gltf_texture.source];

            std::shared_ptr<FImage2D> image = std::make_shared<FImage2D>(
                gltf_image.width, gltf_image.height, 
                TranslateToEPixelFormat(gltf_image.component, gltf_image.bits, gltf_image.pixel_type), 1);
            image->AllocateSpace();
            memcpy(image->GetData(), gltf_image.image.data(), image->GetDataSize());

            std::string TextureName = std::to_string(TextureIndex) + "_" + gltf_texture.name;
            int NumMips = std::min(std::log2(gltf_image.width), std::log2(gltf_image.height)) + 1;
            
            std::shared_ptr<UTexture2D> Texture = std::make_shared<UTexture2D>(TextureName);
            Texture->ImageData = image;
            Texture->UpdateResource();
            
            RHITextureParams &TextureParams = Texture->GetResource()->SamplerRHI.Params;
            if (gltf_texture.sampler != -1)
            {
                tinygltf::Sampler &sampler = model.samplers[gltf_texture.sampler];

                if (sampler.minFilter != -1)
                    TextureParams.Min_Filter = GLTFFilterToETextureFilters(sampler.minFilter);
                if (sampler.magFilter != -1)
                    TextureParams.Mag_Filter = GLTFFilterToETextureFilters(sampler.magFilter);
                TextureParams.Wrap_S = GLTFFilterToETextureWrapModes(sampler.wrapS);
                TextureParams.Wrap_T = GLTFFilterToETextureWrapModes(sampler.wrapT);
            }
            OutTextures.push_back(Texture);
        }

        UMaterial *GLTFMaterial = GetContentManager()->GetMaterialByPath("/Materials/Cesium3DTilesMaterial.nasset");
        if (GLTFMaterial == nullptr) return;

        for (int MaterialIndex = 0; MaterialIndex < model.materials.size(); MaterialIndex++)
        {
            tinygltf::Material &gltf_material = model.materials[MaterialIndex];
            std::shared_ptr<UMaterialInstance> Material(GLTFMaterial->CreateMaterialInstance());

            if (gltf_material.pbrMetallicRoughness.baseColorTexture.index != -1)
            {
                int index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
                Material->SetTextureParameterValue("baseColorTexture", OutTextures[index].get());
            }
            else 
            {
                Material->SetTextureParameterValue("baseColorTexture", GetContentManager()->GetTextureByPath("/Textures/NoColorTexture.nasset"));
            }
            
            if (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
            {
                int index = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
                Material->SetTextureParameterValue("metallicRoughnessTexture", OutTextures[index].get());
            }
            else 
            {
                Material->SetTextureParameterValue("metallicRoughnessTexture", GetContentManager()->GetTextureByPath("/Textures/NoMetallicRoughnessTexture.nasset"));
            }
            
            if (gltf_material.emissiveTexture.index != -1)
            {
                int index = gltf_material.emissiveTexture.index;
                Material->SetTextureParameterValue("emissiveTexture", OutTextures[index].get());
            }
            else 
            {
                Material->SetTextureParameterValue("emissiveTexture", GetContentManager()->GetTextureByPath("/Textures/NoEmissiveTexture.nasset"));
            }
            
            if (gltf_material.normalTexture.index != -1)
            {
                int index = gltf_material.normalTexture.index;
                Material->SetTextureParameterValue("normalTexture", OutTextures[index].get());
            }
            else 
            {
                Material->SetTextureParameterValue("normalTexture", GetContentManager()->GetTextureByPath("/Textures/NoNormalTexture.nasset"));
            }

            OutUniformBuffer = CreateUniformBuffer<FGLTFMaterialBlock>();
            OutUniformBuffer->Data.baseColorFactor.r = gltf_material.pbrMetallicRoughness.baseColorFactor[0];
            OutUniformBuffer->Data.baseColorFactor.g = gltf_material.pbrMetallicRoughness.baseColorFactor[1];
            OutUniformBuffer->Data.baseColorFactor.b = gltf_material.pbrMetallicRoughness.baseColorFactor[2];
            OutUniformBuffer->Data.baseColorFactor.a = gltf_material.pbrMetallicRoughness.baseColorFactor[3];
            OutUniformBuffer->Data.emissiveFactor.r = gltf_material.emissiveFactor[0];
            OutUniformBuffer->Data.emissiveFactor.g = gltf_material.emissiveFactor[1];
            OutUniformBuffer->Data.emissiveFactor.b = gltf_material.emissiveFactor[2];
            OutUniformBuffer->Data.metallicFactor = gltf_material.pbrMetallicRoughness.metallicFactor;
            OutUniformBuffer->Data.roughnessFactor = gltf_material.pbrMetallicRoughness.roughnessFactor;
            OutMaterials.push_back(Material);
            Material->SetParameterValue("FGLTFMaterialBlock", OutUniformBuffer.get());
        }
    }

	GLTFParseResult ParseToStaticMeshes(tinygltf::Model &model, bool need_init)
    {
        GLTFParseResult Result;
        std::vector<std::shared_ptr<UStaticMesh>> &StaticMeshes = Result.StaticMeshes;
        std::vector<std::shared_ptr<UMaterialInstance>> &Materials = Result.Materials;
        std::vector<std::shared_ptr<UTexture>> &Textures = Result.Textures;
        TUniformBufferRef<FGLTFMaterialBlock> &UniformBuffer = Result.UniformBuffer; 
        ParseToMaterials(model, Materials, Textures, UniformBuffer);
        for (auto &gltf_mesh : model.meshes)
        {
            std::shared_ptr<UStaticMesh> StaticMesh = std::make_shared<UStaticMesh>(gltf_mesh.name);
            FStaticMeshLODResources* Resource = new FStaticMeshLODResources();
            for (int prim_index = 0; prim_index < gltf_mesh.primitives.size(); prim_index++)
            {
                tinygltf::Primitive &gltf_prim = gltf_mesh.primitives[prim_index];
                FStaticVertexFactory::FDataType Data;
                FStaticMeshSection* Section = new FStaticMeshSection;
                
                {   // Material
                    if (gltf_prim.material != -1)
                    {
                        Section->MaterialIndex = StaticMesh->MaterialSlots.size();
                        StaticMesh->MaterialSlots.push_back(Materials[gltf_prim.material].get());
                    }
                }

                {   // Index
                    tinygltf::Accessor &indices_accessor = model.accessors[gltf_prim.indices];
                    tinygltf::BufferView &indices_bufferview = model.bufferViews[indices_accessor.bufferView];
                    tinygltf::Buffer &indices_buffer = model.buffers[indices_bufferview.buffer];
                    uint8 *pos = indices_buffer.data.data() + indices_bufferview.byteOffset + indices_accessor.byteOffset;

                    if (indices_accessor.type == TINYGLTF_TYPE_SCALAR)
                    {
                        if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            Section->IndexBuffer.Init(BufferToVector<uint8>(pos, indices_accessor.count, indices_bufferview.byteStride));
                        }
                        else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) 
                        {
                            Section->IndexBuffer.Init(BufferToVector<uint16>(pos, indices_accessor.count, indices_bufferview.byteStride));
                        }
                        else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) 
                        {
                            Section->IndexBuffer.Init(BufferToVector<uint32>(pos, indices_accessor.count, indices_bufferview.byteStride));
                        }
                    }
                }

                bool HaveTexCoords = false;
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
                            if (attr_accessor.type == TINYGLTF_TYPE_VEC3 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                            {
                                uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                                Section->VertexBuffers.Positions.Init(BufferToVector<glm::vec3>(pos, attr_accessor.count, attr_bufferview.byteStride));
                                Section->VertexBuffers.Positions.BindToVertexFactoryData(Data.PositionComponent);
                                StaticMesh->LocalBoundingBox.Min = vec3(attr_accessor.minValues[0], attr_accessor.minValues[1], attr_accessor.minValues[2]);
                                StaticMesh->LocalBoundingBox.Max = vec3(attr_accessor.maxValues[0], attr_accessor.maxValues[1], attr_accessor.maxValues[2]);
                            }
                        }
                        else if (attr_name == "NORMAL")
                        {
                            if (attr_accessor.type == TINYGLTF_TYPE_VEC3 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                            {
                                uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                                Section->VertexBuffers.Normals.Init(BufferToVector<glm::vec3>(pos, attr_accessor.count, attr_bufferview.byteStride));
                                Section->VertexBuffers.Normals.BindToVertexFactoryData(Data.NormalComponent);
                            }
                        }
                        else if (attr_name == "TANGENT")
                        {
                            if (attr_accessor.type == TINYGLTF_TYPE_VEC4 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                            {
                                uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                                Section->VertexBuffers.Tangents.Init(BufferToVector<glm::vec4>(pos, attr_accessor.count, attr_bufferview.byteStride));
                                Section->VertexBuffers.Tangents.BindToVertexFactoryData(Data.TangentComponent);
                            }
                        }
                        else if (std::regex_match(attr_name, match, re))
                        {
                            int texcoord_index = std::stoi(match[1].str());
                            if (texcoord_index < 0 || texcoord_index >= MAX_STATIC_TEXCOORDS)
                            {
                                NILOU_LOG(Error, "Invalid texcoord name: {}", attr_name);
                                continue;
                            }
                            if (attr_accessor.type == TINYGLTF_TYPE_VEC2 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                            {
                                HaveTexCoords = true;
                                uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                                Section->VertexBuffers.TexCoords[texcoord_index].Init(BufferToVector<glm::vec2>(pos, attr_accessor.count, attr_bufferview.byteStride));
                                Section->VertexBuffers.TexCoords[texcoord_index].BindToVertexFactoryData(Data.TexCoordComponent[texcoord_index]);
                            }
                        }
                        else 
                        {
                            std::cout << "[ERROR] Attribute not supported: " << attr_name << std::endl;
                        }
                    }
                }
                Section->VertexFactory.SetData(Data);
                Resource->Sections.push_back(Section);

            }
            StaticMeshes.push_back(StaticMesh);
            StaticMesh->RenderData = new FStaticMeshRenderData;
            StaticMesh->RenderData->LODResources.push_back(Resource);
        }

        if (need_init)
            Result.InitResource();
        return Result;
    }

    void GLTFParseResult::InitResource()
    {
        for (int i = 0; i < this->Textures.size(); i++)
        {
            BeginInitResource(this->Textures[i]->GetResource());
        }
        for (int i = 0; i < this->StaticMeshes.size(); i++)
        {
            this->StaticMeshes[i]->RenderData->InitResources();
        }
        BeginInitResource(this->UniformBuffer.get());
    }
}

namespace nilou {

    const glm::dmat4 X_UP_TO_Z_UP = glm::dmat4( 0.0, 0.0, 1.0, 0.0,
                                                0.0, 1.0, 0.0, 0.0,
                                                -1.0, 0.0, 0.0, 0.0,
                                                0.0, 0.0, 0.0, 1.0);

    const glm::dmat4 Y_UP_TO_Z_UP = glm::dmat4( 1.0, 0.0, 0.0, 0.0,
                                                0.0, 0.0, 1.0, 0.0,
                                                0.0, -1.0, 0.0, 0.0,
                                                0.0, 0.0, 0.0, 1.0);

    using namespace Cesium3DTilesetSelection;

    void BoundingVolumeConvert(FOrientedBoundingBox &Box, tiny3dtiles::BoundingVolume &BoundingVolume, const glm::dmat4 &transform)
    {
        auto &box = BoundingVolume.box;
        dvec3 center = transform * dvec4(box[0], box[1], box[2], 1.0);
        dmat3 halfAxis = dmat3(transform) * dmat3(box[3], box[4], box[5], box[6], box[7], box[8], box[9], box[10], box[11]);
        Box = FOrientedBoundingBox(center, halfAxis);
    }

    bool Cesium3DTile::HasRendableContent() const
    {
        return Content.B3dm.header_loaded();
    }

    Cesium3DTile::~Cesium3DTile()
    {
        auto this_TransformRHI = TransformRHI;
        auto Gltf = Content.Gltf;
        ENQUEUE_RENDER_COMMAND(Cesium3DTile_de)([this_TransformRHI, Gltf](FDynamicRHI*){
            if (this_TransformRHI)
                this_TransformRHI->ReleaseResource();
            for (int i = 0; i < Gltf.Materials.size(); i++)
                Gltf.Materials[i]->ReleaseResources();
            for (int i = 0; i < Gltf.Materials.size(); i++)
                Gltf.Textures[i]->ReleaseResource();
            for (int i = 0; i < Gltf.Materials.size(); i++)
                Gltf.StaticMeshes[i]->ReleaseResources();
            if (Gltf.UniformBuffer)
                Gltf.UniformBuffer->ReleaseResource();
        });
    }

    std::shared_ptr<Cesium3DTile> Cesium3DTile::BuildTile(
        std::shared_ptr<tiny3dtiles::Tile> Tile, const glm::dmat4 &parentTransform, ETileGltfUpAxis TileGltfUpAxis)
    {
        std::shared_ptr<Cesium3DTile> InternalTile = std::make_shared<Cesium3DTile>();
        InternalTile->TileGltfUpAxis = TileGltfUpAxis;
        InternalTile->GeometricError = Tile->geometricError;
        InternalTile->Refine = Tile->refine;
        InternalTile->Transform = parentTransform * Tile->transform;
        BoundingVolumeConvert(InternalTile->BoundingVolume, Tile->boundingVolume, InternalTile->Transform);
        InternalTile->Content.URI = Tile->content.uri;
        InternalTile->TransformRHI = CreateUniformBuffer<FPrimitiveShaderParameters>();
        if (!Tile->content.b3dm.featureTableJSON.empty())
        {
            nlohmann::json json;
            std::stringstream stream(Tile->content.b3dm.featureTableJSON);
            stream >> json;
            if (json.is_object() && json.contains("RTC_CENTER"))
            {
                nlohmann::json rtc_center = json["RTC_CENTER"];
                if (rtc_center.is_array() && 
                    rtc_center.size() == 3)
                {
                    InternalTile->RtcCenter.x = rtc_center.at(0).get<double>();
                    InternalTile->RtcCenter.y = rtc_center.at(1).get<double>();
                    InternalTile->RtcCenter.z = rtc_center.at(2).get<double>();
                }
            }
        }
        

        if (Tile->content.external_tileset != nullptr)
        {
            std::shared_ptr<Cesium3DTile> ExternalRoot = Cesium3DTileset::Build(Tile->content.external_tileset, InternalTile->Transform)->Root;
            InternalTile->Children.push_back(ExternalRoot);
        }
        else 
        {
            InternalTile->Content.B3dm = Tile->content.b3dm;
        }

        for (auto &child : Tile->children)
        {
            InternalTile->Children.push_back(BuildTile(child, InternalTile->Transform, TileGltfUpAxis));
        }

        return InternalTile;
    }

    std::shared_ptr<Cesium3DTileset> Cesium3DTileset::Build(
        std::shared_ptr<tiny3dtiles::Tileset> Tileset, const glm::dmat4 &parentTransform)
    {
        ETileGltfUpAxis TileGltfUpAxis = ETileGltfUpAxis::Y;
        if (Tileset->asset.find("gltfUpAxis") != Tileset->asset.end())
        {
            if (Tileset->asset["gltfUpAxis"] == "X")
                TileGltfUpAxis = ETileGltfUpAxis::X;
            else if (Tileset->asset["gltfUpAxis"] == "Y")
                TileGltfUpAxis = ETileGltfUpAxis::Y;
            else if (Tileset->asset["gltfUpAxis"] == "Z")
                TileGltfUpAxis = ETileGltfUpAxis::Z;
        }
        std::shared_ptr<Cesium3DTileset> InternalTileset = std::make_shared<Cesium3DTileset>();
        InternalTileset->Asset = Tileset->asset;
        InternalTileset->GeometricError = Tileset->geometricError;
        InternalTileset->Root = Cesium3DTile::BuildTile(Tileset->root, parentTransform, TileGltfUpAxis);
        return InternalTileset;
    }

    void UCesium3DTilesetComponent::Update(Cesium3DTileset *Tileset, const std::vector<ViewState> &ViewStates)
    {
        TilesToRenderThisFrame.clear();

        UpdateInternal(Tileset->Root.get(), ViewStates);

        UnloadTiles();

        LoadTiles();
    }

    void UCesium3DTilesetComponent::LoadTiles()
    {
        for (Cesium3DTile *Tile : LoadQueue)
        {
            pool.push_task([this, Tile]() {
                    if (!Tile->HasRendableContent())
                        return;
                    std::unique_lock<std::mutex> lock(Tile->mutex, std::try_to_lock);
                    // If it doesn't own the lock, it means this tile is loading,
                    // then we just return because we don't want to repeat the loading
                    if (!lock.owns_lock())
                        return;
                    tinygltf::Model model;
                    Tile->LoadingState = ETileLoadingState::Loading;
                    Tile->Content.B3dm.load_glb(Tile->Content.URI);
                    tinygltf::TinyGLTF Loader;
                    std::string err;
                    std::string warn;
                    Loader.LoadBinaryFromMemory(&model, &err, &warn, Tile->Content.B3dm.glb.data(), Tile->Content.B3dm.glb.size());
                    auto Result = ParseToStaticMeshes(model, false);
                    std::atomic<bool> RHIInitialized = false;
                    std::mutex rhi_mutex;
                    std::unique_lock<std::mutex> rhi_lock(rhi_mutex);
                    std::condition_variable cv;
                    ENQUEUE_RENDER_COMMAND(UCesium3DTilesetComponent_LoadContent)(
                        [&Result, this, &RHIInitialized, &cv](FDynamicRHI*) 
                        {
                            Result.InitResource();
                            RHIInitialized = true;
                            cv.notify_all();
                        });
                    if (!RHIInitialized)
                    {
                        cv.wait(rhi_lock, [&RHIInitialized](){ return RHIInitialized.load(); });
                    }
                    Tile->Content.Gltf = Result;
                    Tile->LoadingState = ETileLoadingState::Loaded;
                });
        }
        LoadQueue.clear();
    }

    void UCesium3DTilesetComponent::UnloadTiles()
    {
        for (Cesium3DTile *Tile : UnloadQueue)
        {
            ENQUEUE_RENDER_COMMAND(UCesium3DTilesetComponent_UnloadTiles)(
                [this, Tile](FDynamicRHI*) 
                {
                    std::unique_lock<std::mutex> lock(Tile->mutex, std::try_to_lock);
                    // If it doesn't own the lock, it means this tile is being rendered.
                    if (!lock.owns_lock())
                        return;
                    Tile->LoadingState = ETileLoadingState::Unloading;
                    Tile->Content.B3dm.reset_glb();
                    Tile->Content.Gltf.Materials.clear();
                    Tile->Content.Gltf.Textures.clear();
                    Tile->Content.Gltf.StaticMeshes.clear();
                    Tile->Content.Gltf.UniformBuffer = nullptr;
                    Tile->LoadingState = ETileLoadingState::Unloaded;
                });
        }
        UnloadQueue.clear();
    }

    void UCesium3DTilesetComponent::UpdateInternal(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        bool bTileCulled = bEnableFrustumCulling && FrustumCull(Tile, ViewStates);

        if (!bTileCulled)
        {
            bool wantToRefine = !MeetsSSE(Tile, ViewStates);

            if (wantToRefine)
            {
                if (Tile->IsLeaf())
                {
                    AddTileToRenderQueue(Tile);
                }
                else 
                {
                    for (std::shared_ptr<Cesium3DTile> tile : Tile->Children)
                    {
                        UpdateInternal(tile.get(), ViewStates);
                    }
                }
            }
            else 
            {
                if (!Tile->HasRendableContent())
                {
                    for (std::shared_ptr<Cesium3DTile> tile : Tile->Children)
                    {
                        UpdateInternal(tile.get(), ViewStates);
                    }
                }
                else 
                {
                    AddTileToRenderQueue(Tile);
                }
            }

        }

    }

    void UCesium3DTilesetComponent::AddTileToUnloadQueue(Cesium3DTile *Tile)
    {
        if (Tile->LoadingState == ETileLoadingState::Loaded)
            UnloadQueue.push_back(Tile);
    }

    void UCesium3DTilesetComponent::AddTileToRenderQueue(Cesium3DTile *Tile)
    {
        if (Tile->LoadingState == ETileLoadingState::Unloaded)
            LoadQueue.push_back(Tile);
        TilesToRenderThisFrame.push_back(Tile);
        auto ExpelledTile = LruCache.Put_ThreadSafe(Tile, Tile);
        if (ExpelledTile.has_value())
        {
            Cesium3DTile* expelled_tile = ExpelledTile->first;
            AddTileToUnloadQueue(expelled_tile);
        }
    }

    bool UCesium3DTilesetComponent::FrustumCull(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        bool bCullWithChildrenBounds = !Tile->Children.empty();
        if (bCullWithChildrenBounds)
        {
            if (std::any_of(ViewStates.begin(), ViewStates.end(), 
                    [Tile](const ViewState &ViewState) {
                        for (auto &&Child : Tile->Children)
                            if (!ViewState.frustum.IsBoxOutSideFrustumFast(Child->BoundingVolume))
                                return true;
                        return false;
                    }))
            {
                return false;
            }
        }
        else 
        {
            if (std::any_of(ViewStates.begin(), ViewStates.end(), 
                    [Tile](const ViewState &ViewState) {
                        return !ViewState.frustum.IsBoxOutSideFrustumFast(Tile->BoundingVolume);
                    }))
            {
                return false;
            }
        }
        return true;
    }

    bool UCesium3DTilesetComponent::MeetsSSE(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        double MaximumScreenSpaceError = GetMaxScreenSpaceError();

        double largestSSE = 0;
        for (auto &ViewState : ViewStates)
        {
            double distance = glm::distance(ViewState.cameraPosition, Tile->BoundingVolume.Center); 
            double sse = Tile->GeometricError * ViewState.resolution.y / (distance * ViewState.sseDenominator);
            largestSSE = glm::max(largestSSE, sse);
        }
        return largestSSE <= MaximumScreenSpaceError;
    }

    class FCesium3DTilesetSceneProxy : public FPrimitiveSceneProxy
    {
    public:

        FCesium3DTilesetSceneProxy(UCesium3DTilesetComponent *InComponent)
            : FPrimitiveSceneProxy(InComponent)
        {
            std::vector<FDynamicMeshVertex> OutVerts;
            std::vector<uint32> OutIndices;
            BuildCuboidVerts(2, 2, 2, OutVerts, OutIndices);
            IndexBuffer.Init(OutIndices);
            VertexBuffers.InitFromDynamicVertex(&VertexFactory, OutVerts);
            BeginInitResource(&IndexBuffer);
            PreRenderHandle = GetAppication()->GetPreRenderDelegate().Add(this, &FCesium3DTilesetSceneProxy::PreRenderCallBack);
            PostRenderHandle = GetAppication()->GetPostRenderDelegate().Add(this, &FCesium3DTilesetSceneProxy::PostRenderCallBack);
            WireframeMaterial = GetContentManager()->GetMaterialByPath("/Materials/WireframeMaterial.nasset");
        }

        void AddRenderingTiles(std::vector<Cesium3DTile *> Tiles)
        {
            std::unique_lock<std::mutex> lock(mutex);
            TilesRenderingQueue.push(Tiles);
        }

        void SetEcefToAbs(const dmat4 &InEcefToAbs)
        {
            EcefToAbs = InEcefToAbs;
        }

        void SetBoundingBoxVisibility(bool InShowBoundingBox)
        {
            this->bShowBoundingBox = InShowBoundingBox;
        }

        void PreRenderCallBack(FDynamicRHI*, FScene*)
        {
            // Fetch the rendering queue for this frame
            std::unique_lock<std::mutex> lock(mutex);
            if (!TilesRenderingQueue.empty())
            {
                TilesToRenderThisFrame = TilesRenderingQueue.front(); 
                TilesRenderingQueue.pop();
            }
            else 
            {
                TilesToRenderThisFrame.clear();
            }
            lock.unlock();

            // Expell those tile that haven't been loaded, and lock those tiles that are loaded until the end of frame.
            for (auto iter = TilesToRenderThisFrame.begin(); iter != TilesToRenderThisFrame.end();)
            {
                Cesium3DTile *Tile = *iter;
                bool locked = Tile->mutex.try_lock();
                if (!locked)
                {
                    iter = TilesToRenderThisFrame.erase(iter);
                }
                else 
                {
                    iter++;
                }
            }
        }

        void PostRenderCallBack(FDynamicRHI*, FScene*)
        {
            for (Cesium3DTile *Tile : TilesToRenderThisFrame)
            {
                Tile->mutex.unlock();
            }
        }

        virtual void DestroyRenderThreadResources() override
        {
            GetAppication()->GetPreRenderDelegate().Remove(PreRenderHandle);
            GetAppication()->GetPostRenderDelegate().Remove(PostRenderHandle);
            for (Cesium3DTile *Tile : TilesToRenderThisFrame)
            {
                Tile->mutex.unlock();
            }
            VertexBuffers.ReleaseResource();
            IndexBuffer.ReleaseResource();
            for (auto [key, UniformBuffer] : TileBoundingBoxUBO)
                UniformBuffer->ReleaseResource();
            FPrimitiveSceneProxy::DestroyRenderThreadResources();
        }

        virtual void GetDynamicMeshElements(const std::vector<FSceneView*> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    for (Cesium3DTile *Tile : TilesToRenderThisFrame)
                    {
                        dmat4 AxisTransform = dmat4(1);
                        if (Tile->TileGltfUpAxis == ETileGltfUpAxis::Y)
                            AxisTransform = Y_UP_TO_Z_UP;
                        else if (Tile->TileGltfUpAxis == ETileGltfUpAxis::X)
                            AxisTransform = X_UP_TO_Z_UP;
                        
                        dmat4 RtcCenterMatrix = dmat4(
                            dvec4(1, 0, 0, 0), 
                            dvec4(0, 1, 0, 0),
                            dvec4(0, 0, 1, 0),
                            dvec4(Tile->RtcCenter, 1)
                        );
                        Tile->TransformRHI->Data.LocalToWorld = GetLocalToWorld() * EcefToAbs * Tile->Transform * RtcCenterMatrix * AxisTransform;
                        if (!Tile->TransformRHI->IsInitialized())
                            Tile->TransformRHI->InitResource();
                        else
                            Tile->TransformRHI->UpdateUniformBuffer();
                        for (std::shared_ptr<UStaticMesh> StaticMesh : Tile->Content.Gltf.StaticMeshes)
                        {
                            const FStaticMeshLODResources& LODModel = *StaticMesh->RenderData->LODResources[0];
                            for (int SectionIndex = 0; SectionIndex < LODModel.Sections.size(); SectionIndex++)
                            {
                                const FStaticMeshSection &Section = *LODModel.Sections[SectionIndex];
                                FMeshBatch Mesh;
                                Mesh.CastShadow = Section.bCastShadow;
                                Mesh.Element.VertexFactory = &Section.VertexFactory;
                                Mesh.Element.IndexBuffer = &Section.IndexBuffer;
                                Mesh.Element.NumVertices = Section.GetNumVertices();
                                Mesh.MaterialRenderProxy = StaticMesh->MaterialSlots[Section.MaterialIndex]->GetRenderProxy();
                                Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", Tile->TransformRHI->GetRHI());
                                Collector.AddMesh(ViewIndex, Mesh);
                                if (bShowBoundingBox)
                                {
                                    if (TileBoundingBoxUBO.find(Tile) == TileBoundingBoxUBO.end())
                                    {
                                        TileBoundingBoxUBO[Tile] = CreateUniformBuffer<FPrimitiveShaderParameters>();
                                        BeginInitResource(TileBoundingBoxUBO[Tile].get());
                                    }
                                    dmat4 translation;
                                    translation = glm::translate(translation, Tile->BoundingVolume.Center);
                                    TUniformBufferRef<FPrimitiveShaderParameters> UBO = TileBoundingBoxUBO[Tile];
                                    UBO->Data.LocalToWorld = GetLocalToWorld() * EcefToAbs * translation * dmat4(Tile->BoundingVolume.HalfAxes);
                                    UBO->UpdateUniformBuffer();
                                    FMeshBatch DebugBoundingBoxMesh;
                                    DebugBoundingBoxMesh.CastShadow = false;
                                    DebugBoundingBoxMesh.Element.VertexFactory = &VertexFactory;
                                    DebugBoundingBoxMesh.Element.IndexBuffer = &IndexBuffer;
                                    DebugBoundingBoxMesh.Element.NumVertices = VertexBuffers.Positions.GetNumVertices();
                                    DebugBoundingBoxMesh.MaterialRenderProxy = WireframeMaterial->GetRenderProxy();
                                    DebugBoundingBoxMesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", TileBoundingBoxUBO[Tile]->GetRHI());
                                    Collector.AddMesh(ViewIndex, DebugBoundingBoxMesh);
                                }
                            }
                        }
                    }
                }
            }
        }

    protected:
        std::vector<Cesium3DTile *> TilesToRenderThisFrame;
        std::queue<std::vector<Cesium3DTile *>> TilesRenderingQueue;

        dmat4 EcefToAbs;

        bool bShowBoundingBox;
        FStaticMeshVertexBuffers VertexBuffers;
        FStaticMeshIndexBuffer IndexBuffer;
        FStaticVertexFactory VertexFactory;
        std::unordered_map<Cesium3DTile *, TUniformBufferRef<FPrimitiveShaderParameters>> TileBoundingBoxUBO;

    private:

        std::mutex mutex;
        FDelegateHandle PreRenderHandle;
        FDelegateHandle PostRenderHandle;

        UMaterial *WireframeMaterial;

    };

    UCesium3DTilesetComponent::UCesium3DTilesetComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
        , URI("")
        , Georeference(nullptr)
        , MaximumScreenSpaceError(16.0)
        , bEnableFrustumCulling(true)
        , Tileset(nullptr)
        , bShowBoundingBox(false)
        , TilesetForSelection(nullptr)
        , LruCache(MaxTilesToRender)
    {
    }

    void UCesium3DTilesetComponent::TickComponent(double DeltaTime)
    {
        UWorld *World = GetWorld();
        if (World)
        {
            std::vector<ViewState> ViewStates;
            std::vector<ACameraActor*> CameraActors = World->GetCameraActors();
            for (ACameraActor *CameraActor : CameraActors)
            {
                UCameraComponent* CameraComponent = CameraActor->GetCameraComponent();
                ViewState viewState;
                const dmat4 &AbsToEcef = Georeference->GetAbsToEcef();
                viewState.cameraPosition = AbsToEcef * dvec4(CameraComponent->GetComponentLocation(), 1.0);
                dvec3 forward = AbsToEcef * dvec4(CameraComponent->GetForwardVector(), 0.0);
                dvec3 up = AbsToEcef * dvec4(CameraComponent->GetUpVector(), 0.0);
                viewState.frustum = FViewFrustum(
                    viewState.cameraPosition, forward, up, 
                    CameraComponent->GetAspectRatio(), CameraComponent->GetFieldOfView(), 
                    CameraComponent->GetNearClipDistance(), CameraComponent->GetFarClipDistance());
                viewState.resolution = CameraComponent->GetCameraResolution();
                viewState.verticalFOV = CameraComponent->GetFieldOfView();
                viewState.sseDenominator = 2.0 * glm::tan(0.5 * viewState.verticalFOV);
                ViewStates.push_back(viewState);
            }

            if (TilesetForSelection)
            {
                Update(TilesetForSelection.get(), ViewStates);
            }
            else 
            {
                TilesToRenderThisFrame.clear();
            }

            MarkRenderDynamicDataDirty();
        }
    }

    void UCesium3DTilesetComponent::SendRenderDynamicData()
    {
        FCesium3DTilesetSceneProxy *proxy = static_cast<FCesium3DTilesetSceneProxy *>(SceneProxy);
        proxy->SetBoundingBoxVisibility(bShowBoundingBox);
        if (Georeference)
            proxy->SetEcefToAbs(Georeference->GetEcefToAbs());
        if (TilesetForSelection)
            proxy->AddRenderingTiles(TilesToRenderThisFrame);
        UPrimitiveComponent::SendRenderDynamicData();
    }

    FPrimitiveSceneProxy *UCesium3DTilesetComponent::CreateSceneProxy()
    {
        return new FCesium3DTilesetSceneProxy(this);
    }

    void UCesium3DTilesetComponent::OnRegister()
    {
        if (Georeference == nullptr && WorldPrivate != nullptr)
        {
            std::vector<AGeoreferenceActor*> Georeferences;
            WorldPrivate->GetAllActorsOfClass<AGeoreferenceActor>(Georeferences, false);
            if (!Georeferences.empty())
            {
                this->Georeference = Georeferences[0];
            }
            else 
            {
                this->Georeference = WorldPrivate->SpawnActor<AGeoreferenceActor>(FTransform::Identity, "CesiumGeoreference").get();
            }
        }

        UPrimitiveComponent::OnRegister();
    }

    FBoundingBox UCesium3DTilesetComponent::CalcBounds(const FTransform &LocalToWorld) const
    {
        if (TilesetForSelection)
        {
            const dmat4 &EcefToAbs = Georeference->GetEcefToAbs();
            dvec3 center = LocalToWorld.TransformPosition(EcefToAbs * dvec4(TilesetForSelection->Root->BoundingVolume.Center, 1));
            dvec3 xDirection = LocalToWorld.TransformVector(dmat3(EcefToAbs) * TilesetForSelection->Root->BoundingVolume.HalfAxes[0]);
            dvec3 yDirection = LocalToWorld.TransformVector(dmat3(EcefToAbs) * TilesetForSelection->Root->BoundingVolume.HalfAxes[1]);
            dvec3 zDirection = LocalToWorld.TransformVector(dmat3(EcefToAbs) * TilesetForSelection->Root->BoundingVolume.HalfAxes[2]);

            return FBoundingBox(center, xDirection, yDirection, zDirection);
        }
        return UPrimitiveComponent::CalcBounds(LocalToWorld);
    }

    void UCesium3DTilesetComponent::SetURI(const std::string &NewURI)
    { 
        // if (NewURI != URI)
        // {
            URI = NewURI; 
            tiny3dtiles::Loader Loader;
            Tileset = Loader.LoadTileset(URI);
            auto TilesetToRelease = TilesetForSelection;
            if (Tileset)
                TilesetForSelection = Cesium3DTileset::Build(Tileset, dmat4(1));
            else
                TilesetForSelection = nullptr;
            UpdateBounds();
            MarkRenderStateDirty();
        // }
    }

    void UCesium3DTilesetComponent::SetShowBoundingBox(bool InShowBoundingBox)
    {
        bShowBoundingBox = InShowBoundingBox;
        MarkRenderDynamicDataDirty();
    }

    void UCesium3DTilesetComponent::SetGeoreference(AGeoreferenceActor *InGeoreference)
    {
        Georeference = InGeoreference;
        UpdateBounds();
        MarkRenderDynamicDataDirty();
    }
}
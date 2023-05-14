#include <regex>

#include "Cesium3DTileset.h"
#include "Common/World.h"
#include "Common/Actor/CameraActor.h"
#include "Material.h"
#include "Texture2D.h"
#include "Common/Asset/AssetLoader.h"
#include "Common/Components/MeshComponent.h"
#include "Cesium3DTileComponent.h"

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
            
            std::shared_ptr<UTexture2D> Texture = std::make_shared<UTexture2D>();
            Texture->Name = TextureName;
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

	std::shared_ptr<GLTFParseResult> ParseToStaticMeshes(tinygltf::Model &model, bool need_init)
    {
        std::shared_ptr<GLTFParseResult> Result = std::make_shared<GLTFParseResult>();
        std::vector<std::shared_ptr<UStaticMesh>> &StaticMeshes = Result->StaticMeshes;
        std::vector<std::shared_ptr<UMaterialInstance>> &Materials = Result->Materials;
        std::vector<std::shared_ptr<UTexture>> &Textures = Result->Textures;
        TUniformBufferRef<FGLTFMaterialBlock> &UniformBuffer = Result->UniformBuffer; 
        ParseToMaterials(model, Materials, Textures, UniformBuffer);
        for (auto &gltf_mesh : model.meshes)
        {
            std::shared_ptr<UStaticMesh> StaticMesh = std::make_shared<UStaticMesh>();
            StaticMesh->Name = gltf_mesh.name;
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
            Result->InitResource();
        return Result;
    }

    void GLTFParseResult::InitResource()
    {
        check(IsInRenderingThread());
        for (int i = 0; i < this->Textures.size(); i++)
        {
            this->Textures[i]->GetResource()->InitResource();
        }
        for (int i = 0; i < this->StaticMeshes.size(); i++)
        {
            this->StaticMeshes[i]->RenderData->InitResources();
        }
        if (UniformBuffer)
            this->UniformBuffer->InitResource();
    }

    GLTFParseResult::~GLTFParseResult()
    {
        std::vector<std::shared_ptr<class UStaticMesh>> StaticMeshesToDelete = StaticMeshes;
        std::vector<std::shared_ptr<class UMaterialInstance>> MaterialsToDelete = Materials;
        std::vector<std::shared_ptr<class UTexture>> TexturesToDelete = Textures;
        TUniformBufferRef<FGLTFMaterialBlock> UniformBufferToDelete = UniformBuffer;
        ENQUEUE_RENDER_COMMAND(GLTFParseResult_Deconstructor)(
            [StaticMeshesToDelete, MaterialsToDelete, TexturesToDelete, UniformBufferToDelete](FDynamicRHI*)
            {
                for (int i = 0; i < MaterialsToDelete.size(); i++)
                    MaterialsToDelete[i]->ReleaseResources();
                for (int i = 0; i < TexturesToDelete.size(); i++)
                    TexturesToDelete[i]->ReleaseResource();
                for (int i = 0; i < StaticMeshesToDelete.size(); i++)
                    StaticMeshesToDelete[i]->ReleaseResources();
                if (UniformBufferToDelete)
                    UniformBufferToDelete->ReleaseResource();
            }
        );
    }
}

namespace nilou {

    using namespace Cesium3DTilesetSelection;

    ACesium3DTileset::ACesium3DTileset()
        : LruCache(MaxTilesToRender)
        , Georeference(nullptr)
    {
    }

    void ACesium3DTileset::Tick(double DeltaSeconds)
    {
        if (Georeference == nullptr && GetWorld() != nullptr)
        {
            std::vector<AGeoreferenceActor*> Georeferences;
            GetWorld()->GetAllActorsOfClass<AGeoreferenceActor>(Georeferences, false);
            if (!Georeferences.empty())
            {
                this->Georeference = Georeferences[0];
            }
        }
        UWorld *World = GetWorld();
        if (World && Georeference)
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
                    CameraComponent->GetAspectRatio(), CameraComponent->VerticalFieldOfView, 
                    CameraComponent->NearClipDistance, CameraComponent->FarClipDistance);
                viewState.resolution = CameraComponent->ScreenResolution;
                viewState.verticalFOV = CameraComponent->VerticalFieldOfView;
                viewState.sseDenominator = 2.0 * glm::tan(0.5 * viewState.verticalFOV);
                ViewStates.push_back(viewState);
            }

            TilesToRenderThisFrame.clear();
            if (TilesetForSelection)
            {
                Update(TilesetForSelection.get(), ViewStates);
            }
        }

    }

    void ACesium3DTileset::SetURI(const std::string &NewURI)
    {
        if (NewURI != URI)
        {
            URI = NewURI; 
            tiny3dtiles::Loader Loader;
            Tileset = Loader.LoadTileset(URI);
            auto TilesetToRelease = TilesetForSelection;
            if (Tileset)
                TilesetForSelection = Cesium3DTileset::Build(Tileset, dmat4(1));
            else
                TilesetForSelection = nullptr;
        }
    }

    void ACesium3DTileset::Update(Cesium3DTileset *Tileset, const std::vector<ViewState> &ViewStates)
    {
        static UStaticMesh* Cube = GetContentManager()->GetStaticMeshByPath("/StaticMeshes/Cube.nasset");
        static UMaterial* WireframeMaterial = GetContentManager()->GetMaterialByPath("/Materials/WireframeMaterial.nasset");
        for (auto TileComponent : RenderComponentsThisFrame)
        {
            TileComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
            TileComponent->DestroyComponent();
        }
        RenderComponentsThisFrame.clear();

        // NILOU_LOG(Info, "Component count: {}", OwnedComponents.size())

        UpdateInternal(Tileset->Root.get(), ViewStates);

        for (auto iter = TilesToRenderThisFrame.begin(); iter != TilesToRenderThisFrame.end(); iter++)
        {
            Cesium3DTile* Tile = *iter;
            if (Tile->LoadingState == ETileLoadingState::Unloaded)
            {
                pool.push_task(
                    [this](Cesium3DTile* Tile) 
                    {
                        Tile->LoadingState = ETileLoadingState::Loading;
                        tinygltf::Model model;
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
                                Result->InitResource();
                                RHIInitialized = true;
                                cv.notify_all();
                            });
                        if (!RHIInitialized)
                        {
                            cv.wait(rhi_lock, [&RHIInitialized](){ return RHIInitialized.load(); });
                        }
                        Tile->Content.Gltf = Result;
                        Tile->LoadingState = ETileLoadingState::Loaded;
                    }, Tile);
            }
            else if (Tile->LoadingState == ETileLoadingState::Loaded)
            {
                auto ExpelledItem = LruCache.Put(Tile, Tile);
                if (ExpelledItem.has_value())
                {
                    ExpelledItem->first->LoadingState = ETileLoadingState::Unloading;
                    ExpelledItem->first->Content.B3dm.reset_glb();
                    ExpelledItem->first->Content.Gltf = nullptr;
                    ExpelledItem->first->LoadingState = ETileLoadingState::Unloaded;
                }
                auto TileComponent = CreateComponent<UCesium3DTileComponent>(this);
                TileComponent->Tile = Tile;
                TileComponent->Gltf = Tile->Content.Gltf;
                TileComponent->SetReflectionProbeBlendMode(RPBM_Off);
                TileComponent->RegisterComponent();
                TileComponent->AttachToComponent(GetRootComponent());
                RenderComponentsThisFrame.push_back(TileComponent);
                if (bShowBoundingBox) 
                {
                    dmat4 TranslationAndScale = dmat4(
                        dvec4(2, 0, 0, 0),
                        dvec4(0, 2, 0, 0),
                        dvec4(0, 0, 2, 0),
                        dvec4(Tile->BoundingVolume.Center, 1)
                    );
                    dmat4 LocalToWorld = Georeference->GetEcefToAbs() * TranslationAndScale * dmat4(Tile->BoundingVolume.HalfAxes);
                    auto BoundingBoxComponent = CreateComponent<UStaticMeshComponent>(this);
                    BoundingBoxComponent->SetStaticMesh(Cube);
                    BoundingBoxComponent->MaterialSlots[0] = WireframeMaterial;
                    BoundingBoxComponent->SetRelativeTransform(LocalToWorld);
                    BoundingBoxComponent->AttachToComponent(GetRootComponent());
                    BoundingBoxComponent->RegisterComponent();
                    BoundingBoxComponent->SetReflectionProbeBlendMode(RPBM_Off);
                    RenderComponentsThisFrame.push_back(BoundingBoxComponent);
                }
            }
        }
    }

    void ACesium3DTileset::UpdateInternal(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        bool bTileCulled = bEnableFrustumCulling && FrustumCull(Tile, ViewStates);

        if (!bTileCulled)
        {
            bool wantToRefine = !MeetsSSE(Tile, ViewStates);

            if (wantToRefine)
            {
                if (Tile->IsLeaf())
                {
                    TilesToRenderThisFrame.insert(Tile);
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
                    TilesToRenderThisFrame.insert(Tile);
                }
            }

        }

    }

    bool ACesium3DTileset::FrustumCull(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
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

    bool ACesium3DTileset::MeetsSSE(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        double largestSSE = 0;
        for (auto &ViewState : ViewStates)
        {
            double distance = glm::distance(ViewState.cameraPosition, Tile->BoundingVolume.Center); 
            double sse = Tile->GeometricError * ViewState.resolution.y / (distance * ViewState.sseDenominator);
            largestSSE = glm::max(largestSSE, sse);
        }
        return largestSSE <= MaximumScreenSpaceError;
    }
}
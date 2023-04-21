#include "Common/CoreUObject/InheritanceGraph.h"
namespace nilou {
FInheritanceGraph *FInheritanceGraph::GetInheritanceGraph()
{
    static FInheritanceGraph *InheritanceGraph = new FInheritanceGraph;
    return InheritanceGraph;
} 
FInheritanceGraph::FInheritanceGraph() {
	AddEdge(EUClasses::MC_UPrimitiveComponent, EUClasses::MC_UCesium3DTileComponent);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_ACesium3DTileset);
	AddEdge(EUClasses::MC_UObject, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_AArrowActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_ACameraActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_AFFTOceanActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_AGeoreferenceActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_ALightActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_ALineBatchActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_AReflectionProbe);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_ASkyAtmosphereActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_ASphereActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_AStaticMeshActor);
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_AVirtualHeightfieldMeshActor);
	AddEdge(EUClasses::MC_UObject, EUClasses::MC_UActorComponent);
	AddEdge(EUClasses::MC_UPrimitiveComponent, EUClasses::MC_UArrowComponent);
	AddEdge(EUClasses::MC_USceneComponent, EUClasses::MC_UCameraComponent);
	AddEdge(EUClasses::MC_UPrimitiveComponent, EUClasses::MC_UFourierTransformOceanComponent);
	AddEdge(EUClasses::MC_USceneComponent, EUClasses::MC_ULightComponent);
	AddEdge(EUClasses::MC_UPrimitiveComponent, EUClasses::MC_ULineBatchComponent);
	AddEdge(EUClasses::MC_UPrimitiveComponent, EUClasses::MC_UStaticMeshComponent);
	AddEdge(EUClasses::MC_USceneComponent, EUClasses::MC_UPrimitiveComponent);
	AddEdge(EUClasses::MC_USceneCaptureComponentCube, EUClasses::MC_UReflectionProbeComponent);
	AddEdge(EUClasses::MC_USceneComponent, EUClasses::MC_USceneCaptureComponent);
	AddEdge(EUClasses::MC_USceneCaptureComponent, EUClasses::MC_USceneCaptureComponent2D);
	AddEdge(EUClasses::MC_USceneCaptureComponent, EUClasses::MC_USceneCaptureComponentCube);
	AddEdge(EUClasses::MC_UActorComponent, EUClasses::MC_USceneComponent);
	AddEdge(EUClasses::MC_USceneComponent, EUClasses::MC_USkyAtmosphereComponent);
	AddEdge(EUClasses::MC_UPrimitiveComponent, EUClasses::MC_USphereComponent);
	AddEdge(EUClasses::MC_UPrimitiveComponent, EUClasses::MC_UVirtualHeightfieldMeshComponent);
	AddNode(EUClasses::MC_UObject);
	AddEdge(EUClasses::MC_UObject, EUClasses::MC_UMaterial);
	AddEdge(EUClasses::MC_UMaterial, EUClasses::MC_UMaterialInstance);
	AddEdge(EUClasses::MC_UObject, EUClasses::MC_UStaticMesh);
	AddEdge(EUClasses::MC_UObject, EUClasses::MC_UTexture);
	AddEdge(EUClasses::MC_UTexture, EUClasses::MC_UTexture2D);
	AddEdge(EUClasses::MC_UTexture, EUClasses::MC_UTexture2DArray);
	AddEdge(EUClasses::MC_UTexture, EUClasses::MC_UTexture3D);
	AddEdge(EUClasses::MC_UTexture, EUClasses::MC_UTextureCube);
	AddEdge(EUClasses::MC_UTexture, EUClasses::MC_UTextureRenderTarget);
	AddEdge(EUClasses::MC_UTextureRenderTarget, EUClasses::MC_UTextureRenderTarget2D);
	AddEdge(EUClasses::MC_UTextureRenderTarget, EUClasses::MC_UTextureRenderTargetCube);
	AddEdge(EUClasses::MC_UTexture, EUClasses::MC_UVirtualTexture);
}
}
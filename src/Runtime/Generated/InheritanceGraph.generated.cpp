#include "Common/InheritanceGraph.h"
namespace nilou {
FInheritanceGraph *FInheritanceGraph::GetInheritanceGraph()
{
    static FInheritanceGraph *InheritanceGraph = new FInheritanceGraph;
    return InheritanceGraph;
} 
FInheritanceGraph::FInheritanceGraph() {
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_UObject);
	AddEdge(EUClasses::MC_AArrowActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_ACameraActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_ACesiumTilesetActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_AGeoreferenceActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_ALightActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_ALineBatchActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_ASkyAtmosphereActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_ASphereActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_AStaticMeshActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_UActorComponent, EUClasses::MC_UObject);
	AddEdge(EUClasses::MC_UArrowComponent, EUClasses::MC_UPrimitiveComponent);
	AddEdge(EUClasses::MC_UCameraComponent, EUClasses::MC_USceneComponent);
	AddEdge(EUClasses::MC_UCesium3DTilesetComponent, EUClasses::MC_UPrimitiveComponent);
	AddEdge(EUClasses::MC_ULightComponent, EUClasses::MC_USceneComponent);
	AddEdge(EUClasses::MC_ULineBatchComponent, EUClasses::MC_UPrimitiveComponent);
	AddEdge(EUClasses::MC_UStaticMeshComponent, EUClasses::MC_UPrimitiveComponent);
	AddEdge(EUClasses::MC_UPrimitiveComponent, EUClasses::MC_USceneComponent);
	AddEdge(EUClasses::MC_USceneComponent, EUClasses::MC_UActorComponent);
	AddEdge(EUClasses::MC_USkyAtmosphereComponent, EUClasses::MC_USceneComponent);
	AddEdge(EUClasses::MC_USphereComponent, EUClasses::MC_UPrimitiveComponent);
	AddEdge(EUClasses::MC_UVirtualHeightfieldMeshComponent, EUClasses::MC_UPrimitiveComponent);
	AddNode(EUClasses::MC_UObject);
	AddEdge(EUClasses::MC_UStaticMesh, EUClasses::MC_UObject);
	AddEdge(EUClasses::MC_UMaterial, EUClasses::MC_UObject);
	AddEdge(EUClasses::MC_UMaterialInstance, EUClasses::MC_UMaterial);
	AddEdge(EUClasses::MC_UTexture, EUClasses::MC_UObject);
}
}
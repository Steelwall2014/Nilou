#include "Common/InheritanceGraph.h"
namespace nilou {
FInheritanceGraph *FInheritanceGraph::GetInheritanceGraph()
{
    static FInheritanceGraph *InheritanceGraph = new FInheritanceGraph;
    return InheritanceGraph;
} 
FInheritanceGraph::FInheritanceGraph() {
	AddEdge(EUClasses::MC_AActor, EUClasses::MC_UObject);
	AddEdge(EUClasses::MC_ACameraActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_ALightActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_AStaticMeshActor, EUClasses::MC_AActor);
	AddEdge(EUClasses::MC_UActorComponent, EUClasses::MC_UObject);
	AddEdge(EUClasses::MC_UArrowComponent, EUClasses::MC_UPrimitiveComponent);
	AddEdge(EUClasses::MC_UCameraComponent, EUClasses::MC_USceneComponent);
	AddEdge(EUClasses::MC_ULightComponent, EUClasses::MC_USceneComponent);
	AddEdge(EUClasses::MC_UStaticMeshComponent, EUClasses::MC_UPrimitiveComponent);
	AddEdge(EUClasses::MC_UPrimitiveComponent, EUClasses::MC_USceneComponent);
	AddEdge(EUClasses::MC_USceneComponent, EUClasses::MC_UActorComponent);
	AddNode(EUClasses::MC_UObject);
}
}
#include <glad/glad.h>

#include "Common/SceneObject/SceneObjectMesh.h"

#include "SceneObjectOceanSurface.h"


und::SceneObjectOceanSurface::SceneObjectOceanSurface(
    glm::vec2 wind_direction, float wind_speed,
    int fft_pow, unsigned int LODNum, unsigned int TopLODNodeSideNum, float TopLODNodeMeterSize, float A)
    : SceneObjectHugeSurface(kSceneObjectTtypeOceanSurface)
    , WindDirection(glm::normalize(wind_direction))
    , WindSpeed(wind_speed)
    , FFTPow(fft_pow)
    , A(A)
{
    InitializeQuadTree(LODNum, TopLODNodeSideNum, TopLODNodeMeterSize);
    FFTPixelMeterSize = 0.05;// QTree->GetOriginalPatchMeterSize() / 16.f;
    HeightMapMeterSize = FFTPixelMeterSize * std::pow(2.f, fft_pow);
}

#include "SceneObjectWaterbody.h"

#include <waterbody/waterbody_constants.glsl>

und::SceneObjectWaterbody::SceneObjectWaterbody()
	: BaseSceneObject(kSceneObjectTtypeWaterbody)
{
    water_depth = MAX_WATER_OPTICAL_DEPTH;
    fog_density = 1.f;
    zeta = 0.75;     // ǰ��ɢ��ı���
    gf = 0.62;       // ǰ��ɢ��HG��λ�����е�g����Χ[0, 1]
    gb = -0.2;      // ����ɢ��HG��λ�����е�g����Χ[-1, 0]
    hdr_exposure = 10;
    C = 0.01;
    absorbtion_d_400nm = 0.0f;
    absorbtion_y_440nm = 0.0f;
    CDOM_absorbtion = glm::vec3(0.034735, 0.214381, 1.0) * absorbtion_y_440nm;
    minerals_absorbtion = glm::vec3(0.045959, 0.192050, 0.644036) * absorbtion_d_400nm;
    minerals_scattering = glm::vec3(0.0635, 0.075, 0.09);
    phytoplankton_absorbtion = glm::vec3(0.015, 0.01, 0.035) * C;
    phytoplankton_scattering = glm::vec3(0.24264, 0.3, 0.375) * pow(C, 0.62f);
    pure_water_absorbtion = glm::vec3(0.45, 0.0638, 0.0145);
    pure_water_scattering = glm::vec3(0.0007, 0.0015, 0.0038);    // from Optical properties of the clearest natural waters
    total_scattering = pure_water_scattering + minerals_scattering + phytoplankton_scattering;
    total_absorbtion = pure_water_absorbtion + minerals_absorbtion + phytoplankton_absorbtion + CDOM_absorbtion;
    total_extinction = total_scattering + total_absorbtion;
}

void und::SceneObjectWaterbody::UpdateWaterParams()
{
    CDOM_absorbtion = glm::vec3(0.034735, 0.214381, 1.0) * absorbtion_y_440nm;
    minerals_absorbtion = glm::vec3(0.045959, 0.192050, 0.644036) * absorbtion_d_400nm;
    phytoplankton_absorbtion = glm::vec3(0.015, 0.01, 0.035) * C;
    phytoplankton_scattering = glm::vec3(0.24264, 0.3, 0.375) * pow(C, 0.62f);
    total_scattering = pure_water_scattering + minerals_scattering + phytoplankton_scattering;
    total_absorbtion = pure_water_absorbtion + minerals_absorbtion + phytoplankton_absorbtion + CDOM_absorbtion;
    total_extinction = total_scattering + total_absorbtion;
}

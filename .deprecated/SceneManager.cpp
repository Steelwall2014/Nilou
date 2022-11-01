#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Common/AssetLoader.h"
#include "Parser/GLTFParser.h"
#include "Common/Actor/ObserverActor.h"
#include "Common/DrawPass/OceanScatteringPrecomputeSubPass.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "Interface/IApplication.h"

#include "SceneManager.h"
#include "Common/SceneObject.h"
#include "Common/InputManager.h"

namespace und {
	glm::vec2 MouseInput;
	glm::quat SunRotation;
	float accumulatePitch = 0.0f;

	float waterbody_water_depth;
	bool update_water = false;
	SceneManager *g_pSceneManager = new SceneManager;
	int SceneManager::Initialize()
	{
		ResetScene();
		std::shared_ptr<ObserverActor> Observer = std::make_shared<ObserverActor>();
		//std::shared_ptr<SceneLightNode> Light = std::make_shared<SceneLightNode>(glm::vec3(0.998750269, 0, 0.0499791987), glm::vec3(0, 0, 0));

		std::shared_ptr<SceneLightNode> Light = std::make_shared<SceneLightNode>(glm::vec3(-4.5, 7.9, 5.5), glm::vec3(0, 0, 0));
		std::shared_ptr<SceneObjectDirectionalLight> light_obj = std::make_shared<SceneObjectDirectionalLight>();
		light_obj->SetLightColor(glm::vec4(0.9f, 0.7f, 0.5f, 1.0f));
		Light->AddSceneObjectRef(light_obj);
		std::shared_ptr<SceneObjectAtmosphere> Atmosphere = std::make_shared<SceneObjectAtmosphere>(Light);

		std::shared_ptr<SceneLightNode> Light2 = std::make_shared<SceneLightNode>(glm::vec3(0, 7.5, 7.5), glm::vec3(0, 0, 0));
		std::shared_ptr<SceneObjectSpotLight> light_obj2 = std::make_shared<SceneObjectSpotLight>();
		Light2->AddSceneObjectRef(light_obj2);
		std::string skybox_dir = AssetLoader::AssetDir + "Models\\skybox\\";
		_GM->GLDEBUG();
		std::vector<std::string> faces = {
				skybox_dir + "right.jpg",
				skybox_dir + "left.jpg",
				skybox_dir + "top.jpg",
				skybox_dir + "bottom.jpg",
				skybox_dir + "front.jpg",
				skybox_dir + "back.jpg"
		};
		std::shared_ptr<SceneObjectSkybox> Skybox = std::make_shared<SceneObjectSkybox>(faces);
		_GM->GLDEBUG();
		std::shared_ptr<SceneObjectOceanSurface> OceanSurface = std::make_shared<SceneObjectOceanSurface>(
			glm::vec2(-1.f, -1.f),		// wind_direction
			6.5f,						// wind_speed
			9,							// fft_pow
			7,							// LODNum
			5,							// TopLODNodeSideNum
			8192.0f,					// TopLODNodeMeterSize
			0.45f * 1e-3f);							// A
		_GM->GLDEBUG();

		std::shared_ptr<SceneObjectTexture> height_map = std::make_shared<SceneObjectTexture>(
			AssetLoader::AssetDir + "PseudoSeabedDEM3.tif");
		_GM->GLDEBUG();
		std::shared_ptr<SceneObjectTexture> seabedBaseColorMap = std::make_shared<SceneObjectTexture>(
			AssetLoader::AssetDir + "Models\\Ground_Wet_Pebbles\\Ground_Wet_Pebbles_001_basecolor.jpg");
		std::shared_ptr<SceneObjectTexture> seabedNormalMap = std::make_shared<SceneObjectTexture>(
			AssetLoader::AssetDir + "Models\\Ground_Wet_Pebbles\\Ground_Wet_Pebbles_001_normal.jpg");
		std::shared_ptr<SceneObjectTexture> seabedRoughnessMap = std::make_shared<SceneObjectTexture>(
			AssetLoader::AssetDir + "Models\\Ground_Wet_Pebbles\\Ground_Wet_Pebbles_001_roughness.jpg");
		std::shared_ptr<SceneObjectTerrainMaterial> seabedMaterial = std::make_shared<SceneObjectTerrainMaterial>("Seabed Material");
		seabedMaterial->SetBaseColor(seabedBaseColorMap);
		seabedMaterial->SetNormal(seabedNormalMap);
		seabedMaterial->SetRoughness(seabedRoughnessMap);

		std::shared_ptr<SceneObjectTerrainSurface> SeabedSurface = std::make_shared<SceneObjectTerrainSurface>(
			height_map,
			10.f,
			seabedMaterial,
			10.f);
		_GM->GLDEBUG();
		std::shared_ptr<SceneObjectWaterbody> Waterbody = std::make_shared<SceneObjectWaterbody>();
		//std::shared_ptr<SceneObjectOceanSurface> OceanSurface = std::shared_ptr<SceneObjectOceanSurface>(new SceneObjectOceanSurface(		
		//	glm::vec2(0.0f, 0.2f),		// wind_direction
		//	10.f,						// wind_speed
		//	9,							// fft_pow
		//	0.25,						// grid_size
		//	1));							// A))
		m_pScene->AddObserver(Observer);
		m_pScene->AddLight(Light);
		//m_pScene->AddLight(Light2);
		m_pScene->Skybox = Skybox;
		m_pScene->OceanSurface = OceanSurface;
		m_pScene->SeabedSurface = SeabedSurface;
		m_pScene->Atmosphere = Atmosphere;
		m_pScene->Waterbody = Waterbody;
		_GM->GLDEBUG();
		//g_pSceneManager->LoadScene("E:\\Downloads\\glTF-Sample-Models-master\\2.0\\WaterBottle\\glTF\\WaterBottle.gltf");
		LoadScene(AssetLoader::AssetDir + "Models\\simple_shadow.gltf");
		_GM->GLDEBUG();
		//g_pSceneManager->LoadScene(AssetLoader::AssetDir + "simple_waterbottle.gltf");
		LoadScene(AssetLoader::AssetDir + "Models\\WaterBottle.gltf");
		_GM->GLDEBUG();
		//LoadScene(AssetLoader::AssetDir + "Models\\Cube.gltf");

		OceanScatteringPrecompute = std::make_shared<OceanScatteringPrecomputeSubPass>();
		OceanScatteringPrecompute->Initialize(Waterbody);
		waterbody_water_depth = Waterbody->water_depth;

		InputAxisMapping MoveSunHorizontal_mapping("MoveSunHorizontal");
		MoveSunHorizontal_mapping.AddGroup(InputKey::KEY_MOUSEX, -1.0f);
		g_pInputManager->BindAxis(MoveSunHorizontal_mapping, this, &SceneManager::MoveSunHorizontal);

		InputAxisMapping MoveSunVertical_mapping("MoveSunVertical");
		MoveSunVertical_mapping.AddGroup(InputKey::KEY_MOUSEY, 1.0f);
		g_pInputManager->BindAxis(MoveSunVertical_mapping, this, &SceneManager::MoveSunVertical);

		auto Sun = GetScene()->LightNodes[0].lock();
		accumulatePitch = glm::degrees(asin(-Sun->GetLightDir().z));
		return 0;
	}

	void SceneManager::Finalize()
	{
	}

	void SceneManager::Tick(double DeltaTime)
	{
		auto observer = GetScene()->Observer;
		observer->Tick(DeltaTime);

		auto waterbody = GetScene()->Waterbody;
		ImGui::SliderFloat("zeta", &waterbody->zeta, 0.0f, 1.0f);
		ImGui::SliderFloat("gf", &waterbody->gf, 0.0f, 1.0f);
		ImGui::SliderFloat("gb", &waterbody->gb, -1.0f, 0.0f);
		//ImGui::SliderFloat("fog density", &waterbody->fog_density, 0.0f, 1.0f);
		ImGui::SliderFloat("Underwater HDR exposure", &waterbody->hdr_exposure, 0.0f, 100.f);
		ImGui::SliderFloat("Concentration of the main pigment chlorophyll-a", &waterbody->C, 0.0f, 0.1f);
		ImGui::SliderFloat("CDOM absorbtion(440nm)", &waterbody->absorbtion_y_440nm, 0.0f, 0.1f);
		ImGui::SliderFloat("Minerals and organic detritus absorbtion(400nm)", &waterbody->absorbtion_d_400nm, 0.0f, 0.1f);
		ImGui::ColorEdit3("Minerals and organic detritus scattering", (float *)&waterbody->minerals_scattering, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoSmallPreview);
		ImGui::ColorEdit3("Pure water scattering", (float *)&waterbody->pure_water_scattering, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoSmallPreview);
		ImGui::ColorEdit3("Pure water absorbtion", (float *)&waterbody->pure_water_absorbtion, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoSmallPreview);
		//ImGui::SliderFloat("water depth", &waterbody_water_depth, 0.0f, 2000.0f);
		//ImGui::Checkbox("Update Water", &update_water);

		{	// ����̫��λ��
			auto Sun = GetScene()->LightNodes[0].lock();	// Ĭ�ϵ�0����̫���������ʱ����øĵ�
			glm::vec3 SunPos = Sun->GetNodeLocation();
			float yaw = MouseInput.x / 10.f;
			float pitch = MouseInput.y / 10.f;
			glm::quat rotation = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 0, 1));
			SunPos = rotation * SunPos;
			if (/*0.5f < accumulatePitch + pitch && */accumulatePitch + pitch < 85.f)
			{
				rotation = glm::angleAxis(glm::radians(pitch), glm::normalize(glm::vec3(SunPos.y, -SunPos.x, 0)));
				SunPos = rotation * SunPos;
				accumulatePitch += pitch;
			}
			Sun->MoveNodeTo(SunPos);
		}

		CalculateLightParams(frame);
		frame.frameContext.cameraPosition = observer->GetCameraLocation();
		frame.frameContext.projectionMatrix = observer->GetProjectionMatrix();
		frame.frameContext.viewMatrix = observer->GetViewMatrix();
		auto cameraNode = observer->m_pCameraNode;
		auto cameraObj = cameraNode->GetSceneObjectRef();
		frame.frameContext.cameraAspect = cameraObj->AspectRatio;
		frame.frameContext.cameraFarClip = cameraObj->FarClipPlane;
		frame.frameContext.cameraNearClip = cameraObj->NearClipPlane;
		frame.frameContext.cameraFOVy = cameraObj->FieldOfView;
		frame.frameContext.cameraForward = cameraNode->GetForwardVector();
		frame.frameContext.cameraRight = cameraNode->GetRightVector();
		frame.frameContext.cameraUp = cameraNode->GetUpVector();
		frame.frameContext.cameraFrustum = Frustum(frame.frameContext.viewMatrix, frame.frameContext.projectionMatrix);
		for (auto &dbc : frame.batchContexts)
		{
			dbc.modelMatrix = dbc.node->GetWorldTransform().ToMatrix();
		}
		if (ImGui::Button("Update Water"))
		{
			waterbody->UpdateWaterParams();
			OceanScatteringPrecompute->DrawOrCompute(waterbody);
			update_water = false;
		}
		ImGui::Text("Camera Position: %.6f, %.6f, %.6f", frame.frameContext.cameraPosition.x, frame.frameContext.cameraPosition.y, frame.frameContext.cameraPosition.z);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		MouseInput = glm::vec2(0, 0);
	}

	int SceneManager::LoadScene(const std::string scene_file_name)
	{
		return LoadScene(scene_file_name.c_str());
	}

	int SceneManager::LoadScene(const char *scene_file_name)
	{
		return LoadGLTFScene(scene_file_name);		// ��ʱֻ֧��gltf
	}

	const std::shared_ptr<Scene> SceneManager::GetScene() const
	{
		// TODO: ���������cpu����
		return m_pScene;
	}
	const std::shared_ptr<Scene> SceneManager::GetSceneForPhysicalSimulation() const
	{
		// TODO: ���������cpu����
		return m_pScene;
	}
	void SceneManager::ResetScene()
	{
		m_pScene = std::make_shared<Scene>();
	}
	std::shared_ptr<BaseSceneNode> SceneManager::GetRootNode() const
	{
		return m_pScene ? m_pScene->SceneGraph : nullptr;
	}
	std::weak_ptr<SceneGeometryNode> SceneManager::GetSceneGeometryNode(int index) const
	{
		return m_pScene->GeometryNodes[index];
	}
	std::shared_ptr<SceneObjectGeometry> SceneManager::GetSceneGeometryObject(int index) const
	{
		return m_pScene ? m_pScene->Geometries[index] : nullptr;
	}
	int SceneManager::GetSceneGeometryNodeCount() const
	{
		return m_pScene ? m_pScene->GeometryNodes.size() : 0;
	}
	void SceneManager::CalculateLightParams(FrameVariables &frame)
	{
		frame.frameContext.lights.clear();
		for (auto node : g_pSceneManager->GetScene()->LightNodes)
		{
			std::shared_ptr<SceneLightNode> light_node = node.lock();
			std::shared_ptr<SceneObjectLight> light_object = light_node->GetSceneObjectRef();
			Light light;
			light.lightPosition = light_node->GetNodeLocation();
			light.lightColor = light_object->GetLightColor();
			light.lightIntensity = light_object->GetIntensity();
			light.lightCastShadow = light_object->GetCastShadows();
			light.nearClipDistance = light_object->GetNearClip();
			light.farClipDistance = light_object->GetFarClip();
			light.lightDirection = light_node->GetLightDir();
			if (light_object->GetType() == kSceneObjectTypePointLight)
			{
				light.lightType = LightType::Point;
				auto light_obj = std::dynamic_pointer_cast<SceneObjectPointLight>(light_object);

				AttenCurve dist_atten = light_obj->GetDistAttenCurve();
				light.lightDistAttenCurveType = dist_atten.type;
				memcpy(&light.lightDistAttenCurveParams, &dist_atten.u, sizeof(dist_atten.u));
			}
			else if (light_object->GetType() == kSceneObjectTypeDirectionalLight)
			{
				light.lightType = LightType::Directional;
				auto light_obj = std::dynamic_pointer_cast<SceneObjectDirectionalLight>(light_object);

				AttenCurve dist_atten = light_obj->GetDistAttenCurve();
				light.lightDistAttenCurveType = dist_atten.type;
				memcpy(&light.lightDistAttenCurveParams, &dist_atten.u, sizeof(dist_atten.u));
			}
			else if (light_object->GetType() == kSceneObjectTypeSpotLight)
			{
				light.lightType = LightType::Spot;
				auto light_obj = std::dynamic_pointer_cast<SceneObjectSpotLight>(light_object);

				AttenCurve dist_atten = light_obj->GetDistAttenCurve();
				light.lightDistAttenCurveType = dist_atten.type;
				memcpy(&light.lightDistAttenCurveParams, &dist_atten.u, sizeof(dist_atten.u));

				AttenCurve angle_atten = light_obj->GetAngleAttenCurve();
				light.lightAngleAttenCurveType = angle_atten.type;
				memcpy(&light.lightAngleAttenCurveParams, &angle_atten.u, sizeof(angle_atten.u));
			}
			frame.frameContext.lights.push_back(light);
		}
	}
	
	bool SceneManager::LoadGLTFScene(const char *gltf_scene_file_name)
	{
		std::shared_ptr<tinygltf::Model> model = g_pAssetLoader->SyncReadGLTFModel(gltf_scene_file_name);
		std::unique_ptr<Scene> scene = g_pGLTFParser->Parse(*model);
		if (scene == nullptr)
			return -1;

		for (auto &child : scene->SceneGraph->GetChildren())
			m_pScene->SceneGraph->AppendChild(child);
		for (auto &elem : scene->GeometryNodes)
			m_pScene->GeometryNodes.push_back(elem);
		for (auto &elem : scene->LightNodes)
			m_pScene->LightNodes.push_back(elem);
		for (auto &elem : scene->Lights)
			m_pScene->Lights.push_back(elem);
		for (auto &elem : scene->Materials)
			m_pScene->Materials.push_back(elem);
		for (auto &elem : scene->Geometries)
			m_pScene->Geometries.push_back(elem);

		return 0;
	}
	void SceneManager::MoveSunHorizontal(float AxisValue)
	{
		auto mouseright_state = g_pInputManager->GetKeyState(InputKey::KEY_MOUSE_MIDDLE);
		if (g_pApp->IsCursorEnabled() && mouseright_state.bDown)
			MouseInput.x += AxisValue;
	}
	void SceneManager::MoveSunVertical(float AxisValue)
	{
		auto mouseright_state = g_pInputManager->GetKeyState(InputKey::KEY_MOUSE_MIDDLE);
		if (g_pApp->IsCursorEnabled() && mouseright_state.bDown)
			MouseInput.y += AxisValue;
	}
}


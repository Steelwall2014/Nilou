#pragma once
#include "Common/Transform.h"
#include <iostream>
// #include <glm/glm.hpp>
// #include <glm/gtc/quaternion.hpp>
// #include "Common/SceneObject/SceneObjectTransform.h"

void UNDDEBUG_PrintGLM(glm::quat q);
void UNDDEBUG_PrintGLM(glm::vec3 v);
void UNDDEBUG_PrintGLM(glm::vec4 v);
void UNDDEBUG_PrintGLM(glm::mat4 v);
void UNDDEBUG_PrintGLM(nilou::FRotator r);
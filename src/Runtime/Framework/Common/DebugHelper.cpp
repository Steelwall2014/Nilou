#include "DebugHelper.h"

void UNDDEBUG_PrintGLM(glm::quat q)
{
	std::cout << "quat w: " << q.w << " x: " << q.x << " y: " << q.y << " z: " << q.z << std::endl;
}

void UNDDEBUG_PrintGLM(glm::vec3 v)
{
	std::cout << "vec3 x: " << v.x << " y: " << v.y << " z: " << v.z << std::endl;
}

void UNDDEBUG_PrintGLM(glm::vec4 v)
{
	std::cout << "vec4 x: " << v.x << " y: " << v.y << " z: " << v.z << " w: " << v.w << std::endl;
}

void UNDDEBUG_PrintGLM(glm::mat4 m)
{    
	m = glm::transpose(m);
	std::cout << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << std::endl;
	std::cout << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[1][3] << std::endl;
	std::cout << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[2][3] << std::endl;
	std::cout << m[3][0] << " " << m[3][1] << " " << m[3][2] << " " << m[3][3] << std::endl;
}

void UNDDEBUG_PrintGLM(nilou::FRotator r)
{
	std::cout 
		<< "Rotator Pitch: " << glm::degrees(r.Pitch) << "," << r.Pitch
		<< " Yaw: " << glm::degrees(r.Yaw) << "," << r.Yaw
		<< " Roll: " << glm::degrees(r.Roll) << "," << r.Roll << std::endl;
}

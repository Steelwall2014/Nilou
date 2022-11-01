#pragma once
// #include <glm/glm.hpp>
#include "Common/Maths.h"

class CoordinateAxis
{
private:
	glm::vec3 x_axis[6];
	glm::vec3 y_axis[6];
	glm::vec3 z_axis[6];
	glm::vec3 x_axis2[6];
	glm::vec3 y_axis2[6];
	glm::vec3 z_axis2[6];
	glm::vec4 x_color;
	glm::vec4 y_color;
	glm::vec4 z_color;
	unsigned int x_vao, y_vao, z_vao;
	unsigned int x_vao2, y_vao2, z_vao2;
	unsigned int x_axis_buffer, y_axis_buffer, z_axis_buffer;
	unsigned int x_axis_buffer2, y_axis_buffer2, z_axis_buffer2;
	unsigned int vert_shader, frag_shader;
	unsigned int program;
	const char *vert_shader_code = R"(
		#version 330 core
		layout (location=0) in vec3 Position;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main()
		{
			gl_Position = projection * view * model * vec4(Position, 1);
		}
		)";
	const char *frag_shader_code = R"(
		#version 330 core
		layout (location = 0) out vec4 FragColor;
		uniform vec4 color;

		void main()
		{
			FragColor = color;
		}
		)";
public:
	CoordinateAxis();
	CoordinateAxis(float length, float width);
	void drawAxis(glm::mat4 projection, glm::mat4 view, glm::mat4 model, char axis);
	void drawAxes(glm::mat4 projection, glm::mat4 view, glm::mat4 model);
};
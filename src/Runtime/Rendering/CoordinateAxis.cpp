#include <glad/glad.h>

#include "CoordinateAxis.h"

CoordinateAxis::CoordinateAxis()
{
}

CoordinateAxis::CoordinateAxis(float length, float width)
{
	float delta = width / 2;
	x_axis[0] = { 0, -delta, 0 };
	x_axis[1] = { 0, delta, 0 };
	x_axis[2] = { length, -delta, 0 };
	x_axis[3] = { length, -delta, 0 };
	x_axis[4] = { 0, delta, 0 };
	x_axis[5] = { length, delta, 0 };

	x_axis2[0] = { 0, 0, -delta };
	x_axis2[1] = { 0, 0, delta };
	x_axis2[2] = { length, 0, -delta };
	x_axis2[3] = { length, 0, -delta };
	x_axis2[4] = { 0, 0, delta };
	x_axis2[5] = { length, 0, delta };

	y_axis[0] = { -delta, 0, 0 };
	y_axis[1] = { delta, 0, 0 };
	y_axis[2] = { -delta, length, 0 };
	y_axis[3] = { -delta, length, 0 };
	y_axis[4] = { delta, 0, 0 };
	y_axis[5] = { delta, length, 0 };

	y_axis2[0] = { 0, 0, -delta };
	y_axis2[1] = { 0, 0, delta };
	y_axis2[2] = { 0, length, -delta };
	y_axis2[3] = { 0, length, -delta };
	y_axis2[4] = { 0, 0, delta };
	y_axis2[5] = { 0, length, delta };

	z_axis[0] = { 0, -delta, 0 };
	z_axis[1] = { 0, delta, 0 };
	z_axis[2] = { 0, -delta, length };
	z_axis[3] = { 0, -delta, length };
	z_axis[4] = { 0, delta, 0 };
	z_axis[5] = { 0, delta, length };

	z_axis2[0] = { -delta, 0, 0 };
	z_axis2[1] = { delta, 0, 0 };
	z_axis2[2] = { -delta, 0, length };
	z_axis2[3] = { -delta, 0, length };
	z_axis2[4] = { delta, 0, 0 };
	z_axis2[5] = { delta, 0, length };

	x_color = { 1, 0, 0, 1 };
	y_color = { 0, 1, 0, 1 };
	z_color = { 0, 0, 1, 1 };

	vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shader, 1, &vert_shader_code, NULL);
	glCompileShader(vert_shader);
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &frag_shader_code, NULL);
	glCompileShader(frag_shader);

	program = glCreateProgram();
	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader); 
	glLinkProgram(program);
	unsigned int Position_location = glGetAttribLocation(program, "Position");

	glGenVertexArrays(1, &x_vao);
	glBindVertexArray(x_vao);
	glGenBuffers(1, &x_axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, x_axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), x_axis, GL_STATIC_DRAW);
	glEnableVertexAttribArray(Position_location);
	glVertexAttribPointer(Position_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &x_vao2);
	glBindVertexArray(x_vao2);
	glGenBuffers(1, &x_axis_buffer2);
	glBindBuffer(GL_ARRAY_BUFFER, x_axis_buffer2);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), x_axis2, GL_STATIC_DRAW);
	glEnableVertexAttribArray(Position_location);
	glVertexAttribPointer(Position_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &y_vao);
	glBindVertexArray(y_vao);
	glGenBuffers(1, &y_axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, y_axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), y_axis, GL_STATIC_DRAW);
	glEnableVertexAttribArray(Position_location);
	glVertexAttribPointer(Position_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &y_vao2);
	glBindVertexArray(y_vao2);
	glGenBuffers(1, &y_axis_buffer2);
	glBindBuffer(GL_ARRAY_BUFFER, y_axis_buffer2);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), y_axis2, GL_STATIC_DRAW);
	glEnableVertexAttribArray(Position_location);
	glVertexAttribPointer(Position_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &z_vao);
	glBindVertexArray(z_vao);
	glGenBuffers(1, &z_axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, z_axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), z_axis, GL_STATIC_DRAW);
	glEnableVertexAttribArray(Position_location);
	glVertexAttribPointer(Position_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &z_vao2);
	glBindVertexArray(z_vao2);
	glGenBuffers(1, &z_axis_buffer2);
	glBindBuffer(GL_ARRAY_BUFFER, z_axis_buffer2);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), z_axis2, GL_STATIC_DRAW);
	glEnableVertexAttribArray(Position_location);
	glVertexAttribPointer(Position_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glBindVertexArray(0);

}

void CoordinateAxis::drawAxis(glm::mat4 projection, glm::mat4 view, glm::mat4 model, char axis)
{
	glDisable(GL_CULL_FACE);
	unsigned int vao;
	unsigned int vao2;
	glm::vec4 color;
	switch (axis)
	{
	case 'x':
		vao = x_vao;
		vao2 = x_vao2;
		color = x_color;
		break;
	case 'y':
		vao = y_vao;
		vao2 = y_vao2;
		color = y_color;
		break;
	case 'z':
		vao = z_vao;
		vao2 = z_vao2;
		color = z_color;
		break;
	default:
		return;
	}
	glUseProgram(program);
	glUniform4fv(glGetUniformLocation(program, "color"), 1, &color[0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &model[0][0]);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(vao2);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glEnable(GL_CULL_FACE);
}

void CoordinateAxis::drawAxes(glm::mat4 projection, glm::mat4 view, glm::mat4 model)
{
	//glDisable(GL_CULL_FACE);
	drawAxis(projection, view, model, 'x');
	drawAxis(projection, view, model, 'y');
	drawAxis(projection, view, model, 'z');
}
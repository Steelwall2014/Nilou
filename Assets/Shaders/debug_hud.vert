#version 460 core

layout(location = 0) in vec3 inputPosition;
layout(location = 1) in vec2 inputUV;

out vec2 UV;

void main(){
	gl_Position = vec4(inputPosition, 1.0f);
	UV = inputUV;
}

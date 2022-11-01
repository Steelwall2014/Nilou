#version 460 core

in vec2 UV;

out vec4 color;

uniform sampler3D Texture;
uniform float scale;

void main(){
    color = vec4(texture(Texture, vec3(UV.x, UV.y, 106.5/128)).rgb * scale, 1.f);
}
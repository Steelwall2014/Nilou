#version 460 core

in vec2 UV;

out vec4 color;

uniform sampler2D Texture;
uniform float scale;

void main(){
    color = vec4(texture(Texture, vec2(UV)).rgb * scale, 1.f);
    //color = vec3(1, 0, 0);
}
//#version 330 core
//
//in vec2 UV;
//
//out vec3 color;
//
//uniform sampler2D depthSampler;
//
//void main(){
//    //color = texture(depthSampler, UV).rgb;
//    color = vec3(texture(depthSampler, UV).rgb/10);
//}
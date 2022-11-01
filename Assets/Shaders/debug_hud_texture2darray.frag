#version 460 core

in vec2 UV;

out vec4 color;

uniform sampler2DArray Texture;
uniform float layer_index;

void main(){
        color = vec4(pow(texture(Texture, vec3(UV, layer_index)).rrr, vec3(20.0f)), 1.f);
    
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
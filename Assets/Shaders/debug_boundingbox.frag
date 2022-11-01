#version 460 core
in vec3 Culled;
out vec4 color;

void main(){
    color = vec4(Culled, 1.f);
}

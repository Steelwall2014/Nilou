#version 460
layout (location = 0) out vec4 FragColor;

in vec2 uv;

uniform sampler2D SceneColor;

void main()
{
#if USING_OPENGL
	FragColor = texture(SceneColor, vec2(1-uv.x, uv.y));
#else
	FragColor = texture(SceneColor, uv);
#endif
}
#version 330 core 

layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aColor; 
layout (location = 2) in vec2 aTexCoord;

uniform mat4 objectToWorld;
uniform mat4 worldToView;
uniform mat4 projection;

out vec3 ourColor;
out vec2 TexCoord;


void main(){ 
	gl_Position = projection * worldToView * objectToWorld * vec4(aPos,  1.0f);
	ourColor = aColor;
	TexCoord = aTexCoord;
}



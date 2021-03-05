#version 440 core

layout (location = 0) in vec3 aPos;

uniform mat4 objectToWorld;
uniform mat4 worldToView;
uniform mat4 projection;

void main(){
	gl_Position = projection * worldToView * objectToWorld * vec4(aPos, 1.0f);
}
#version 330 core 

layout (location = 0) in vec3 aPos; 

uniform mat4 objectToWorld;
uniform mat4 lightSpaceMatrix;

void main(){ 
	gl_Position = lightSpaceMatrix * objectToWorld * vec4(aPos,  1.0f);
}
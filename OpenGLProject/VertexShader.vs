#version 330 core 

layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 TexCoord;

uniform mat4 objectToWorld;
uniform mat4 worldToView;
uniform mat4 projection;

uniform mat3 normalMatrix;

uniform vec3 lightPos;

out vec3 Normal;
out vec3 FragPos;
out vec2 texCoord;

void main(){ 
	gl_Position = projection * worldToView * objectToWorld * vec4(aPos,  1.0f);
	Normal = normalMatrix * aNormal;
	FragPos = (worldToView * objectToWorld * vec4(aPos,  1.0f)).xyz;
	texCoord = TexCoord;
}



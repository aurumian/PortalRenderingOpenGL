#version 440 core 

layout (location = 0) in vec3 aPos; 

layout (std140) uniform GlobalMatrices
{
	uniform mat4 worldToView;
	uniform mat4 projection;
};

uniform mat4 objectToWorld;
uniform mat4 portalDimsScaler;

out VS_OUT {
    vec3 psFragPos;
} vs_out;

void main(){ 
	gl_Position = projection * worldToView * objectToWorld * vec4(aPos,  1.0f);
	vs_out.psFragPos = mat3(portalDimsScaler) * aPos;
}
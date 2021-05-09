#version 440 core 

#define MAX_NUM_DIR_LIGHTS 4

layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 TexCoord;

layout (std140) uniform GlobalMatrices
{
	mat4 worldToView;
	mat4 projection;
};

uniform mat4 objectToWorld;

uniform mat4 objectToInsideWorld;

layout (std140) uniform PortalBlock 
{
	vec4 portalPlaneEq;
	vec4 clippingPlane2;
};


uniform mat3 normalMatrix;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 texCoord;
	vec3 worldPos;
} vs_out;

out float gl_ClipDistance[2];

void main(){ 
	vec4 insideWorldPos = objectToInsideWorld * vec4(aPos,  1.0);
	vec4 worldPos = objectToWorld * vec4(aPos,  1.0);
	// clip everyting in front of the portal (between virtual camera and the portal)
	vec4 viewPos = worldToView * worldPos;
	gl_ClipDistance[0] = dot(viewPos, portalPlaneEq);
	gl_ClipDistance[1] = dot(viewPos, clippingPlane2);
	vs_out.Normal = normalMatrix * aNormal;
	vs_out.FragPos = viewPos.xyz;
	vs_out.texCoord = TexCoord;
	vs_out.worldPos = insideWorldPos.xyz;

	gl_Position = projection * viewPos;
}
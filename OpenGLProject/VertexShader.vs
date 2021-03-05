#version 440 core 

layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 TexCoord;


uniform mat4 objectToWorld;
uniform mat4 worldToView;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform mat4 lightSpaceMatrix2;

uniform vec4 portalPlaneEq;

uniform mat3 normalMatrix;

uniform vec3 lightPos;
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 texCoord;
    vec4 FragPosLightSpace;
    vec4 FragPosLightSpace2;
} vs_out;

out float gl_ClipDistance[1];

void main(){ 
	// clip everyting in front of the portal (between virtual camera and the portal)
	vec4 viewPos = worldToView * objectToWorld * vec4(aPos,  1.0f);
	gl_ClipDistance[0] = dot(viewPos, portalPlaneEq);
	vs_out.Normal = normalMatrix * aNormal;
	vs_out.FragPos = (worldToView * objectToWorld * vec4(aPos,  1.0f)).xyz;
	vs_out.texCoord = TexCoord;
	vs_out.FragPosLightSpace = lightSpaceMatrix * objectToWorld * vec4(aPos,  1.0f);
	vs_out.FragPosLightSpace2 = lightSpaceMatrix2 * objectToWorld * vec4(aPos,  1.0f);
	gl_Position = projection * worldToView * objectToWorld * vec4(aPos,  1.0f);
	
}



#version 440 core 

#define MAX_NUM_DIR_LIGHTS 4

layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 TexCoord;

layout (std140) uniform GlobalMatrices
{
	uniform mat4 worldToView;
	uniform mat4 projection;
};

uniform mat4 objectToWorld;

uniform mat4 lightSpaceMatrix;
uniform mat4 lightSpaceMatrix2;

uniform vec4 portalPlaneEq;

uniform mat3 normalMatrix;

uniform vec3 lightPos;
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 texCoord;
	vec4 fragPosLightSpace[MAX_NUM_DIR_LIGHTS];
    vec4 FragPosLightSpace;
    vec4 FragPosLightSpace2;
} vs_out;



out float gl_ClipDistance[1];



struct DirLight {			// base alignment | ofsset | aligned offset
	vec3 direction;			// 16	|	0	|	0
	float ambientStrength;	// 4	|	12	|	12
	vec3 color;				// 16	|	16	|	16
	float intensity;		// 4	|	28	|	28
	uint smStencilRef;		// 4	|	32	|	32
	mat4 lightSpaceMatrix;	// [0]16|	36	|	48
							// [1]16|	64	|	64
							// [2]16|	80	|	80
							// [3]16|	96	|	96
}; // total size - 112 bytes

layout (std140) uniform  DirLights {
	DirLight dirLights[MAX_NUM_DIR_LIGHTS]; // 112	| 0		| 0
	uint numDirLights;						// 4	| 448	| 448
};

void main(){ 
	vec4 worldPos = objectToWorld * vec4(aPos,  1.0f);
	// clip everyting in front of the portal (between virtual camera and the portal)
	vec4 viewPos = worldToView * worldPos;
	gl_ClipDistance[0] = dot(viewPos, portalPlaneEq);
	vs_out.Normal = normalMatrix * aNormal;
	vs_out.FragPos = (worldToView * worldPos).xyz;
	vs_out.texCoord = TexCoord;
	//vs_out.FragPosLightSpace = lightSpaceMatrix * objectToWorld * vec4(aPos,  1.0f);
	for (int i = 0; i < numDirLights; ++i)
	{
		vs_out.fragPosLightSpace[i] = dirLights[i].lightSpaceMatrix * worldPos;
	}
	//vs_out.FragPosLightSpace = dirLights[0].lightSpaceMatrix * objectToWorld * vec4(aPos,  1.0f);

	//vs_out.FragPosLightSpace2 = lightSpaceMatrix2 * objectToWorld * vec4(aPos,  1.0f);
	//vs_out.FragPosLightSpace2 = dirLights[2].lightSpaceMatrix * objectToWorld * vec4(aPos,  1.0f);
	gl_Position = projection * worldToView * worldPos;
	
}



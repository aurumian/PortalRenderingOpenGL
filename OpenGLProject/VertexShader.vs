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

layout (std140) uniform PortalBlock 
{
	vec4 portalPlaneEq;
	vec4 clippingPlane2;
};


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



out float gl_ClipDistance[2];



struct DirLight {			// base alignment | ofsset | aligned offset
	vec3 direction;			// 16	|	0	|	0
	float ambientStrength;	// 4	|	12	|	12
	vec3 color;				// 16	|	16	|	16
	float intensity;		// 4	|	28	|	28
	uint smStencilRef;		// 4	|	32	|	32
	uint smIndex;			// 4	|   36	|	36
	mat4 lightSpaceMatrix;	// [0]16|	40	|	48
							// [1]16|	64	|	64
							// [2]16|	80	|	80
							// [3]16|	96	|	96
	vec4 lsPortalEq;		// 16	|	112	|	112
}; // total size - 128 bytes

layout (std140) uniform  DirLights {
	DirLight dirLights[MAX_NUM_DIR_LIGHTS]; // 128	| 0		| 0
	uint numDirLights;						// 4	| 512	| 512
}; // total size - 528(because the uint is padded to 16 bytes when at the end)

void main(){ 
	vec4 worldPos = objectToWorld * vec4(aPos,  1.0f);
	// clip everyting in front of the portal (between virtual camera and the portal)
	vec4 viewPos = worldToView * worldPos;
	gl_ClipDistance[0] = dot(viewPos, portalPlaneEq);
	gl_ClipDistance[1] = dot(viewPos, clippingPlane2);
	vs_out.Normal = normalMatrix * aNormal;
	vs_out.FragPos = (worldToView * worldPos).xyz;
	vs_out.texCoord = TexCoord;

	for (int i = 0; i < numDirLights; ++i)
	{
		vs_out.fragPosLightSpace[i] = dirLights[i].lightSpaceMatrix * worldPos;
	}

	gl_Position = projection * worldToView * worldPos;
	
}



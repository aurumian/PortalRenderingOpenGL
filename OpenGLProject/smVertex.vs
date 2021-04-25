#version 440 core 

layout (location = 0) in vec3 aPos; 

layout (std140) uniform GlobalMatrices
{
	uniform mat4 worldToView;
	uniform mat4 projection;
};

uniform mat4 objectToWorld;


uniform vec4 portalPlaneEq;

out float gl_ClipDistance[1];

void main(){ 
	vec4 viewPos = worldToView * objectToWorld * vec4(aPos,  1.0f);
	gl_ClipDistance[0] = dot(viewPos, portalPlaneEq);
	gl_Position = projection * viewPos;	
}

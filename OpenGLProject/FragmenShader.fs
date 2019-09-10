#version 330 core 
out vec4 FragColor; 

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;
uniform float mix;
void main() 
{ 
	FragColor = mix(texture(ourTexture1, TexCoord*2), texture(ourTexture2, TexCoord), mix);
}

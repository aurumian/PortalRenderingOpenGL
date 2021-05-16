#pragma once
#include <glad/glad.h>
#include <list>
#include <vector>

#include "Lighting.h"

class Portal;
class Material;
class DirLight;

const GLsizei SHADOW_MAP_SIZE = 1024 * 4;

class ShadowmapTexture
{
public:
	GLuint fbo;
	GLuint depth_view;
	GLuint stencil_view;
};

class Shadows
{
public:
	Shadows();
	static void ConfigureFBOAndTextureForShadowmap(GLuint& fbo, GLuint& tex, GLuint& stencil_view);
	void RenderShadowmap(ShadowedDirLight& light);
	void FreePool();
protected:
	ShadowmapTexture shadowmaps[Lighting::MAX_DIR_LIGHT_COUNT];
	std::vector<ShadowmapTexture*> stPool;
};

void RenderShadowmap(GLuint fbo, DirLight& light, const std::list<Portal*>& portalsToRender);

extern Material* smMat;
extern Material* clearDepthMat;


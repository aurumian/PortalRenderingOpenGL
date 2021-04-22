#include "Shadows.h"

#include <unordered_map>

#include "MeshRenderer.h"
#include "Lighting.h"
#include "Portal.h"
#include "Common.h"
#include "Material.h"

Material* smMat;
Material* clearDepthMat;

Shadows::Shadows()
{
	for (size_t i = 0; i < Lighting::MAX_DIR_LIGHT_COUNT; ++i)
	{
		ShadowmapTexture& st = shadowmaps[i];
		ConfigureFBOAndTextureForShadowmap(st.fbo, st.depth_view, st.stencil_view);
		stPool.push_back(&st);
	}
}

void Shadows::FreePool()
{
	stPool.clear();
	for (size_t i = 0; i < Lighting::MAX_DIR_LIGHT_COUNT; ++i)
	{
		ShadowmapTexture& st = shadowmaps[i];
		stPool.push_back(&st);
	}
}

void Shadows::RenderShadowmap(DrawableDirLight& light)
{
	// get a shadowmap from the pool
	if (stPool.size() == 0)
		return;
	auto* sm = stPool.back();
	stPool.pop_back();
	
	// assign the shadowmap to drawable light
	light.shadowmap = sm;

	auto* ps = light.light->GetPortalSpace();

	// get portals that this light might go through
	auto& portals = ps->GetPortals();

	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, sm->fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


	Cam cam = GetLightCam(*light.light);
	SetGlobalProjectionMatrix(cam.projection);
	SetGlobalViewMatrix(cam.worldToView);

	unordered_map<const Portal*, glm::mat4> wtvs;


	// set prevStencil  values to zero for each portal
	for (auto* p : ps->GetPortals())
	{
		if (p == nullptr)
			continue;
		p->prevStencil = 0;
		auto iter = light.perPortal.find(p);
		if (iter != light.perPortal.end())
			p->stencilVal = (*iter).second.stencilVal;
		else
			// so it would still leave a shadow
			p->stencilVal = 0;
		wtvs[p] = cam.worldToView;
	}

	{
		// render portal planes first to not render the pixels that will be covered by them anyway ?
		// render the scene first time
		// redner portal planes again to apply stencil values and clear depth
		// for each portal draw the scene again to update the depth map
	}


	// render the scene without portals
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	//DrawScene(cam, smMat);
	ps->Draw(cam, smMat);
	glDisable(GL_CULL_FACE);

	glEnable(GL_STENCIL_TEST);
	// redner portal planes to apply stencil values
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	for (const Portal* p : portals) {
		glStencilFunc(GL_ALWAYS, p->stencilVal, 0xFF);

		MeshRenderer* r = p->GetComponent<MeshRenderer>();
		r->Draw(smMat);
	}

	// draw portal planes again to clear depth where the portals are
	// but only where the stencil is equal to the portal's stencil
	// draw only the planes that will be redrawn later
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glDepthFunc(GL_ALWAYS);
	for (auto& p : light.perPortal) {
		if (p.first == nullptr)
			continue;
		glStencilFunc(GL_EQUAL, p.first->stencilVal, 0xFF);

		MeshRenderer* r = p.first->GetComponent<MeshRenderer>();
		// use shader that sets depth to 1.0 (clears it)
		r->Draw(clearDepthMat);
	}
	glDepthFunc(GL_LESS);

	// for each portal draw the scene again to update the depth map
	for (auto& pa : light.perPortal) {
		Portal* p = pa.first;
		if (p == nullptr)
			continue;
		cam.worldToView = wtvs[p];
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		wtvs[p] = DrawPortalContents(*p, cam, smMat);
		glDisable(GL_CULL_FACE);
	}

	glDisable(GL_STENCIL_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
}

void Shadows::ConfigureFBOAndTextureForShadowmap(GLuint& fbo, GLuint& tex, GLuint& stencil_view) {
	// https://stackoverflow.com/questions/27535727/opengl-create-a-depth-stencil-texture-for-reading
	// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml
	glGenFramebuffers(1, &fbo);

	// allocate space for the texture and set up a few parameters
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// create stecil view for the texture
	glGenTextures(1, &stencil_view);
	glTextureView(stencil_view, GL_TEXTURE_2D, tex, GL_DEPTH24_STENCIL8, 0, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, stencil_view);


	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

// when i render a shadowmap for a light i need to:
// get a shadowmap texture from a pool
// assign that texture (both depth_view and stencil_view) to drawable light
// calculate and assign per portal stencil values, light directions and lightspace matrices
// 
//

/*
* Renders a shadowmap for a light taking portals into account
*
*/
void RenderShadowmap(GLuint fbo, DirLight& light, const list<Portal*>& portalsToRender) {
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


	Cam cam = GetLightCam(light);
	SetGlobalProjectionMatrix(cam.projection);
	SetGlobalViewMatrix(cam.worldToView);

	unordered_map<const Portal*, glm::mat4> wtvs;


	// set prevStencil  values to zero for each portal
	for (Portal* p : portalsToRender) {
		p->prevStencil = 0;

		wtvs[p] = cam.worldToView;
	}

	{
		// render portal planes first to not render the pixels that will be covered by them anyway ?
		// render the scene first time
		// redner portal planes again to apply stencil values and clear depth
		// for each portal draw the scene again to update the depth map
	}


	// render the scene without portals
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	DrawScene(cam, smMat);
	glDisable(GL_CULL_FACE);

	glEnable(GL_STENCIL_TEST);
	// redner portal planes to apply stencil values
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	for (const Portal* p : portalsToRender) {
		glStencilFunc(GL_ALWAYS, p->stencilVal, 0xFF);

		MeshRenderer* r = p->GetComponent<MeshRenderer>();
		r->Draw(smMat);
	}

	// draw portal planes again to clear depth where the portals are
	// but only where the stencil is equal to the portal's stencil
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glDepthFunc(GL_ALWAYS);
	for (const Portal* p : portalsToRender) {
		glStencilFunc(GL_EQUAL, p->stencilVal, 0xFF);

		MeshRenderer* r = p->GetComponent<MeshRenderer>();
		// use shader that sets depth to 1.0 (clears it)
		r->Draw(clearDepthMat);
	}
	glDepthFunc(GL_LESS);

	// for each portal draw the scene again to update the depth map
	for (const Portal* p : portalsToRender) {
		cam.worldToView = wtvs[p];
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		wtvs[p] = DrawPortalContents(*p, cam, smMat);
		glDisable(GL_CULL_FACE);
	}

	glDisable(GL_STENCIL_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
}
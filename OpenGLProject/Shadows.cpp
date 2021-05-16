#include "Shadows.h"

#include <unordered_map>

#include "MeshRenderer.h"
#include "Lighting.h"
#include "Portal.h"
#include "Common.h"
#include "Material.h"
#include "PortalSpace.h"
#include "Camera.h"
#include "Actor.h"

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

void Shadows::RenderShadowmap(ShadowedDirLight& light)
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


	Camera cam = GetLightCam(*light.light);
	SetGlobalProjectionMatrix(cam.GetProjectionMatrix());
	SetGlobalViewMatrix(cam.GetWorldToViewMatrix());

	unordered_map<const Portal*, glm::mat4> wtvs;


	// set prevStencil  values to zero for each portal
	for (auto* p : ps->GetPortals())
	{
		if (p == nullptr)
			continue;
		p->prevStencil = 0;
		// so it would still leave a shadow
		p->stencilVal = 0;
		wtvs[p] = cam.GetWorldToViewMatrix();
	}
	for (auto* ddl : light.drawableLights)
	{
		if (ddl->portal == nullptr)
			continue;
		ddl->portal->stencilVal = ddl->stencilVal;
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
	ps->Draw(&cam, smMat);
	glDisable(GL_CULL_FACE);

	// render current portal space pices of inbetween objects
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		for (const auto o : ps->inbetweenObjects)
		{
			const Camera& camera = cam;
			glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix())));
			//glm::mat3 normalMat = glm::mat3(camera.GetWorldToViewMatrix());
			glm::vec3 vn = normalMat * o->enteredNormal;
			glm::vec4 eq = o->enteredPortal->GetViewspacePortalEquation(camera.GetWorldToViewMatrix(), camera.IsOrtho());
			if (glm::dot(vn, glm::vec3(eq)) < 0.0f)
				eq = -eq;

			// temp
			MeshRenderer* mr = o->actor->GetComponent<MeshRenderer>();

			glEnable(GL_CLIP_DISTANCE1);
			SetGlobalClippingPlane2(eq);
			mr->Draw(smMat);
			glDisable(GL_CLIP_DISTANCE1);

		}
		glDisable(GL_CULL_FACE);
	}

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
	for (auto& p : light.drawableLights) {
		if (p->portal == nullptr)
			continue;
		glStencilFunc(GL_EQUAL, p->stencilVal, 0xFF);

		MeshRenderer* r = p->portal->GetComponent<MeshRenderer>();
		// use shader that sets depth to 1.0 (clears it)
		r->Draw(clearDepthMat);
	}
	glDepthFunc(GL_LESS);

	const Camera& camera = cam;
	// for each portal draw the scene again to update the depth map
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	for (auto& pa : light.drawableLights) {
		Portal* p = pa->portal;
		if (p == nullptr)
			continue;
		cam.SetWorldToViewMatrix(wtvs[p]);
		{
			for (const auto o : ps->inbetweenObjects)
			{
				if (o->enteredPortal != p)
				{
					continue;
				}

				SetGlobalViewMatrix(camera.GetWorldToViewMatrix());
				SetGlobalProjectionMatrix(camera.GetProjectionMatrix());
				glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix())));
				glm::vec3 vn = normalMat * o->enteredNormal;
				glm::vec4 eq = o->enteredPortal->GetViewspacePortalEquation(camera.GetWorldToViewMatrix(), camera.IsOrtho());
				if (glm::dot(vn, glm::vec3(eq)) < 0.0f)
				{

					// temp
					MeshRenderer* mr = o->actor->GetComponent<MeshRenderer>();
					glEnable(GL_CLIP_DISTANCE1);
					SetGlobalClippingPlane2(eq);
					glStencilFunc(GL_EQUAL, p->stencilVal, 0xFF);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					mr->Draw(smMat);
					glDisable(GL_CLIP_DISTANCE1);
				}

			}
		}
		wtvs[p] = DrawPortalContents(*p, cam, smMat);
	}
	glDisable(GL_CULL_FACE);

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
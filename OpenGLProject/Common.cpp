#include "Common.h"
#include "Material.h"

#include "UniformBufferObject.h"
#include "Actor.h"
#include "MeshRenderer.h"
#include "Portal.h"
#include "Lighting.h"
#include "Shadows.h"
#include "PortalSpace.h"

UniformBufferObject* globalMatrices;
UniformBufferObject* portalBlock;

int screenWidth = SCR_WIDTH, screenHeight = SCR_HEIGHT;
Material sceneMat;

PortalSpace defaultPortalSpace;

PortalSpace* currentPortalSpace;

extern PortalRenderTree prTree;

extern MeshRenderer* lpRenderer;

GLuint gBuffer;
GLuint gPosition, gNormal, gAlbedoSpec, gWorldPosition;

Material gPassMat;

Material gPassInsMat;


void CreateGlobalMatricesBuffer() {
	globalMatrices = new UniformBufferObject("GlobalMatrices", sizeof(glm::mat4) * 2 + sizeof(glm::mat3));
}

void CreatePortalBlockUbo()
{
	portalBlock = new UniformBufferObject("PortalBlock", sizeof(glm::vec4) * 2);
}

void SetGlobalViewMatrix(const glm::mat4& view)
{
	globalMatrices->SetBufferSubData(0, sizeof(glm::mat4), glm::value_ptr(view));
}

void SetGlobalProjectionMatrix(const glm::mat4& projection)
{
	globalMatrices->SetBufferSubData(sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
}

void SetGlobalViewspacePortalEquation(const glm::vec4& eq)
{
	portalBlock->SetBufferSubData(0, sizeof(glm::vec4), glm::value_ptr(eq));
}

void SetGlobalClippingPlane2(const glm::vec4& eq)
{
	portalBlock->SetBufferSubData(sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(eq));
}

PortalSpace* GetDefaultPortalSpace()
{
	return &defaultPortalSpace;
}

UniformBufferObject* GetPortalBlockUbo()
{
	return portalBlock;
}

// temp
extern Actor bulb;
void SetCommonShaderValues(MeshRenderer* mr, const Camera& camera, const Material* matOverride)
{
	Shader* sp = mr->GetMaterial()->GetShader();
	sp->Use();
	sp->setFloat("material.shiness", 32.0f);
	// set shadowmaps for the shader
	lighting->SetShadowmaps(sp);


	//temp
	sp->setVec3("pointLight.position", camera.GetWorldToViewMatrix() * glm::vec4(bulb.transform.position, 1.0f));
	sp->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	sp->setVec3("pointLight.position", camera.GetWorldToViewMatrix() * glm::vec4(bulb.transform.position, 1.0f));


	// set normal matrix
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix() * mr->GetTransform().GetTransformMatrix())));
	sp->SetMat3("normalMatrix", normalMatrix);
}



void DrawScene(const Camera& camera, const PortalRenderTree& prTree, const Material* matOverride)
{
	// algorithm(using PortalRenderTree):
	// having rendered the scene
	// get portals at depth 1
	// render portal planes setting the stencil values if the depth test passes
	// clear depth where the portals' respective stencil values are set (is one fullscreen render call better than a draw call for each plane? - it can work only for depth 1)
	// render contents of each of the portals
	// depth 2+:
	// get portals at the depth
	// render portal planes setting the stencil values:
	//	at depth 2+ rendering portals is harder since i can only increment stencil values (I can download AMD_stencil_operation_extended extension for amd cards to change it to any value)
	//	without extension:
	//		the stencil values have to be updated incrementally
	//		variation 1:
	//		render all portal planes each time incrementing the stencil value until the stencil value is what it's supposed to be for the portal
	//		(remove the portal from the list when the value is right, keep rendering until the list is empty)
	//		if portals are grouped by curent stencil value (parent portal's value at firts iteration) gpu instancing can be used to make rendering faster (if the portals use the same mesh)
	//		variation 2:
	//		render the portal planes without modifying the stencil (so that stencil does not interfere with depth oreder of portals)
	//		since we render the portal planes anyway we can draw them before rendering the contents of the parent portal(s) (which is an optimization - a selective z-prepass) - wrong 'cause it obstructs scene objects
	//		set depth test to equal
	//		set stencil values for each portal individually (incrementally obviously)
	//		slightly increases number of draw calls (compared to variation 1)
	//		removes some of the overhead of variation 1 and makes the algo simpler
	//	with extension:
	//		set stencil test to equal to parent's
	//		draw portal planes (obviously in parent's stencil) without modifying stencil
	//		set depth test to equal
	//		draw the portal planes  setting the new stencil values
	//		return depth test to previous value
	//	render the portal's contents
	// keep rendering until there's no more portals left
	// 
	//

	// clear stencil
	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);

	// for all possible depths
	size_t depth = 0;
	for (auto iter = prTree.Begin(); iter != prTree.End(); depth++)
	{

		// render the portal planes inside the portals without changing the stencil values
		auto it = iter;
		{

			// get iterator to next depth
			while (it != prTree.End() && (*it)->GetDepth() == depth)
			{
				++it;
			}
			auto i = it;

			glEnable(GL_DEPTH_CLAMP);

			while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
			{
				BeginDrawInsidePortal(*(*i)->GetParent());
				DrawPortalPlane(*(i), false, matOverride);
				EndDrawInsidePortal();
				++i;
			}

			glDisable(GL_DEPTH_CLAMP);

		}

		// render this space's pieces of inbetween objects
		{
			auto i = iter;
			while (i != prTree.End() && (*i)->GetDepth() == depth)
			{
				for (const auto o : i.operator*()->GetDestPortalSpace()->inbetweenObjects)
				{
					if (depth > 0 && o->enteredPortal->dest == i.operator*()->GetPortal())
					{
						continue;
					}
					const Camera& camera = i.operator*()->GetCamera();
					glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix())));
					//glm::mat3 normalMat = glm::mat3(camera.GetWorldToViewMatrix());
					glm::vec3 vn = normalMat * o->enteredNormal;
					glm::vec4 eq = o->enteredPortal->GetViewspacePortalEquation(camera.GetWorldToViewMatrix(), camera.IsOrtho());
					if (glm::dot(vn, glm::vec3(eq)) < 0.0f)
						eq = -eq;


					// temp
					MeshRenderer* mr = o->actor->GetComponent<MeshRenderer>();
					{
						SetCommonShaderValues(mr, camera, matOverride);
					}


					BeginDrawInsidePortal(**i);
					glEnable(GL_CLIP_DISTANCE1);
					SetGlobalClippingPlane2(eq);
					lighting->AddLights(i.operator*()->GetDestPortalSpace(), camera);
					lighting->SendToGPU();
					glStencilFunc(GL_EQUAL, i.operator*()->GetStencil(), 0xFF);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					mr->Draw(matOverride);
					lighting->ClearLights();
					glDisable(GL_CLIP_DISTANCE1);
					EndDrawInsidePortal();
					
				}

				++i;
			}
		}


		// Render portals' contents
		{
			while (iter != prTree.End() && (*iter)->GetDepth() == depth)
			{
				DrawPortalContents(**iter, matOverride);
				++iter;
			}
		}


		// update stencil
		{
			GLint dFunc;
			glGetIntegerv(GL_DEPTH_FUNC, &dFunc);
			glDepthFunc(GL_EQUAL);
			if (depth == 0)
			{
				auto i = it;
				{
					glEnable(GL_DEPTH_CLAMP);
					while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
					{
						DrawPortalPlane(*(*i), true, matOverride);
						++i;
					}
					glDisable(GL_DEPTH_CLAMP);
				}
			}
			else
			{

				auto i = it;
				while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
				{
					BeginDrawInsidePortal(*(*i)->GetParent());
					glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
					stencil_t s = (*i)->GetParent()->GetStencil();
					while (s < (*i)->GetStencil())
					{
						glStencilFunc(GL_EQUAL, s++, 0xFF);
						DrawPortalPlane(**i, false, matOverride);
					}
					EndDrawInsidePortal();
					++i;
				}
			}
			glDepthFunc(dFunc);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		}

		// clear depth - right now i don't need depth buffer, so i can just clear it
		// alternatively i can use glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE), glDepthFunc(GL_ALWAYS) and a shader that clears the depth
		// for each drawn plane (or a fullscreen plane with glStencilFunc(GL_GREATER, 0, 0xFF), glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP) - possible only the first time)
		//glClear(GL_DEPTH_BUFFER_BIT);

		{
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			// draw portal planes again to clear depth where the portals are
			// but only where the stencil is equal to the portal's stencil
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glDepthFunc(GL_ALWAYS);
			glEnable(GL_DEPTH_CLAMP);
			auto i = iter;
			while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
			{
				glStencilFunc(GL_EQUAL, (*i)->GetStencil(), 0xFF);

				SetGlobalViewMatrix((*i)->GetParent()->GetCamera().GetWorldToViewMatrix());
				SetGlobalProjectionMatrix((*i)->GetParent()->GetCamera().GetProjectionMatrix());

				MeshRenderer* r = (*i)->GetPortal()->GetComponent<MeshRenderer>();
				// use shader that sets depth to 1.0 (clears it)
				r->Draw(clearDepthMat);
				++i;
			}
			glDisable(GL_DEPTH_CLAMP);
			glDepthFunc(GL_LESS);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}



		// temp draw the inside slice
		{
			auto i = iter;
			while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
			{
				const Camera& camera = i.operator*()->GetParent()->GetCamera();
				for (const auto o : i.operator*()->GetPortal()->GetPortalSpace()->inbetweenObjects)
				{
					if (o->enteredPortal != i.operator*()->GetPortal())
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
						{
							Shader* sp = mr->GetMaterial()->GetShader();
							sp->Use();
							sp->setFloat("material.shiness", 32.0f);
							// set shadowmaps for the shader
							lighting->SetShadowmaps(sp);


							//temp
							sp->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
							sp->setVec3("pointLight.position", camera.GetWorldToViewMatrix() * (*i)->GetPortal()->dest->GetPortallingMat() * glm::vec4(bulb.transform.position, 1.0f));


							// set normal matrix
							if (matOverride == nullptr) {
								glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix() * mr->GetTransform().GetTransformMatrix())));
								sp->SetMat3("normalMatrix", normalMatrix);
							}
						}

						BeginDrawInsidePortal(*((*i)->GetParent()));
						glEnable(GL_CLIP_DISTANCE1);
						SetGlobalClippingPlane2(eq);
						lighting->AddLights(i.operator*()->GetDestPortalSpace(), camera);
						for (size_t k = 0; k < lighting->dirLights.numDirLights; ++k)
						{
							auto& dl = lighting->dirLights.lights[k];
							dl.lightSpaceMatrix = dl.lightSpaceMatrix * (*i)->GetPortal()->dest->GetPortallingMat();
							// TODO: properly calulate new light direction for inside pieces
							//dl.direction = glm::mat3((*i)->GetPortal()->dest->GetPortallingMat()) * dl.direction;
						}
						lighting->SendToGPU();
						glStencilFunc(GL_EQUAL, i.operator*()->GetStencil(), 0xFF);
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
						mr->Draw(matOverride);
						lighting->ClearLights();
						glDisable(GL_CLIP_DISTANCE1);
					}

				}
				++i;
			}
		}


	}

	glDisable(GL_STENCIL_TEST);
}


void ForwardRenderScene(const Camera& camera) {
	//open gl render config
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// render shadowmaps
	{
		if (PortalSpace::shadowmappedLights.size() > 0)
			shadows->RenderShadowmap(*(*GetDefaultPortalSpace()->shadowmappedLights.begin()));
	}

	// compute the portal rendering tree
	prTree.ConstructTree(currentPortalSpace->GetPortals(), camera);

	DrawScene(camera, prTree);

	shadows->FreePool();
}

void CreateGBuffer()
{
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);



	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// world position color buffer
	glGenTextures(1, &gWorldPosition);
	glBindTexture(GL_TEXTURE_2D, gWorldPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gWorldPosition, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw "Couldn't create gBuffer";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferedRenderScene(const Camera& camera)
{
	if (PortalSpace::shadowmappedLights.size() > 0)
		shadows->RenderShadowmap(**(PortalSpace::shadowmappedLights.begin()));

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	// compute the portal rendering tree
	prTree.ConstructTree(currentPortalSpace->GetPortals(), camera);

	// gPass
	{
		//currentPortalSpace->Draw(&camera, &gPassMat);

		GeometryPass(camera, prTree, &gPassMat);
	}

	// copy stencil
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
	glBlitFramebuffer(
		0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT,  GL_STENCIL_BUFFER_BIT, GL_NEAREST
	);

	// light pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, gWorldPosition);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	for (auto iter = prTree.Begin(); iter != prTree.End(); ++iter)
	{
		glStencilFunc(GL_EQUAL, (*iter)->GetStencil(), 0xFF);
		lighting->AddLights((*iter)->GetDestPortalSpace(), (*iter)->GetCamera());
		lighting->SendToGPU();
		Shader* sp = lpRenderer->GetMaterial()->shader;
		sp->Use();
		lighting->SetShadowmaps(lpRenderer->GetMaterial()->shader);
		sp->setInt("gPosition", 8);
		sp->setInt("gNormal", 9);
		sp->setInt("gAlbedoSpec", 10);
		sp->setInt("gWorldPosition", 11);
		sp->setFloat("material.shiness", 32.0f);
		sp->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
		if ((*iter)->GetPortal() != nullptr)
			sp->setVec3("pointLight.position", camera.GetWorldToViewMatrix() * (*iter)->GetPortal()->dest->GetPortallingMat() * glm::vec4(bulb.transform.position, 1.0f));
		else
			sp->setVec3("pointLight.position", camera.GetWorldToViewMatrix() * glm::vec4(bulb.transform.position, 1.0f));

		lpRenderer->Draw();
		lighting->ClearLights();
	}
	
	glEnable(GL_DEPTH_TEST);
	
	glDisable(GL_STENCIL_TEST);

	shadows->FreePool();
}


void GeometryPass(const Camera& camera, const PortalRenderTree& prTree, const Material* matOverride)
{
	// basically a copy of DrawScene optimized for deferred shading 

	// clear stencil
	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);

	// for all possible depths
	size_t depth = 0;
	for (auto iter = prTree.Begin(); iter != prTree.End(); depth++)
	{

		// render the portal planes inside the portals without changing the stencil values
		auto it = iter;
		{

			// get iterator to next depth
			while (it != prTree.End() && (*it)->GetDepth() == depth)
			{
				++it;
			}
			auto i = it;
			glEnable(GL_DEPTH_CLAMP);
			while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
			{
				BeginDrawInsidePortal(*(*i)->GetParent());
				DrawPortalPlane(*(i), false, matOverride);
				EndDrawInsidePortal();
				++i;
			}
			glDisable(GL_DEPTH_CLAMP);

		}

		// render this space's pieces of inbetween objects
		{
			auto i = iter;
			while (i != prTree.End() && (*i)->GetDepth() == depth)
			{
				for (const auto o : i.operator*()->GetDestPortalSpace()->inbetweenObjects)
				{
					if (depth > 0 && o->enteredPortal->dest == i.operator*()->GetPortal())
					{
						continue;
					}
					const Camera& camera = i.operator*()->GetCamera();
					glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix())));
					//glm::mat3 normalMat = glm::mat3(camera.GetWorldToViewMatrix());
					glm::vec3 vn = normalMat * o->enteredNormal;
					glm::vec4 eq = o->enteredPortal->GetViewspacePortalEquation(camera.GetWorldToViewMatrix(), camera.IsOrtho());
					if (glm::dot(vn, glm::vec3(eq)) < 0.0f)
						eq = -eq;


					// temp
					MeshRenderer* mr = o->actor->GetComponent<MeshRenderer>();
					{
						glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix() * mr->GetTransform().GetTransformMatrix())));
						matOverride->shader->Use();
						matOverride->shader->SetMat3("normalMatrix", normalMatrix);
					}


					BeginDrawInsidePortal(**i);
					glEnable(GL_CLIP_DISTANCE1);
					SetGlobalClippingPlane2(eq);
					glStencilFunc(GL_EQUAL, i.operator*()->GetStencil(), 0xFF);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					mr->Draw(matOverride);
					glDisable(GL_CLIP_DISTANCE1);
					EndDrawInsidePortal();

				}

				++i;
			}
		}


		// Render portals' contents
		{
			while (iter != prTree.End() && (*iter)->GetDepth() == depth)
			{
				DrawPortalContents(**iter, matOverride);
				++iter;
			}
		}


		// update stencil
		{
			GLint dFunc;
			glGetIntegerv(GL_DEPTH_FUNC, &dFunc);
			glDepthFunc(GL_EQUAL);
			if (depth == 0)
			{
				auto i = it;
				{
					glEnable(GL_DEPTH_CLAMP);
					while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
					{
						DrawPortalPlane(*(*i), true, matOverride);
						++i;
					}
					glDisable(GL_DEPTH_CLAMP);
				}
			}
			else
			{

				auto i = it;
				while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
				{
					BeginDrawInsidePortal(*(*i)->GetParent());
					glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
					stencil_t s = (*i)->GetParent()->GetStencil();
					while (s < (*i)->GetStencil())
					{
						glStencilFunc(GL_EQUAL, s++, 0xFF);
						DrawPortalPlane(**i, false, matOverride);
					}
					EndDrawInsidePortal();
					++i;
				}
			}
			glDepthFunc(dFunc);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		}

		// clear depth
		{
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			// draw portal planes again to clear depth where the portals are
			// but only where the stencil is equal to the portal's stencil
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glDepthFunc(GL_ALWAYS);
			glEnable(GL_DEPTH_CLAMP);
			auto i = iter;
			while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
			{
				glStencilFunc(GL_EQUAL, (*i)->GetStencil(), 0xFF);

				SetGlobalViewMatrix((*i)->GetParent()->GetCamera().GetWorldToViewMatrix());
				SetGlobalProjectionMatrix((*i)->GetParent()->GetCamera().GetProjectionMatrix());

				MeshRenderer* r = (*i)->GetPortal()->GetComponent<MeshRenderer>();
				// use shader that sets depth to 1.0 (clears it)
				r->Draw(clearDepthMat);
				++i;
			}
			glDisable(GL_DEPTH_CLAMP);
			glDepthFunc(GL_LESS);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}



		// temp draw the inside slice
		{
			auto i = iter;
			while (i != prTree.End() && (*i)->GetDepth() == depth + 1)
			{
				const Camera& camera = i.operator*()->GetParent()->GetCamera();
				for (const auto o : i.operator*()->GetPortal()->GetPortalSpace()->inbetweenObjects)
				{
					if (o->enteredPortal != i.operator*()->GetPortal())
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
						Shader* sp = gPassInsMat.shader;
						// set normal matrix
						sp->Use();
						glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(camera.GetWorldToViewMatrix() * mr->GetTransform().GetTransformMatrix())));
						sp->SetMat3("normalMatrix", normalMatrix);
						sp->setMat4("objectToInsideWorld", o->enteredPortal->PortalTransformToDest(mr->GetTransform()).GetTransformMatrix());

						BeginDrawInsidePortal(*((*i)->GetParent()));
						glEnable(GL_CLIP_DISTANCE1);
						SetGlobalClippingPlane2(eq);
						glStencilFunc(GL_EQUAL, i.operator*()->GetStencil(), 0xFF);
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
						mr->Draw(&gPassInsMat);
						glDisable(GL_CLIP_DISTANCE1);
					}

				}
				++i;
			}
		}
		

	}

	glDisable(GL_STENCIL_TEST);
}

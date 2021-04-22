#include "Portal.h"

#include "Common.h"
#include "MeshRenderer.h"
#include "Material.h"

glm::vec4 Portal::GetViewspacePortalEquation(glm::mat4 worldToView) const
{
	glm::vec3 normal = glm::transpose(glm::inverse(glm::mat3(worldToView * transform.GetTransformMatrix()))) * glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 q = worldToView * glm::vec4(transform.position, 1.0f);
	// make sure (pos, 1.0f) dot equation < 0 when the point is between the camera and the portal plane
	if (glm::dot(normal, q) < 0)
		normal = -normal;
	return glm::vec4(normal, -glm::dot(normal, q));
	// this equation can be used to check a point position around this portal:
	// if (point.pos, 1.0f) dot equation < 0 when the point is between the camera and the portal plane
	// else it's between the plane and camera's far plane
}


PortalRenderTree Portal::GetPortalRenderTree(const list<Portal*>& portals) {
	
	size_t maxDepth = GetMaxRenderDepth(portals);
	/* use depth first search to construct the tree
	*/

	PortalRenderTree tree;
	PortalRenderTreeNode* node = new PortalRenderTreeNode();
	tree.root = node;

	for (Portal* p : portals) {
		PortalRenderTreeNode* n = new PortalRenderTreeNode();
		n->parent = tree.root;
		n->portal = p;
		n->portalMat = p->GetPortallingMat();
		if (node == tree.root) {
			node->firstChild = n;
			node = n;
		}
		else {
			node->right = n;
			node = n;
		}
	}
	
	size_t depth = 1;
	stencil_t stencil = 2;
	stencil_t numPortals = portals.size();
	node = tree.root->firstChild;
	node->stencil = 1;
	while (node != tree.root) {
		PortalRenderTreeNode* nod = node;
		if (depth < maxDepth)
			for (Portal* p : node->portal->cbsPortals) {
				if (numPortals > maxNumPortalRenderings)
					break;
				PortalRenderTreeNode* n = new PortalRenderTreeNode();
				n->parent = node;
				n->portal = p;
				n->portalMat = node->portalMat * p->GetPortallingMat();
				if (nod == node) {
					nod->firstChild = n;
					nod = n;
				}
				else {
					nod->right = n;
					nod = n;
				}
				numPortals++;
			}
		if (node->firstChild != nullptr) {
			node = node->firstChild;
			depth++;
			node->stencil = stencil++;
		}
		else if (node->right != nullptr) {
			node = node->right;
			node->stencil = stencil++;
		}
		else {
			while (node->right == nullptr && node != tree.root) {
				node = node->parent;
				depth--;
			}
			if (node != tree.root) {
				node = node->right;
				node->stencil = stencil++;
			}
		}
	}

	return tree;
}

/*
	uses breadth first search to determine max rendering depth
*/
size_t Portal::GetMaxRenderDepth(const list<Portal*>& portals) {
	stencil_t numPortalRenderings = 0;
	size_t depth = 1;

	list<Portal*> pl = portals;

	size_t depthIncrAfter = pl.size();
	size_t nextDepthIncr = 0;
	while (!pl.empty()) {
		Portal* p = pl.front();
		pl.pop_front();
		numPortalRenderings++;
		if (numPortalRenderings >= maxNumPortalRenderings)
			break;
		depthIncrAfter--;
		for (Portal* po : p->cbsPortals) {
			nextDepthIncr++;
			pl.push_back(po);
		}
		if (depthIncrAfter == 0) {
			if (!pl.empty())
				depth++;
			depthIncrAfter = nextDepthIncr;
			nextDepthIncr = 0;
		}
	}

	return depth;
}

glm::mat4 Portal::GetPortallingMat() const{
	if (dest == nullptr)
		return glm::mat4(1.0f);
	static const glm::mat4 negXZ{ -1.0f, 0.0f,  0.0f, 0.0f,
							       0.0f, 1.0f,  0.0f, 0.0f,
							       0.0f, 0.0f, -1.0f, 0.0f,
								   0.0f, 0.0f,  0.0f, 1.0f };
	return 	transform.GetTransformMatrix() *
			negXZ *
			dest->transform.GetInverseTransformMatrix();
}





void DrawPortalPlane(const Portal& p, const glm::mat4& worldToView) {
	glStencilFunc(GL_ALWAYS, p.stencilVal, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	MeshRenderer* r = p.GetComponent<MeshRenderer>();
	Shader* sp = r->GetMaterial()->GetShader();

	sp->Use();
	sp->setVec3("lightColor", { 0.1f, 0.1f, 0.1f });
	r->Draw();
}

glm::mat4 DrawPortalContents(const Portal& p, const Cam& cam, Material* matOverride) {

	// update world to view matrix 
	glm::mat4 newWTV = cam.worldToView * p.GetPortallingMat();

	Cam c = cam;
	c.worldToView = newWTV;

	// draw the scene from a new perspective
	{
		glStencilFunc(GL_EQUAL, p.stencilVal, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		Shader* sp;
		if (matOverride == nullptr)
			// temp
			sp = sceneMat.GetShader();
		else
			sp = sceneMat.shader;
		sp->Use();
		sp->setVec4("portalPlaneEq", p.dest->GetViewspacePortalEquation(newWTV));

		// temp
		portallingMat = p.GetPortallingMat();

		glEnable(GL_CLIP_DISTANCE0);
		DrawScene(c, matOverride);
		// enable this!!!
		//p.GetPortalSpace()->Draw(c, matOverride);
		glDisable(GL_CLIP_DISTANCE0);
	}

	return newWTV;
}
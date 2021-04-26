#include "Portal.h"

#include "Common.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "PortalSpace.h"


// need to update to support ortho projection matrices - the location is different from the view
glm::vec4 Portal::GetViewspacePortalEquation(glm::mat4 worldToView, float isOrtho) const
{
	glm::vec3 normal = glm::transpose(glm::inverse(glm::mat3(worldToView * transform.GetTransformMatrix()))) * glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 q = worldToView * glm::vec4(transform.position, 1.0f);
	if (isOrtho)
	{
		// for orthograthic projection normal dot forward must be greater than zero
		if (glm::dot(normal, { 0.0f, 0.0f, 1.0f }) < 0.0f)
			normal = -normal;
	}
	else
	{
		// make sure (pos, 1.0f) dot equation < 0 when the point is between the camera and the portal plane
		if (glm::dot(normal, q) < 0.0f) // == dot((0.0, 0.0, 0.0, 1.0), (normal, -dot(normal,q)) > 0.0f
			normal = -normal;
	}
	return glm::vec4(normal, -glm::dot(normal, q));
	// this equation can be used to check a point position around this portal:
	// if (point.pos, 1.0f) dot equation < 0 when the point is between the camera and the portal plane
	// else it's between the portal plane and the camera's far plane
}

// for now tested only for orthogonal projection matrix
glm::vec4 Portal::GetNdcSpacePortalEquation(glm::mat4 worldToClip) const
{
	glm::vec3 normal = glm::transpose(glm::inverse(glm::mat3(worldToClip * transform.GetTransformMatrix()))) * glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec4 pos = worldToClip * glm::vec4(transform.position, 1.0f);
	glm::vec3 q = glm::vec3(pos) / pos.w;
	// make sure (pos, 1.0f) dot equation < 0.0 when the point is between the camera and the portal plane
	if (-normal.z - glm::dot(normal, q) > 0.0f) // == dot((0.0f, 0.0f, -1.0f, 1.0f), (normal, -dot(normal,q)) > 0.0f
		normal = -normal;
	return glm::vec4(normal, -glm::dot(normal, q));
	// this equation can be used to check a point position around this portal:
	// if (point.pos, 1.0f) dot equation < 0 when the point is between the camera' near plane and the portal plane
	// else it's between the portal plane and the camera's far plane
}

//PortalRenderTree Portal::GetPortalRenderTree(PortalSpace::PortalContainerConstRef portals) {
//	
//	size_t maxDepth = GetMaxRenderDepth(portals);
//	/* use depth first search to construct the tree
//	*/
//
//	PortalRenderTree tree(portals);
//	PortalRenderTreeNode* node = new PortalRenderTreeNode();
//	tree.root = node;
//
//	for (Portal* p : portals) {
//		PortalRenderTreeNode* n = new PortalRenderTreeNode();
//		n->parent = tree.root;
//		n->portal = p;
//		n->portallingMat = p->GetPortallingMat();
//		if (node == tree.root) {
//			node->firstChild = n;
//			node = n;
//		}
//		else {
//			node->right = n;
//			node = n;
//		}
//	}
//	
//	size_t depth = 1;
//	stencil_t stencil = 2;
//	stencil_t numPortals = portals.size();
//	node = tree.root->firstChild;
//	node->stencil = 1;
//	while (node != tree.root) {
//		PortalRenderTreeNode* nod = node;
//		if (depth < maxDepth)
//			//for (Portal* p : node->portal->cbsPortals) {
//			for (Portal* p : node->portal->dest->GetPortalSpace()->GetPortals()) {
//				if (p == node->portal->dest) // for cases when portal leads into the same space
//					continue;
//				if (numPortals > maxNumPortalRenderings)
//					break;
//				PortalRenderTreeNode* n = new PortalRenderTreeNode();
//				n->parent = node;
//				n->portal = p;
//				n->portallingMat = node->portallingMat * p->GetPortallingMat();
//				if (nod == node) {
//					nod->firstChild = n;
//					nod = n;
//				}
//				else {
//					nod->right = n;
//					nod = n;
//				}
//				numPortals++;
//			}
//		if (node->firstChild != nullptr) {
//			node = node->firstChild;
//			depth++;
//			node->stencil = stencil++;
//		}
//		else if (node->right != nullptr) {
//			node = node->right;
//			node->stencil = stencil++;
//		}
//		else {
//			while (node->right == nullptr && node != tree.root) {
//				node = node->parent;
//				depth--;
//			}
//			if (node != tree.root) {
//				node = node->right;
//				node->stencil = stencil++;
//			}
//		}
//	}
//
//	return tree;
//}

/*
	uses breadth first search to determine max rendering depth
*/
size_t Portal::GetMaxRenderDepth(PortalSpace::PortalContainerConstRef portals) {
	stencil_t numPortalRenderings = 0;
	size_t depth = 1;

	list<Portal*> pl;
	for (auto* p : portals)
		pl.push_back(p);

	size_t depthIncrAfter = pl.size();
	size_t nextDepthIncr = 0;
	while (!pl.empty()) {
		Portal* p = pl.front();
		pl.pop_front();
		numPortalRenderings++;
		if (numPortalRenderings > maxNumPortalRenderings)
			break;
		depthIncrAfter--;
		/*for (Portal* po : p->cbsPortals) {
			nextDepthIncr++;
			pl.push_back(po);
		}*/
		for (Portal* po : p->portalSpace->GetPortals()) {
			if (po == p)
				continue;
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

PortalRenderTree::PortalRenderTree()
{
	numNodes = 0;
}

PortalRenderTree::PortalRenderTree(PortalSpace::PortalContainerConstRef portals)
{
	ConstructTree(portals);
}

void PortalRenderTree::ConstructTree(PortalSpace::PortalContainerConstRef portals)
{
	// use breadth first search to construct the tree
	{
		numNodes = 0;

		list<pair<PortalRenderTreeNode*, Portal*>> pl;
		for (auto* p : portals)
			pl.push_back({ nullptr, p });

		size_t depth = 1;
		size_t depthIncrAfter = pl.size();
		size_t nextDepthIncr = 0;
		while (!pl.empty()) {
			auto p = pl.front();
			pl.pop_front();

			auto* node = arr + numNodes;
			node->parent = p.first;
			if (node->parent != nullptr)
			{
				if (node->parent->firstChild == nullptr)
					node->parent->firstChild = node;
				node->portallingMat = node->parent->portallingMat * p.second->GetPortallingMat();
			}
			else
			{
				node->portallingMat = glm::mat4(1.0f) * p.second->GetPortallingMat();
			}

			node->portal = p.second;

			node->depth = depth;
			if ((node - 1)->parent == node->parent)
				(node - 1)->right = node;
			node->right = nullptr;
			node->firstChild = nullptr;

			depthIncrAfter--;
			numNodes++;
			if (numNodes >= ARR_SIZE)
				break;
			for (Portal* po : p.second->dest->portalSpace->GetPortals()) {
				if (po == p.second->dest)
					continue;
				nextDepthIncr++;
				pl.push_back({ node, po });
			}
			if (depthIncrAfter == 0)
			{
				depth++;
				depthIncrAfter = nextDepthIncr;
				nextDepthIncr = 0;
			}
		}
	}

	// use depth first search to calculate stencil
	{
		stencil_t stencil = 1;
		PortalRenderTreeNode* node;
		if (numNodes > 0)
			node = arr;
		else
			node = nullptr;
		while (node != nullptr)
		{
			node->stencil = stencil++;
			if (node->firstChild != nullptr)
			{
				node = node->firstChild;
				continue;
			}
			if (node->right != nullptr)
			{
				node = node->right;
				continue;
			}
			node = node->parent;
			while (node != nullptr && node->right == nullptr) {
				node = node->parent;
			}
			if (node != nullptr)
			{
				node = node->right;
			}
		}
	}
}

PortalRenderTree::Iterator PortalRenderTree::Begin()
{
	return Iterator(this);
}
const PortalRenderTree::Iterator& PortalRenderTree::End() const
{
	return end;
}

PortalRenderTree::Iterator::Iterator()
{
	this->tree = nullptr;
	this->node = nullptr;
}

PortalRenderTree::Iterator::Iterator(PortalRenderTree* tree)
{
	this->tree = tree;
	if (tree->numNodes > 0)
		this->node = tree->arr;
	else 
		this->node = nullptr;
}

PortalRenderTree::Iterator& PortalRenderTree::Iterator::operator++()
{
	node += 1;
	if (node >= tree->arr + tree->numNodes)
		node = nullptr;
	return *this;
}

PortalRenderTree::Iterator PortalRenderTree::Iterator::operator++(int)
{
	Iterator iter = *this;
	++(*this);
	return iter;
}

PortalRenderTreeNode*& PortalRenderTree::Iterator::operator*()
{
	return node;
}

bool PortalRenderTree::Iterator::operator==(const Iterator& other) const
{
	return this->node == other.node;
}

bool PortalRenderTree::Iterator::operator!=(const Iterator& other) const
{
	return this->node != other.node;
}

void DrawPortalPlane(const Portal& p) {
	glStencilFunc(GL_ALWAYS, p.stencilVal, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	MeshRenderer* r = p.GetComponent<MeshRenderer>();
	Shader* sp = r->GetMaterial()->GetShader();

	sp->Use();
	sp->setVec3("lightColor", { 0.1f, 0.1f, 0.1f });
	r->Draw();
}

void DrawPortalPlane(const PortalRenderTreeNode* p, bool setStencil)
{
	DrawPortalPlane(*p, setStencil);
}

void DrawPortalPlane(const PortalRenderTreeNode& p, bool setStencil) {
	if (setStencil) 
	{
		glStencilFunc(GL_ALWAYS, p.GetStencil(), 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	}

	MeshRenderer* r = p.GetPortal()->GetComponent<MeshRenderer>();
	Shader* sp = r->GetMaterial()->GetShader();

	sp->Use();
	sp->setVec3("lightColor", { 0.1f, 0.1f, 0.1f });
	r->Draw();
}

Cam BeginDrawInsidePortal(const PortalRenderTreeNode& p, const Cam& cam)
{
	Cam c = cam;
	glStencilFunc(GL_EQUAL, p.GetStencil(), 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	c.worldToView = cam.worldToView * p.GetPortallingMat();
	SetGlobalViewMatrix(c.worldToView);
	SetGlobalViewspacePortalEquation(p.GetPortal()->dest->GetViewspacePortalEquation(c.worldToView, c.isOrtho));
	glEnable(GL_CLIP_DISTANCE0);
	return c;
}

void EndDrawInsidePortal()
{
	glDisable(GL_CLIP_DISTANCE0);
}

void DrawPortalContents(const PortalRenderTreeNode& p, const Cam& cam, Material* matOverride) 
{
	Cam c = BeginDrawInsidePortal(p, cam);
	p.GetPortal()->dest->GetPortalSpace()->Draw(&c, matOverride);
	EndDrawInsidePortal();
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

		SetGlobalViewspacePortalEquation(p.dest->GetViewspacePortalEquation(newWTV, c.isOrtho));

		glEnable(GL_CLIP_DISTANCE0);
		p.dest->GetPortalSpace()->Draw(&c, matOverride);
		glDisable(GL_CLIP_DISTANCE0);
	}

	return newWTV;
}

PortalPtr PortalRenderTreeNode::GetPortal() const
{
	return portal;
}

stencil_t PortalRenderTreeNode::GetStencil() const
{
	return stencil;
}

glm::mat4 PortalRenderTreeNode::GetPortallingMat() const
{
	return portallingMat;
}

size_t PortalRenderTreeNode::GetDepth() const
{
	return depth;
}

PortalRenderTreeNode* PortalRenderTreeNode::GetParent() const
{
	return parent;
}

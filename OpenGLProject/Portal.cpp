#include "Portal.h"

#include "Common.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "SubScene.h"



bool Portal::IsVisible(const Camera& cam)
{
	if (!cam.IsOrtho())
		return Physics::CheckOverlap(cam.GetPyramid(), plane);
	else
		return true;
}

Transform Portal::PortalTransformToDest(const Transform& transform)
{
	static const Rotator rot = glm::vec3(0.0f, 180.0f, 0.0f);

	Transform t;
	t.position = dest->GetPortallingMat() * glm::vec4(transform.position, 1.0f);
	t.SetScale(transform.GetScale());
	t.rotation = dest->transform.rotation.GetQuaterion() * rot.GetQuaterion() * glm::inverse(this->transform.rotation.GetQuaterion()) * transform.rotation.GetQuaterion();

	return t;
}

glm::vec4 Portal::GetViewspacePortalEquation(glm::mat4 worldToView, bool isOrtho) const
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
		// make sure (pos, 1.0f) dot equation < 0.0 when the point is between the camera and the portal plane
		if (glm::dot(normal, q) < 0.0f) // == dot((0.0, 0.0, 0.0, 1.0), (normal, -dot(normal,q)) >= 0.0f
			return -glm::vec4(normal, -glm::dot(normal, q));
	}
	return glm::vec4(normal, -glm::dot(normal, q));
	// this equation can be used to check a point position around this portal:
	// if (point.pos, 1.0f) dot equation < 0 when the point is between the camera and the portal plane
	// else it's behind the portal plane
}

/*
	uses breadth first search to determine max rendering depth
*/
size_t Portal::GetMaxRenderDepth(SubScene::PortalContainerConstRef portals) {
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
		for (Portal* po : p->subScene->GetPortals()) {
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

PortalRenderTree::PortalRenderTree(SubScene::PortalContainerConstRef portals, const Camera& cam)
{
	ConstructTree(portals, cam);
}

void PortalRenderTree::ConstructTree(SubScene::PortalContainerConstRef portals, const Camera& cam, const size_t maxDepth)
{
	// use breadth first search to construct the tree
	{
		numNodes = 1;

		auto* root = arr;
		root->depth = 0;
		root->firstChild = nullptr;
		root->parent = nullptr;
		root->portal = nullptr;
		root->camera = cam;
		root->right = nullptr;
		root->stencil = 0;
		root->vsPortalEq = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

		list<pair<PortalRenderTreeNode*, Portal*>> pl;
		// TODO: add only portals that can be seen through the parent portal
		for (auto* p : portals)
		{
			if (p->IsVisible(cam))
				pl.push_back({ root, p });
		}

		if (maxDepth == 0)
			return;

		size_t depth = 1;
		size_t depthIncrAfter = pl.size();
		size_t nextDepthIncr = 0;
		while (!pl.empty()) {
			auto p = pl.front();
			pl.pop_front();

			auto* node = arr + numNodes;

			node->parent = p.first;

			node->camera = node->parent->camera;
			node->camera.SetWorldToViewMatrix(node->camera.GetWorldToViewMatrix() * p.second->GetPortallingMat());

			if (node->parent->firstChild == nullptr)
				node->parent->firstChild = node;

			node->portal = p.second;

			node->depth = depth;

			if ((node - 1)->parent == node->parent)
				(node - 1)->right = node;

			node->right = nullptr;

			node->firstChild = nullptr;

			node->vsPortalEq = p.second->GetViewspacePortalEquation(p.first->camera.GetWorldToViewMatrix(), p.first->camera.IsOrtho());

			depthIncrAfter--;
			numNodes++;
			if (numNodes >= ARR_SIZE)
				break;

			if (depthIncrAfter == 0) {
				++depth;
				if (depth > maxDepth)
					break;
			}

			for (Portal* po : p.second->dest->subScene->GetPortals()) {
				if (po == p.second->dest)
					continue;
				// TODO: add only portals that can be seen through the parent portal
				if (!po->IsVisible(node->GetCamera()))
					continue;

				nextDepthIncr++;
				pl.push_back({ node, po });
			}

			if (depthIncrAfter == 0)
			{
				depthIncrAfter = nextDepthIncr;
				nextDepthIncr = 0;
			}
		}
	}

	// use depth first search to calculate stencil
	{
		stencil_t stencil = 1;
		PortalRenderTreeNode* node;
		auto* const root = arr;
		if (numNodes > 1)
			node = arr + 1;
		else
			node = root;
		while (node != root)
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
			while (node != root && node->right == nullptr) {
				node = node->parent;
			}
			if (node != root)
			{
				node = node->right;
			}
		}
	}
}

PortalRenderTree::Iterator PortalRenderTree::Begin() const
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

PortalRenderTree::Iterator::Iterator(const PortalRenderTree* tree)
{
	this->tree = tree;
	if (tree->numNodes > 0)
		this->node = const_cast<PortalRenderTreeNode*>(tree->arr);
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

void DrawPortalPlane(const PortalRenderTreeNode* p, bool setStencil, const Material* matOverride)
{
	DrawPortalPlane(*p, setStencil, matOverride);
}

void DrawPortalPlane(const PortalRenderTreeNode& p, bool setStencil, const Material* matOverride) {
	if (setStencil) 
	{
		glStencilFunc(GL_ALWAYS, p.GetStencil(), 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	}

	MeshRenderer* r = p.GetPortal()->GetComponent<MeshRenderer>();
	Shader* sp;
	if (matOverride == nullptr)
		sp = r->GetMaterial()->GetShader();
	else
		sp = matOverride->shader;

	sp->Use();
	sp->setVec3("lightColor", { 0.1f, 0.1f, 0.1f });
	r->Draw(matOverride);
}

void BeginDrawInsidePortal(const PortalRenderTreeNode& p)
{
	const auto& c = p.GetCamera();
	glStencilFunc(GL_EQUAL, p.GetStencil(), 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	SetGlobalViewMatrix(c.GetWorldToViewMatrix());
	SetGlobalProjectionMatrix(c.GetProjectionMatrix());
	SetGlobalViewspacePortalEquation(p.GetViewspacePortalEquation());
	glEnable(GL_CLIP_DISTANCE0);
}

void EndDrawInsidePortal()
{
	glDisable(GL_CLIP_DISTANCE0);
}

void DrawPortalContents(const PortalRenderTreeNode& p, const Material* matOverride) 
{
	BeginDrawInsidePortal(p);
	p.GetDestSubScene()->Draw(&p.GetCamera(), matOverride);
	EndDrawInsidePortal();
}

glm::mat4 DrawPortalContents(const Portal& p, const Camera& cam, Material* matOverride) {

	// update world to view matrix 
	glm::mat4 newWTV = cam.GetWorldToViewMatrix() * p.GetPortallingMat();

	Camera c = cam;
	c.SetWorldToViewMatrix(newWTV);

	// draw the scene from a new perspective
	{
		glStencilFunc(GL_EQUAL, p.stencilVal, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		SetGlobalViewspacePortalEquation(p.dest->GetViewspacePortalEquation(newWTV, c.IsOrtho()));

		glEnable(GL_CLIP_DISTANCE0);
		p.dest->GetSubScene()->Draw(&c, matOverride);
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

size_t PortalRenderTreeNode::GetDepth() const
{
	return depth;
}

PortalRenderTreeNode* PortalRenderTreeNode::GetParent() const
{
	return parent;
}

const Camera& PortalRenderTreeNode::GetCamera() const
{
	return camera;
}

const glm::vec4& PortalRenderTreeNode::GetViewspacePortalEquation() const
{
	return vsPortalEq;
}

SubScene* PortalRenderTreeNode::GetDestSubScene() const
{
	if (portal == nullptr)
		return currentSubScene;
	else
		return portal->dest->GetSubScene();
}

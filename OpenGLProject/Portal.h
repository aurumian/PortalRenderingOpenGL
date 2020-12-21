#pragma once

#include "Actor.h"

class Tree;

typedef uint8_t stencil_t;

class Portal : public Actor
{
public:
	static const stencil_t maxNumPortalRenderings = 8;

	stencil_t stencilVal;
	stencil_t prevStencil;
	// portals that can be seen through this portal
	vector<Portal*> cbsPortals;

	Portal* dest;

	size_t GetMaxRenderDepth() { return maxRenderDepth; }

	void SetMaxRenderDepth(size_t v) {
		if (v > 0)
			maxRenderDepth = v;
	}

	glm::vec4 GetViewspacePortalEquation(glm::mat4 worldToView) const;

	glm::mat4 GetPortallingMat() const;

	static Tree GetPortalRenderTree(const list<Portal*>& portals);

	static size_t GetMaxRenderDepth(const list<Portal*>& portals);

private:
	size_t maxRenderDepth = 1;
};

class Node {
public:
	Portal* portal = nullptr;
	Node* firstChild = nullptr;
	Node* right = nullptr;
	Node* parent = nullptr;
	uint8_t stencil = 0;
	glm::mat4 portalMat = glm::mat4(1.0f);
};

class Tree {
public:
	Node* root;
};


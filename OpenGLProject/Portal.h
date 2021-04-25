#pragma once

#include "Actor.h"
#include <unordered_map>
#include <list>

class PortalRenderTree;
struct Cam;
class Material;

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

	glm::vec4 GetViewspacePortalEquation(glm::mat4 worldToView, float isOrtho = false) const;

	glm::vec4 GetNdcSpacePortalEquation(glm::mat4 worldToClip) const;

	glm::mat4 GetPortallingMat() const;

	static PortalRenderTree GetPortalRenderTree(const list<Portal*>& portals);

	static size_t GetMaxRenderDepth(const list<Portal*>& portals);

private:
	size_t maxRenderDepth = 1;
};

class PortalRenderTreeNode {
public:
	Portal* portal = nullptr;
	PortalRenderTreeNode* firstChild = nullptr;
	PortalRenderTreeNode* right = nullptr;
	PortalRenderTreeNode* parent = nullptr;
	uint8_t stencil = 0;
	glm::mat4 portalMat = glm::mat4(1.0f);
};

class PortalRenderTree {
public:
	PortalRenderTreeNode* root;
};

/*@pre Stencil testing must be enabled
  @pre Stencil buffer should be cleared with 0's
 */
void DrawPortalPlane(const Portal& p, const glm::mat4& worldToView);

void UpdateStencil(const std::list<Portal*>& pl, std::unordered_map<const Portal*, glm::mat4>& wtvs);

glm::mat4 DrawPortalContents(const Portal& p1, const Cam& cam, Material* matOverride = nullptr);



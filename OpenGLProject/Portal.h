#pragma once

#include "MathInclude.h"
#include "Actor.h"
#include "PortalSpace.h"

#include <unordered_map>
#include <list>
#include <map>

#include "Physics.h"

class PortalRenderTree;
class Material;
class Camera;

typedef uint8_t stencil_t;


//struct Vec2Cmp
//{
//	bool operator()(const glm::vec2& v1, const glm::vec2& v2) const
//	{
//		if (v1.x != v2.x)
//			return v1.x < v2.x;
//		return v2.y < v2.y;
//	}
//};

class Portal;
typedef Portal* PortalPtr;
typedef const Portal* PortalConstPtr;
class Portal : public Actor
{
public:

	static const size_t maxNumPortalRenderings = 16;

	stencil_t stencilVal;
	stencil_t prevStencil;
	// portals that can be seen through this portal
	vector<PortalPtr> cbsPortals;
	RectanglePlane plane = RectanglePlane(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.5f, 0.5f));

	PortalPtr dest;

	bool IsVisible(const Camera& cam);



	glm::vec4 GetViewspacePortalEquation(glm::mat4 worldToView, float isOrtho = false) const;

	glm::mat4 GetPortallingMat() const;

	//static PortalRenderTree GetPortalRenderTree(PortalSpace::PortalContainerConstRef portals);

	static size_t GetMaxRenderDepth(PortalSpace::PortalContainerConstRef portals);


	
};


class PortalRenderTreeNode {
public:
	PortalPtr GetPortal() const;
	stencil_t GetStencil() const;
	glm::mat4 GetPortallingMat() const;
	size_t GetDepth() const;
	PortalRenderTreeNode* GetParent() const;

	friend class PortalRenderTree;
private:
	PortalPtr portal = nullptr;
	PortalRenderTreeNode* firstChild = nullptr;
	PortalRenderTreeNode* right = nullptr;
	PortalRenderTreeNode* parent = nullptr;
	stencil_t stencil = 0;
	glm::mat4 portallingMat = glm::mat4(1.0f);
	size_t depth = 0;
};

class PortalRenderTree {
public:
	PortalRenderTree();

	PortalRenderTree(PortalSpace::PortalContainerConstRef portals, const Camera& cam);

	/*
	* Constructs a tree discarding previous elements
	*/
	void ConstructTree(PortalSpace::PortalContainerConstRef portals, const Camera& cam);

	class Iterator {
	public:
		Iterator();
		Iterator(PortalRenderTree* tree);
		Iterator& operator++();
		Iterator operator++(int);
		PortalRenderTreeNode*& operator*();
		bool operator==(const Iterator& other) const;
		bool operator!=(const Iterator& other) const;
	private:
		PortalRenderTree* tree;
		PortalRenderTreeNode* node;
	};

	Iterator Begin();
	const Iterator& End() const;

private:
	static const size_t ARR_SIZE = Portal::maxNumPortalRenderings;
	PortalRenderTreeNode arr[ARR_SIZE];
	size_t numNodes = 0;
	Iterator end;
};

/*@pre Stencil testing must be enabled
  @pre Stencil buffer should be cleared with 0's
 */
void DrawPortalPlane(const Portal& p);

void UpdateStencil(const std::list<PortalPtr>& pl, std::unordered_map<PortalConstPtr, glm::mat4>& wtvs);

glm::mat4 DrawPortalContents(const Portal& p1, const Camera& cam, Material* matOverride = nullptr);

// The procedure assumes the projection and worldToView matrices are already set
void DrawPortalPlane(const PortalRenderTreeNode& p, bool setStencil = true);
void DrawPortalPlane(const PortalRenderTreeNode* p, bool setStencil = true);

void DrawPortalContents(const PortalRenderTreeNode& p, const Camera& cam, Material* matOverride = nullptr);

Camera BeginDrawInsidePortal(const PortalRenderTreeNode& p, const Camera& cam);

void EndDrawInsidePortal();
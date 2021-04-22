#pragma once

#include <unordered_set>

#include "Lighting.h"

class Actor;
class Portal;


class PortalSpace
{
public:
	// all drawable lights of a portal space
	static std::unordered_set<PortalShadowedDirLight*> shadowmappedLights;

	std::unordered_set<DrawableDirLight*> drawableDirLights;

	std::unordered_set<DirLight*> dirLights;

	void AddActor(Actor* actor);

	void RemoveActor(Actor* actor);

	void Draw(const Cam& cam, Material* matOverride = nullptr);

	void AddPortal(Portal* p);

	const std::unordered_set<Portal*>& GetPortals();

protected:
	std::unordered_set<Actor*> actors;
	std::unordered_set<MeshRenderer*> renderers;
	std::unordered_set<Portal*> portals;
};
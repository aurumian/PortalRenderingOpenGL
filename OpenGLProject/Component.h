#pragma once

#include "Transform.h"

class Actor;


class Component {
public:

	Transform GetTransform();

	virtual ~Component() {}

	Actor* GetOwner() {
		return owner;
	}

private:
	Actor* owner;
	friend class Actor;
};
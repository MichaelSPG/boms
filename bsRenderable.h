#pragma once

#include "bsConfig.h"

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabbUtil.h>


/*	Base class for all renderable objects.
	In order for a renderable to be rendered, it must be attached to a scene node.
*/
class bsRenderable
{
	friend class bsSceneNode;
public:

	//All the types of subclasses of this class can have.
	enum RenderableType
	{
		MESH,
		WIREFRAME_PRIMITIVE,
		LINE,
		LIGHT
	};

	bsRenderable()
	{
		mAabb.setEmpty();
	}

	virtual ~bsRenderable() {}

	/*	Returns the type of this renderable, making it possible to cast a pointer to this
		base class to a pointer to a subclass safely.
	*/
	virtual inline RenderableType getRenderableType() const = 0;

	//Whether this renderable is ready for rendering
	virtual bool hasFinishedLoading() const = 0;

	virtual const hkAabb& getAabb() const
	{
		return mAabb;
	}

protected:
	hkAabb			mAabb;
};

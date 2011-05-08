#ifndef BS_RENDERABLE_H
#define BS_RENDERABLE_H

#include "bsConfig.h"

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>
#include <Common/Base/Types/Geometry/Aabb/hkAabbUtil.h>

class bsSceneNode;


class bsRenderable
{
	friend class bsSceneNode;
public:

	enum RenderableIdentifier
	{
		MESH,
		WIREFRAME_PRIMITIVE,
		LINES
	};

	virtual ~bsRenderable() {}

	virtual inline RenderableIdentifier getRenderableIdentifier() const = 0;

	//Whether this renderable is ready for rendering
	virtual bool isOkForRendering() const = 0;

	virtual const hkAabb& getAabb() const
	{
		return mAabb;
	}

protected:
	hkAabb			mAabb;
};

#endif // BS_RENDERABLE_H

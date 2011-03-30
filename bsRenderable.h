#ifndef RENDERABLE_H
#define RENDERABLE_H

class bsSceneNode;

class bsRenderable
{
	friend class bsSceneNode;
public:

	enum RenderableIdentifier
	{
		MESH,
		WIREFRAME_PRIMITIVE
	};

	virtual ~bsRenderable() {}

	virtual inline const RenderableIdentifier getRenderableIdentifier() const = 0;
};

#endif
#ifndef RENDERABLE_H
#define RENDERABLE_H

class SceneNode;

class Renderable
{
	friend class SceneNode;
public:

	enum RenderableIdentifier
	{
		MESH,
		WIREFRAME_PRIMITIVE
	};

	virtual ~Renderable() {}

	virtual inline const RenderableIdentifier getRenderableIdentifier() const = 0;
};

#endif
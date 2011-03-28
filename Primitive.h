#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "Renderable.h"

#include "Dx11Renderer.h"

class ShaderManager;
class VertexShader;
class hkAabb;
class PixelShader;
class VertexShader;

struct Vertex
{
	XMFLOAT3 Pos;
};

class Primitive : public Renderable
{
	friend class Node;
	friend class OctNode;
	friend class SceneNode;
	friend class SceneGraph;

public:
	Primitive();
	~Primitive();

	//nodeAabb should be set to false for anything but scene nodes and oct nodes.
	void createPrimitive(Dx11Renderer* dx11Renderer, ShaderManager* shaderManager,
		const hkAabb& aabb, bool nodeAabb = true);

	void draw(Dx11Renderer* dx11Renderer);

	inline const RenderableIdentifier getRenderableIdentifier() const
	{
		return WIREFRAME_PRIMITIVE;
	}
	
//private://TODO: uncomment
	std::shared_ptr<VertexShader>	mVertexShader;
	std::shared_ptr<PixelShader>	mPixelShader;

	ID3D11Buffer*		mBuffer;

	ID3D11Buffer*		mVertexBuffer;
	ID3D11Buffer*		mIndexBuffer;

	XMFLOAT4			mColor;
};

#endif // PRIMITIVE_H
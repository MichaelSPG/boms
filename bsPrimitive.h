#ifndef BS_PRIMITIVE_H
#define BS_PRIMITIVE_H

#include "bsRenderable.h"

#include "bsDx11Renderer.h"

class bsShaderManager;
class bsVertexShader;
class hkAabb;
class bsPixelShader;
class bsVertexShader;


class bsPrimitive : public bsRenderable
{
	friend class bsSceneNode;
	friend class bsSceneGraph;

public:
	bsPrimitive();
	~bsPrimitive();

	//nodeAabb should be set to false for anything but scene nodes and oct nodes.
	void createPrimitive(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager,
		const hkAabb& aabb, bool nodeAabb = true);

	void draw(bsDx11Renderer* dx11Renderer);

	inline const RenderableIdentifier getRenderableIdentifier() const
	{
		return WIREFRAME_PRIMITIVE;
	}

	bool isOkForRendering() const
	{
		return mFinished;
	}
	
//private://TODO: uncomment
	std::shared_ptr<bsVertexShader>	mVertexShader;
	std::shared_ptr<bsPixelShader>	mPixelShader;

	ID3D11Buffer*	mBuffer;

	ID3D11Buffer*	mVertexBuffer;
	ID3D11Buffer*	mIndexBuffer;

	XMFLOAT4		mColor;

	bool			mFinished;
};

#endif // BS_PRIMITIVE_H

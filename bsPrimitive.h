#if 0

#pragma once


#include <memory>

#include <d3d11.h>
#include <Windows.h>
#include <xnamath.h>

#include <Common/Base/hkBase.h>

#include "bsRenderable.h"

class bsShaderManager;
class bsVertexShader;
class hkAabb;
class bsPixelShader;
class bsVertexShader;
class bsDx11Renderer;


/*	Class for wireframe drawing of primitive objects.
	Currently only supports creation from AABBs.
*/
class bsPrimitive : public bsRenderable
{
	friend class bsSceneNode;
	friend class bsSceneGraph;

public:
	bsPrimitive();
	~bsPrimitive();

	//Creates a primitive from an AABB.
	void createPrimitive(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager,
		const hkAabb& aabb);

	void draw(bsDx11Renderer* dx11Renderer);

	inline RenderableType getRenderableType() const
	{
		return WIREFRAME_PRIMITIVE;
	}

	inline bool isOkForRendering() const
	{
		return mFinished;
	}
	
private:
	std::shared_ptr<bsVertexShader>	mVertexShader;
	std::shared_ptr<bsPixelShader>	mPixelShader;

	ID3D11Buffer*	mBuffer;

	ID3D11Buffer*	mVertexBuffer;
	ID3D11Buffer*	mIndexBuffer;

	XMFLOAT4		mColor;

	bool			mFinished;
};

#endif

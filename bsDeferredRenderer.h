#pragma once


#include <memory>
#include <vector>
#include <functional>

#include <d3d11.h>

#include "bsRenderSystem.h"

class bsDx11Renderer;
class bsCamera;
class bsRenderQueue;
class bsShaderManager;
class bsRenderTarget;
class bsWindow;
class bsFullScreenQuad;
class bsVertexShader;
class bsPixelShader;


/*	Class for deferred shading.
	Renders the geometry of a scene into a G buffers which is then used by other shaders
	to construct the final image.
*/
class bsDeferredRenderer : public bsRenderSystem
{
public:
	struct GBuffer
	{
		bsRenderTarget*	position;
		bsRenderTarget*	normal;
		bsRenderTarget*	diffuse;
	};


	bsDeferredRenderer(bsDx11Renderer* dx11Renderer, bsCamera* camera,
		bsShaderManager* shaderManager, bsWindow* window);

	virtual ~bsDeferredRenderer();

	/*	Renders a single frame, and executes all registered end-of-render callback
		functions just before the frame is presented.
	*/
	virtual void renderOneFrame();

	/*	All functions registered here will be called after rendering is done, but before
		present is called, making it possible to draw UI elements and similar on top of
		the rendered frame.
	*/
	inline void registerEndOfRenderCallback(const std::function<void(void)>& func)
	{
		mEndOfRenderCallbacks.push_back(func);
	}

	inline const bsRenderQueue* getRenderQueue() const
	{
		return mRenderQueue;
	}


private:
	//Creates shaders used for merging the GBuffer into the final render output.
	void createShaders();

	bsDx11Renderer*		mDx11Renderer;
	bsCamera*			mCamera;
	bsRenderQueue*		mRenderQueue;
	bsShaderManager*	mShaderManager;

	GBuffer				mGBuffer;
	bsRenderTarget*		mLightRenderTarget;

	bsFullScreenQuad*	mFullScreenQuad;
	std::shared_ptr<bsVertexShader>	mMergerVertexShader;
	std::shared_ptr<bsPixelShader>	mMergerPixelShader;

	std::vector<std::function<void(void)>>	mEndOfRenderCallbacks;

	ID3D11RasterizerState*	mGeometryRasterizerState;
	ID3D11RasterizerState*	mLightRasterizerState;

	ID3D11BlendState*		mGeometryBlendState;
	ID3D11BlendState*		mLightBlendState;

	ID3D11DepthStencilState*	mDepthEnabledStencilState;
	ID3D11DepthStencilState*	mDepthDisabledStencilState;
};

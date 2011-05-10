#ifndef BS_RENDERTARGET_H
#define BS_RENDERTARGET_H

#include "bsConfig.h"

#include <d3d11.h>


/**	Defines a render target which can be used as both a render target, texture and shader
	resource (input for shaders).
*/
class bsRenderTarget
{
	friend class bsDx11Renderer;

public:
	bsRenderTarget(unsigned int width, unsigned int height, ID3D11Device* d3d11Device);

	~bsRenderTarget();
	
	inline ID3D11RenderTargetView* getRenderTargetView() const
	{
		return mRenderTargetView;
	}

	inline ID3D11ShaderResourceView* getShaderResourceView() const
	{
		return mShaderResourceView;
	}

	inline ID3D11Texture2D* getRenderTargetTexture() const
	{
		return mRenderTargetTexture;
	}


private:
	ID3D11Texture2D*			mRenderTargetTexture;
	ID3D11RenderTargetView*		mRenderTargetView;
	ID3D11ShaderResourceView*	mShaderResourceView;
};

#endif // BS_RENDERTARGET_H

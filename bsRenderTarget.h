#pragma once

#include <d3d11.h>


/*	Defines a render target which can be used as both a render target, texture and shader
	resource (input for shaders).
	Remember to register instances of this class with the bsDx11Renderer so they will be
	notified of screen resizing.
*/
class bsRenderTarget
{
public:
	bsRenderTarget(unsigned int width, unsigned int height, ID3D11Device& device);

	~bsRenderTarget();


	/*	Called when the screen is resized.
		This resizes the render target to match the size of the screen.
	*/
	bool windowResized(unsigned int width, unsigned int height, ID3D11Device& device);
	
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
	bool createTexture(unsigned int width, unsigned int height, ID3D11Device& device);

	bool createView(ID3D11Device& device);

	bool createShaderResourceView(ID3D11Device& device);


	DXGI_FORMAT					mFormat;
	ID3D11Texture2D*			mRenderTargetTexture;
	ID3D11RenderTargetView*		mRenderTargetView;
	ID3D11ShaderResourceView*	mShaderResourceView;
};

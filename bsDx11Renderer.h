#ifndef BS_DX11_RENDERER_H
#define BS_DX11_RENDERER_H

#include "bsConfig.h"

#include <Windows.h>
#include <d3d11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>

class bsRenderTarget;


class bsDx11Renderer
{
public:
	bsDx11Renderer(HWND hWnd, int renderWindowWidth, int renderWindowHeight);

	~bsDx11Renderer();

	//Presents drawn primitives, waits for v-sync if enabled.
	void present();

	inline ID3D11DeviceContext* getDeviceContext() const
	{
		return mDeviceContext;
	}

	inline ID3D11Device* getDevice() const
	{
		return mDevice;
	}

	HRESULT compileShader(const char* fileName, const char* entryPoint,
		const char* shaderModel, ID3DBlob** blobOut);

	inline void setVsyncEnabled(bool enabled)
	{
		mVsyncEnabled = enabled;
	}

	inline bool getVsyncEnabled() const
	{
		return mVsyncEnabled;
	}

	/*	Sets an array of render targets to be used as current render targets.
		Can be used to unbind by setting renderTargets to null and renderTargetCount to
		the amount of render targets to unbind.
	*/
	void setRenderTargets(bsRenderTarget** renderTargets, unsigned int renderTargetCount);

	//Sets the back buffer as the active render target.
	void setBackBufferAsRenderTarget();

	//Clears render targets, making it possible to draw on them again.
	//color should be a pointer to an array of 4 floats (RGBA), or null.
	void clearRenderTargets(bsRenderTarget** renderTargets, unsigned int count,
		float* colorRgba);

	void clearBackBuffer();


private:
	IDXGISwapChain*			mSwapChain;
	ID3D11Device*			mDevice;
	ID3D11DeviceContext*	mDeviceContext;
	ID3D11RenderTargetView*	mBackBufferRenderTargetView;

	ID3D11Texture2D*		mDepthStencil;
	ID3D11DepthStencilView*	mDepthStencilView;

	bool	mVsyncEnabled;
	float	mRenderTargetClearColor[4];
};

#endif // BS_DX11_RENDERER_H

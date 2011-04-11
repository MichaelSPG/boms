#ifndef BS_DX11_RENDERER_H
#define BS_DX11_RENDERER_H

#include "bsConfig.h"

#include <d3d11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>

#include "bsMath.h"

#include <vector>
#include <exception>

#include "bsVertexTypes.h"

struct SimpleVertex 
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct CBWireFrame
{
	XMFLOAT4X4	world;
	XMFLOAT4	color;
};

struct CBWorld
{
	XMFLOAT4X4	world;
};

struct CBViewProjection
{
	XMFLOAT4X4 viewProjection;
};

class bsDx11Renderer
{
public:
	bsDx11Renderer(HWND hWnd, int renderWindowWidth, int renderWindowHeight);

	~bsDx11Renderer();

	//Clears render view target and depth stencil.
	void preRender();

	//Presents drawn primitives.
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


private:
	IDXGISwapChain*			mSwapChain;
	ID3D11Device*			mDevice;
	ID3D11DeviceContext*	mDeviceContext;
	ID3D11RenderTargetView*	mRenderTargetView;

	ID3D11Texture2D*		mDepthStencil;
	ID3D11DepthStencilView*	mDepthStencilView;

	bool mVsyncEnabled;
};

#endif // BS_DX11_RENDERER_H

#ifndef DX11_RENDERER_H
#define DX11_RENDERER_H

#include <d3d11.h>
#include <D3DX11.h>
//#define _XM_NO_INTRINSICS_
//#define XM_NO_OPERATOR_OVERLOADS//http://msdn.microsoft.com/en-us/library/ee418732%28v=VS.85%29.aspx#AvoidOverLoad
//#include <xnamath.h>
#include "Math.h"

#include <vector>
#include <exception>
#define UNICODE
#include <Core/DXUT.h>
#include <Optional/DXUTsettingsdlg.h>
#undef UNICODE

#include "VertexTypes.h"
#include "Renderable.h"
#include "Font.h"
#include "Log.h"

struct SimpleVertex 
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct CBChangesNever
{
	//XMMATRIX mView;
	XMFLOAT4X4 mView;
};

struct CBChangesOnResize
{
	//XMMATRIX mProjection;
	XMFLOAT4X4 mProjection;
};

struct CBChangesEveryFrame
{
	//XMMATRIX mWorld;
	XMFLOAT4X4 mWorld;
	XMFLOAT4 vMeshColor;
};

struct CBWireFrame
{
	XMFLOAT4X4	world;
	XMFLOAT4	color;
};

struct CBCamera
{
	XMFLOAT4X4 camera;
};

class Dx11Renderer
{
public:
	Dx11Renderer()
	: mSwapChain(nullptr)
	, mDevice(nullptr)
	, mDeviceContext(nullptr)
	, mRenderTargetView(nullptr)

	, mDepthStencil(nullptr)
	, mDepthStencilView(nullptr)

	, mFont(nullptr)
	{
	}

	~Dx11Renderer();

	void init(HWND hWnd, int renderWindowWidth, int renderWindowHeight);

	void preRender();
	void render();
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


private:
	void initDXUT(HWND hWnd);

	IDXGISwapChain*			mSwapChain;
	ID3D11Device*			mDevice;
	ID3D11DeviceContext*	mDeviceContext;
	ID3D11RenderTargetView*	mRenderTargetView;

	ID3D11Texture2D*		mDepthStencil;
	ID3D11DepthStencilView*	mDepthStencilView;

	Font* mFont;
};

#endif
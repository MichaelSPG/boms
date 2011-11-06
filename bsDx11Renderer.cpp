#include "StdAfx.h"

#include "bsDx11Renderer.h"

#include <string>

#include "bsLog.h"
#include "bsRenderTarget.h"
#include "bsAssert.h"


bsDx11Renderer::bsDx11Renderer(HWND hWnd, int renderWindowWidth, int renderWindowHeight)
	: mSwapChain(nullptr)
	, mDevice(nullptr)
	, mDeviceContext(nullptr)

	, mDepthStencil(nullptr)
	, mDepthStencilView(nullptr)

	, mVsyncEnabled(true)
{
	bsLog::log("Starting initialization of Direct3D 11");

	mRenderTargetClearColor[0] = mRenderTargetClearColor[1] = mRenderTargetClearColor[2] = 0.3f;
	mRenderTargetClearColor[3] = 0.0f;

	createRenderWindow(hWnd, renderWindowWidth, renderWindowHeight);

#ifdef BS_DEBUG
	//Set debug info in D3D objects to help debugging their lifetimes if necessary.
	std::string debugString("IDXGISwapChain");
	mSwapChain->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	debugString = "ID3D11Device";
	mDevice->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	debugString = "ID3D11DeviceContext";
	mDeviceContext->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());
	
	debugString = "ID3D11RenderTargetView Back buffer";
	mBackBufferRenderTargetView->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());
		
	debugString = "ID3D11Texture2D_DepthStencil";
	mDepthStencil->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());

	debugString = "ID3D11DepthStencilView";
	mDepthStencilView->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
		debugString.c_str());
#endif

	bsLog::log("DirectX initialization completed successfully");
}

bsDx11Renderer::~bsDx11Renderer()
{
	bsLog::log("Uninitializing DirectX");

	destroyCurrentRenderWindow();

	bsLog::log("DirectX successfully uninitialized");
}

bool bsDx11Renderer::createRenderWindow(HWND hWnd, int renderWindowWidth,
	int renderWindowHeight)
{
	//Swap chain & device
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = renderWindowWidth;
	swapChainDesc.BufferDesc.Height = renderWindowHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = true;

	unsigned int deviceFlags = 0;
#ifdef BS_DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_10_0;

	bsLog::log("Creating D3D11 device and swap chain");
	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags,
		featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &swapChainDesc, &mSwapChain,
		&mDevice, &fl, &mDeviceContext)))
	{
		BS_ASSERT2(false, "Failed to create D3D11 device");

		return false;
	}

	//Back buffer
	bsLog::log("Creating back buffer");
	ID3D11Texture2D* backBuffer = nullptr;
	if (FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
	{
		BS_ASSERT2(false, "Failed to create back buffer");

		return false;
	}

	
	bsLog::log("Creating back buffer render target view");
	if (FAILED(mDevice->CreateRenderTargetView(backBuffer, nullptr, &mBackBufferRenderTargetView)))
	{
		BS_ASSERT2(false, "Failed to create back buffer render target view");

		return false;
	}
	
	backBuffer->Release();

	//Depth stencil
	D3D11_TEXTURE2D_DESC depthDesc = { 0 };
	depthDesc.Width = renderWindowWidth;
	depthDesc.Height = renderWindowHeight;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	bsLog::log("Creating depth stencil texture");
	if (FAILED(mDevice->CreateTexture2D(&depthDesc, nullptr, &mDepthStencil)))
	{
		BS_ASSERT2(false, "Failed to create depth stencil texture");

		return false;
	}


	/*
	//Defaults
	D3D11_DEPTH_STENCIL_DESC dsd;
	memset(&dsd, 0, sizeof(dsd));
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.StencilEnable = false;
	dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;//D3D11_COMPARISON_LESS_EQUAL
	ID3D11DepthStencilState* dss;
	mDevice->CreateDepthStencilState(&dsd, &dss);
	mDeviceContext->OMSetDepthStencilState(dss, 0);
	*/
	
	//Depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	memset(&depthViewDesc, 0, sizeof(depthViewDesc));
	depthViewDesc.Format = depthDesc.Format;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthViewDesc.Texture2D.MipSlice = 0;

	bsLog::log("Creating depth stencil view");
	if (FAILED(mDevice->CreateDepthStencilView(mDepthStencil, &depthViewDesc,
		&mDepthStencilView)))
	{
		BS_ASSERT2(false, "Failed to create depth stencil view");

		return false;
	}

	//Viewport
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)renderWindowWidth;
	viewport.Height = (float)renderWindowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	mDeviceContext->RSSetViewports(1, &viewport);

	return true;
}

void bsDx11Renderer::destroyCurrentRenderWindow()
{
	if (mDepthStencil)
	{
		mDepthStencil->Release();
	}
	if (mDepthStencilView)
	{
		mDepthStencilView->Release();
	}

	if (mBackBufferRenderTargetView)
	{
		mBackBufferRenderTargetView->Release();
	}

	if (mSwapChain)
	{
		mSwapChain->Release();
	}

	if (mDeviceContext)
	{
		mDeviceContext->ClearState();
		mDeviceContext->Flush();
		mDeviceContext->Release();
	}
	if (mDevice)
	{
		mDevice->Release();
	}
}

void bsDx11Renderer::present() const
{
	if (FAILED(mSwapChain->Present(mVsyncEnabled ? 1 : 0, 0)))
	{
		BS_ASSERT(!"IDXGISwapChain::Present failed");
	}
}

HRESULT bsDx11Renderer::compileShader(const char* fileName, const char* entryPoint,
	const char* shaderModel, ID3DBlob** blobOut) const
{
	HRESULT hResult;

	unsigned int shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#ifdef BS_DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif
	const unsigned int effectFlags = 0;

	ID3DBlob* errorBlob;
	hResult = D3DX11CompileFromFileA(fileName, nullptr, nullptr, entryPoint, shaderModel,
		shaderFlags, effectFlags, nullptr, blobOut, &errorBlob, nullptr);

	if (FAILED(hResult))
	{
		if (errorBlob != nullptr)
		{
			bsLog::logf(bsLog::SEV_ERROR, "Failed to compile shader '%s. Error message: %s",
				fileName, errorBlob->GetBufferPointer());

			OutputDebugStringA((char*)errorBlob->GetBufferPointer());

			BS_ASSERT2(false, "Failed to compile shader");
		}
		else
		{
			bsLog::logf(bsLog::SEV_ERROR, "Failed to compile shader '%s'", fileName);

		}
	}

	if (errorBlob)
	{
		errorBlob->Release();
	}

	return hResult;
}

void bsDx11Renderer::setRenderTargets(bsRenderTarget** renderTargets,
	unsigned int renderTargetCount)
{
	BS_ASSERT2(renderTargetCount > 0 && renderTargetCount <= D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		"Render target count out of valid range (0-8)");

	//Array of render targets to be set.
	ID3D11RenderTargetView* renderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };

	//If renderTargets is null, nothing will be copied, will unbind render targets instead.
	if (renderTargets)
	{
		for (unsigned int i = 0; i < renderTargetCount; ++i)
		{
			renderTargetViews[i] = renderTargets[i]->mRenderTargetView;
		}
	}

	mDeviceContext->OMSetRenderTargets(renderTargetCount, renderTargetViews, mDepthStencilView);
}

void bsDx11Renderer::setBackBufferAsRenderTarget()
{
	mDeviceContext->OMSetRenderTargets(1, &mBackBufferRenderTargetView, mDepthStencilView);
}

void bsDx11Renderer::clearRenderTargets(bsRenderTarget** renderTargets, unsigned int count)
{
	//Clear every render target provided with pre-defined clear color.
	for (unsigned int i = 0; i < count; ++i)
	{
		mDeviceContext->ClearRenderTargetView(renderTargets[i]->mRenderTargetView,
			mRenderTargetClearColor);
	}
}

void bsDx11Renderer::clearBackBuffer()
{
	mDeviceContext->ClearRenderTargetView(mBackBufferRenderTargetView, mRenderTargetClearColor);
	//Clear depth buffer to 1.0 (max depth)
	mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void bsDx11Renderer::setRenderTargetClearColor( float* colorRgba )
{
	memcpy(mRenderTargetClearColor, colorRgba,
		sizeof(mRenderTargetClearColor[0]) * ARRAYSIZE(mRenderTargetClearColor));
}

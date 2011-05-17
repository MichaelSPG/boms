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
	bsLog::logMessage("Starting initialization of Direct3D 11");
	/*
	RECT rect;
	GetWindowRect(hWnd, &rect);
	renderWindowWidth = rect.right - rect.left;
	renderWindowHeight = rect.bottom - rect.top;
	*/

	//Swap chain & device
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	memset(&swapChainDesc, 0, sizeof(swapChainDesc));
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
#if BS_DEBUG_LEVEL > 0
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_10_0;

	bsLog::logMessage("Creating D3D11 device and swap chain");
	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags,
		featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &swapChainDesc, &mSwapChain,
		&mDevice, &fl, &mDeviceContext)))
	{
		BS_ASSERT(!"Failed to create D3D11 device");
	}

	//Back buffer
	ID3D11Texture2D* backBuffer = nullptr;
	bsLog::logMessage("Creating back buffer");
	if (FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
	{
		BS_ASSERT(!"Failed to create back buffer");
	}

	
	//D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	bsLog::logMessage("Creating back buffer render target view");
	if (FAILED(mDevice->CreateRenderTargetView(backBuffer, nullptr, &mBackBufferRenderTargetView)))
	{
		BS_ASSERT(!"Failed to create back buffer render target view");
	}
	
	backBuffer->Release();

	//Depth stencil
	D3D11_TEXTURE2D_DESC depthDesc;
	memset(&depthDesc, 0, sizeof(depthDesc));
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

	bsLog::logMessage("Creating depth stencil texture");
	if (FAILED(mDevice->CreateTexture2D(&depthDesc, nullptr, &mDepthStencil)))
	{
		BS_ASSERT(!"Failed to create depth stencil texture");
	}

	//Depth stencil view

	D3D11_DEPTH_STENCIL_DESC dsd;
	dsd.DepthEnable;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	memset(&depthViewDesc, 0, sizeof(depthViewDesc));
	depthViewDesc.Format = depthDesc.Format;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthViewDesc.Texture2D.MipSlice = 0;

	bsLog::logMessage("Creating depth stencil view");
	if (FAILED(mDevice->CreateDepthStencilView(mDepthStencil, &depthViewDesc,
		&mDepthStencilView)))
	{
		BS_ASSERT(!"Failed to create depth stencil view");
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

	mRenderTargetClearColor[0] = mRenderTargetClearColor[1] = mRenderTargetClearColor[2] = 0.3f;
	mRenderTargetClearColor[3] = 0.0f;


#if BS_DEBUG_LEVEL > 0
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

	bsLog::logMessage("DirectX initialization completed successfully");
}

bsDx11Renderer::~bsDx11Renderer()
{
	bsLog::logMessage("Uninitializing DirectX", pantheios::SEV_NOTICE);

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

	bsLog::logMessage("DirectX successfully uninitialized");
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

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* errorBlob;
	hResult = D3DX11CompileFromFileA(fileName, nullptr, nullptr, entryPoint, shaderModel,
		shaderFlags, 0, nullptr, blobOut, &errorBlob, nullptr);

	if (FAILED(hResult))
	{
		if (errorBlob != nullptr)
		{
			std::string errorMessage("Failed to compile shader '");
			errorMessage += fileName;
			errorMessage += ". Error message: ";
			errorMessage += (char*)errorBlob->GetBufferPointer();
			
			bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);

			OutputDebugStringA((char*)errorBlob->GetBufferPointer());

			BS_ASSERT(!"Failed to compile shader");
		}
		else
		{
			std::string errorMessage("Failed to compile shader '");
			errorMessage += fileName;
			errorMessage += ". Error message: ";

			bsLog::logMessage(errorMessage.c_str(), pantheios::SEV_ERROR);
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

#include "bsDx11Renderer.h"

#include <assert.h>
#include <string>

#include "bsLog.h"


bsDx11Renderer::bsDx11Renderer(HWND hWnd, int renderWindowWidth, int renderWindowHeight)
	: mSwapChain(nullptr)
	, mDevice(nullptr)
	, mDeviceContext(nullptr)
	, mRenderTargetView(nullptr)

	, mDepthStencil(nullptr)
	, mDepthStencilView(nullptr)
{
	bsLog::logMessage("Starting initialization of Direct3D 11");

	//Swap chain & device
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
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
#ifdef _DEBUG
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
		bsLog::logMessage("Failed to create D3D11 device", pantheios::SEV_CRITICAL);
		assert(!"Failed to create D3D11 device");
	}

	//Back buffer
	ID3D11Texture2D *backBuffer = nullptr;
	bsLog::logMessage("Creating back buffer");
	if (FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
	{
		bsLog::logMessage("Failed to create back buffer", pantheios::SEV_CRITICAL);
		assert(!"Failed to create back buffer");
	}

	//D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	bsLog::logMessage("Creating render target view");
	if (FAILED(mDevice->CreateRenderTargetView(backBuffer, nullptr, &mRenderTargetView)))
	{
		bsLog::logMessage("Failed to create render target view", pantheios::SEV_CRITICAL);
		assert(!"Failed to create render target view");
	}

	backBuffer->Release();

	//Depth stencil
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
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
		bsLog::logMessage("Failed to create depth stencil texture", pantheios::SEV_CRITICAL);
		assert(!"Failed to create depth stencil texture");
	}

	//Depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	ZeroMemory(&depthViewDesc, sizeof(depthViewDesc));
	depthViewDesc.Format = depthDesc.Format;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthViewDesc.Texture2D.MipSlice = 0;

	bsLog::logMessage("Creating depth stencil view");
	if (FAILED(mDevice->CreateDepthStencilView(mDepthStencil, &depthViewDesc,
		&mDepthStencilView)))
	{
		bsLog::logMessage("Failed to create depth stencil view", pantheios::SEV_CRITICAL);
		assert(!"Failed to create depth stencil view");
	}

	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	//Viewport
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)renderWindowWidth;
	viewport.Height = (float)renderWindowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	mDeviceContext->RSSetViewports(1, &viewport);

#ifdef _DEBUG
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

	debugString = "ID3D11RenderTargetView";
	mRenderTargetView->SetPrivateData(WKPDID_D3DDebugObjectName, debugString.size(),
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

	if (mRenderTargetView)
	{
		mRenderTargetView->Release();
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

void bsDx11Renderer::preRender()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	mDeviceContext->ClearRenderTargetView(mRenderTargetView, ClearColor);
	
	//Clear depth buffer to max (1.0)
	mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void bsDx11Renderer::present()
{
	if (FAILED(mSwapChain->Present(1, 0)))
	{
		bsLog::logMessage("IDXGISwapChain::Present failed", pantheios::SEV_ERROR);

		assert(!"IDXGISwapChain::Present failed");
	}
}

HRESULT bsDx11Renderer::compileShader(const char* fileName, const char* entryPoint,
	const char* shaderModel, ID3DBlob** blobOut)
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

			assert(!"Failed to compile shader");
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

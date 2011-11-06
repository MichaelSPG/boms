#pragma once

#include <Windows.h>

#include <d3d11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>

#include <vector>

class bsRenderTarget;


class bsDx11Renderer
{
public:
	bsDx11Renderer(HWND hWnd, unsigned int renderWindowWidth, unsigned int renderWindowHeight);

	~bsDx11Renderer();


	void resizeWindow(HWND hWnd, unsigned int windowWidth, unsigned int windowHeight);

	void setFullscreenMode(bool enableFullscreen);

	bool isFullscreenEnabled() const
	{
		return mIsFullscreenEnabled;
	}

	//Presents drawn primitives, waits for v-sync if enabled.
	void present() const;

	inline ID3D11DeviceContext* getDeviceContext() const
	{
		return mDeviceContext;
	}

	inline ID3D11Device* getDevice() const
	{
		return mDevice;
	}

	HRESULT compileShader(const char* fileName, const char* entryPoint,
		const char* shaderModel, ID3DBlob** blobOut) const;

	inline void setVsyncEnabled(bool enabled)
	{
		mVsyncEnabled = enabled;
	}

	inline bool getVsyncEnabled() const
	{
		return mVsyncEnabled;
	}

	//This color is used to clear render targets and the back buffer.
	void setRenderTargetClearColor(float* colorRgba);

	/*	Sets an array of render targets to be used as current render targets.
		Can be used to unbind by setting renderTargets to null and renderTargetCount to
		the amount of render targets to unbind.
	*/
	void setRenderTargets(bsRenderTarget** renderTargets, unsigned int renderTargetCount);

	//Sets the back buffer as the active render target.
	void setBackBufferAsRenderTarget();

	//Clears render targets, making it possible to draw on them again.
	//Use setRenderTargetClearColor to change the color used to clear the render targets.
	void clearRenderTargets(bsRenderTarget** renderTargets, unsigned int count);

	void clearBackBuffer();

	void registerRenderTarget(bsRenderTarget& renderTarget);

	void unregisterRenderTarget(bsRenderTarget& renderTarget);

	/*	Add a listener which will be called whenever the screen is resized.
		Parameters to callback function are new window width and height, in that order.
	*/
	void addResizeListener(const std::function<void(unsigned int, unsigned int)>& callback);


private:
	bool createRenderWindow(HWND hWnd, unsigned int renderWindowWidth, unsigned int renderWindowHeight);

	void destroyCurrentRenderWindow();

	bool createDeviceAndSwapChain(HWND hWnd, unsigned int windowWidth, unsigned int windowHeight);

	bool createBackBufferAndDepthStencil(unsigned int windowWidth, unsigned int windowHeight);

	void createViewport(unsigned int windowWidth, unsigned int windowHeight);


	IDXGISwapChain*			mSwapChain;
	ID3D11Device*			mDevice;
	ID3D11DeviceContext*	mDeviceContext;
	ID3D11RenderTargetView*	mBackBufferRenderTargetView;

	ID3D11Texture2D*		mDepthStencil;
	ID3D11DepthStencilView*	mDepthStencilView;

	ID3D11Buffer*	mScreenSizeBuffer;

	bool	mVsyncEnabled;
	bool	mIsFullscreenEnabled;

	float	mRenderTargetClearColor[4];

	std::vector<bsRenderTarget*>	mRenderTargets;

	std::vector<std::function<void(unsigned int, unsigned int)>> mResizeListeners;
};

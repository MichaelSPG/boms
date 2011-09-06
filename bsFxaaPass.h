#pragma once

#include <memory>

#include <xnamath.h>

#include "bsFullScreenQuad.h"
#include "bsDx11Renderer.h"

class bsPixelShader;
class bsVertexShader;
class bsShaderManager;


class bsFxaaPass
{
public:
	bsFxaaPass(bsShaderManager* shaderManager, bsDx11Renderer* dx11Renderer,
		float screenWidth, float screenHeight);

	~bsFxaaPass();

	void draw();

	
	inline void setEnabled(bool enabled)
	{
		mEnabled = enabled;
	}

	inline bool isEnabled() const
	{
		return mEnabled;
	}


private:
	ID3D11DeviceContext*	mDeviceContext;
	bsShaderManager*		mShaderManager;

	bsFullScreenQuad	mFullscreenQuad;

	std::shared_ptr<bsVertexShader>	mVertexShader;
	std::shared_ptr<bsPixelShader>	mFxaaPixelShader;
	std::shared_ptr<bsPixelShader>	mPassthroughPixelShader;

	ID3D11Buffer*	mOneOverScreenSizeBuffer;
	XMFLOAT2		mOneOverScreenSize;

	bool	mEnabled;
};

#pragma once

#include <memory>

#include "bsFullScreenQuad.h"
#include "bsDx11Renderer.h"

class bsPixelShader;
class bsVertexShader;
class bsShaderManager;
struct ID3D11DeviceContext;


class bsFxaaPass
{
public:
	bsFxaaPass(bsShaderManager* shaderManager, bsDx11Renderer* dx11Renderer);


	void draw();


private:
	ID3D11DeviceContext*	mDeviceContext;
	bsShaderManager*		mShaderManager;

	bsFullScreenQuad	mFullscreenQuad;

	std::shared_ptr<bsVertexShader>	mVertexShader;
	std::shared_ptr<bsPixelShader>	mFxaaPixelShader;
	std::shared_ptr<bsPixelShader>	mPassthroughPixelShader;
};

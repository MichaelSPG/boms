#ifndef BS_FULLSCREENQUAD_H
#define BS_FULLSCREENQUAD_H

#include "bsConfig.h"

#include <d3d11.h>


class bsFullScreenQuad
{
	friend class bsRenderQueue;
public:
	bsFullScreenQuad(ID3D11Device* const device);

	~bsFullScreenQuad();

	void draw(ID3D11DeviceContext* const deviceContext) const;

private:
	

	ID3D11Buffer*	mVertexBuffer;
	ID3D11Buffer*	mIndexBuffer;

	ID3D11SamplerState*	mSamplerState;
};

#endif // BS_FULLSCREENQUAD_H
#pragma once

#include "bsConfig.h"

#include <d3d11.h>


/*	Used for drawing full screen quads (two triangles with coordinates between
	-1,-1-1 and	1,1,1).
	
*/
class bsFullScreenQuad
{
	friend class bsRenderQueue;
public:
	bsFullScreenQuad(ID3D11Device* const device);

	~bsFullScreenQuad();

	/*	Draws the full screen quad.
		Remember to set an appropriate shader before calling this.
	*/
	void draw(ID3D11DeviceContext* const deviceContext) const;

private:
	ID3D11Buffer*	mVertexBuffer;
	ID3D11Buffer*	mIndexBuffer;

	ID3D11SamplerState*	mSamplerState;
};

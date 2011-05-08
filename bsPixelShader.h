#ifndef BS_PIXEL_SHADER_H
#define BS_PIXEL_SHADER_H

#include "bsConfig.h"

#include <d3d11.h>

struct ID3D11PixelShader;


class bsPixelShader
{
	friend class bsShaderManager;

public:
	bsPixelShader(ID3D11PixelShader* pixelShader, unsigned int id)
		: mPixelShader(pixelShader)
		, mID(id)
	{
	}

	~bsPixelShader()
	{
		mPixelShader->Release();
	}

	inline ID3D11PixelShader* getPixelShader() const
	{
		return mPixelShader;
	}
	
private:
	//Not copyable
	bsPixelShader(const bsPixelShader&);
	void operator=(const bsPixelShader&);

	ID3D11PixelShader*	mPixelShader;
	unsigned int		mID;
};

#endif // BS_PIXEL_SHADER_H

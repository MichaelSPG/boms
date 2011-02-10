#ifndef PIXEL_SHADER_H
#define PIXEL_SHADER_H

struct ID3D11PixelShader;


class PixelShader
{
	friend class ShaderManager;

public:
	PixelShader(ID3D11PixelShader* pixelShader)
		: mPixelShader(pixelShader)
	{
	}

	~PixelShader()
	{
	}

	inline ID3D11PixelShader* getPixelShader() const
	{
		return mPixelShader;
	}
	
private:
	inline void setPixelShader(ID3D11PixelShader* pixelShader)
	{
		mPixelShader = pixelShader;
	}

	ID3D11PixelShader* mPixelShader;
};

#endif // PIXEL_SHADER_H
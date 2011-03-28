#ifndef PIXEL_SHADER_H
#define PIXEL_SHADER_H

struct ID3D11PixelShader;


class PixelShader
{
	friend class ShaderManager;

public:
	PixelShader(ID3D11PixelShader* pixelShader, unsigned int id)
		: mPixelShader(pixelShader)
		, mID(id)
	{
	}

	~PixelShader()
	{
		mPixelShader->Release();
	}

	inline ID3D11PixelShader* getPixelShader() const
	{
		return mPixelShader;
	}
	
private:
	//Not copyable
	PixelShader(const PixelShader&);
	void operator=(const PixelShader&);

	ID3D11PixelShader*	mPixelShader;
	unsigned int		mID;
};

#endif // PIXEL_SHADER_H
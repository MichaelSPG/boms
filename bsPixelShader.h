#ifndef PIXEL_SHADER_H
#define PIXEL_SHADER_H

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

#endif // PIXEL_SHADER_H
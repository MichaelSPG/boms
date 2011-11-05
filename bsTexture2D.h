#pragma once

#include "D3D11.h"


class bsTexture2D
{
public:
	bsTexture2D(ID3D11ShaderResourceView* texture, ID3D11Device& device, unsigned int id);

	~bsTexture2D();

	void apply(ID3D11DeviceContext& deviceContext);

	inline ID3D11ShaderResourceView* getShaderResourceView() const
	{
		return mShaderResourceView;
	}

	inline unsigned int getID() const
	{
		return mID;
	}


	/*	Called when asynchronous loading of this texture has finished.
	*/
	void loadingCompleted(ID3D11ShaderResourceView* texture, bool success);


private:
	ID3D11ShaderResourceView*	mShaderResourceView;
	ID3D11SamplerState*			mSamplerState;

	//0 if loading has not completed, positive otherwise.
	volatile unsigned int		mLoadingCompleted;
	
	//Unique ID.
	unsigned int				mID;
};

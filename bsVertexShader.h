#pragma once

#include "bsConfig.h"

#include <d3d11.h>

struct ID3D11VertexShader;
struct ID3D11InputLayout;


/*	Contains a single vertex shader and its input layout.
	Use bsShaderManager to create shaders.
*/
class bsVertexShader
{
	friend class bsShaderManager;

public:
	bsVertexShader(ID3D11VertexShader* vertexShader, ID3D11InputLayout* inputLayout,
		unsigned int id)
		: mVertexShader(vertexShader)
		, mInputLayout(inputLayout)
		, mID(id)
	{
	}

	~bsVertexShader()
	{
		mVertexShader->Release();
		mInputLayout->Release();
	}

	inline ID3D11VertexShader* getD3dVertexShader() const
	{
		return mVertexShader;
	}

	inline ID3D11InputLayout* getInputLayout() const
	{
		return mInputLayout;
	}
	
private:
	//Not copyable
	bsVertexShader(const bsVertexShader&);
	void operator=(const bsVertexShader&);

	ID3D11VertexShader*	mVertexShader;
	ID3D11InputLayout*	mInputLayout;
	unsigned int		mID;
	std::vector<D3D11_INPUT_ELEMENT_DESC>	mInputLayoutDescriptions;
};

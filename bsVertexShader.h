#ifndef BS_VERTEX_SHADER_H
#define BS_VERTEX_SHADER_H

struct ID3D11VertexShader;
struct ID3D11InputLayout;


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

	inline ID3D11VertexShader* getVertexShader() const
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

#endif // BS_VERTEX_SHADER_H

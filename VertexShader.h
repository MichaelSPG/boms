#ifndef VERTEX_SHADER_H
#define VERTEX_SHADER_H

struct ID3D11VertexShader;
struct ID3D11InputLayout;


class VertexShader
{
	friend class ShaderManager;

public:
	VertexShader(ID3D11VertexShader* vertexShader, ID3D11InputLayout* inputLayout,
		unsigned int id)
		: mVertexShader(vertexShader)
		, mInputLayout(inputLayout)
		, mID(id)
	{
	}

	~VertexShader()
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
	VertexShader(const VertexShader&);
	void operator=(const VertexShader&);

	ID3D11VertexShader*	mVertexShader;
	ID3D11InputLayout*	mInputLayout;
	unsigned int		mID;
	std::vector<D3D11_INPUT_ELEMENT_DESC>	mInputLayoutDescriptions;
};

#endif // VERTEX_SHADER_H
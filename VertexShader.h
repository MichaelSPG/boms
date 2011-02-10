#ifndef VERTEX_SHADER_H
#define VERTEX_SHADER_H

struct ID3D11VertexShader;
struct ID3D11InputLayout;


class VertexShader
{
	friend class ShaderManager;

public:
	VertexShader(ID3D11VertexShader* vertexShader, ID3D11InputLayout* inputLayout)
		: mVertexShader(vertexShader)
		, mInputLayout(inputLayout)
	{
	}
	~VertexShader()
	{
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
	inline void setVertexShader(ID3D11VertexShader* vertexShader)
	{
		mVertexShader = vertexShader;
	}

	inline void setInputLayout(ID3D11InputLayout* inputLayout)
	{
		mInputLayout = inputLayout;
	}

	ID3D11VertexShader*	mVertexShader;
	ID3D11InputLayout*	mInputLayout;
};

#endif // VERTEX_SHADER_H
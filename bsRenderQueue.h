#ifndef BS_RENDERQUEUE_H
#define BS_RENDERQUEUE_H

#include <vector>
#include <memory>
#include <sstream>
#include <string>

#include <D3DX11.h>
#include <Windows.h>
#include <xnamath.h>

class bsSceneNode;
class bsRenderable;
class bsMesh;
class bsPrimitive;
class bsDx11Renderer;
class bsShaderManager;
class bsVertexShader;
class bsPixelShader;
class bsCamera;
struct ID3D11DeviceContext;
struct ID3D11Buffer;


struct bsFrameStats
{
	bsFrameStats()
	{
		reset();
	}

	//Sets everything to 0.
	inline void reset()
	{
		memset(this, 0, sizeof(*this));
	}

	inline std::string getFrameStatsString() const
	{
		std::stringstream ss;
		ss.setf(std::ios::floatfield, std::ios::fixed);
		ss.precision(3);
		ss << "Time taken: " << timeTakenMs << " ms";
		ss.precision(0);
		ss  << "\nVisible scene nodes: " << visibleSceneNodeCount
			<< "\nUnique meshes drawn: " << uniqueMeshesDrawn
			<< "\nTotal meshes drawn: " << totalMeshesDrawn
			<< "\nTotal lines drawn: " << linesDrawn;

		return ss.str();
	}

	inline std::wstring getFrameStatsWideString() const
	{
		std::wstringstream ss;
		ss.setf(std::ios::floatfield, std::ios::fixed);
		ss.precision(3);
		ss << L"Time taken: " << timeTakenMs << L" ms";
		ss.precision(0);
		ss  << L"\nVisible scene nodes: " << visibleSceneNodeCount
			<< L"\nUnique meshes drawn: " << uniqueMeshesDrawn
			<< L"\nTotal meshes drawn: " << totalMeshesDrawn
			<< L"\nTotal lines drawn: " << linesDrawn;

		return ss.str();
	}


	float	timeTakenMs;

	unsigned int	visibleSceneNodeCount;
	unsigned int	uniqueMeshesDrawn;
	unsigned int	totalMeshesDrawn;
	unsigned int	linesDrawn;
};

class bsRenderQueue
{
public:
	bsRenderQueue(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager);
	~bsRenderQueue();

	//Clears all current collections of renderables.
	void reset();

	void draw();

	inline void setCamera(bsCamera* camera)
	{
		mCamera = camera;
	}

	inline const bsFrameStats& getFrameStats() const
	{
		return mFrameStats;
	}

private:
	void sortSceneNodes();

	void setWorldConstantBuffer(const XMFLOAT4X4& world);

	void setWireframeConstantBuffer(const XMFLOAT4X4& world, const XMFLOAT4& color);

	//Unbinds only.
	void unbindGeometryShader();

	bsCamera*		mCamera;
	
	bsDx11Renderer*		mDx11Renderer;
	bsShaderManager*	mShaderManager;
	ID3D11Buffer*		mWorldBuffer;
	ID3D11Buffer*		mWireframeWorldBuffer;

	std::shared_ptr<bsPixelShader>	mWireframePixelShader;
	std::shared_ptr<bsVertexShader>	mWireframeVertexShader;

	std::shared_ptr<bsPixelShader>	mMeshPixelShader;
	std::shared_ptr<bsVertexShader>	mMeshVertexShader;

	bsFrameStats		mFrameStats;
};

#endif // BS_RENDERQUEUE_H

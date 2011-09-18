#pragma once


#include <vector>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

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
class bsLine3D;
class bsLight;
struct CBLight;

struct ID3D11DeviceContext;
struct ID3D11Buffer;

/*	Contains stats about a frame.
	Data can be taken out manually, or a string can be created following a
	predefined pattern.
*/
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
		ss << "Rendering time taken: " << timeTakenMs << " ms";
		ss.precision(0);
		ss  << "\nVisible scene nodes: " << visibleSceneNodeCount
			<< "\nUnique meshes drawn: " << uniqueMeshesDrawn
			<< "\nTotal meshes drawn: " << totalMeshesDrawn
			<< "\nTotal lines drawn: " << linesDrawn
			<< "\nVisible lights: " << visibleLights;

		return ss.str();
	}

	inline std::wstring getFrameStatsStringWide() const
	{
		std::wstringstream ss;
		ss.setf(std::ios::floatfield, std::ios::fixed);
		ss.precision(3);
		ss << L"Rendering time taken: " << timeTakenMs << L" ms";
		ss.precision(0);
		ss  << L"\nVisible scene nodes: " << visibleSceneNodeCount
			<< L"\nUnique meshes drawn: " << uniqueMeshesDrawn
			<< L"\nTotal meshes drawn: " << totalMeshesDrawn
			<< L"\nTotal lines drawn: " << linesDrawn
			<< L"\nVisible lights: " << visibleLights;

		return ss.str();
	}


	float	timeTakenMs;

	unsigned int	visibleSceneNodeCount;
	unsigned int	uniqueMeshesDrawn;
	unsigned int	totalMeshesDrawn;
	unsigned int	linesDrawn;
	unsigned int	visibleLights;
};


/*	The render queue is responsible for drawing everything to the screen.
	Organizes various groups of renderable objects to make it easy to draw individual
	types of renderables.
*/
class bsRenderQueue
{
public:
	bsRenderQueue(bsDx11Renderer* dx11Renderer, bsShaderManager* shaderManager);

	~bsRenderQueue();

	//Clears all current collections of renderables.
	void reset();

	//Draws the geometry
	void drawGeometry();

	void drawLines();

	void drawLights();

	/*	Sets the active camera.
		This camera is used to generate a list of scene nodes that are visible per frame,
		i.e. ones that are inside the frustum.
	*/
	inline void setCamera(bsCamera* camera)
	{
		mCamera = camera;
	}

	/*	Gets the current frame stats.
		If called between the start and end of all the draw functions,
		the stats may be incomplete.
	*/
	inline const bsFrameStats& getFrameStats() const
	{
		return mFrameStats;
	}

private:
	/*	Gets the renderables from the scene nodes and groups them based on what kind of
		renderable they are
	*/
	void sortRenderables();

	void sortLights();

	//Functions to draw individual renderable types.
	void drawMeshes();

	void setWorldConstantBuffer(const XMMATRIX& world);

	void setWireframeConstantBuffer(const XMMATRIX& world, const XMFLOAT4& color);

	void setLightConstantBuffer(const CBLight& cbLight);

	void unbindGeometryShader();

	bsCamera*		mCamera;
	
	bsDx11Renderer*		mDx11Renderer;
	bsShaderManager*	mShaderManager;
	ID3D11Buffer*		mWorldBuffer;
	ID3D11Buffer*		mWireframeWorldBuffer;
	ID3D11Buffer*		mLightBuffer;

	std::shared_ptr<bsPixelShader>	mWireframePixelShader;
	std::shared_ptr<bsVertexShader>	mWireframeVertexShader;

	std::shared_ptr<bsPixelShader>	mMeshPixelShader;
	std::shared_ptr<bsVertexShader>	mMeshVertexShader;

	std::shared_ptr<bsPixelShader>	mLightPixelShader;
	std::shared_ptr<bsVertexShader>	mLightVertexShader;

	bsFrameStats		mFrameStats;

	std::unordered_map<bsMesh*, std::vector<bsSceneNode*>>		mMeshesToDraw;
	std::unordered_map<bsLine3D*, std::vector<bsSceneNode*>>	mLinesToDraw;
	std::vector<std::pair<bsLight*, XMFLOAT3>>	mLightPositionPairs;
};

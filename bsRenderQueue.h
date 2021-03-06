#pragma once


#include <vector>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include <D3DX11.h>
#include <Windows.h>
#include <xnamath.h>

class bsEntity;
class bsRenderable;
class bsMeshRenderer;
class bsPrimitive;
class bsDx11Renderer;
class bsShaderManager;
class bsVertexShader;
class bsPixelShader;
class bsCamera;
class bsLineRenderer;
class bsLight;
class bsText3D;
class bsScene;
struct CBLight;
struct bsFrustum;

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
		ss.imbue(std::locale(""));

		ss  << "\nVisible entities: " << visibleEntityCount
			<< "\nUnique meshes drawn: " << uniqueMeshesDrawn
			<< "\nTotal meshes drawn: " << totalMeshesDrawn
			<< "\nTotal lines drawn: " << linesDrawn
			<< "\nVisible lights: " << visibleLights
			<< "\nTris (w/instanced): " << totalTrianglesDrawn
			<< '(' << totalTrianglesDrawnNotInstanced << ')';

		return ss.str();
	}

	inline std::wstring getFrameStatsStringWide() const
	{
		std::wstringstream ss;
		ss.setf(std::ios::floatfield, std::ios::fixed);
		ss.imbue(std::locale(""));

		ss  << L"\nVisible entities: " << visibleEntityCount
			<< L"\nUnique meshes drawn: " << uniqueMeshesDrawn
			<< L"\nTotal meshes drawn: " << totalMeshesDrawn
			<< L"\nTotal lines drawn: " << linesDrawn
			<< L"\nVisible lights: " << visibleLights
			<< L"\nTris (w/instanced): " << totalTrianglesDrawn
			<< L'(' << totalTrianglesDrawnNotInstanced << L')';

		return ss.str();
	}


	unsigned int	visibleEntityCount;
	unsigned int	uniqueMeshesDrawn;
	unsigned int	totalMeshesDrawn;
	unsigned int	linesDrawn;
	unsigned int	visibleLights;

	unsigned int	totalTrianglesDrawn;
	unsigned int	totalTrianglesDrawnNotInstanced;
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

	void startFrame();

	//Draws the geometry
	void drawGeometry();

	void drawLines();

	void drawLights();

	void drawTexts();

	/*	Sets the active camera.
		This camera is used to generate a list of entities that are visible per frame,
		i.e. ones that are inside the frustum.
	*/
	inline void setCamera(bsCamera* camera)
	{
		mCamera = camera;
	}

	/*	Register a scene with the render queue.
		This results in the registered scene being drawn on the screen.
		Only one scene can be registered at a time.
	*/
	inline void registerScene(const bsScene& scene)
	{
		mScene = &scene;
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
	/*	Add a batch of entities (or all entities) and cull the ones that are not visible
		to the specified frustum.
	*/
	void addAndCullObjects(const bsEntity** entities, unsigned int entityCount,
		const bsFrustum& frustum);

	/*	Gets the renderables from the entities and groups them based on what kind of
		renderable they are
	*/
	void sortRenderables(const bsEntity** entities, unsigned int entityCount);

	void sortLights();

	//Functions to draw individual renderable types.
	void drawMeshesInstanced();

	void drawMeshInstanced(const bsMeshRenderer& meshRenderer, const XMMATRIX* transforms,
		unsigned int transformCount);

	void drawPointLights();

	void drawSpotLights();


	void setWorldConstantBuffer(const XMMATRIX& world);

	void setWireframeConstantBuffer(const XMMATRIX& world, const XMFLOAT4& color);

	void setLightConstantBuffer(const CBLight& cbLight);

	void unbindGeometryShader();

	bsCamera*		mCamera;

	//The currently registered scene which will be rendered.
	const bsScene* mScene;
	
	bsDx11Renderer*		mDx11Renderer;
	bsShaderManager*	mShaderManager;
	ID3D11Buffer*		mWorldBuffer;
	ID3D11Buffer*		mWireframeWorldBuffer;
	ID3D11Buffer*		mLightBuffer;
	ID3D11Buffer*		mMaterialBuffer;

	ID3D11SamplerState*	mLightSamplerState;


	std::shared_ptr<bsPixelShader>	mWireframePixelShader;
	std::shared_ptr<bsVertexShader>	mWireframeVertexShader;

	std::shared_ptr<bsVertexShader>	mMeshInstancedVertexShader;

	std::shared_ptr<bsPixelShader>	mInstancedTexturedMeshPixelShader;
	std::shared_ptr<bsPixelShader>	mInstancedTexturedMeshNormalPixelShader;

	std::shared_ptr<bsPixelShader>	mLightPixelShader;
	std::shared_ptr<bsVertexShader>	mLightVertexShader;

	std::shared_ptr<bsVertexShader>	mLightInstancedVertexShader;
	std::shared_ptr<bsPixelShader>	mPointLightInstancedPixelShader;
	std::shared_ptr<bsPixelShader>	mSpotLightInstancedPixelShader;

	bsFrameStats		mFrameStats;

	struct MeshRendererHasher
	{
		size_t operator()(const bsMeshRenderer* meshRenderer) const;
	};

	typedef std::unordered_map<const bsMeshRenderer*, std::vector<const bsEntity*>,
		MeshRendererHasher> MeshRendererMap;

	MeshRendererMap mMeshesToDraw;
	//std::unordered_map<const bsMeshRenderer*, std::vector<const bsEntity*>>		mMeshesToDraw;
	std::unordered_map<const bsLineRenderer*, std::vector<const bsEntity*>>	mLinesToDraw;
	std::vector<std::pair<const bsLight*, XMFLOAT3>>	mPointLightPositionPairs;
	std::vector<std::pair<const bsLight*, const bsEntity*>>	mSpotLightPositionPairs;
	std::vector<std::pair<const bsLight*, XMFLOAT3>>	mDirectionalLightPositionPairs;


	std::vector<std::pair<const bsEntity*, const bsText3D*>> mText3dToDraw;
};

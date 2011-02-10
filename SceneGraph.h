#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vector>
#include <memory>


class ShaderManager;
class OctNode;
class Camera;
class Dx11Renderer;


class SceneGraph
{
public:
	SceneGraph();
	~SceneGraph();

	void init(unsigned short treeDepth, Dx11Renderer* renderer, ShaderManager* shadermanager);
	
	/**	Returns the amount of created objects and increments it by 1.
		Used to assign unique IDs to objects.
	*/
	inline int getNumCreatedObjects()
	{
		return mNumCreatedObjects++;
	}

	/**	Returns the amount of created objects without incrementing the value.
		
	*/
	inline int getNumCreatedObjectsNoIncrement() const
	{
		return mNumCreatedObjects;
	}

	inline Dx11Renderer* getRenderer()
	{
		return mRenderer;
	}

	void drawAABBs(Dx11Renderer* dx11Renderer) const;

private:
	std::vector<std::shared_ptr<OctNode>>	mNodes;
	std::shared_ptr<OctNode> mRootNode;

	int				mNumCreatedObjects;
	Dx11Renderer*	mRenderer;
	Camera*			mCamera;
};

#endif // SCENE_GRAPH_H
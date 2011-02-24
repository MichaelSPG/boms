#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vector>
#include <memory>

#include "Dx11Renderer.h"
#include "Convert.h"

class ShaderManager;
class OctNode;
class SceneNode;
class Camera;
class Dx11Renderer;


class SceneGraph
{
	friend class Node;
	friend class SceneNode;
	friend class OctNode;

public:
	SceneGraph();
	~SceneGraph();

	void init(unsigned short treeDepth, Dx11Renderer* renderer, ShaderManager* shaderManager);

	SceneNode* createSceneNode(const hkVector4& position = hkVector4(0.0f, 0.0f, 0.0f, 0.0f));

	//Puts a scene node in the oct node it belongs in (the smallest one that contains it).
	//Leave depth at 1 if calling from an external class.
	void placeSceneNode(SceneNode* sceneNode, OctNode* octNode, unsigned short depth = 1);
	
	/**	Increments the amount of created objects and returns it.
		Used to assign unique IDs to objects.
	*/
	inline int getNumCreatedObjects()
	{
		return ++mNumCreatedObjects;
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


	bool displayEmptyAabbs;

private:
	OctNode*	mRootNode;

	std::vector<SceneNode*>	mSceneNodes;
	std::vector<OctNode*>	mOctNodes;

	int				mNumCreatedObjects;
	Dx11Renderer*	mRenderer;
	Camera*			mCamera;
	unsigned short	mMaxTreeDepth;
};

#endif // SCENE_GRAPH_H
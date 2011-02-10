#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "ShaderManager.h"

class Dx11Renderer;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void initShaderManager(Dx11Renderer* dx11Renderer);

	inline ShaderManager* getShaderManager() const
	{
		return mShaderManager;
	}
	
private:
	ShaderManager	*mShaderManager;
};

#endif // RESOURCE_MANAGER_H
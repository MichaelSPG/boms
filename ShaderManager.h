#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <map>
#include <memory>
#include <string>

#include "Dx11Renderer.h"

#include "VertexShader.h"
#include "PixelShader.h"
//class VertexShader;
//class PixelShader;


class ShaderManager
{
	friend class ResourceManager;

	ShaderManager(Dx11Renderer* dx11Renderer);
	~ShaderManager();

public:
	const std::shared_ptr<VertexShader>& getVertexShader(const std::string& fileName);

	const std::shared_ptr<PixelShader>& getPixelShader(const std::string& fileName);
	
private:
	std::shared_ptr<VertexShader>& createVertexShader(const std::string& fileName);

	std::shared_ptr<PixelShader>& createPixelShader(const std::string& fileName);

	std::map<std::string, std::shared_ptr<VertexShader>> mVertexShaders;
	std::map<std::string, std::shared_ptr<PixelShader>> mPixelShaders;

	Dx11Renderer* mDx11Renderer;
};

#endif // SHADER_MANAGER_H
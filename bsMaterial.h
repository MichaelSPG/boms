#pragma once

#include <memory>

#include <xnamath.h>

#include "bsPixelShader.h"
#include "bsVertexShader.h"
#include "bsTexture2D.h"

class bsTexture2D;
struct ID3D11SamplerState;


/*	Create with bsMaterialCache.
	
*/
struct bsMaterial
{
	explicit bsMaterial(unsigned int ID)
		: diffuse(nullptr)
		, normal(nullptr)
		, pixelShader(nullptr)
		, vertexShader(nullptr)
		, uvTile(1.0f, 1.0f)
		, ID(ID)
	{}

	bool operator==(const bsMaterial& other) const
	{
		return ID == other.ID;
	}

	//Diffuse texture.
	std::shared_ptr<bsTexture2D> diffuse;
	//Normal map.
	std::shared_ptr<bsTexture2D> normal;

	std::shared_ptr<bsPixelShader> pixelShader;
	std::shared_ptr<bsVertexShader> vertexShader;

	XMFLOAT2 uvTile;

	const unsigned int ID;
};

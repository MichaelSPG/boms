#include "StdAfx.h"

#include "bsMaterialCache.h"
#include "bsMaterial.h"
#include "bsAssert.h"
#include "bsLog.h"


bsMaterialCache::bsMaterialCache()
	: mNumCreatedMaterials(0)
{
	
}

bsMaterialCache::~bsMaterialCache()
{
#ifdef BS_DEBUG
	//Warn if there are external references to any materials.
	for (auto itr = mMaterials.begin(), end = mMaterials.end(); itr != end; ++itr)
	{
		if (!itr->second.unique())
		{
			bsLog::logf(bsLog::SEV_WARNING, "All references to texture '%s' have not"
				" been released when bsTextureCache is being destroyed (%u external refs)",
				itr->first.c_str(), itr->second.use_count() - 1);
		}
	}
#endif //ifdef BS_DEBUG
}

std::shared_ptr<bsMaterial> bsMaterialCache::getMaterial(const std::string& materialName)
{
	auto itr = mMaterials.find(materialName);
	if (itr != std::end(mMaterials))
	{
		//Found it.
		return itr->second;
	}

	//Didn't find it, log an error message.
	bsLog::logf(bsLog::SEV_ERROR, "The requested material '%s' does not exist",
		materialName.c_str());
	
	BS_ASSERT2(false, "Failed to find material");

	return nullptr;
}

std::shared_ptr<bsMaterial> bsMaterialCache::createNewMaterial(const std::string& materialName)
{
	auto itr = mMaterials.find(materialName);
	if (itr != std::end(mMaterials))
	{
		//A material with the provided name already exists, return null instead of
		//overwriting it.

		bsLog::logf(bsLog::SEV_WARNING, "bsMaterialCache::createNewMaterial called with"
			" name '%s', which already existed in the cache.", materialName.c_str());

		return nullptr;
	}

	//Material does not exist, create it.

	std::shared_ptr<bsMaterial> material(std::make_shared<bsMaterial>(getNewMaterialID()));

	mMaterials.insert(std::make_pair(materialName, material));

	return material;
}

#pragma once

#include <unordered_map>
#include <string>
#include <memory>

struct bsMaterial;


class bsMaterialCache
{
public:
	bsMaterialCache();

	~bsMaterialCache();

	/*	Returns the material with the provided name, or null if a material with that
		name does not exist.
	*/
	std::shared_ptr<bsMaterial> getMaterial(const std::string& materialName);

	/*	Creates a material with the provided name.
		If a material with the provided name already exists, null with be returned.
	*/
	std::shared_ptr<bsMaterial> createNewMaterial(const std::string& materialName);


private:
	//Non-copyable.
	bsMaterialCache(const bsMaterialCache&);
	bsMaterialCache& operator=(const bsMaterialCache&);

	unsigned int getNewMaterialID()
	{
		return ++mNumCreatedMaterials;
	}

	std::unordered_map<std::string, std::shared_ptr<bsMaterial>> mMaterials;

	unsigned int		mNumCreatedMaterials;
};

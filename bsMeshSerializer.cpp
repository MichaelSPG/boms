#include "bsMeshSerializer.h"

#include <fstream>

#include "bsLog.h"


bool bsMeshSerializer::load(const std::string& meshName, bsSerializedMesh& mesh)
{
	std::ifstream inStream(meshName, std::ios::binary);
	try
	{
		//Load it
		boost::archive::binary_iarchive inArchive(inStream);
		inArchive >> mesh;
	}
	catch (const std::exception& e)
	{
		//Uh oh
		std::string message("bsMeshSerializer::load: An exception occured while loading '");
		message += meshName + "'. Message: " + e.what() + "\nMaybe you tried to load an "
			"unsupported file (only .bsm is supported)";
		bsLog::logMessage(message.c_str(), pantheios::SEV_ERROR);

		inStream.close();
		
		return false;
	}
	inStream.close();

	return true;
}

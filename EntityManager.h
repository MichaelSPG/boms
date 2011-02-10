#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <vector>
#include <algorithm>
#include "Entity.h"

class EntityManager
{
public:
	EntityManager();
	~EntityManager();

	void render();
	
private:
	std::vector<Entity*>	mEntityVector;
};

#endif // ENTITY_MANAGER_H
#include "EntityManager.h"

EntityManager::EntityManager()
{

}

EntityManager::~EntityManager()
{

}

void EntityManager::render()
{
	
	//Sletting av flere objekter
	/*
	#include <algorithm>
	mEntityVector.erase(remove_if(mEntityVector.begin(), mEntityVector.end(), 
		[](int e) {return e % 2;}), mEntityVector.end());
		//Funksjon som returnerer true om objekt skal slettes
	*/




	
	std::for_each(mEntityVector.cbegin(), mEntityVector.cend(),
		[](Entity* ent)
		{
			if (ent->isActive())
				ent->render();
		});
}

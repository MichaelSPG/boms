#pragma once

#include <Physics/Dynamics/Collide/ContactListener/hkpContactListener.h>


/*	Listener which keeps track of the number of contact points in a physics world.
	This listener must be added to the physics world before any contact generating
	objects, or it may report invalid numbers.
*/
class bsContactCounter : public hkpContactListener
{
public:
	bsContactCounter()
		: mNumContacts(0)
	{}


	virtual void collisionAddedCallback(const hkpCollisionEvent& /*event*/)
	{
		++mNumContacts;
	}

	virtual void collisionRemovedCallback(const hkpCollisionEvent& /*event*/)
	{
		--mNumContacts;
	}

	/*	Returns the current number of contact points in the physics world this
		listener is in.
	*/
	inline unsigned int getNumContacts() const
	{
		return mNumContacts;
	}


private:
	unsigned int mNumContacts;
};

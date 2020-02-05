///HEADER

#pragma  once

#include "../External/Includes/glm/glm.hpp"
#include <string>
#include <vector>

class RigidbodyComponent;


struct CollisionContact 
{
	RigidbodyComponent *rbdyA;
	RigidbodyComponent *rbdyB;
	glm::vec3 p0;
	//More stuff
};



class PhysicsManager 
{
public:
	PhysicsManager();
	virtual ~PhysicsManager();

	void PhysicsUpdate(float dt);

	void RegisterRigidbody(RigidbodyComponent* rgbdy);
	std::vector<RigidbodyComponent*> const& GetRigidbodies() const;

private:
	bool CheckCollision(RigidbodyComponent *A, RigidbodyComponent *B);

private:
	std::vector<RigidbodyComponent*> m_rigidbodies;
	float timeAccumulator;

	std::vector<CollisionContact> m_contacts;
};
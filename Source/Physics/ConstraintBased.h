///HEADER STUFF

#pragma once

///INCLUDES
#include <vector>
#include <tuple>
#include "../External/Includes/glm/glm.hpp"

#include "../PhysicsManager.h"


namespace AuxMath
{
	//SETUP ALL THE MATRICES USED FOR THE SOLVER
	void JacobianSetup(std::vector<CollisionContact> const& contacts,
	std::vector<std::vector<LinearAngularPair>>& Jsp,
		std::vector<std::vector<unsigned>>& Jmap);



	//Setup the velocity vectors
	void VelocityVectorSetup(std::vector<RigidbodyComponent*> const& bodies,
		std::vector<LinearAngularPair>& Vprev,
		std::vector<LinearAngularPair>& Vcurr);



	//Setup the velocity vectors
	void VelocityVectorUpdate(std::vector<RigidbodyComponent*> const& bodies,
		std::vector<LinearAngularPair>& Vprev,
		std::vector<LinearAngularPair>& Vcurr);



	//Solving for position constraint
	void PositionConstraintSolver(std::vector<float>& Cp,
		std::vector<CollisionContact> const& contacts);



	//Solving for constraint velocity
	void VelocityConstraintSolver(float dt, 
		std::vector<float>& Cv, std::vector<float>& Cp,
		std::vector<LinearAngularPair> const& Vcurr,
		std::vector <RigidbodyComponent*> const& bodies,
		std::vector<std::vector<LinearAngularPair>> const& Jsp,
		std::vector<std::vector<unsigned>> const& Jmap);


	//Solving for Fc, constraint forces
	void ImpulseSolver(std::vector<LinearAngularPair>& Fc,
		std::vector<float> const& Lambda,
		std::vector <RigidbodyComponent*> const& bodies,
		std::vector<std::vector<LinearAngularPair>> const& Jsp,
		std::vector<std::vector<unsigned>> const& Jmap);



	//Solving for lambda, lagrange multipliers
	void LagrangeMultipliersSolver(float dt,
		std::vector<float>& Lambda,
		std::vector <RigidbodyComponent*> const& bodies,
		std::vector<LinearAngularPair>& Vprev,
		std::vector<LinearAngularPair>& Vcurr,
		std::vector<std::vector<LinearAngularPair>> const& Jsp,
		std::vector<std::vector<unsigned>> const& Jmap,
		std::vector<float> const& Cp,
		std::vector<float> const& Cv);
}
///HEADER STUFF

#include "ConstraintBased.h"
#include <iostream>
#include <assert.h>
#include <limits>

#include "../GameObject.h"
#include "../RigidbodyComponent.h"
#include "../TransformComponent.h"


namespace AuxMath
{
	//SETUP ALL THE MATRICES USED FOR THE SOLVER
	void JacobianSetup(std::vector<CollisionContact> const& contacts,
		std::vector<std::vector<LinearAngularPair>>& Jsp,
		std::vector<std::vector<unsigned>>& Jmap)
	{
		//Loop through all contact manifolds 
		for (int index = 0; index < contacts.size(); ++index)
		{
			CollisionContact const& ct = contacts[index];
			RigidbodyComponent *rb1 = ct.rbdyA;
			RigidbodyComponent *rb2 = ct.rbdyB;

			assert(ct.manifold.ptsA.size() == ct.manifold.ptsB.size());

			//Experiment with normal
			//(normal should go from body 1 to 2, that is to say, A to B)
			glm::vec4 n(0);
			for (int i = 0; i < ct.manifold.ptsA.size(); ++i)
				n += ct.manifold.ptsB[i].world - ct.manifold.ptsA[i].world;
			n.w = 0.0f;
			n = glm::normalize(-n);

			///IMPORTANT!!!
			//For now Im making the normal negative to force it to be pointing away from body 1
			//Should find a better way of getting this normal than depending on the penetration 

			//CONSTRAINTS LOOP-----------------------------------------------------------
			for (int i = 0; i < ct.manifold.ptsA.size(); ++i)
			{
				//Get all info for the jacobian
				glm::vec4 const& p1 = rb1->BodyToWorldContact(ct.manifold.ptsA[i].body);
				glm::vec4 const& p2 = rb2->BodyToWorldContact(ct.manifold.ptsB[i].body);
				glm::vec4 x1 = rb1->GetOwner()->GetComponent<Transform>()->GetPosition();
				glm::vec4 x2 = rb2->GetOwner()->GetComponent<Transform>()->GetPosition();
				glm::vec4 r1 = p1 - x1;
				glm::vec4 r2 = p2 - x2;

				//Jsp filling with the jacobian info
				LinearAngularPair body1 = {};
				body1.linear = -n;
				body1.angular = -AuxMath::Simplex::vec4Cross(r1, n);
				LinearAngularPair body2 = {};
				body2.linear = n;
				body2.angular = AuxMath::Simplex::vec4Cross(r2, n);
				Jsp.push_back({ body1, body2 });
			}
		}
	}



	//Setup the velocity vectors
	void VelocityVectorSetup(std::vector<RigidbodyComponent*> const& bodies,
		std::vector<LinearAngularPair>& Vprev,
		std::vector<LinearAngularPair>& Vcurr)
	{
		for (RigidbodyComponent *body : bodies) 
		{
			LinearAngularPair vpair = {};

			vpair.linear = body->GetVelocity();
			vpair.angular = body->GetAngularVelocity();

			Vprev.push_back(vpair);
			Vcurr.push_back(vpair);
		}
	}



	//Update the velocity vectors
	void VelocityVectorUpdate(std::vector<RigidbodyComponent*> const& bodies,
		std::vector<LinearAngularPair>& Vprev,
		std::vector<LinearAngularPair>& Vcurr) 
	{
		int index = 0;
		for (RigidbodyComponent *body : bodies)
		{
			LinearAngularPair vpair = {};
			vpair.linear = body->GetVelocity();
			vpair.angular = body->GetAngularVelocity();

			Vprev[index] = Vcurr[index];
			Vcurr[index] = vpair;
		}
	}



	//Solving for position constraint
	void PositionConstraintSolver(std::vector<float>& Cp,
		std::vector<CollisionContact> const& contacts)
	{
		//Loop through all contact manifolds 
		for (int index = 0; index < contacts.size(); ++index)
		{
			CollisionContact const& ct = contacts[index];
			RigidbodyComponent *rb1 = ct.rbdyA;
			RigidbodyComponent *rb2 = ct.rbdyB;

			//Experiment with normal
			//(normal should go from body 1 to 2, that is to say, A to B)
			glm::vec4 n(0);
			for (int i = 0; i < ct.manifold.ptsA.size(); ++i)
				n += ct.manifold.ptsB[i].world - ct.manifold.ptsA[i].world;
			n.w = 0.0f;
			n = glm::normalize(-n);

			//CONSTRAINTS LOOP-----------------------------------------------------------
			for (int i = 0; i < ct.manifold.ptsA.size(); ++i)
			{
				glm::vec4 const& p1 = rb1->BodyToWorldContact(ct.manifold.ptsA[i].body);
				glm::vec4 const& p2 = rb2->BodyToWorldContact(ct.manifold.ptsB[i].body);

				float sum = glm::dot(p2 - p1, n);
				Cp.push_back(sum);
			}
		}
	}



	//Solving for velocity constraint
	void VelocityConstraintSolver(float dt,
		std::vector<float>& Cv, std::vector<float>& Cp,
		std::vector<LinearAngularPair> const& Vcurr,
		std::vector <RigidbodyComponent*> const& bodies,
		std::vector<std::vector<LinearAngularPair>> const& Jsp, 
		std::vector<std::vector<unsigned>> const& Jmap)
	{
		unsigned s = Jsp.size();
		for (int i = 0; i < s; ++i)
		{
			unsigned b1 = Jmap[i][0];
			unsigned b2 = Jmap[i][1];

			RigidbodyComponent *rb1 = bodies[b1];
			RigidbodyComponent *rb2 = bodies[b2];

			float sum = 0.0f;
			if (rb1->getMassInv() > 0.0f) 
				sum += Jsp[i][0] * Vcurr[b1];
			if (rb2->getMassInv() > 0.0f) 
				sum += Jsp[i][1] * Vcurr[b2];

			//Baumgarte stabilization test
			float beta = 0.75f;
			sum += (beta/dt) * Cp[i];

			Cv.push_back(sum);
		}
	}



	//Solving for Fc, constraint forces
	void ImpulseSolver(std::vector<LinearAngularPair>& Fc,
		std::vector<float> const& Lambda,
		std::vector <RigidbodyComponent*> const& bodies,
		std::vector<std::vector<LinearAngularPair>> const& Jsp,
		std::vector<std::vector<unsigned>> const& Jmap)
	{
		unsigned n = bodies.size();
		unsigned s = Jsp.size();

		for (int i = 0; i < n; ++i)
			Fc.push_back({});

		for (int i = 0; i < s; ++i)
		{
			unsigned b1 = Jmap[i][0];
			unsigned b2 = Jmap[i][1];

			Fc[b1] += Jsp[i][0] * Lambda[i];
			Fc[b2] += Jsp[i][1] * Lambda[i];

			//DEBUGGING
			float len1 = glm::length(Fc[b2].linear);
			float len2 = glm::length(Fc[b2].angular);
			if (len1 > 20.0f || len2 > 20.0f)
				int a = 123;
		}
	}



	//Solving for lambda, lagrange multipliers
	void LagrangeMultipliersSolver(float dt, 
		std::vector<float>& Lambda,
		std::vector <RigidbodyComponent*> const& bodies,
		std::vector<LinearAngularPair>& Vprev,
		std::vector<LinearAngularPair>& Vcurr,
		std::vector<std::vector<LinearAngularPair>> const& Jsp,
		std::vector<std::vector<unsigned>> const& Jmap,
		std::vector<float> const& Cp,
		std::vector<float> const& Cv)
	{
		unsigned n = bodies.size();
		unsigned s = Jsp.size();

		std::vector<std::vector<LinearAngularPair>> Bsp;
		std::vector<LinearAngularPair> a; 
		for (int i = 0; i < n; ++i)
			a.push_back({});
		std::vector<float> d;
			
		//Have to setup Bsp = Minv * Jt
		//Have to setup a = B*Lambda
		for (int i = 0; i < s; ++i)
		{
			unsigned b1 = Jmap[i][0];
			unsigned b2 = Jmap[i][1];
			RigidbodyComponent *rb1 = bodies[b1];
			RigidbodyComponent *rb2 = bodies[b2];

			//Mass matrices
			glm::mat4 IdMassInv_1 = glm::mat4(1) * rb1->getMassInv();
			glm::mat4 IwInv_1 = rb1->GetInertiaTensorWorldInv();
			glm::mat4 IdMassInv_2 = glm::mat4(1) * rb2->getMassInv();
			glm::mat4 IwInv_2 = rb2->GetInertiaTensorWorldInv();

			LinearAngularPair MassTimesJsp_1 = {};
			MassTimesJsp_1.linear = IdMassInv_1 * Jsp[i][0].linear;
			MassTimesJsp_1.angular = IwInv_1 * Jsp[i][0].angular;

			LinearAngularPair MassTimesJsp_2 = {};
			MassTimesJsp_2.linear = IdMassInv_2 * Jsp[i][1].linear;
			MassTimesJsp_2.angular = IwInv_2 * Jsp[i][1].angular;

			//Fill a
			a[b1] += (MassTimesJsp_1 * Lambda[i]); //a[b1] will store a couple of vec3
			a[b2] += (MassTimesJsp_2 * Lambda[i]); //a[b2] will store a couple of vec3

			//Fill Bsp
			Bsp.push_back({ MassTimesJsp_1 , MassTimesJsp_2 });
		}

		//Have to setup d = diagonal of JB
		for (int i = 0; i < s; ++i)
		{
			float f1 = (Jsp[i][0] * Bsp[i][0]); // BSP ACTS AS TRANSPOSED!!!
			float f2 = (Jsp[i][1] * Bsp[i][1]); // BSP ACTS AS TRANSPOSED!!!
			d.push_back(f1 + f2); //Gotta change way the multiplication happens
		}

		//Get the lambdas
		for (int i = 0; i < s; ++i)
		{
			unsigned b1 = Jmap[i][0];
			unsigned b2 = Jmap[i][1];
			RigidbodyComponent *rb1 = bodies[b1];
			RigidbodyComponent *rb2 = bodies[b2];

			//--------------------------------------------------------
			///glm::mat4 IdMassInv_1 = glm::mat4(1) * rb1->getMassInv();
			///glm::mat4 IwInv_1 = rb1->GetInertiaTensorWorldInv();
			///glm::mat4 IdMassInv_2 = glm::mat4(1) * rb2->getMassInv();
			///glm::mat4 IwInv_2 = rb2->GetInertiaTensorWorldInv();
			///LinearAngularPair MassTimesForceTorque1 = {};
			///MassTimesForceTorque1.linear = IdMassInv_1 * rb1->Force;
			///MassTimesForceTorque1.angular = IwInv_1 * rb2->Torque;
			///LinearAngularPair MassTimesForceTorque2 = {};
			///MassTimesForceTorque2.linear = IdMassInv_2 * rb1->Force;
			///MassTimesForceTorque2.angular = IwInv_2 * rb2->Torque;
			//--------------------------------------------------------

			float beta = -Cv[i];				   ////////////////************************
			//float beta = (-1)*(1.00f/dt) *(Cp[i]); ////////////////************************
			
			float Nu = (beta - (Jsp[i][0] * Vprev[b1] + Jsp[i][1] * Vprev[b2]));//dt;

			float DeltaLambda = (Nu - (Jsp[i][0] * a[b1]) - (Jsp[i][1] * a[b2])) / d[i];
			float prevLambda = Lambda[i];
			
			Lambda[i] = std::max(0.0f, prevLambda + DeltaLambda);
			if (std::abs(Lambda[i]) > 600.0f)
				int a = 123;

			DeltaLambda = Lambda[i] - prevLambda;

			a[b1] += Bsp[i][0] * DeltaLambda; // BSP ACTS AS TRANSPOSED!!!
			a[b2] += Bsp[i][1] * DeltaLambda; // BSP ACTS AS TRANSPOSED!!!
		}
	}
}
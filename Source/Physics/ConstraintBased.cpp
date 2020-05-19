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
	void JacobianSetup(std::vector<ContactSet> const& contacts,
		std::vector<std::vector<LinearAngularPair>>& Jsp)
	{
		//Loop through all contact manifolds 
		for (int index = 0; index < contacts.size(); ++index)
		{
			ContactSet const& ct = contacts[index];
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
			///n = -glm::vec4(ct.manifold.restitution.x, ct.manifold.restitution.y, ct.manifold.restitution.z, 1.0f);/////////****

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
		std::vector<LinearAngularPair>& Vcurr)
	{
		for (RigidbodyComponent *body : bodies) 
		{
			LinearAngularPair vpair = {};

			vpair.linear = body->GetVelocity();
			vpair.angular = body->GetAngularVelocity();

			Vcurr.push_back(vpair);
		}
	}



	//GONNA STOP USING THIS
	//Update the velocity vectors
	void VelocityVectorUpdate(std::vector<RigidbodyComponent*> const& bodies,
		std::vector<LinearAngularPair>& Vcurr) 
	{
		int index = 0;
		for (RigidbodyComponent *body : bodies)
		{
			LinearAngularPair vpair = {};
			vpair.linear = body->GetVelocity();
			vpair.angular = body->GetAngularVelocity();

			Vcurr[index++] = vpair;
		}
	}



	//Solving for position constraint
	void PositionConstraintSolver(std::vector<float>& Cp,
		std::vector<ContactSet> const& contacts)
	{
		//Loop through all contact manifolds 
		for (int index = 0; index < contacts.size(); ++index)
		{
			ContactSet const& ct = contacts[index];
			RigidbodyComponent *rb1 = ct.rbdyA;
			RigidbodyComponent *rb2 = ct.rbdyB;

			//Experiment with normal
			//(normal should go from body 1 to 2, that is to say, A to B)
			glm::vec4 n(0);
			for (int i = 0; i < ct.manifold.ptsA.size(); ++i)
				n += (ct.manifold.ptsB[i].world - ct.manifold.ptsA[i].world);
			n.w = 0.0f;
			n = glm::normalize(-n);
			
			//CONSTRAINTS LOOP (each A-B pair of points is a constraint)
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
			sum += Jsp[i][0] * Vcurr[b1]; 
			sum += Jsp[i][1] * Vcurr[b2];

			//Baumgarte stabilization test
			///float beta = 0.0005f / dt;
			///sum = sum + beta * Cp[i];

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
		}
	}



	//Solving for lambda, lagrange multipliers
	void LagrangeMultipliersSolver(float dt, 
		std::vector<float>& Lambda,
		std::vector <RigidbodyComponent*> const& bodies,
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
			glm::mat4 IdMassInv_1 = glm::mat4(1) * rb1->getMassInv();
			glm::mat4 IwInv_1 = rb1->GetInertiaTensorWorldInv();
			glm::mat4 IdMassInv_2 = glm::mat4(1) * rb2->getMassInv();
			glm::mat4 IwInv_2 = rb2->GetInertiaTensorWorldInv();
			LinearAngularPair MinvFext_b1 = {};
			MinvFext_b1.linear = IdMassInv_1 * (rb1->Force);
			MinvFext_b1.angular = IwInv_1 * (rb1->Torque);
			LinearAngularPair MinvFext_b2 = {};
			MinvFext_b2.linear = IdMassInv_2 * (rb2->Force);
			MinvFext_b2.angular = IwInv_2 * (rb2->Torque);
			//--------------------------------------------------------

			float slope = -0.0f;
			if (Cp[i] > slope)
			{
				Lambda[i] = 0.0f;
			}
			else 
			{
				//float beta = -(0.05f / dt) * (Cp[i]); //kinda works
				float beta = -(0.1f / dt) * (Cp[i] - slope);

				float JspMinvFext = (Jsp[i][0] * MinvFext_b1 + Jsp[i][1] * MinvFext_b2);
				float JspVcurr = (Jsp[i][0] * Vcurr[b1] + Jsp[i][1] * Vcurr[b2]);

				float Ni = ((beta - JspVcurr) / dt) - JspMinvFext;
				if (std::abs(d[i]) < 0.000001f)
					d[i] = 0.0f;
				float DeltaLambda = (d[i] == 0.0f) ? 0.0f : (Ni - (Jsp[i][0] * a[b1]) - (Jsp[i][1] * a[b2])) / d[i];

				float previous = Lambda[i];
				Lambda[i] = std::max(0.0f, previous + DeltaLambda);
				DeltaLambda = Lambda[i] - previous;

				a[b1] = a[b1] + Bsp[i][0] * DeltaLambda; // BSP ACTS AS TRANSPOSED!!!
				a[b2] = a[b2] + Bsp[i][1] * DeltaLambda; // BSP ACTS AS TRANSPOSED!!!
			}
		}
	}
}
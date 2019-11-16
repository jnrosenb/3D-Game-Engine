///HEADER STUFF

#include "PathFollowComponent.h"
#include "GameObject.h"
#include "Interpolation.h"
#include "Quaternion.h"
#include "TransformComponent.h"
#include "AnimationComponent.h"

#include <algorithm>

// TODO Temporary (while no world exists)
#include "InputManager.h"
extern InputManager *inputMgr;
#include "DeferredRenderer.h"
extern Renderer *renderer;


#define EPSILON 0.000001f


PathFollowComponent::PathFollowComponent(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::PATH_FOLLOW)
{
}

PathFollowComponent::~PathFollowComponent()
{
	std::cout << "Destroying PathFollow Component" << std::endl;
}


void PathFollowComponent::DeserializeInit()
{
	speed = 7.0f;
	globalT = 1.0f;
	t1 = globalT * 0.15f;
	t2 = globalT * (1.0f - 0.15f);
	animTime = 0.0f;
	animDist = 0.0f;

	isRunning = false;
	drawPath = false;

	//Generate curve
	AuxMath::GenerateCurve(200, globalT, m_points, m_curve, m_curve_vector);

	//Fill arclen table
	generateArcLenTable();
}


void PathFollowComponent::Update(float dt)
{
	//TODO temporal
	handleInput(dt);

	//TODO - Update code for the path
	if (isRunning) 
	{
		//Calculate the time to look into the curve
		animTime += dt;
		float arclenTime = globalT;
		float actualSpeed = speed;
		float thresshold = 0.2f;
		

		//Get the maximum time (as in the max value animValue can take)
		///float DistanceWithConstantSpeed = globalT * speed;
		float DistanceWithConstantSpeed = std::get<1>(arclenTable[arclenTable.size() - 1]);
		float maxAnimTime_withArcos = (std::get<1>(arclenTable[arclenTable.size() - 1])) / speed;
		float maxAnimTimeGT = globalT;
		float maxAnimTime = (DistanceWithConstantSpeed) / (speed * (1.0f - thresshold));


		
		t1 = thresshold * maxAnimTime;
		t2 = (1.0f - thresshold) * maxAnimTime;
		if (animTime <= t1)
		{
			actualSpeed = speed * animTime / t1;
		}
		else if (animTime <= t2)
		{
		}
		else if (animTime > t2)
		{
			actualSpeed = speed * (maxAnimTime - animTime) / (maxAnimTime - t2);
			if (actualSpeed < 0)
				int a = 234;
		}

		AnimationComponent *animComp = m_owner->GetComponent<AnimationComponent>();
		if (animComp)
		{
			animComp->animSpeed = 0.25f + 0.1f * actualSpeed;
		}

		float dpos = dt * actualSpeed;
		animDist += dpos;

		//Looking in arclen table. Here we should have speed determined
		for (int i = 0; i < arclenTable.size() - 1; ++i) 
		{
			std::tuple<float, float> pair_prev = arclenTable[i];
			std::tuple<float, float> pair_post = arclenTable[i + 1];
			float time_0 = std::get<0>(pair_prev);
			float dist_0 = std::get<1>(pair_prev);
			float time_1 = std::get<0>(pair_post);
			float dist_1 = std::get<1>(pair_post);
		
			if (animDist >= dist_0 && animDist < dist_1)
			{
				float alpha = (animDist - dist_0) / (dist_1 - dist_0);
				arclenTime = time_0 + alpha * (time_1 - time_0);
				break;
			}
		}

		if (arclenTime >= globalT)
		{
			animDist = 0.0f;
			animTime = 0.0f;
			arclenTime = 0.0f; //arclenTime - globalT;
		}

		//Find the point in the curve stuff
		glm::vec4 point;
		auto iter = m_curve.upper_bound(arclenTime);
		glm::vec4 lower = iter->second;
		float lowerT = iter->first;
		
		auto iterNext = ++iter;
		float alpha = (arclenTime - lowerT) / (iterNext->first - lowerT);
		point = (1 - alpha) * lower + alpha * iterNext->second;

		//Build a model matrix which will be used for moving the obj
		//Or just translate it for now
		Transform *T = this->m_owner->GetComponent<Transform>();
		glm::vec4 dx = point;
		T->SetPosition(glm::vec3(dx));
		
		//Rotate to face direction
		float PI = 4.0f * atan(1.0f);
		glm::vec3 tangent = glm::normalize(glm::vec3(iterNext->second - lower));
		glm::vec3 forward = glm::normalize(glm::vec3(T->GetForward()));
		float dot = glm::dot<3, float, glm::qualifier::highp>(forward, tangent);
		if (1.0f - fabs(dot) > 0.01f) 
		{
			float rad_angle = acosf(dot);
			float yRot = (rad_angle  * 180.0f) / PI;

			//Do not rotate if the angle between them is smaller 
			//than thresshold value
			float thressholdAngle = 3.0f;
			if (fabs(yRot) > thressholdAngle) 
			{
				//Correct so it goes in correct dir
				glm::vec3 right = T->GetRight();
				if (glm::dot(tangent, right) < 0.0f)
					yRot = -yRot;

				if (yRot > 0.0f)
					T->rotate(0, 3.0f, 0);
				if (yRot < 0.0f)
					T->rotate(0, -3.0f, 0);
			}
		}
	}
}


//Used to compare the tuples and order them
struct CompareFirst
{
	bool operator()(std::tuple<float,float> const& left, std::tuple<float, float> const& right) const
	{
		return std::get<0>(left) < std::get<0>(right);
	}
};


void PathFollowComponent::generateArcLenTable() 
{
	//Table init to 0, 0 [time, arclen accum val]
	arclenTable.push_back({ 0.0f, 0.0f });

	//Init and put first element on stack
	std::list<std::tuple<float,float>> list;

	list.insert(list.end(), {0.0f, globalT});

	while (list.size() > 0.0f)
	{
		//min_element(mymap.begin(), mymap.end(), CompareSecond());
		auto listIter = std::min_element(list.begin(), list.end(), CompareFirst());
		std::tuple<float, float> pair = *listIter;
		list.remove(*listIter);

		float ta = std::get<0>(pair);
		float tb = std::get<1>(pair);

		float tm = (ta + tb) / 2.0f;
		auto iter = m_curve.lower_bound(tm);
		tm = iter->first;

		glm::vec4 a = m_curve[ta];
		glm::vec4 m = iter->second;
		glm::vec4 b = m_curve[tb];

		float A = glm::length(m - a);
		float B = glm::length(b - m);
		float C = glm::length(b - a);

		if (fabs(A+B - C) > EPSILON) 
		{
			list.insert(list.end(), {ta, tm});
			list.insert(list.end(), {tm, tb});
		}
		else 
		{
			//Get the accumulated arclen until A
			float accumArcLen = std::get<1>(arclenTable[arclenTable.size() - 1]);
			arclenTable.push_back({ tm, accumArcLen + A });
			if (tb > tm)
				arclenTable.push_back({ tb, accumArcLen + A + B });
		}
	}
}


void PathFollowComponent::LateUpdate(float dt)
{
	//Draw the control points
	if (drawPath) 
	{
		DeferredRenderer *rend = static_cast<DeferredRenderer*>(renderer);
		rend->DrawControlPoints(this->m_points);
		rend->DrawCurve(this->m_curve_vector, glm::vec3(0,1,0));
	}
}



////////////////////////////////
////	HANDLE INPUT	    ////
////////////////////////////////
void PathFollowComponent::handleInput(float dt)
{
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_K))
	{
		isRunning = !isRunning;
	}
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_L))
	{
		drawPath = !drawPath;
	}
	if (inputMgr->getKeyTrigger(SDL_SCANCODE_J))
	{
		isRunning = true;
		animTime = 0.0f;
		animDist = 0.0f;
	}
}



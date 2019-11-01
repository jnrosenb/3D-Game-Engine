///HEADER STUFF

#include "AnimationComponent.h"
#include "RendererComponent.h"

#include "GameObject.h"
#include "Model.h"
#include "Interpolation.h"

// TODO Temporary (while no world exists)
#include "ResourceManager.h"
extern ResourceManager *resMgr;

// TODO For now, camera will handle all input
#include "InputManager.h"
extern InputManager *inputMgr;


AnimationComponent::AnimationComponent(GameObject *owner) :
	BaseComponent(owner, COMPONENT_TYPES::ANIMATION)
{
}

AnimationComponent::~AnimationComponent()
{
	std::cout << "Destroying Animation Component" << std::endl;

	BoneTransformations.clear();
}


void AnimationComponent::PassAnimationInfo()
{
	DeserializeInit();
}


void AnimationComponent::DeserializeInit()
{
	///FOR NOW, HARDCODE THE CURRENT ANIMATION
	//this->SwitchAnimation("Armature|walk");
	this->SwitchAnimation("mixamo.com");


	//SHITTY WAY------------------------------------
	BoneTransformations = std::vector<glm::mat4>(100); //TODO CHANGE THIS URGENT
}


void AnimationComponent::SwitchAnimation(std::string clipName, bool loops) 
{
	// TODO - This is temporary, should be stuff the anim comp has access to
	Render *renderComp = this->m_owner->GetComponent<Render>();
	std::unordered_map<std::string, Bone> const& boneMap = renderComp->model->boneMap;
	std::unordered_map<std::string, Animation> const& animMap = renderComp->model->animMap;

	this->currentAnimation = clipName;
	this->currentTPS = animMap.find(currentAnimation)->second.ticksPerSecond;
	this->duration = animMap.find(currentAnimation)->second.duration;
	this->AnimationTime = 0.0f;
	this->looping = loops;
}


void AnimationComponent::Update(float dt)
{
	//TODO temporal
	handleInput(dt);

	//TODO - Cache somehow this, cause it is inneficient
	Render *renderComp = this->m_owner->GetComponent<Render>();
	std::unordered_map<std::string, Bone>& boneMap = renderComp->model->boneMap;
	std::unordered_map<std::string, Animation> const& animMap = renderComp->model->animMap;


	AnimationTime += dt * currentTPS;
	Animation const& animation = animMap.find(currentAnimation)->second;

	//Loop through the channels, find the interp, and do something with it
	for (auto const& animChannel : animation.channels)
	{
		std::string const& bone = animChannel.boneName;

		//Interp position and the others
		glm::vec3 interpPos = CalculateInterpPos(AnimationTime, animChannel);
		AuxMath::Quaternion interpQuat = CalculateInterpRot(AnimationTime, animChannel);
		glm::vec3 interpScale = CalculateInterpScale(AnimationTime, animChannel); //TODO - special scaling

		//Build the VQS matrix for this bone
		glm::mat4 vqs = AuxMath::GenVQSMatrix(interpPos, interpQuat, interpScale);

		//Pass the bone its VQS matrix
		Bone& b = boneMap.find(bone)->second;
		b.vqs = vqs;
		b.updatedVQS = true;
	}

	//After updating, we could make one go from root to childs, updating the matrices recursively
	Bone& root = renderComp->model->boneMap["RootNode"];
	renderComp->model->ProcessRecursiveTransformationFromRoot(root, glm::mat4(1), BoneTransformations);

	//Restart animation
	if (AnimationTime > duration && looping)
		AnimationTime = AnimationTime - duration;
}


//Calculate interpolation position given the current time in ticks and a particular channel
glm::vec3 AnimationComponent::CalculateInterpPos(float AnimationTime, AnimChannel const& animChannel) 
{
	PosKey prevKey = animChannel.PositionKeys[0];
	for (PosKey const& posk : animChannel.PositionKeys)
	{
		//Return the key's position
		if (fabs(posk.time - AnimationTime) <= EPSILON)
			return posk.position;

		//Return interp
		if (prevKey.time <= AnimationTime && posk.time >= AnimationTime)
		{
			float alpha = (AnimationTime - prevKey.time) / (posk.time - prevKey.time);
			return AuxMath::Lerp(prevKey.position, posk.position, alpha);
		}

		//Prepare for next iter
		else prevKey = posk;
	}

	//If we exit the loop and do not find, just return last key for now
	return prevKey.position;
}


//Calculate interpolation position given the current time in ticks and a particular channel
AuxMath::Quaternion AnimationComponent::CalculateInterpRot(float AnimationTime, AnimChannel const& animChannel)
{
	RotKey prevKey = animChannel.RotationKeys[0];
	for (RotKey const& rotk : animChannel.RotationKeys)
	{
		//Return the key's position
		if (fabs(rotk.time - AnimationTime) <= EPSILON)
			return rotk.quaternion;

		//Return interp
		if (prevKey.time <= AnimationTime && rotk.time >= AnimationTime)
		{
			float alpha = (AnimationTime - prevKey.time) / (rotk.time - prevKey.time);
			return AuxMath::Slerp(prevKey.quaternion, rotk.quaternion, alpha);
		}

		//Prepare for next iter
		else prevKey = rotk;
	}

	//If we exit the loop and do not find, just return last key for now
	return prevKey.quaternion;
}


//Calculate interpolation position given the current time in ticks and a particular channel
glm::vec3 AnimationComponent::CalculateInterpScale(float AnimationTime, AnimChannel const& animChannel)
{
	ScaKey prevKey = animChannel.ScalingKeys[0];
	for (ScaKey const& scak : animChannel.ScalingKeys)
	{
		//Return the key's position
		if (fabs(scak.time - AnimationTime) <= EPSILON)
			return scak.scale;

		//Return interp
		if (prevKey.time <= AnimationTime && scak.time >= AnimationTime)
		{
			float alpha = (AnimationTime - prevKey.time) / (scak.time - prevKey.time);
			return AuxMath::Lerp(prevKey.scale, scak.scale, alpha);
		}

		//Prepare for next iter
		else prevKey = scak;
	}

	//If we exit the loop and do not find, just return last key for now
	return prevKey.scale;
}



////////////////////////////////
////	HANDLE INPUT	    ////
////////////////////////////////
void AnimationComponent::handleInput(float dt)
{
	if (inputMgr->getKeyPress(SDL_SCANCODE_1))
	{
		this->SwitchAnimation("Armature|start", false);
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_2))
	{
		this->SwitchAnimation("Armature|walk", true);
	}
	if (inputMgr->getKeyPress(SDL_SCANCODE_3))
	{
		this->SwitchAnimation("Armature|stop", false);
	}
}



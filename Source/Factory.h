///HEADER STUFF

#pragma once

//Rapidjson stuff
#include "../External/Includes/SDL2/SDL_surface.h"
#include "../External/Includes/rapidjson/document.h"
#include "../External/Includes/rapidjson/writer.h"
#include "../External/Includes/rapidjson/stringbuffer.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class GameObject; 
using namespace rapidjson;

#include "Paths.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "Renderer.h"
#include "RendererComponent.h"
#include "TransformComponent.h"
#include "AnimationComponent.h"
#include "ParticleSystemComponent.h"
#include "Particle.h"
#include "RigidbodyComponent.h"
#include "PathFollowComponent.h"
#include "IKGoalComponent.h"
//--------------------
extern GameobjectManager* goMgr;
extern ResourceManager* resMgr;
extern Renderer *renderer;
extern EnginePaths *globalPaths;


////////////////////////////
////	FACTORY			////
////////////////////////////
class Factory 
{
public:
	Factory()
	{}


	//TODO - FIX, extremely dirty!!!
	void unloadScene() 
	{
		delete goMgr;
		goMgr = new GameobjectManager();
	}


	void LoadScene(std::string const& sceneName, bool is_global_path = false)
	{
		// TODO - make path relative (or use base directory macro)
		std::string path = globalPaths->AssetsDir + "Prefabs\\Scenes\\" + sceneName;
		if (is_global_path)
			path = sceneName;
		std::string example = loadFile(path.c_str());
		Document d;
		d.Parse(example.c_str());

		//FIRST, LETS GET THE OBJECTS
		assert(d.HasMember("Objects"));
		Value& v = d["Objects"];
		assert(v.IsArray());
		for (Value::ConstValueIterator itr = v.Begin(); itr != v.End(); ++itr) 
		{
			GameObject *go = new GameObject(); 
			const rapidjson::Value& attribute = *itr;
			assert(attribute.IsObject());

			//This is for the gameobj creation
			if (attribute.HasMember("name"))
			{
				const Value& acc = attribute["name"];
				assert(acc.IsString());
				std::string obj_path = acc.GetString();
				LoadComponents(go, obj_path);
				goMgr->AddGameObjects(go);
			}

			//First attempt at overriding in here
			if (attribute.HasMember("transform"))
			{
				Transform *transform = go->GetComponent<Transform>();
				if (transform == nullptr)
					return;

				const Value& compOverride = attribute["transform"];
				assert(compOverride.IsObject());
				if (compOverride.HasMember("position"))
				{
					const Value& acc = compOverride["position"];
					assert(acc.IsArray());
					transform->m_position = glm::vec4(acc[0].GetFloat(), acc[1].GetFloat(), acc[2].GetFloat(), 1.0f);
				}
				if (compOverride.HasMember("rotation"))
				{
					const Value& acc = compOverride["rotation"];
					assert(acc.IsArray());
					transform->m_rotation = glm::vec4(acc[0].GetFloat(), acc[1].GetFloat(), acc[2].GetFloat(), 1.0f);
				}
				if (compOverride.HasMember("scale"))
				{
					const Value& acc = compOverride["scale"];
					assert(acc.IsArray());
					transform->m_scale = glm::vec4(acc[0].GetFloat(), acc[1].GetFloat(), acc[2].GetFloat(), 1.0f);
				}
			}

			//Second attempt at overriding in here
			if (attribute.HasMember("renderer"))
			{
				Render *renderComp = go->GetComponent<Render>();
				if (renderComp == nullptr)
					return;

				const Value& compOverride = attribute["renderer"];
				assert(compOverride.IsObject());
				if (compOverride.HasMember("x_tiling"))
				{
					const Value& ac4 = compOverride["x_tiling"];
					assert(ac4.IsInt());
					renderComp->xTiling = ac4.GetInt();
				}
				if (compOverride.HasMember("y_tiling"))
				{
					const Value& ac4 = compOverride["y_tiling"];
					assert(ac4.IsInt());
					renderComp->yTiling = ac4.GetInt();
				}


			}
		}

		//After first batch creation, call begin on all gameObjects
		goMgr->CallBeginOnGameObjects();

		//LOAD WHITE TEXTURE FOR WHEN NO TEXTURE IS USED
		///SDL_Surface *surfaceWhite = resMgr->loadSurface("../White.jpg");
		///renderer->generateTextureFromSurface(surfaceWhite, "../White.jpg", 5);
		///renderer->WhiteTex = ;

		//SKYBOX
		if (d.HasMember("Skydome"))
		{
			Value& v = d["Skydome"];
			assert(v.IsObject());

			std::string hdrtex = "";
			std::string hdrirr = "";

			if (v.HasMember("Texture")) 
			{
				const Value& acc = v["Texture"];
				assert(acc.IsString());
				hdrtex = acc.GetString();
			}
			if (v.HasMember("Irradiance"))
			{
				const Value& acc = v["Irradiance"];
				assert(acc.IsString());
				hdrirr = acc.GetString();
			}

			//Create the skydome obj and keep it in renderer
			HDRImageDesc descTex = resMgr->loadHDR(hdrtex);
			HDRImageDesc descIrr = resMgr->loadHDR(hdrirr);
			renderer->CreateSkydome(descTex, descIrr);
		}

		//ALSO, LIGHT INFORMATION
		if (d.HasMember("Lights")) 
		{
			Value& v = d["Lights"];
			assert(v.IsArray());
			int pointLights = 0;
			for (Value::ConstValueIterator itr = v.Begin(); itr != v.End(); ++itr)
			{
				const rapidjson::Value& attribute = *itr;
				assert(attribute.IsObject());

				assert(attribute.HasMember("Type"));
				const Value& acc = attribute["Type"];
				assert(acc.IsString());
				std::string lightType = acc.GetString();

				if (lightType == "Point")
				{
					assert(pointLights <= MAX_LIGHT_COUNT); //TODO - NO more than max number of lights
					assert(attribute.HasMember("Position"));
					const Value& ac0 = attribute["Position"];
					assert(ac0.IsArray());
					renderer->Light_Positions[pointLights] = glm::vec4(ac0[0].GetFloat(), ac0[1].GetFloat(), ac0[2].GetFloat(), 1.0f);

					assert(attribute.HasMember("Color"));
					const Value& ac1 = attribute["Color"];
					assert(ac1.IsArray());
					renderer->Light_Colors[pointLights] = glm::vec4(ac1[0].GetFloat(), ac1[1].GetFloat(), ac1[2].GetFloat(), 1.0f);

					assert(attribute.HasMember("Radius"));
					const Value& ac2 = attribute["Radius"];
					assert(ac2.IsFloat());
					renderer->Light_Radius[pointLights] = ac2.GetFloat();
					
					++pointLights;
				}
				else if (lightType == "Directional")
				{
					assert(attribute.HasMember("Position"));
					const Value& ac0 = attribute["Position"];
					assert(ac0.IsArray());
					renderer->sun.eye = glm::vec4(ac0[0].GetFloat(), ac0[1].GetFloat(), ac0[2].GetFloat(), 1.0f);

					assert(attribute.HasMember("Color"));
					const Value& ac00 = attribute["Color"];
					assert(ac00.IsArray());
					renderer->sun.color = glm::vec4(ac00[0].GetFloat(), ac00[1].GetFloat(), ac00[2].GetFloat(), 1.0f);

					assert(attribute.HasMember("Look"));
					const Value& ac1 = attribute["Look"];
					assert(ac1.IsArray());
					glm::vec4 look = glm::vec4(ac1[0].GetFloat(), ac1[1].GetFloat(), ac1[2].GetFloat(), 0.0f);
					renderer->sun.look = glm::normalize(look);

					if (attribute.HasMember("Near")) 
					{
						const Value& ac3 = attribute["Near"];
						assert(ac3.IsFloat());
						renderer->sun.m_near = ac3.GetFloat();
					}

					if (attribute.HasMember("Far")) 
					{
						const Value& ac3 = attribute["Far"];
						assert(ac3.IsFloat());
						renderer->sun.m_far = ac3.GetFloat();
					}

					if (attribute.HasMember("Width"))
					{
						const Value& ac3 = attribute["Width"];
						assert(ac3.IsInt());
						renderer->sun.width = ac3.GetInt();
					}


					if (attribute.HasMember("Height"))
					{
						const Value& ac3 = attribute["Height"];
						assert(ac3.IsInt());
						renderer->sun.height = ac3.GetInt();
					}


					if (attribute.HasMember("Intensity"))
					{
						const Value& ac3 = attribute["Intensity"];
						assert(ac3.IsFloat());
						renderer->sun.color.a = ac3.GetFloat();
					}


					if (attribute.HasMember("ShadowIntensity"))
					{
						const Value& ac3 = attribute["ShadowIntensity"];
						assert(ac3.IsFloat());
						renderer->sun.shadowIntensity = ac3.GetFloat();
					}
				}
			}

			//Set the actual number of lights on the scene
			renderer->lightCount += pointLights;
		}
	}


	void LoadComponents(GameObject* go, std::string const& obj_path) 
	{
		std::string full_path = globalPaths->AssetsDir + "Prefabs\\Gameobjects\\" + obj_path;
		std::string example = loadFile(full_path.c_str());
		Document doc;
		doc.Parse(example.c_str());

		//FIRST, LETS GET THE OBJECTS
		assert(doc.HasMember("tag"));
		Value& v = doc["tag"];
		assert(v.IsString());
		std::string tag = v.GetString();

		assert(doc.HasMember("components"));
		v = doc["components"];
		assert(v.IsArray());
		for (Value::ConstValueIterator itr = v.Begin(); itr != v.End(); ++itr)
		{
			const rapidjson::Value& attribute = *itr;
			assert(attribute.IsObject());
			assert(attribute.HasMember("id"));
			std::string comp_name = attribute["id"].GetString();

			if (comp_name == "transform")
			{
				Transform *transform = go->AddComponent<Transform>();

				//position
				assert(attribute.HasMember("position"));
				const Value& acc = attribute["position"];
				assert(acc.IsArray());
				transform->translate( acc[0].GetFloat(), acc[1].GetFloat(), acc[2].GetFloat() );
				assert(attribute.HasMember("rotation"));
				const Value& ac2 = attribute["rotation"];
				assert(ac2.IsArray());
				transform->m_rotation = glm::vec4(ac2[0].GetFloat(), ac2[1].GetFloat(), ac2[2].GetFloat(), 1.0f);
				assert(attribute.HasMember("scale"));
				const Value& ac3 = attribute["scale"];
				assert(ac3.IsArray());
				transform->scale( ac3[0].GetFloat(), ac3[1].GetFloat(), ac3[2].GetFloat() );

				transform->DeserializeInit();
			}
			else if (comp_name == "render")
			{
				Render *render = go->AddComponent<Render>();
				
				if (attribute.HasMember("load_model")) 
				{
					const Value& ac2 = attribute["load_model"];
					assert(ac2.IsBool());
					bool use_model = ac2.GetBool();
					render->use_loaded_mesh = use_model;

					if (use_model)
					{
						assert(attribute.HasMember("model_name"));
						const Value& acc = attribute["model_name"];
						assert(acc.IsString());
						std::string modelPath = acc.GetString();
						render->modelPath = modelPath;
					}
					else
					{
						assert(attribute.HasMember("primitive"));
						const Value& ac3 = attribute["primitive"];
						assert(ac3.IsString());
						std::string primitive = ac3.GetString();
						render->primitive = primitive;
					}
				}
				else
				{
					assert(attribute.HasMember("primitive"));
					const Value& ac3 = attribute["primitive"];
					assert(ac3.IsString());
					std::string primitive = ac3.GetString();
					render->primitive = primitive;
				}

				if (attribute.HasMember("x_tiling"))
				{
					const Value& ac4 = attribute["x_tiling"];
					assert(ac4.IsInt());
					render->xTiling = ac4.GetInt();
				}

				if (attribute.HasMember("y_tiling"))
				{
					const Value& ac4 = attribute["y_tiling"];
					assert(ac4.IsInt());
					render->yTiling = ac4.GetInt();
				}

				if (attribute.HasMember("roughness"))
				{
					const Value& ac4 = attribute["roughness"];
					assert(ac4.IsFloat());
					render->SetClampedRoughness(ac4.GetFloat());
				}

				if (attribute.HasMember("metallic"))
				{
					const Value& ac4 = attribute["metallic"];
					assert(ac4.IsFloat());
					render->metallic = ac4.GetFloat();
				}

				if (attribute.HasMember("diffuse_texture")) 
				{
					const Value& ac4 = attribute["diffuse_texture"];
					assert(ac4.IsString());
					std::string diffuseTexture = ac4.GetString();
					
					SDL_Surface *surf = resMgr->loadSurface(diffuseTexture);
					renderer->generateTextureFromSurface(surf, diffuseTexture, 5);
					render->diffuseTexture = diffuseTexture;
					render->useDiffuseTexture = true;
				}

				if (attribute.HasMember("roughness_texture"))
				{
					const Value& ac4 = attribute["roughness_texture"];
					assert(ac4.IsString());
					std::string roughnessTexture = ac4.GetString();

					SDL_Surface *surf = resMgr->loadSurface(roughnessTexture);
					renderer->generateTextureFromSurface(surf, roughnessTexture, 5);
					render->roughnessTex = roughnessTexture;

					//This is so its not considered in shader
					render->roughness = -1.0f;
				}

				if (attribute.HasMember("metallic_texture"))
				{
					const Value& ac4 = attribute["metallic_texture"];
					assert(ac4.IsString());
					std::string metallicTexture = ac4.GetString();

					SDL_Surface *surf = resMgr->loadSurface(metallicTexture);
					renderer->generateTextureFromSurface(surf, metallicTexture, 5);
					render->metallicTex = metallicTexture;

					//This is so its not considered in shader
					render->metallic = -1.0f;
				}

				if (attribute.HasMember("normal_map"))
				{
					const Value& ac4 = attribute["normal_map"];
					assert(ac4.IsString());
					std::string normalMap = ac4.GetString();

					SDL_Surface *surf = resMgr->loadSurface(normalMap);
					renderer->generateTextureFromSurface(surf, normalMap, 5);
					render->normalMap = normalMap;
					render->useNormalMap = true;
				}

				if (attribute.HasMember("diffuse"))
				{
					assert(attribute.HasMember("diffuse"));
					const Value& ac4 = attribute["diffuse"];
					assert(ac4.IsArray());
					render->diffuseColor = glm::vec4(ac4[0].GetFloat(), ac4[1].GetFloat(), ac4[2].GetFloat(), 1.0f);
				}

				if (attribute.HasMember("specular"))
				{
					assert(attribute.HasMember("specular"));
					const Value& ac4 = attribute["specular"];
					assert(ac4.IsArray());
					render->specularColor = glm::vec4(ac4[0].GetFloat(), ac4[1].GetFloat(), ac4[2].GetFloat(), 1.0f);
				}

				if (attribute.HasMember("shader")) 
				{
					const Value& ac4 = attribute["shader"];
					assert(ac4.IsString());
					std::string shaderName = ac4.GetString();
					render->ShaderName = shaderName;
				}

				render->DeserializeInit();
			}
			else if (comp_name == "rigidbody")
			{
				RigidbodyComponent *rgbdy = go->AddComponent<RigidbodyComponent>();

				if (attribute.HasMember("mass"))
				{
					const Value& ac4 = attribute["mass"];
					assert(ac4.IsFloat());
					rgbdy->mass = ac4.GetFloat();
				}

				if (attribute.HasMember("shape"))
				{
					const Value& ac4 = attribute["shape"];
					assert(ac4.IsString());
					std::string shape = ac4.GetString();
					
					// TODO
				}

				//TEMPORARY, REMOVE LATER
				if (attribute.HasMember("player"))
				{
					const Value& ac4 = attribute["player"];
					assert(ac4.IsBool());
					bool isPlayer = ac4.GetBool();
					rgbdy->isPlayer = isPlayer;
				}

				rgbdy->DeserializeInit();
			}
			else if (comp_name == "particleSystem")
			{
				ParticleSystemComponent *psc = go->AddComponent<ParticleSystemComponent>();

				if (attribute.HasMember("load_model"))
				{
					const Value& ac2 = attribute["load_model"];
					assert(ac2.IsBool());
					bool use_model = ac2.GetBool();
					psc->use_loaded_mesh = use_model;

					if (use_model)
					{
						assert(attribute.HasMember("model_name"));
						const Value& acc = attribute["model_name"];
						assert(acc.IsString());
						std::string modelPath = acc.GetString();
						psc->modelPath = modelPath;
					}
					else
					{
						assert(attribute.HasMember("primitive"));
						const Value& ac3 = attribute["primitive"];
						assert(ac3.IsString());
						std::string primitive = ac3.GetString();
						psc->primitive = primitive;
					}
				}
				else
				{
					assert(attribute.HasMember("primitive"));
					const Value& ac3 = attribute["primitive"];
					assert(ac3.IsString());
					std::string primitive = ac3.GetString();
					psc->primitive = primitive;
				}

				if (attribute.HasMember("particleCount"))
				{
					const Value& ac4 = attribute["particleCount"];
					assert(ac4.IsInt());
					psc->count = ac4.GetInt();
				}

				if (attribute.HasMember("x_tiling"))
				{
					const Value& ac4 = attribute["x_tiling"];
					assert(ac4.IsInt());
					psc->xTiling = ac4.GetInt();
				}

				if (attribute.HasMember("y_tiling"))
				{
					const Value& ac4 = attribute["y_tiling"];
					assert(ac4.IsInt());
					psc->yTiling = ac4.GetInt();
				}

				if (attribute.HasMember("diffuse_texture"))
				{
					const Value& ac4 = attribute["diffuse_texture"];
					assert(ac4.IsString());
					std::string diffuseTexture = ac4.GetString();

					SDL_Surface *surf = resMgr->loadSurface(diffuseTexture);
					renderer->generateTextureFromSurface(surf, diffuseTexture, 5);
					psc->diffuseTexture = diffuseTexture;
					psc->useDiffuseTexture = true;
				}

				if (attribute.HasMember("advectors"))
				{
					//Advectors, will be used before operators, since they define a base velocity not based on previous values
					const Value& ac4 = attribute["advectors"];
					assert(ac4.IsArray());
					for (Value::ConstValueIterator itr = ac4.Begin(); itr != ac4.End(); ++itr)
					{
						const rapidjson::Value& attribute = *itr;
						assert(attribute.IsObject());
						assert(attribute.HasMember("type"));
						const Value& ac5 = attribute["type"];
						assert(ac5.IsString());
						std::string type = ac5.GetString();

						if (type == "vortex")
						{
							Operator *op = new VortexOperation();
							VortexOperation *castOp = static_cast<VortexOperation*>(op);
							if (attribute.HasMember("rotation_rate"))
							{
								const Value& acc = attribute["rotation_rate"];
								assert(acc.IsFloat());
								castOp->rotationRate = acc.GetFloat();
							}
							if (attribute.HasMember("tightness"))
							{
								const Value& acc = attribute["tightness"];
								assert(acc.IsFloat());
								castOp->tightness = acc.GetFloat();
							}
							psc->RegisterParticleAdvector(op);
						}
						if (type == "gravitational_pull")
						{
							Operator *op = new GravitationalOperator();
							GravitationalOperator *castOp = static_cast<GravitationalOperator*>(op);

							if (attribute.HasMember("mass"))
							{
								const Value& acc = attribute["mass"];
								assert(acc.IsFloat());
								castOp->mass = acc.GetFloat();
							}
							psc->RegisterParticleAdvector(op);
						}
						if (type == "spiral")
						{
							Operator *op = new SpiralOperation();
							SpiralOperation *castOp = static_cast<SpiralOperation*>(op);

							if (attribute.HasMember("rotation_rate"))
							{
								const Value& acc = attribute["rotation_rate"];
								assert(acc.IsFloat());
								castOp->rotationRate = acc.GetFloat();
							}
							psc->RegisterParticleAdvector(op);
						}
					}
				}

				if (attribute.HasMember("operators"))
				{
					//Operators will integrate based on previous velocity values
					const Value& ac4 = attribute["operators"];
					assert(ac4.IsArray());
					for (Value::ConstValueIterator itr = ac4.Begin(); itr != ac4.End(); ++itr)
					{
						const rapidjson::Value& attribute = *itr;
						assert(attribute.IsObject());

						assert(attribute.HasMember("type"));
						const Value& ac5 = attribute["type"];
						assert(ac5.IsString());
						std::string type = ac5.GetString();
						
						if (type == "bounce")
						{
							//Friction thing
							//Damping thing
						}
						else if (type == "damping")
						{
							//Target vel
							//Speed of convergence
						}
					}
				}

				psc->DeserializeInit();
			}
			else if (comp_name == "PathFollow")
			{
				PathFollowComponent *pathFollow = go->AddComponent<PathFollowComponent>();

				//ControlPoints
				if (attribute.HasMember("control_points"))
				{
					const Value& acc = attribute["control_points"];
					assert(acc.IsArray());
					int count = 0;
					for (Value::ConstValueIterator itr = acc.Begin(); itr != acc.End(); ++itr)
					{
						const rapidjson::Value& attribute = *itr;
						assert(attribute.IsArray());
						glm::vec4 point = glm::vec4(attribute[0].GetFloat(), attribute[1].GetFloat(), attribute[2].GetFloat(), 1.0f);
						pathFollow->m_points.push_back(point);
					}
				}
				pathFollow->DeserializeInit();
			}
			else if (comp_name == "ik_goal") 
			{
				IKGoalComponent *goalComp = go->AddComponent<IKGoalComponent>();

				//ControlPoints
				if (attribute.HasMember("end_effectors"))
				{
					const Value& acc = attribute["end_effectors"];
					assert(acc.IsArray());
					int count = 0;
					for (Value::ConstValueIterator itr = acc.Begin(); itr != acc.End(); ++itr)
					{
						const rapidjson::Value& attribute = *itr;
						assert(attribute.IsString());
						goalComp->endEffectorsKeys.push_back(attribute.GetString());
					}
				}

				if (attribute.HasMember("start_end_effector"))
				{
					const Value& acc = attribute["start_end_effector"];
					assert(acc.IsString());
					goalComp->endEffector = acc.GetString();
				}

				if (attribute.HasMember("start_joint_depth"))
				{
					const Value& acc = attribute["start_joint_depth"];
					assert(acc.IsInt());
					goalComp->jointDepth = acc.GetInt();
				}

				if (attribute.HasMember("steps"))
				{
					const Value& acc = attribute["steps"];
					assert(acc.IsInt());
					goalComp->steps = acc.GetInt();
				}

				//Call this to end setup
				goalComp->DeserializeInit();
			}
		}
	}

	std::string loadFile(const char *path)
	{
		std::string out, line;
		std::ifstream in(path);
		if (in.is_open())
		{
			getline(in, line);
			while (in)
			{
				out += line + "\n";
				getline(in, line);
			}
			in.close();
			return out;
		}
		else
		{
			std::cout << "Failed to open file!" << std::endl;
			return "";
		}
	}
};


////////////////////////////
////	COMPONENTS		////
////////////////////////////
/*
class ComponentFactory 
{

public:
	//Fill the componentTypeMap hash
	ComponentFactory() 
	{
		//not sure if necessary but still
		for (int i = 0; i < COMPONENT_TYPES::COUNT; ++i) 
			ComponentTypeMap.push_back(0);

		ComponentTypeMap[TRANSFORM] = new Transform(0);
		ComponentTypeMap[RENDERER] = new Render(0);
		ComponentTypeMap[ANIMATION] = new AnimationComponent(0);
		ComponentTypeMap[PARTICLE_SYSTEM] = new ParticleSystemComponent(0);
		//------------------
		ComponentTypeMap[PATH_FOLLOW] = new AnimationComponent(0);
		ComponentTypeMap[IK_GOAL] = new IKGoalComponent(0);

		std::cout << "Created component clone factory" << std::endl;
	}

	~ComponentFactory()
	{
		std::cout << "Destroying component clone factory" << std::endl;
		for (auto comp : ComponentTypeMap)
			delete comp;
		ComponentTypeMap.clear();
	}

	BaseComponent* GetComponent(COMPONENT_TYPES type, GameObject *owner) 
	{
		//TODO - make this a safer call? (assert or something)
		BaseComponent *comp = ComponentTypeMap[type]->clone();
		comp->m_owner = owner;
		///comp->DeserializeInit(); //TODO check if it was ok to comment
		return comp;
	}

private:
	std::vector<BaseComponent*> ComponentTypeMap;
};
//*/

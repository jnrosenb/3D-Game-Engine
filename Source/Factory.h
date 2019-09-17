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

#include "GameObject.h"
#include "GameObjectManager.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "RendererComponent.h"
#include "TransformComponent.h"
extern GameobjectManager* goMgr;
extern ResourceManager* resMgr;
extern Renderer *renderer;


//PATHS TO DIFFERENT ASSET DIRECTORIES
static char const* abs_path = "C:\\Users\\Jose\\Desktop\\OpenGl_Framework\\Source\\";
static std::string const GameobjectsDir = "..\\Assets\\Archetypes\\Gameobjects\\";
static std::string const ScenesDir = "..\\Assets\\Archetypes\\Scenes\\";
static std::string const MaterialsDir = "..\\Assets\\Archetypes\\Materials\\";


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
		std::string path = abs_path + ScenesDir + sceneName; 
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
			for (rapidjson::Value::ConstMemberIterator itr2 = attribute.MemberBegin(); itr2 != attribute.MemberEnd(); ++itr2) 
			{
				std::string obj_path = itr2->value.GetString();
				LoadComponents(go, obj_path);
				goMgr->AddGameObjects(go);
			}
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
					renderer->sun.look = glm::vec4(ac1[0].GetFloat(), ac1[1].GetFloat(), ac1[2].GetFloat(), 0.0f);

					if (attribute.HasMember("Near")) 
					{
						const Value& ac3 = attribute["Near"];
						assert(ac3.IsFloat());
						renderer->sun.near = ac3.GetFloat();
					}

					if (attribute.HasMember("Far")) 
					{
						const Value& ac3 = attribute["Far"];
						assert(ac3.IsFloat());
						renderer->sun.far = ac3.GetFloat();
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
		std::string full_path = abs_path + GameobjectsDir + obj_path;
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

				assert(attribute.HasMember("model_name"));
				const Value& acc = attribute["model_name"];
				assert(acc.IsString());
				std::string modelPath = acc.GetString();

				assert(attribute.HasMember("load_model"));
				const Value& ac2 = attribute["load_model"];
				assert(ac2.IsBool());
				bool use_model = ac2.GetBool();

				assert(attribute.HasMember("primitive"));
				const Value& ac3 = attribute["primitive"];
				assert(ac3.IsString());
				std::string primitive = ac3.GetString();

				if (attribute.HasMember("diffuse_texture")) 
				{
					const Value& ac4 = attribute["diffuse_texture"];
					assert(ac4.IsString());
					std::string diffuseTexture = ac4.GetString();
					
					SDL_Surface *surf = resMgr->loadSurface(diffuseTexture);
					renderer->generateTextureFromSurface(surf, diffuseTexture);
					render->diffuseTexture = diffuseTexture;
				}

				render->use_loaded_mesh = use_model;
				if (use_model) 
					render->modelPath = modelPath;
				else
					render->primitive = primitive;

				if (attribute.HasMember("shader")) 
				{
					const Value& ac4 = attribute["shader"];
					assert(ac4.IsString());
					std::string shaderName = ac4.GetString();
					render->ShaderName = shaderName;
				}

				render->DeserializeInit();
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
#include "TransformComponent.h"
#include "RendererComponent.h"
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
		ComponentTypeMap[RENDERER]  = new Render(0);

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

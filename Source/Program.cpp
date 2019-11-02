//ENTRY POINT

//TODO - MOVE THESE INTO MACRO INCLUDE
#define USING_IMGUI			0
#define USING_GLEW			0
#define USING_GLAD			1

#define VIEWPORT_WIDTH						1280
#define VIEWPORT_HEIGHT						720
#define DEFERRED							1


#if USING_IMGUI
    #include "../External/Includes/ImGui/imgui.h"
    #include "../External/Includes/ImGui/imgui_impl_sdl.h"
    #include "../External/Includes/ImGui/imgui_impl_opengl3.h"
#endif

///#include "../External/Includes/GL/glew.h"
#include "../External/Includes/SDL2/SDL.h"
#include "../External/Includes/Glad_/Glad/glad.h"
#include <gl/GL.h>
#include "../External/Includes/glm/glm.hpp"
#include <iostream>


//Temp includes
#include "ForwardRenderer.h"
#include "DeferredRenderer.h"
#include "InputManager.h"
#include "GameObjectManager.h"
#include "FrameRateController.h"
#include  "ResourceManager.h"
#include "GameObject.h"
#include "Factory.h"
#include "RendererComponent.h"
#include "TransformComponent.h"

//Quaternion test
#include "Quaternion.h"

//Temporary globals
ComponentFactory factory;
Factory superFactory;
Renderer *renderer = NULL;
InputManager *inputMgr = NULL;
ResourceManager *resMgr = NULL;
GameobjectManager *goMgr = NULL;
FrameRateController *frc = NULL;


int main(int argc, char **argv)
{
    // SDL init and create a window---------------------------------------------------
    if( SDL_Init(SDL_INIT_VIDEO) != 0 )
    {
        return -1;
    }

    //We are in windows platform
	//TODO - handle opengl different versions
    char const *glsl_version = "#version 330 core";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	//MSAA EXPERIMENT
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


    //Windows properties
    const char *title = "Graphics Framework";
    int width = 1080;
    int height = 720;
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    //Create window with the properties set
    SDL_Window *window = SDL_CreateWindow( title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE );
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

	//Check later
    ///SDL_GL_SetSwapInterval(1); //VSync stuff

    ///GLEW stuff--------------------------------------------------------------------
    ///if (glewInit() != GLEW_OK)
    ///{
    ///    return 1;
    ///}
	///std::cout << glFramebufferTexture << std::endl;
	///std::cout << glFramebufferTexture2D << std::endl; 
	///std::cout << glBindFramebuffer << std::endl;
	//GLAD stuff
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Create the temp stuff
	#if DEFERRED
	renderer = new DeferredRenderer();
	#else
	renderer = new ForwardRenderer();
	#endif
	frc = new FrameRateController(60);
	inputMgr = new InputManager();
	goMgr = new GameobjectManager();
	resMgr = new ResourceManager();

	//Load the first scene
	superFactory.LoadScene("TestScene01.json");
	///superFactory.LoadScene("AnimationScene.json");
	renderer->init();


	//TEMP QUATERNION TEST
	///AuxMath::Quaternion q1 = AuxMath::Quaternion::QuaternionFromAA(90, glm::vec3(1, 0, 0));
	///AuxMath::Quaternion q1_Inverse   = q1.Inverse();
	///AuxMath::Quaternion q1_conjugate = q1.Conjugate();
	///AuxMath::Quaternion q2 = q1 * q1_Inverse;
	///AuxMath::Quaternion q3 = q1 * q1_conjugate;
	///q1.print("q1");
	///q1_Inverse.print("Inverse");
	///q1_conjugate.print("Conjugate");
	///q2.print("times conjugate");
	///q3.print("times inverse  ");
	///
	///glm::vec3 r;
	///r = AuxMath::Quaternion::Rotate1(90, glm::vec3(1, 0, 0), glm::vec3(1, 0, 0));
	///r = AuxMath::Quaternion::Rotate1(90, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
	///r = AuxMath::Quaternion::Rotate1(90, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
	///
	///r = AuxMath::Quaternion::Rotate1(120, glm::vec3(1, 1, 1), glm::vec3(1, 0, 0));
	///r = AuxMath::Quaternion::Rotate1(120, glm::vec3(1, 1, 1), glm::vec3(0, 1, 0));
	///r = AuxMath::Quaternion::Rotate1(120, glm::vec3(1, 1, 1), glm::vec3(0, 0, 1));
	///
	/////Same with matrix
	///glm::mat4 R = AuxMath::Quaternion::QuaternionFromAA(120, glm::vec3(1, 1, 1)).GetRotationMatrix();
	///glm::vec4 r2 = R * glm::vec4(0, 0, 1, 1);
	///
	///r = AuxMath::Quaternion::Rotate1(240, glm::vec3(1, 1, 1), glm::vec3(1, 0, 0));
	///r = AuxMath::Quaternion::Rotate1(240, glm::vec3(1, 1, 1), glm::vec3(0, 1, 0));
	///r = AuxMath::Quaternion::Rotate1(240, glm::vec3(1, 1, 1), glm::vec3(0, 0, 1));
	///
	///r = AuxMath::Quaternion::Rotate1(360, glm::vec3(1, 1, 1), glm::vec3(1, 0, 0));
	///r = AuxMath::Quaternion::Rotate1(360, glm::vec3(1, 1, 1), glm::vec3(0, 1, 0));
	///r = AuxMath::Quaternion::Rotate1(360, glm::vec3(1, 1, 1), glm::vec3(0, 0, 1));

	

    #if USING_IMGUI
        //SETUP IMGUI CONTEXT----------------------------------------------------------
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplSDL2_InitForOpenGL(window, context);
        ImGui_ImplOpenGL3_Init(glsl_version);
    #endif

    //Boolean for exiting main loop
    bool done = false;

    //ERASE LATER
    bool showDemoWin = 1;

    //MAIN LOOP
	while (!done)
	{
		frc->FrameStart();

		//EVENT LOOP
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{

			#if USING_IMGUI
			ImGui_ImplSDL2_ProcessEvent(&event);
			#endif

			switch (event.type)
			{
			case SDL_QUIT:
				done = true;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					done = true;
				break;
			case SDL_DROPFILE:
				char* dropped_filedir = event.drop.file;
				superFactory.unloadScene();
				superFactory.LoadScene(dropped_filedir, true);
				SDL_free(dropped_filedir);
				break;
			};
		}

		//UPDATE------------------------------
		#if USING_IMGUI
		// Start the Dear ImGui frame //What is ImGui frame?
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		// 1. Show big demo window. Sample code in ImGui::ShowDemoWindow()
		/*if (showDemoWin)
			ImGui::ShowDemoWindow(&showDemoWin);*/

		ImGui::Begin("Controller");


		ImGui::End();

		//RENDER - BLIT BITMAP------------------
		ImGui::Render();
		#endif

		//frameRate controller
		float dt = frc->getFrameTime() / 1000.0f;
		//std::cout << static_cast<int>(1 / dt) << std::endl;

		//Update stuff
		inputMgr->update(dt);
		goMgr->Update(dt);
		renderer->Update(dt);

		//RENDER
		renderer->Draw();
        
        #if USING_IMGUI
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        #endif

		//TODO - find out more about this
        SDL_GL_SwapWindow(window);
		frc->FrameEnd();
    }

	//Custom stuff cleanup
	delete renderer;
	delete inputMgr;
	delete resMgr;
	delete frc;
	delete goMgr;

    // clean up
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

	//Imgui cleanup
	#if USING_IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	#endif

    return 0;
}
///HEADER

#include "ImGuiManager.h"
#include <iostream>


ImGuiManager::ImGuiManager()
{
	std::cout << "ImGuiManager constructor." << std::endl;
}

ImGuiManager::~ImGuiManager()
{
	std::cout << "ImGuiManager destructor." << std::endl;

	//Imgui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::Init(SDL_Window *win, SDL_GLContext *context, char const *glsl_version)
{
	std::cout << "ImGuiManager Init!" << std::endl;
	this->window = win;

	//SETUP IMGUI CONTEXT
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, *context);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void ImGuiManager::Update()
{
	/*
	//Start the Dear ImGui frame - What is ImGui frame?
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	//Sample code in ImGui::ShowDemoWindow()
	//if (showDemoWin)
	//	ImGui::ShowDemoWindow(&showDemoWin);

	
	ImGui::Begin("Controller");

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	ImGui::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange", (unsigned int *)&io.ConfigFlags, ImGuiConfigFlags_NoMouseCursorChange);
	//ImGui::SameLine();
	ImGui::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
	ImGui::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
	ImGui::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
	ImGui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);


	static char str0[128] = "Hello, world!";
	ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));
	//ImGui::SameLine();

	///static char str1[128] = "";
	///ImGui::InputTextWithHint("input text (w/ hint)", "enter text here", str1, IM_ARRAYSIZE(str1));

	///static int i0 = 123;
	///ImGui::InputInt("input int", &i0);
	//ImGui::SameLine(); 

	static int i1 = 0;
	ImGui::SliderInt("slider int", &i1, -1, 3);
	ImGui::SameLine();

	static float f1 = 0.123f, f2 = 0.0f;
	ImGui::SliderFloat("slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");
	ImGui::SliderFloat("slider float (curve)", &f2, -10.0f, 10.0f, "%.4f", 2.0f);

	static float angle = 0.0f;
	ImGui::SliderAngle("slider angle", &angle);

	static float f0 = 0.001f;
	ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

	static double d0 = 999999.00000001;
	ImGui::InputDouble("input double", &d0, 0.01f, 1.0f, "%.8f");

	static float f10 = 1.e10f;
	ImGui::InputFloat("input scientific", &f10, 0.0f, 0.0f, "%e");
	//ImGui::SameLine();

	static float vec4a[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
	ImGui::InputFloat3("input float3", vec4a);

	ImGui::End();
	//*/
}


void ImGuiManager::ClothUpdate(float *mass, float *damping, bool *useG, bool *useWind, 
	float *windIntensity, float *restLen, float *coeff1, float *coeff2, float *coeff3, glm::vec4& windDir)
{
	//Start the Dear ImGui frame - What is ImGui frame?
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	ImGui::Begin("Controller");
	ImGui::Text("To move the cloth around, use the directional keys.");
	ImGui::SliderFloat("Mass", mass, 0.1f, 5.0f, "ratio = %.3f");
	ImGui::SliderFloat("Damping", damping, 0.0f, 1.0f, "%.4f", 2.0f);

	ImGui::Checkbox("Activate gravity", useG);

	ImGui::Checkbox("Activate wind", useWind);
	ImGui::SliderFloat("Wind Intensity", windIntensity, 0.0f, 2.0f, "%.4f");
	float x = windDir.x;
	float y = windDir.y;
	float z = windDir.z;
	ImGui::SliderFloat("Wind direction x", &x, 0.0f, 1.0f, "%.4f");
	ImGui::SliderFloat("Wind direction y", &y, 0.0f, 1.0f, "%.4f");
	ImGui::SliderFloat("Wind direction z", &z, 0.0f, 1.0f, "%.4f");
	windDir = glm::vec4(x, y, z, 0.0f);

	ImGui::SliderFloat("Rest Length", restLen, 0.0f, 1.0f, "%.4f");

	ImGui::SliderFloat("kCoeff_straight", coeff1, 0.0f, 5.0f, "%.4f");
	ImGui::SliderFloat("kCoeff_diagonal", coeff2, 0.0f, 5.0f, "%.4f");
	ImGui::SliderFloat("kCoeff_Bending", coeff3, 0.0f, 5.0f, "%.4f");

	ImGui::End();

}


void ImGuiManager::Draw()
{
	//RENDER - BLIT BITMAP
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
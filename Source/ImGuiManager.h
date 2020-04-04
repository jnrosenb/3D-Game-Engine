///HEADER

#pragma  once

#include "../External/Includes/ImGui/imgui.h"
#include "../External/Includes/ImGui/imgui_impl_sdl.h"
#include "../External/Includes/ImGui/imgui_impl_opengl3.h"
#include "../External/Includes/SDL2/SDL.h"
#include "../External/Includes/Glad_/Glad/glad.h"
#include <gl/GL.h>
#include <string>
#include "../External/Includes/glm/glm.hpp"


class ImGuiManager
{
public:
	ImGuiManager();
	virtual ~ImGuiManager();

	void Init(SDL_Window *win, SDL_GLContext *context, char const *glsl_version);
	void Update(float dt, int fps);
	void Draw();

	//Temporary
	void ClothUpdate(float *mass, float *damping, bool *useG, bool *useWind, 
		float *windIntensity, float *restLen, float *coeff1, float *coeff2, float *coeff3, 
		glm::vec4& windDir);

private:
	SDL_Window *window;
	
	//FPS window
	int windowSize = 16;
	int accumIndex = 0;
	float accum[16] = {};
};
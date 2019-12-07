///HEADER STUFF

#pragma once

#include "Paths.h"
#include <Windows.h>


EnginePaths::EnginePaths() 
{
	SetupProjectPaths();
}

void EnginePaths::SetupProjectPaths() 
{
	//Windows stuff
	size_t const maxPath = 512;
	char szPath[maxPath];
	if (GetModuleFileName(NULL, szPath, maxPath))
	{
		std::string path = static_cast<std::string>(szPath);
		size_t index = path.find_last_of("/\\");
		ExeDir = path.substr(0, index);
		index = ExeDir.find_last_of("/\\");

		//Base directory of project
		BaseDir = ExeDir.substr(0, index) + "\\";

		//This is the path for the EXE
		ExeDir += "\\";

		//This is the path for the Shader (code)
		ShaderDir = ExeDir + "Shaders\\";

		//This is the path for the Assets
		AssetsDir = BaseDir + "Assets\\";

		//This is the path for the Source (code)
		SourceDir = BaseDir + "Source\\";
	}
}

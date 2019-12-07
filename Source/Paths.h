///HEADER STUFF

#pragma once

#include <string>

//TODO - This should be a namespace!!!
class EnginePaths
{
public:
	std::string BaseDir;
	std::string ExeDir;
	std::string AssetsDir;
	std::string SourceDir;
	std::string ShaderDir;

public:
	EnginePaths();
	void SetupProjectPaths();
};
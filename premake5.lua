dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

dependencies.load()

workspace "t6-gsc-utils"
	location "./build"
	objdir "%{wks.location}/obj/%{cfg.buildcfg}"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
	targetname "%{prj.name}"

	configurations { "Debug", "Release", }

	language "C++"
	cppdialect "C++20"

	architecture "x86"

	systemversion "latest"
	symbols "On"
	staticruntime "On"
	editandcontinue "Off"
	warnings "Extra"
	characterset "ASCII"

	flags
	{
		"NoIncrementalLink",
		"MultiProcessorCompile",
	}
	
	filter "configurations:Release"
		optimize "Full"
		defines { "NDEBUG" }
	filter {}

	filter "configurations:Debug"
		optimize "Debug"
		defines { "DEBUG", "_DEBUG" }
	filter {}

	startproject "t6-gsc-utils"

	project "t6-gsc-utils"
		kind "SharedLib"
		language "C++"

		files 
		{
			"./src/**.h",
			"./src/**.hpp",
			"./src/**.cpp",
		}

		includedirs 
		{
			"%{prj.location}/src",
			"./src",
		}

		resincludedirs 
		{
			"$(ProjectDir)src"
		}
	
		pchheader "stdinc.hpp"
		pchsource "src/stdinc.cpp"
		buildoptions { "/Zm100 -Zm100" }

		dependencies.imports()

	group "Dependencies"
	dependencies.projects()
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
	startproject "t6-gsc-utils"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"

	configurations { "Debug", "Release" }

	language "C++"
	cppdialect "C++20"

	architecture "x86"
	platforms "Win32"

	disablewarnings 
	{
		"4324",
		"6031",
		"6053",
		"26495",
		"26812",
	}

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
		"No64BitChecks",
	}

	filter "platforms:Win*"
		defines { "_WINDOWS", "WIN32" }
	filter {}

	filter "configurations:Release"
		optimize "Full"
		defines { "NDEBUG" }
		flags { "LinkTimeOptimization" }
	filter {}

	filter "configurations:Debug"
		optimize "Debug"
		defines { "DEBUG", "_DEBUG" }
	filter {}

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
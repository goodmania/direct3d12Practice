workspace "D3D12Practice"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}

project "D3D12Practice"
	location "D3D12Practice"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/../assets/shader/**.hlsl"
	}

	includedirs
	{
		"%{prj.name}/include"
	}

	libdirs 
	{
		
	}

	links
	{
		
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			-- WINDOWS
		}

	filter "configurations:Debug"
		defines "VOE_DEBUG"
		buildoptions "/MTd"
		symbols "On"

	filter "configurations:Release"
		defines "VOE_RELEASE"
		buildoptions "/MD"
		optimize "On"

	filter "configurations:Dist"
		defines "VOE_DIST"
		buildoptions "/MD"
		optimize "On"

	filter { "files:**.hlsl" }
    flags "ExcludeFromBuild"
    shadermodel "6.0"

    filter { "files:**_p.hlsl" }
   	removeflags "ExcludeFromBuild"
   	shadertype "Pixel"
   	shaderentry "ForPixel"

	filter { "files:**_v.hlsl" }
   	removeflags "ExcludeFromBuild"
   	shadertype "Vertex"
   	shaderentry "ForVertex"

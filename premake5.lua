--GLFW uses different static libs depending on visual studio version.
--Because of this the project only supports the currently included VS versions.
glfwTargetVcVersion = "null"
if _ACTION == "vs2012" then
	glfwTargetVcVersion = "lib-vc2012"
elseif _ACTION == "vs2013" then
	glfwTargetVcVersion = "lib-vc2013"
elseif _ACTION == "vs2015" then
	glfwTargetVcVersion = "lib-vc2015"
elseif _ACTION == "vs2017" then
	glfwTargetVcVersion = "lib-vc2017"
elseif _ACTION == "vs2019" then
	glfwTargetVcVersion = "lib-vc2019"
elseif _ACTION == "vs2022" then
	glfwTargetVcVersion = "lib-vc2022"
end

if glfwTargetVcVersion == "null" then
	print("VS versions 12, 13, 15, 17, 19, 22 are currently the only actions supported. Given action: " .. _ACTION)
	os.exit()
end

--Libs--
--Static libs
GLFW_LABEL = "GLFW"
GLFW_DIR = "%{wks.location}/Dough/Dough/libs/glfw-3.3.8.bin.WIN64/"

VULKAN_LABEL = "Vulkan"
VULKAN_DIR = "%{wks.location}/Dough/Dough/libs/Vulkan/"

--Build from source libs
GLM_LABEL = "GLM"
GLM_DIR = "%{wks.location}/Dough/Dough/libs/glm/"

STB_LABEL = "STB"
STB_DIR = "%{wks.location}/Dough/Dough/libs/stb/"

IMGUI_LABEL = "ImGUI"
IMGUI_DIR = "%{wks.location}/Dough/Dough/libs/imgui/"

TINY_OBJ_LOADER_LABEL = "TinyObjLoader"
TINY_OBJ_LOADER_DIR = "%{wks.location}/Dough/Dough/libs/tinyobjloader/"

TRACY_LABEL = "Tracy"
TRACY_DIR = "%{wks.location}/Dough/Dough/libs/tracy/"

libIncludeDirs = {
	[GLFW_LABEL] = GLFW_DIR .. "include/",
	[VULKAN_LABEL] = VULKAN_DIR .. "include/"
}

configurations { "DEBUG", "TRACING", "RELEASE" }
architecture("x86_64")

defines {
	--Windows
	"NOMINMAX",

	--Vulkan
	"VK_USE_PLATFORM_WIN32_KHR",

	--GLFW
	"GLFW_INCLUDE_VULKAN",

	--GLM
	"GLM_ENABLE_EXPERIMENTAL",
	"GLM_FORCE_DEPTH_ZERO_TO_ONE",
	"GLM_FORCE_USE_RIGHT_HANDED",
	"GLM_FORCE_RADIANS",

	--STB
	"STB_IMAGE_IMPLEMENTATION",
	
	--Tiny Obj Loader
	"TINYOBJLOADER_IMPLEMENTATION"
}
	
ENGINE_PROJ_NAME = "DoughEngine"
project(ENGINE_PROJ_NAME)
	kind("StaticLib")
	language("C++")
	cppdialect("C++17")

	includedirs { libIncludeDirs, "Dough/src/", "Dough/libs/" }
	libdirs { GLFW_DIR .. glfwTargetVcVersion, VULKAN_DIR }
	links { "glfw3", "vulkan-1" }

	--NOTE:: Architecture is placed before config because only one architecture is used. This results in less folders being made.
	outputDir = "%{cfg.architecture}/%{cfg.buildcfg}/"
	targetdir(outputDir .. "final/")
	objdir(outputDir .. "inter/")

	files {
		--Engine
		"Dough/src/dough/**.h",
		"Dough/src/dough/**.cpp",
		"Dough/src/editor/**.h",
		"Dough/src/editor/**.cpp",

		--Libs
		STB_DIR .. "**.h",
		GLM_DIR .. "**.hpp",
		IMGUI_DIR .. "**.h",
		IMGUI_DIR .. "**.cpp",
		TINY_OBJ_LOADER_DIR .. "tinyobjloader.h"
	}

		tracyFiles = {
		TRACY_DIR .. "public/tracy/Tracy.hpp",
		TRACY_DIR .. "public/TracyClient.cpp"
	}
	files (tracyFiles)

	filter("configurations:DEBUG")
		defines { "DEBUG", "_DEBUG" }
		symbols("On")
		removefiles(tracyFiles)

	filter("configurations:TRACING")
		defines { "NDEBUG", "_NDEBUG", "_TRACING", "TRACY_ENABLE", "_CRT_SECURE_NO_WARNINGS" } --Tracy uses some functions that cause warnings
		optimize("On")

	filter("configurations:RELEASE")
		defines { "NDEBUG", "_NDEBUG", "_RELEASE" }
		optimize("On")
		removefiles(tracyFiles)

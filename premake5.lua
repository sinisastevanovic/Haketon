workspace "HaketonApp"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "HaketonApp"

outputdir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

include "HaketonExternal.lua"
include "HaketonApp"
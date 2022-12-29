workspace "HaketonApp"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "HaketonApp"

outputdir = "%{cfg.platform}-%{cfg.buildcfg}"

include "HaketonExternal.lua"
include "HaketonApp"
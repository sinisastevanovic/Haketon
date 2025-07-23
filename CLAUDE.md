# Haketon Engine - Project Overview

Haketon is a C++ game engine with an integrated editor, built using modern graphics APIs and a custom ECS (Entity-Component-System) architecture.

## Project Structure

### Core Projects
- **Haketon** (`/Haketon/`) - Main engine library containing core systems
- **HaketonEditor** (`/HaketonEditor/`) - Visual editor application built on top of the engine
- **Sandbox** (`/Sandbox/`) - Test/example application for engine features
- **HaketonHeaderTool** (`/HaketonHeaderTool/`) - C# tool for code generation and reflection

### Key Directories

#### Engine Core (`/Haketon/src/Haketon/`)
- **Core/** - Application framework, logging, reflection, serialization
- **Renderer/** - Graphics rendering systems (OpenGL/Vulkan support)
- **Scene/** - ECS architecture with entities, components, and scene management
- **Events/** - Event system for input and application events
- **Math/** - Mathematical utilities and helper functions
- **ImGui/** - Dear ImGui integration for UI
- **Platform/** - Platform-specific implementations (Windows, OpenGL)

#### Editor (`/HaketonEditor/src/`)
- **Panels/** - Editor UI panels (Scene Hierarchy, Console, Property Editor)
- **DetailCustomization/** - Custom property editors for components
- **GeneratedFiles/** - Auto-generated reflection code

## Build System

- **Build Tool**: Premake5 (Lua-based build configuration)
- **Platform**: Windows (Visual Studio)
- **Architecture**: x86_64
- **Configurations**: Debug, Release, Dist

### Key Dependencies
- **Graphics**: GLFW, Glad, OpenGL/Vulkan
- **UI**: Dear ImGui, ImGuizmo
- **Math**: GLM
- **ECS**: EnTT
- **Serialization**: RapidJSON
- **Reflection**: RTTR (Run Time Type Reflection)
- **Image**: stb_image
- **Shader Compilation**: shaderc, SPIRV-Cross

## Common Development Tasks

### Building the Project
```bash
# Generate Visual Studio project files
scripts/Win-GenProjects.bat

# Then build in Visual Studio or use:
# msbuild Haketon.sln /p:Configuration=Debug
```

### Running the Editor
The main entry point is `HaketonEditor` - this should be set as the startup project.

### Scene System
- Scenes are serialized to `.haketon` files in `/HaketonEditor/assets/scenes/`
- The editor supports loading/saving scenes and entity manipulation
- Components are automatically reflected using the header tool

### Shader Development
- Shaders are located in `/assets/shaders/` with `.glsl` extension
- The engine supports both OpenGL and Vulkan shader compilation
- Cached shaders are stored in `/assets/cache/shader/`

### Adding New Components
1. Create component class in `/Haketon/src/Haketon/Scene/Components/`
2. Add reflection macros for property editor support
3. Run header tool to generate reflection code
4. Register in the component system

### Editor Panels
- Scene Hierarchy: Entity management and selection
- Console: Debug output and logging
- Property Editor: Component property editing with customizations

## Development Notes

### Code Generation
The project uses a custom header tool (`HaketonHeaderTool`) written in C# for:
- Automatic reflection code generation
- Component registration
- Property editor metadata

### Architecture Patterns
- ECS (Entity-Component-System) for game objects
- Module system for editor functionality
- Event-driven architecture for input and application events
- Reflection system for editor property binding

### Graphics Pipeline
- Supports both OpenGL and Vulkan rendering backends
- 2D rendering system with batch processing
- Framebuffer system for editor viewports
- Shader hot-reloading support

## Current Status
Based on recent commits, the project is actively maintained with recent updates to:
- spdlog (1.15.3) for logging
- ImGui (1.92.1) for UI
- GLFW (3.4) for windowing
- UTF-8 compiler support

The editor includes functional scene hierarchy, property editing, and basic 2D rendering capabilities.
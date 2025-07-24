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

### Adding Custom Property Editors (DetailCustomizations)

The editor uses a PropertyEditorModule system to register custom UI controls for specific types. This allows you to create specialized editors for any type (components, properties, math types, etc.).

#### Creating a Property Detail Customization

**1. Create Header File** (`/HaketonEditor/src/Panels/DetailCustomization/YourTypeDetailCustomization.h`):
```cpp
#pragma once
#include "IDetailCustomization.h"
#include <YourIncludedType.h>

class YourTypePropertyDetailCustomization : public IPropertyDetailCustomization
{
public:
    bool CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly = false) override;

private:
    // Your helper functions
    bool DrawCustomControl(YourType& value);
};
```

**2. Create Implementation** (`/HaketonEditor/src/Panels/DetailCustomization/YourTypeDetailCustomization.cpp`):
```cpp
#include "YourTypeDetailCustomization.h"
#include <imgui/imgui.h>

bool YourTypePropertyDetailCustomization::CustomizeDetails(rttr::variant& Value, rttr::property& Property, bool bReadOnly)
{
    if(bReadOnly)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);  
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.25f);
    }

    // Handle metadata (e.g., degrees conversion, min/max values)
    bool someMetadata = Property.get_metadata("SomeFlag") ? true : false;
    YourType value = Value.get_value<YourType>();
    
    bool valueChanged = DrawCustomControl(value);
    
    if(valueChanged && !bReadOnly)
    {
        Value = value; // Update the variant
    }

    if(bReadOnly)
    {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }

    return valueChanged && !bReadOnly;
}
```

**3. Register the Customization** (in `/HaketonEditor/src/HaketonEditorApp.cpp`):
```cpp
// Add include
#include "Panels/DetailCustomization/YourTypeDetailCustomization.h"

// In HaketonEditor constructor, after PropertyEditor setup:
PropertyEditor->RegisterPropertyDetailCustomization("YourTypeName", []()
{
    return CreateRef<YourTypePropertyDetailCustomization>();
});
```

**4. Type Name Registration**:
- For RTTR types, use `ValueType.get_name().to_string()` to see the exact type name
- Common examples:
  - `"Vector3"` for glm::vec3
  - `"Vector4"` for glm::vec4  
  - `"std::basic_string<char,std::char_traits<char>,std::allocator<char> >"` for std::string
  - `"YourComponentName"` for custom components

#### Component Detail Customization (Alternative)

For full component customization (not just properties), use `IDetailCustomization`:

```cpp
class YourComponentDetailCustomization : public IDetailCustomization
{
public:
    void CustomizeDetails(rttr::instance Instance) override;
};

// Register with:
PropertyEditor->RegisterDetailCustomization("YourComponentName", []()
{
    return CreateRef<YourComponentDetailCustomization>();
});
```

#### Examples in Codebase
- **Vec3PropertyDetailCustomization**: Custom XYZ controls with reset buttons
- **TagComponentDetailCustomization**: Simple string input for tag editing

#### Best Practices
- Handle `bReadOnly` parameter for disabled states
- Support property metadata (Min, Max, Degrees, Tooltip, etc.)
- Return `true` only when values actually change
- Use proper ImGui ID management to avoid conflicts
- Follow existing UI patterns and styling

## Development Notes

### While developing
- Don't add comments except for critical caveats. Rely on self-explanatory code.
- When adding new files, remember to add them to the corresponding premake5.lua file if needed.

### Code Generation
The project uses a custom header tool (`HaketonHeaderTool`) written in C# for:
- Automatic reflection code generation
- Component registration
- Property editor metadata
- It uses an AST system for parsing C++ code
- Tokenization in `Tokenizer.cs` and `Token.cs`
- AST node definition in `AST.cs`
- Parsing in `Parser.cs`
- Finally code generation in `CodeGenerator.cs`
- Has unit tests, so if you add functionality, make sure to also add unit tests for that

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
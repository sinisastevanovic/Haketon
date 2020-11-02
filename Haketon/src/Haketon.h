#pragma once

// For use by Haketon applications
// TODO: Use Armadillo instead of glm? Profile differences!
#include "Haketon/Core/Application.h"
#include "Haketon/Core/Layer.h"
#include "Haketon/Core/Log.h"

#include "Haketon/Core/Timestep.h"

#include "Haketon/Core/Input.h"
#include "Haketon/Core/KeyCodes.h"
#include "Haketon/Core/MouseCodes.h"
#include "Haketon/Renderer/OrthographicCameraController.h"

#include "Haketon/ImGui/ImGuiLayer.h"

#include "Haketon/Scene/Scene.h"
#include "Haketon/Scene/Components.h"
#include "Haketon/Scene/ScriptableEntity.h"
#include "Haketon/Scene/Entity.h"

// ----Renderer---------------------
#include "Haketon/Renderer/Renderer.h"
#include "Haketon/Renderer/RenderCommand.h"
#include "Haketon/Renderer/Renderer2D.h"

#include "Haketon/Renderer/Buffer.h"
#include "Haketon/Renderer/Shader.h"
#include "Haketon/Renderer/Framebuffer.h"
#include "Haketon/Renderer/Texture.h"
#include "Haketon/Renderer/SubTexture2D.h"
#include "Haketon/Renderer/VertexArray.h"

#include "Haketon/Renderer/OrthographicCamera.h"
// ----------------------------------
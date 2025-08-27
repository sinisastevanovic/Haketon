#include "hkpch.h"
#include "Scene.h"
#include "Haketon/Renderer/Renderer2D.h"
#include "Components.h"
#include "Entity.h"
#include "Components/TagComponent.h"

#include <glm/glm.hpp>

#include "Components/CameraComponent.h"
#include "Haketon/Math/Math.h"
#include "SceneCamera.h"
#include "Components/ParentComponent.h"
#include "Components/UUIDComponent.h"
#include "Haketon/Asset/AssetManager.h"
#include "Haketon/Core/Serialization/RapidJsonDeserializer.h"
#include "Haketon/Core/Serialization/RapidJsonSerializer.h"
#include "Scripts/CameraController.h"
#include "Scripts/TestScript.h"

namespace Haketon
{
    Scene::~Scene()
    {
        DestroyAllEntities();
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };

        // TODO: Do I want every entity to have a transform component? If not, also add a check to SceneSerializer
        entity.AddComponent<UUIDComponent>();
        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<WorldTransformComponent>();
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    void Scene::DestroyAllEntities()
    {
        auto view = m_Registry.view<entt::entity>();
        for (auto entityID : view)
        {
            if(m_Registry.valid(entityID))
                m_Registry.destroy(entityID);
        }
    }

    Entity Scene::GetEntityByUUID(const UUID& uuid)
    {
        auto view = m_Registry.view<UUIDComponent>();
        for (auto [entityID, uuidComp] : view.each())
        {
            if (uuidComp.Uuid == uuid)
                return Entity{ entityID, this };
        }
        return Entity{ entt::null, this };
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
        if (!m_IsPaused)
        {
            m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
           {
              if(!nsc.Instance && nsc.InstantiateScript)
              {
                  nsc.Instance = nsc.InstantiateScript();
                  if(nsc.Instance)
                  {
                      nsc.Instance->m_Entity = Entity{ entity, this };
                      nsc.Instance->SetData(nsc.Data);
                      nsc.Instance->OnCreate();
                  }
                  else
                  {
                      HK_CORE_ERROR("Failed to instantiate script: {0}", nsc.ScriptClassName);
                  }
              }

               if(nsc.Instance)
                   nsc.Instance->OnUpdate(ts);
           });
        }
        
        // Render 2D
        Ref<Camera> primaryCamera = nullptr;
        glm::mat4 cameraTransform;
        auto cameraCompGroup = m_Registry.group<CameraComponent>(entt::get<TransformComponent>); // TODO: WHY CANT I USE TWO GROUPS??
        for(auto entity : cameraCompGroup)
        {
            auto [transform, camera] = cameraCompGroup.get<TransformComponent, CameraComponent>(entity);
            if(camera.Primary)
            {
                primaryCamera = camera.Camera;
                cameraTransform = transform.GetTransform();
                break;
            }
        }

        if(primaryCamera)
        {
            Renderer2D::BeginScene(*primaryCamera, cameraTransform);

            /*auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for(auto entity : group)
            {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

                Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
            }*/
            DrawScene();

            Renderer2D::EndScene();
        }       
    }
    
    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& Camera)
    {
        Renderer2D::BeginScene(Camera);

        /*auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for(auto entity : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

            Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
        }*/
        DrawScene();

        Renderer2D::EndScene();    
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        // Resize cameras
        auto view = m_Registry.view<CameraComponent>();
        for(auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if(!cameraComponent.FixedAspectRatio)
                cameraComponent.Camera->SetViewportSize(width, height);
        }
    }

    Entity Scene::GetPrimaryCameraEntity()
    {
        auto View = m_Registry.view<CameraComponent>();
        for(auto CurrEntity : View)
        {
            const auto& Camera = View.get<CameraComponent>(CurrEntity);
            if(Camera.Primary)
                return Entity{CurrEntity, this};
        }

        return { entt::null, nullptr };
    }

    Ref<Scene> Scene::Copy(Scene* sceneToCopy)
    {
        if (!sceneToCopy)
            return nullptr;
        
        Ref<Scene> newScene = AssetManager::CreateTransientAsset<Scene>();
        newScene->SetName(sceneToCopy->GetName());

        RapidJsonSerializer rs;
        rs.SerializeScene(sceneToCopy);

        RapidJsonDeserializer rd;
        rd.Parse(rs.GetString());
        rd.DeserializeScene(newScene.get());
        return newScene;
    }

    Ref<Scene> Scene::Create(const std::filesystem::path& filePath)
    {
        Ref<Scene> scene = CreateRef<Scene>();
        RapidJsonDeserializer rd;
        rd.ParseFile(filePath.string());
        rd.DeserializeScene(scene.get());

        return scene;
    }

    void Scene::OnComponentAdded(Entity entity, Component* component)
    {
        // TODO: This sucks!

        if(m_ViewportWidth > 0 && m_ViewportHeight > 0)
        {
            CameraComponent* cameraComp = dynamic_cast<CameraComponent*>(component);
            if(cameraComp)
                cameraComp->Camera->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
        }      
    }

    void Scene::DrawScene()
    {
        // Update transforms
        auto view = m_Registry.view<TransformComponent, WorldTransformComponent>(entt::exclude<ParentComponent>);
        for(auto entity : view)
        {
            auto [transform, worldTransform] = view.get<TransformComponent, WorldTransformComponent>(entity);

            worldTransform.Transform = transform.GetTransform();
        }

        auto childGroup = m_Registry.group<ParentComponent>(entt::get<TransformComponent, WorldTransformComponent>);
        childGroup.sort<ParentComponent>([](const auto& lhs, const auto& rhs)
        {
            return lhs.Depth < rhs.Depth;
        });

        for (auto entity : childGroup)
        {
            auto [parent, transform, worldTransform] = childGroup.get<ParentComponent, TransformComponent, WorldTransformComponent>(entity);

            if (m_Registry.valid(parent.Parent))
            {
                const auto& parentWorld = m_Registry.get<WorldTransformComponent>(parent.Parent);
                worldTransform.Transform = parentWorld.Transform * transform.GetTransform();
            }
            else
            {
                worldTransform.Transform = transform.GetTransform();
                m_Registry.remove<ParentComponent>(entity);
            }
        }

        // Draw Sprites
        auto spriteView = m_Registry.view<WorldTransformComponent, SpriteRendererComponent>();
        for (auto entity : spriteView)
        {
            auto [worldTransform, sprite] = spriteView.get<WorldTransformComponent, SpriteRendererComponent>(entity);
            Renderer2D::DrawSprite(worldTransform.Transform, sprite, (int)entity);
        }
    }

    uint32_t Scene::CalculateChildDepth(entt::entity parentEntity)
    {
        if (auto* parent = m_Registry.try_get<ParentComponent>(parentEntity))
        {
            if (m_Registry.valid(parent->Parent))
            {
                return parent->Depth + 1;
            }
        }
        return 1;
    }

    void Scene::Attach(entt::entity child, entt::entity parent)
    {
        auto* existingParent = m_Registry.try_get<ParentComponent>(child);
        if (existingParent && existingParent->Parent == parent)
        {
            return;
        }
        
        // Check if the new parent is attached to us...
        if (auto* parentsParent = m_Registry.try_get<ParentComponent>(parent))
        {
            if (parentsParent->Parent == child)
                return;
        }
        
        // Get the current world transform of the child before attaching
        auto* childWorldTransform = m_Registry.try_get<WorldTransformComponent>(child);
        auto* childTransform = m_Registry.try_get<TransformComponent>(child);
        
        if (childWorldTransform && childTransform)
        {
            glm::mat4 currentWorldTransform = childWorldTransform->Transform;
            
            // Get the parent's world transform
            auto* parentWorldTransform = m_Registry.try_get<WorldTransformComponent>(parent);
            if (parentWorldTransform)
            {
                // Calculate the new local transform: local = inverse(parentWorld) * childWorld
                glm::mat4 newLocalTransform = glm::inverse(parentWorldTransform->Transform) * currentWorldTransform;
                
                // Decompose and apply to the child's TransformComponent
                FVec3 translation, rotation, scale;
                Math::DecomposeTransform(newLocalTransform, translation, rotation, scale);
                
                childTransform->Position = translation;
                childTransform->Rotation = rotation;
                childTransform->Scale = scale;
            }
        }
        
        // Now attach the child to the parent
        uint32_t depth = CalculateChildDepth(parent);
        UUID parentHandle = m_Registry.get<UUIDComponent>(parent).Uuid;
        if (existingParent)
        {
            existingParent->Parent = parent;
            existingParent->Depth = depth;
            existingParent->ParentHandle = parentHandle;
        }
        else
        {
            m_Registry.emplace<ParentComponent>(child, parent, depth, parentHandle);
        }
    }

    void Scene::Detach(entt::entity child)
    {
        if (auto* parent = m_Registry.try_get<ParentComponent>(child))
        {
            // Preserve world transform when detaching
            auto* childWorldTransform = m_Registry.try_get<WorldTransformComponent>(child);
            auto* childTransform = m_Registry.try_get<TransformComponent>(child);
        
            if (childWorldTransform && childTransform)
            {
                // The current world transform becomes the new local transform
                glm::mat4 worldTransform = childWorldTransform->Transform;
            
                // Decompose and apply to the child's TransformComponent
                FVec3 translation, rotation, scale;
                Math::DecomposeTransform(worldTransform, translation, rotation, scale);
            
                childTransform->Position = translation;
                childTransform->Rotation = rotation;
                childTransform->Scale = scale;
            }
        
            m_Registry.remove<ParentComponent>(child);
            // TODO: What to do with children of this comp??
        }
    }
}

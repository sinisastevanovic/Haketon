#include "hkpch.h"
#include "Scene.h"
#include "Haketon/Renderer/Renderer2D.h"
#include "Components.h"
#include "Entity.h"
#include "Components/TagComponent.h"

#include <glm/glm.hpp>

#include "Components/CameraComponent.h"
#include "SceneCamera.h"

//#include "rttr/type"


namespace Haketon
{

    Scene::Scene()
    {

    }

    Scene::~Scene()
    {
        
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };

        // TODO: Do I want every entity to have a transform component? If not, also add a check to SceneSerializer
        entity.AddComponent<TransformComponent>();
        // TODO: Maybe instead of tag component, save name in entity class? (Although we want to keep it small, so we can easily copy it)
        entity.AddComponent<TagComponent>(name);


        m_Registry.visit(entity, [&](const entt::type_info info)
        {
            auto &&storage = m_Registry.storage(info);
            auto data = storage.data();
        });
        
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    void Scene::DestroyAllEntities()
    {
        m_Registry.each([&](auto Entity)
        {
            if(m_Registry.valid(Entity))
                m_Registry.destroy(Entity);
        });
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
        m_Registry.view<NativeScriptComponent>().each([=](auto entity, NativeScriptComponent& nsc)
        {
           if(!nsc.Instance)
           {
               nsc.Instance = nsc.InstantiateScript();
               nsc.Instance->m_Entity = Entity{ entity, this };
               nsc.Instance->OnCreate();
           }

            nsc.Instance->OnUpdate(ts);
        });
    
        
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

            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for(auto entity : group)
            {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

                Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
            }

            Renderer2D::EndScene();
        }       
    }
    
    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& Camera)
    {
        Renderer2D::BeginScene(Camera);

        auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for(auto entity : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

            //Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
            Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
        }

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

        return {};
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
}

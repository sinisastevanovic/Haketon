#include "hkpch.h"
#include "Scene.h"
#include "Haketon/Renderer/Renderer2D.h"
#include "Components.h"
#include "Entity.h"

#include <glm/glm.hpp>

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

        // TODO: Do I want every entity to have a transform component?
        entity.AddComponent<TransformComponent>();
        // TODO: Maybe instead of tag component, save name in entity class? (Although we want to keep it small, so we can easily copy it)
        entity.AddComponent<TagComponent>(name);
        return entity;
    }

    void Scene::OnUpdate(Timestep ts)
    {
        auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for(auto entity : group)
        {
            auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

            Renderer2D::DrawQuad(transform, sprite.Color);
        }
    }
}

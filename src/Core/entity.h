#pragma once
#include "math.h"
#include "../AssetManagement/assets.h"
#include "entt.hpp"
#include "scene.h"

using namespace glm;
class Entity
{
    friend class Scene;
    friend class Serializer;

public:
    Entity(const Entity&) = default;
    operator entt::entity() const { return m_enttID; }

    template<typename T, typename... Args>
    T& addComponent(Args&&... args) const
    {
        ASSERT_ERROR(!hasComponent<T>(), "Component already exists in entity!");
        T& component = m_scene->m_entities.emplace<T>(m_enttID, std::forward<Args>(args)...);
        return component;
    }

    template<typename T>
    T& getComponent() const
    {
        ASSERT_ERROR(hasComponent<T>(), "Component doesnt exist in entity!");
        return m_scene->m_entities.get<T>(m_enttID);
    }

    template<typename T>
    bool hasComponent() const
    {
        return m_scene->m_entities.all_of<T>(m_enttID);
    }

    template<typename T>
    void removeComponent() const
    {
        ASSERT_ERROR(hasComponent<T>(), "Component doesnt exist in entity!");
        m_scene->m_entities.remove<T>(m_enttID);
    }

private:
    Entity(entt::entity enttID, Scene* scene)
        : m_enttID(enttID), m_scene(scene){}
    entt::entity m_enttID = entt::null;
    Scene* m_scene;
};


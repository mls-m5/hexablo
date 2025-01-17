// Copyright (c) 2023. Johan Lind

#include "jleComponent.h"
#include "jleObject.h"
#include "jleScene.h"

#include <cereal/types/optional.hpp>

template <class Archive>
void
jleObject::serialize(Archive &archive)
{

    cereal::base_class<jleResourceInterface>(this);

    try {
        archive(CEREAL_NVP(__templatePath));
    } catch (std::exception &e) {
    }

    archive(CEREAL_NVP(_instanceName),
            CEREAL_NVP(_transform),
            CEREAL_NVP(__childObjects),
            CEREAL_NVP(_components));

    for (auto &&child : __childObjects) {
        child->_parentObject = this;
    }

    // Update the internal world matrix for children
    getTransform().propagateMatrix();

    for (auto &&component : _components) {
        component->_attachedToObject = this;
        component->_containedInScene = _containedInScene;
    }
}
template <typename T>
inline std::shared_ptr<T>
jleObject::addComponent()
{
    static_assert(std::is_base_of<jleComponent, T>::value, "T must derive from jleComponent");

    for (auto &component : _components) {
        if (std::dynamic_pointer_cast<T>(component)) {
            return std::dynamic_pointer_cast<T>(component);
        }
    }

    std::shared_ptr<T> newComponent = std::make_shared<T>(this, _containedInScene);
    _components.push_back(newComponent);

    addComponentStart(newComponent.get());

    return newComponent;
};

inline std::shared_ptr<jleComponent>
jleObject::addComponentByName(const std::string &component_name)
{
    auto newComponent = jleTypeReflectionUtils::instantiateComponentByString(component_name);
    if(!newComponent)
    {
        LOGE << "Attempting to add non-existent component: " << component_name;
        return nullptr;
    }
    newComponent->_attachedToObject = this;
    newComponent->_containedInScene = _containedInScene;

    _components.push_back(newComponent);

    addComponentStart(newComponent.get());

    return newComponent;
}

template <typename T>
void
jleObject::addComponent(const std::shared_ptr<T>& component)
{
    static_assert(std::is_base_of<jleComponent, T>::value, "T must derive from jleComponent");

    const auto& c = std::static_pointer_cast<jleComponent>(component);
    c->_attachedToObject = this;
    c->_containedInScene = _containedInScene;

    _components.push_back(component);

    addComponentStart(component.get());

}

template <typename T>
inline std::shared_ptr<T>
jleObject::getComponent()
{
    static_assert(std::is_base_of<jleComponent, T>::value, "T must derive from jleComponent");

    for (auto &component : _components) {
        if (std::dynamic_pointer_cast<T>(component)) {
            return std::dynamic_pointer_cast<T>(component);
        }
    }

    return nullptr;
};

template <typename T>
inline std::shared_ptr<T>
jleObject::addDependencyComponent(const jleComponent *component)
{
    static_assert(std::is_base_of<jleComponent, T>::value, "T must derive from jleComponent");

    std::shared_ptr<T> c = component->_attachedToObject->getComponent<T>();
    if (!c) {
        c = component->_attachedToObject->addComponent<T>();
    }

    return c;
}

template <typename T>
inline std::shared_ptr<T>
jleObject::spawnChildObject()
{
    // We still want the object to be spawned initially in the scene,
    // but to immediately be moved over to the object's ownership.
    // This is because the scene will run the start() methods on the new object.
    std::shared_ptr<T> object = _containedInScene->spawnObject<T>();
    attachChildObject(object);
    return object;
}

inline std::shared_ptr<jleObject>
jleObject::spawnChildObject(const std::string &objName)
{
    auto object = _containedInScene->spawnObjectTypeByName(objName);
    attachChildObject(object);
    return object;
}


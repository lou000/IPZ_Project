#include "entity.h"
#include "gtx/quaternion.hpp"
#include "gtx/transform.hpp"


Entity::Entity(vec3 pos, std::shared_ptr<Model> model)
    :enabled(true), renderable(true), pos(pos), model(model)
{

}

mat4 Entity::getModelMatrix()
{
    return translate(mat4(1.0f), pos) * glm::scale(mat4(1.0f),scale) * toMat4(rotation);
}

void Entity::setOverrideColor(vec4 color)
{
    overrideColor = color;
}

void Entity::setModel(std::shared_ptr<Model> model)
{
    this->model = model;
}

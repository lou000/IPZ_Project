#include "entity.h"
#include "gtx/quaternion.hpp"
#include "gtx/transform.hpp"


Entity::Entity(vec3 pos, std::shared_ptr<Model> model)
    :pos(pos), model(model), renderable(true), m_enabled(true)
{

}

mat4 Entity::getModelMatrix()
{
    return translate(mat4(1.0f), pos) * glm::scale(mat4(1.0f),scale) * toMat4(rotation);
}

bool Entity::enabled() const
{
    return m_enabled;
}


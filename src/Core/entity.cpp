#include "entity.h"
#include "gtx/quaternion.hpp"
#include "gtx/transform.hpp"


mat4 Entity::getModelMatrix()
{
    return translate(mat4(1.0f), pos) * glm::scale(mat4(1.0f),scale) * toMat4(rotation);
}

bool Entity::enabled() const
{
    return m_enabled;
}


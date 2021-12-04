#include "entity.h"
#include "gtx/quaternion.hpp"
#include "gtx/transform.hpp"


Entity::Entity(vec3 pos, std::vector<std::shared_ptr<Mesh> > meshes)
    :enabled(true), renderable(true), pos(pos), meshes(meshes)
{

}

mat4 Entity::getModel()
{
    return translate(mat4(1.0f), pos) * glm::scale(mat4(1.0f),scale) * toMat4(rotation);
}

void Entity::setOverrideColor(vec4 color)
{
    overrideColor = color;
}

void Entity::setMesh(std::shared_ptr<Mesh> mesh)
{
    meshes.clear();
    meshes.push_back(mesh);
}

#include "Transform.h"

Transform::Transform(void)
{
    this->position = glm::vec3(0.0, 0.0, 0.0);
    this->rotation = glm::quat();
    this->scale = glm::vec3(1.0, 1.0, 1.0);
}

glm::mat4 Transform::getModelMatrix()
{
    return glm::mat4(glm::translate(this->position) * glm::mat4_cast(this->rotation) * glm::scale(this->scale));
}

void Transform::setPosition(const float &value0, const float &value1, const float &value2)
{
    this->position.x = value0;
    this->position.y = value1;
    this->position.z = value2;
}

void Transform::setPosition(const glm::vec3 &position)
{
    this->position = position;
}

void Transform::setRotation(const float &value0, const float &value1, const float &value2)
{
    this->rotation = glm::quat(glm::vec3(value0, value1, value2));
}

void Transform::setRotation(const glm::vec3 &yawPitchRoll)
{
    this->rotation = glm::quat(yawPitchRoll);
}

void Transform::setScale(const float &value0, const float &value1, const float &value2)
{
    this->scale.x = value0;
    this->scale.y = value1;
    this->scale.z = value2;
}

void Transform::setScale(const glm::vec3 &scale)
{
    this->scale = scale;
}

glm::vec3 Transform::eulerAngles()
{
    return glm::eulerAngles(this->rotation);
}

#pragma once
#include "Commons.h"

class Transform {
    public:
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;

        Transform(void);

        void setPosition(const float &value0, const float &value1, const float &value2);
        void setPosition(const glm::vec3 &position);

        void setRotation(const float &value0, const float &value1, const float &value2);
        void setRotation(const glm::vec3 &yawPitchRoll);

        void setScale(const float &value0, const float &value1, const float &value2);
        void setScale(const glm::vec3 &scale);

        glm::vec3 eulerAngles();
        glm::mat4 getModelMatrix();
};



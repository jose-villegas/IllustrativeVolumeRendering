#pragma once
#include "Commons.h"
#include "Shader.h"

class ShaderProgram {
    public:

        struct UniformBlockInfo {
            std::string uniformBlockName;
            GLubyte *dataPointer;
            GLint blockSize;
            GLuint UB;
            GLuint *indices;
            GLint *offset;
            UniformBlockInfo(const std::string &uniformBlockName, GLubyte *dataPointer, GLint blockSize, GLuint UB);
            ~UniformBlockInfo();
        };

    private:
        // stores uniform blocks shared between all shader programs if they
        // have this uniform block
        static std::unordered_map<std::string, UniformBlockInfo *> uniformBlocks;
        // stores uniform variables they are unique to every shaderprogram
        // so if multiple shader share the same uniform variable it this
        // uniform needs to be set again per shaderprogram
        std::unordered_map<std::string, unsigned int> uniformLoc;
        // ogl indentifier for this program
        unsigned int programID;
        // associated shaders count
        unsigned int fragmentShaderCount;
        unsigned int vertexShaderCount;
        // shaders related to this shaderprogram
        std::vector<Shader *> attachedShaders;

    public:
        ShaderProgram(void);
        ~ShaderProgram(void);

        unsigned int programId() const
        {
            return programID;
        }

        void attachShader(Shader *pShader);
        bool link() const;
        void use() const;
        void disable() const;
        // adds a new uniform related to the shaderprogram
        unsigned int addUniform(const std::string &sUniformName);
        // returns the location integer of this uniform
        unsigned int getUniform(const std::string &sUniformName) const;
        // adds a new uniform block to the binding point
        unsigned int addUniformBlock(const std::string &sUniformBlockName, const unsigned int &bindingPoint);
        // returns a struct with all the uniform block info
        UniformBlockInfo *getUniformBlock(const std::string &sUniformBlockName) const;
        // sets to out indices and offset the indices and offsets related
        // to the uniform variable names in the uniform block with this name
        // reserves memory accordly to count in outIndices and outOffset
        void getUniformBlockIndexAndOffset(const std::string &uniformBlockName, const char *names[], GLuint *outIndices[], GLint *outOffset[], const unsigned int &count) const;
        // sets the indices and offsets related to the uniform variable names
        // in the uniform block with this name into passed uniformblock info struct
        // reserver memory accordly to count in outUBF indices and offset
        void setUniformBlockInfoIndexAndOffset(UniformBlockInfo *outUBF, const char *names[], const unsigned int &count) const;

        /*
         * Validates the uniform name and location
         * and forwards the rvalue to a specific
         * overloaded function that calls:
         * glUniform{1|2|3|4}{f|i|ui} based on number of parameters
         * glUniform{2|3|4}fv for type glm::vec{2|3|4}
         * glUniformMatrix{2|3|4}fv for type glm::mat{2|3|4}
         */
        template<typename T> void setUniform(const std::string &sUniformName, T &&value0) const;
        template<typename T> void setUniform(const std::string &sUniformName, T &&value0, T &&value1) const;
        template<typename T> void setUniform(const std::string &sUniformName, T &&value0, T &&value1, T &&value2) const;
        template<typename T> void setUniform(const std::string &sUniformName, T &&value0, T &&value1, T &&value2, T &&value3) const;

        void setUniform(unsigned int uniformLocation, const float &value0, const float &value1) const;
        void setUniform(unsigned int uniformLocation, const float &value0, const float &value1, const float &value2) const;
        void setUniform(unsigned int uniformLocation, const float &value0, const float &value1, const float &value2, const float &value3) const;

        void setUniform(unsigned int uniformLocation, const int &value0, const int &value1) const;
        void setUniform(unsigned int uniformLocation, const int &value0, const int &value1, const int &value2) const;
        void setUniform(unsigned int uniformLocation, const int &value0, const int &value1, const int &value2, const int &value3) const;

        void setUniform(unsigned int uniformLocation, const unsigned int &value0, const unsigned int &value1) const;
        void setUniform(unsigned int uniformLocation, const unsigned int &value0, const unsigned int &value1, const unsigned int &value2) const;
        void setUniform(unsigned int uniformLocation, const unsigned int &value0, const unsigned int &value1, const unsigned int &value2, const unsigned int &value3) const;

        void setUniform(unsigned int uniformLocation, const float &value0) const;
        void setUniform(unsigned int uniformLocation, const int &value0) const;
        void setUniform(unsigned int uniformLocation, const unsigned int &value0) const;

        void setUniform(unsigned int uniformLocation, const glm::mat4 &value0) const;
        void setUniform(unsigned int uniformLocation, const glm::mat3 &value0) const;
        void setUniform(unsigned int uniformLocation, const glm::mat2 &value0) const;

        void setUniform(unsigned int uniformLocation, const glm::vec4 &value0) const;
        void setUniform(unsigned int uniformLocation, const glm::vec3 &value0) const;
        void setUniform(unsigned int uniformLocation, const glm::vec2 &value0) const;
};

template<typename T>
void ShaderProgram::setUniform(const std::string &sUniformName, T &&value0) const
{
    GLint uniformLocation = getUniform(sUniformName);

    if (uniformLocation == -1) {
        return;
    }

    setUniform(uniformLocation, std::forward<T>(value0));
}

template<typename T>
void ShaderProgram::setUniform(const std::string &sUniformName, T &&value0, T &&value1) const
{
    GLint uniformLocation = getUniform(sUniformName);

    if (uniformLocation == -1) {
        return;
    }

    setUniform(uniformLocation, std::forward<T>(value0), std::forward<T>(value1));
}

template<typename T>
void ShaderProgram::setUniform(const std::string &sUniformName, T &&value0, T &&value1, T &&value2) const
{
    GLint uniformLocation = getUniform(sUniformName);

    if (uniformLocation == -1) {
        return;
    }

    setUniform(uniformLocation, std::forward<T>(value0), std::forward<T>(value1), std::forward<T>(value2));
}

template<typename T>
void ShaderProgram::setUniform(const std::string &sUniformName, T &&value0, T &&value1, T &&value2, T &&value3) const
{
    GLint uniformLocation = getUniform(sUniformName);

    if (uniformLocation == -1) {
        return;
    }

    setUniform(uniformLocation, std::forward<T>(value0), std::forward<T>(value1), std::forward<T>(value2), std::forward<T>(value3));
}

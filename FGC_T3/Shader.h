#pragma once
#include "Commons.h"

class Shader {
    public:

        enum ShaderType {
            Vertex = GL_VERTEX_SHADER,
            Fragment = GL_FRAGMENT_SHADER,
            Geometry = GL_GEOMETRY_SHADER,
        };

        Shader(const ShaderType &shaderType);
        Shader(const ShaderType &shaderType, const std::string &source, const bool &loadFromFile = true);
        ~Shader() {};

        bool loadFromString(const std::string &sSource);
        bool loadFromFile(const std::string &sFilename);
        /*
         * Concatenates the string data at the end of
         * the specified token to  the shader source code
         * Method:    loadFromString
         * FullName:  Shader::loadFromString
         * Access:    public
         * Returns:   bool
         * Parameter: const std::string & sSource
         * Parameter: const std::string & token
         * Parameter: const std::string data
         */
        bool loadFromString(std::string &sSource, const std::string &token, const std::string &data);
        /*
        * Concatenates the string data at the end of
        * the specified token to  the shader source code
        * Method:    loadFromString
        * FullName:  Shader::loadFromString
        * Access:    public
        * Returns:   bool
        * Parameter: const std::string & sFilename
        * Parameter: const std::string & token
        * Parameter: const std::string data
        */
        bool loadFromFile(const std::string &sFilename, const std::string &token, const std::string &data);
        // Converts a file to a string
        static const std::string fileToString(const std::string &sFilename);
        bool compile();
        GLuint getId() const
        {
            return id;
        }
        Shader::ShaderType getType() const
        {
            return shaderType;
        }
        std::string getSourceCode() const
        {
            return sourceCode;
        }

    private:

        GLuint id;
        ShaderType shaderType;
        std::string sourceCode;
        std::string shaderName;
        bool compilationCheck();
        std::string getShaderTypeString();
};

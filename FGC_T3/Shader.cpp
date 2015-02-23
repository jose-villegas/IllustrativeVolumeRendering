#include "Shader.h"

Shader::Shader(const ShaderType &shaderType)
{
    this->shaderType = shaderType;
    this->id = glCreateShader(shaderType);
}

Shader::Shader(const ShaderType &shaderType, const std::string &source, const bool &loadFromFile /*= true*/)
{
    this->shaderType = shaderType;
    this->id = glCreateShader(shaderType);
    loadFromFile ? this->loadFromFile(source) : this->loadFromString(source);
}

bool Shader::loadFromString(const std::string &sSource)
{
    if (sSource.empty()) { return false; }

    this->sourceCode = sSource;
    const char *source = sSource.c_str();
    // Associate source with this shader ID
    glShaderSource(id, 1, &source, NULL);
    // Successful shader file load
    std::cout << "Shader(" << this << "): " << getShaderTypeString() << " file " << shaderName << " loaded successfully" << std::endl;
    // std::cout << std::endl << sSource << std::endl;
    return true;
}

bool Shader::loadFromString(std::string &sSource, const std::string &token, const std::string &data)
{
    // Find token at source code
    size_t glslTokenIndex = sSource.find(token);

    if (glslTokenIndex == std::string::npos) {
        std::cout << "Shader(" << this << "): " << "Token " << token << " not found in" << getShaderTypeString() << " source code" << std::endl;
        return loadFromString(sSource);
    }

    // Insert after token
    sSource.insert(glslTokenIndex + token.size(), data);
    std::cout << "Shader(" << this << "): " << getShaderTypeString() << " data modified, raw string inserted at token " << token << std::endl;
    // Load normally
    return loadFromString(sSource);
}

bool Shader::loadFromFile(const std::string &sFilename)
{
    // Convert the file to a string
    std::string source = fileToString(sFilename);

    if (source.empty()) {
        std::cout << "Shader(" << this << "): " << "Error Opening " << getShaderTypeString() << " file: " << sFilename << std::endl;
        return false;
    }

    shaderName = sFilename;
    // Load Source and Associate with this shader ID
    return loadFromString(source);
}

bool Shader::loadFromFile(const std::string &sFilename, const std::string &token, const std::string &data)
{
    // Convert the file to a string
    std::string source = fileToString(sFilename);

    if (source.empty()) {
        std::cout << "Shader(" << this << "): " << "Error Opening " << getShaderTypeString() << " file: " << sFilename << std::endl;
        return false;
    }

    shaderName = sFilename;
    // Load Source and Associate with this shader ID
    return loadFromString(source, token, data);
}


const std::string Shader::fileToString(const std::string &sFilename)
{
    std::ifstream file(sFilename, std::ifstream::in);

    if (!file.good()) {
        return std::string();
    }

    std::stringstream sourceSS;
    // Dump File Content into stream buffer
    sourceSS << file.rdbuf();
    file.close();
    // Return casted data
    return sourceSS.str();
}


bool Shader::compile()
{
    glCompileShader(id);
    return compilationCheck();
}

bool Shader::compilationCheck()
{
    GLint shaderStatus;
    glGetShaderiv(id, GL_COMPILE_STATUS, &shaderStatus);

    if (shaderStatus == GL_FALSE) {
        // Get Info Log Size
        GLint infoLength;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLength);
        // Save Info Log Data
        GLchar *strInfoLog = new GLchar[infoLength + 1];
        glGetShaderInfoLog(id, infoLength, NULL, strInfoLog);
        // Write Compilation Errors to Utils Logger
        std::cout << "\n" << "Shader(" << this << "): " << getShaderTypeString() << " compilation errors:\n" << std::string(strInfoLog) << std::endl;
        // Free Reserved Memory for InfoLog
        delete[] strInfoLog;
        // Return Failure
        return false;
    } else {
        std::cout << "Shader(" << this << "): " << getShaderTypeString() << " file " << shaderName << " compilation successful" << std::endl;
        return true;
    }
}

std::string Shader::getShaderTypeString()
{
    switch (shaderType) {
        case Shader::Vertex:
            return "Vertex shader";
            break;

        case Shader::Fragment:
            return "Fragment shader";
            break;

        case Shader::Geometry:
            return "Geometry shader";
            break;

        default:
            break;
    }

    return "";
}

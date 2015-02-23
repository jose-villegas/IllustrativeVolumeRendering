#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(void)
{
    this->programID           = glCreateProgram();
    this->fragmentShaderCount = 0;
    this->vertexShaderCount   = 0;

    if (this->programID <= 0) {
        std::cout << "ShaderProgram(" << this << "): " << "Error Creating Shader Program" << std::endl;
    } else {
        std::cout << "ShaderProgram(" << this << "): " << "Shader Program " << programID << " created successfully" << std::endl;
    }
}

ShaderProgram::~ShaderProgram(void)
{
    for (auto it = this->attachedShaders.begin(); it != this->attachedShaders.end(); ++it) {
        delete(*it);
    }

    glDeleteProgram(this->programID);
}

void ShaderProgram::attachShader(Shader *pShader)
{
    glAttachShader(this->programID, pShader->getId());
    attachedShaders.push_back(pShader);

    if (pShader->getType() == Shader::Fragment) { this->fragmentShaderCount++; }

    if (pShader->getType() == Shader::Vertex) { this->vertexShaderCount++; }
}

bool ShaderProgram::link() const
{
    // Needs at least one fragment shader and one vertex shader to link the program
    if (this->fragmentShaderCount >= 1 && this->vertexShaderCount >= 1) {
        // Link Attached Shaders to Program
        glLinkProgram(this->programID);
        // Check Linking Status
        GLint linkStatus;
        glGetProgramiv(this->programID, GL_LINK_STATUS, &linkStatus);

        if (linkStatus == GL_FALSE) {
            // get error string
            char errbuf[4096]; GLsizei len;
            glGetProgramInfoLog(this->programID, sizeof(errbuf), &len, errbuf);
            std::cout << "ShaderProgram(" << this << "): " << "Shader program linking failed \n" << errbuf << std::endl;
            return false;
        }

        std::cout << "ShaderProgram(" << this << "): " << "Shader program linking successful" << std::endl;
        return true;
    }

    return false;
}

void ShaderProgram::use() const
{
    glUseProgram(this->programID);
}

void ShaderProgram::disable() const
{
    glUseProgram(0);
}

GLuint ShaderProgram::getUniform(const std::string &sUniformName) const
{
    // Find uniform with this name
    auto it = this->uniformLoc.find(sUniformName);

    if (it != uniformLoc.end()) {
        return it->second;
    } else {
        return -1;
    }
}

ShaderProgram::UniformBlockInfo *ShaderProgram::getUniformBlock(const std::string &sUniformBlockName) const
{
    // Find uniform with this name
    auto it = this->uniformBlocks.find(sUniformBlockName);

    if (it != uniformBlocks.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

GLuint ShaderProgram::addUniform(const std::string &sUniformName)
{
    // Try to obtain uniform location
    GLint nUniformLoc = glGetUniformLocation(this->programID, sUniformName.c_str());

    // Check if an uniform with this name actually exists
    if (nUniformLoc == -1) {
        std::cout << "ShaderProgram(" << this << "): " << "Could not add uniform: (" << sUniformName << ") location returned -1" << std::endl;
        return -1;
    }

    // Return Location
    this->uniformLoc[sUniformName] = nUniformLoc;
    std::cout << "ShaderProgram(" << this << "): " << "Uniform (" << sUniformName << ") bound to location: " << std::to_string(nUniformLoc) << std::endl;
    return nUniformLoc;
}

unsigned int ShaderProgram::addUniformBlock(const std::string &sUniformBlockName, const unsigned int &bindingPoint)
{
    auto it  = this->uniformBlocks.find(sUniformBlockName);
    // Query block index
    GLuint blockIndex = glGetUniformBlockIndex(this->programID, sUniformBlockName.c_str());

    // There is a uniform block with this name already saved
    if (it != this->uniformBlocks.end()) {
        std::cout << "ShaderProgram(" << this << "): " << "There is a uniform block with name (" << sUniformBlockName << ") already saved, binding UBO to programID" << std::endl;
        // we still trying to bind the uniform block to this shaderprogram since the blockIndex can change between shaderprograms
        glUniformBlockBinding(this->programID, blockIndex, bindingPoint); return true;
    }

    // No uniform with this name
    if (blockIndex == GL_INVALID_INDEX) {
        std::cout << "ShaderProgram(" << this << "): " << "No uniform block found with name (" << sUniformBlockName << ")" << std::endl;
        return -1;
    }

    // Query block uniform block size
    GLint blockSize;
    glGetActiveUniformBlockiv(this->programID, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
    GLubyte *blockBuffer = (GLubyte *)malloc(blockSize);
    // Create Buffer Object
    GLuint UB;
    glGenBuffers(1, &UB);
    glBindBuffer(GL_UNIFORM_BUFFER, UB);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, blockBuffer, GL_DYNAMIC_DRAW);
    // Bind the buffer
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, UB);
    // Return uniform buffer id
    std::cout << "ShaderProgram(" << this << "): " << "Uniform block (" << sUniformBlockName << ") saved successfully" << std::endl;
    glUniformBlockBinding(this->programID, blockIndex, bindingPoint);
    // Store pointer to uniform block struct in uniformBlocks map
    this->uniformBlocks[sUniformBlockName] = new UniformBlockInfo(sUniformBlockName, blockBuffer, blockSize, UB);
    // Return Success
    return UB;
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const float &value0) const
{
    glUniform1f(uniformLocation, value0);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const int &value0) const
{
    glUniform1i(uniformLocation, value0);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const unsigned int &value0) const
{
    glUniform1ui(uniformLocation, value0);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const glm::mat4 &value0) const
{
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value0));
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const glm::mat3 &value0) const
{
    glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value0));
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const glm::mat2 &value0) const
{
    glUniformMatrix2fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value0));
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const glm::vec4 &value0) const
{
    glUniform4fv(uniformLocation, 1, glm::value_ptr(value0));
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const glm::vec3 &value0) const
{
    glUniform3fv(uniformLocation, 1, glm::value_ptr(value0));
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const glm::vec2 &value0) const
{
    glUniform2fv(uniformLocation, 1, glm::value_ptr(value0));
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const float &value0, const float &value1) const
{
    glUniform2f(uniformLocation, value0, value1);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const float &value0, const float &value1, const float &value2) const
{
    glUniform3f(uniformLocation, value0, value1, value2);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const float &value0, const float &value1, const float &value2, const float &value3) const
{
    glUniform4f(uniformLocation, value0, value1, value2, value3);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const int &value0, const int &value1) const
{
    glUniform2i(uniformLocation, value0, value1);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const int &value0, const int &value1, const int &value2) const
{
    glUniform3i(uniformLocation, value0, value1, value2);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const int &value0, const int &value1, const int &value2, const int &value3) const
{
    glUniform4i(uniformLocation, value0, value1, value2, value3);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const unsigned int &value0, const unsigned int &value1) const
{
    glUniform2ui(uniformLocation, value0, value1);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const unsigned int &value0, const unsigned int &value1, const unsigned int &value2) const
{
    glUniform3ui(uniformLocation, value0, value1, value2);
}

void ShaderProgram::setUniform(unsigned int uniformLocation, const unsigned int &value0, const unsigned int &value1, const unsigned int &value2, const unsigned int &value3) const
{
    glUniform4ui(uniformLocation, value0, value1, value2, value3);
}

void ShaderProgram::getUniformBlockIndexAndOffset(const std::string &uniformBlockName, const char *names[], GLuint *outIndices[], GLint *outOffset[], const unsigned int &count) const
{
    // No uniform block with this name
    if (this->getUniformBlock(uniformBlockName) == nullptr) { return; }

    *outIndices = new GLuint[count];
    *outOffset = new GLint[count];
    // Get Uniform Memory Location
    glGetUniformIndices(this->programID, count, names, (*outIndices));
    glGetActiveUniformsiv(this->programID, count, (*outIndices), GL_UNIFORM_OFFSET, (*outOffset));
}

void ShaderProgram::setUniformBlockInfoIndexAndOffset(UniformBlockInfo *outUBF, const char *names[], const unsigned int &count) const
{
    if (outUBF == nullptr) { return; }

    // No uniform block with this name
    if (this->getUniformBlock(outUBF->uniformBlockName) == nullptr) { return; }

    outUBF->indices = new GLuint[count];
    outUBF->offset = new GLint[count];
    // Get Uniform Memory Location
    glGetUniformIndices(this->programID, count, names, outUBF->indices);
    glGetActiveUniformsiv(this->programID, count, outUBF->indices, GL_UNIFORM_OFFSET, outUBF->offset);
}

ShaderProgram::UniformBlockInfo::UniformBlockInfo(const std::string &uniformBlockName, GLubyte *dataPointer, GLint blockSize, GLuint UB)
{
    this->uniformBlockName = uniformBlockName;
    this->dataPointer = dataPointer;
    this->blockSize = blockSize;
    this->UB = UB;
    this->indices = nullptr;
    this->offset = nullptr;
}

ShaderProgram::UniformBlockInfo::~UniformBlockInfo()
{
    delete this->dataPointer;
    delete[] this->indices;
    delete[] this->offset;
}

std::unordered_map<std::string, ShaderProgram::UniformBlockInfo *> ShaderProgram::uniformBlocks;

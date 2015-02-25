#include "RawDataModel.h"
#include "TransferFunction.h"

RawDataModel::RawDataModel(void)
{
    isLoaded = false;
    sModelName = (char *)calloc(1024, sizeof(char));
    width = height = numCuts = 1;
    stepSize = 0.001f;
    threshold = 0.15f;
    backFaceTexture = 0;
    depthRenderBuffer = 0;
    frameBuffer = 0;
    vertexBuffer = 0;
    transferFunctionTexture = 0;
    volumeTexture = 0;

    for (int i = 0; i < 256; i++) transferFunc[i] = glm::vec4((float)i / 255.f);

    setupVolumeShaders();
    createTransferFunctionTexture();
    stf.loadStyles();
}


RawDataModel::~RawDataModel(void)
{
    isLoaded = false;
    glDeleteTextures(1, &transferFunctionTexture);
    glDeleteTextures(1, &volumeTexture);
    free(dataScalars);
    dataScalars = nullptr;
}

void RawDataModel::load(const char *pszFilepath, int width, int height, int numCuts)
{
    isLoaded = false;

    // Initialize VBO for rendering Volume
    if (!createVertexBuffer()) {
        return;
    }

    // Load Volume data on 3D texture
    if (!loadVolumeFromFile8(pszFilepath, width, height, numCuts)) {
        return;
    } else {
        // gradients = new glm::vec3[width * height * numCuts];
        // generateGradients(1);
        // filterNxNxN(3);
    }

    // Initialize texture for backface
    if (!createBackFaceTexture()) {
        return;
    }

    // Initialize FBO
    if (!createFrameBuffer()) {
        return;
    }

    // Success
    this->width = width;
    this->height = height;
    this->numCuts = numCuts;
    float maxSize = std::max(std::max(width, height), numCuts);
    cubeSizes = glm::vec3(width / maxSize, height / maxSize, numCuts / maxSize);
    // move object to center
    this->transform.scale = cubeSizes;
    this->transform.setPosition(-cubeSizes.x / 2.f, -cubeSizes.y / 2.f, 0.f);
    // create matrices
    this->projection = glm::perspective(45.0f, (GLfloat)MainData::rootWindow->getSize().x / MainData::rootWindow->getSize().y, 0.1f, 500.f);
    this->view = glm::lookAt(glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
    this->model = this->transform.getModelMatrix();
    this->normalMatrix = glm::inverse(glm::transpose(view * model));
    this->viewProjection = projection * view;
    this->modelViewProjection = this->viewProjection * model;
    // copy asset location
    memcpy(sModelName, pszFilepath, 1024);
    isLoaded = true;
}

bool RawDataModel::createVertexBuffer()
{
    try {
        if (vertexBuffer > 0) return true;

        GLfloat vertices[24] = {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f
        };
        // Counter clockwise bounding box faces
        GLuint indices[36] = {
            1, 5, 7,
            7, 3, 1,
            0, 2, 6,
            6, 4, 0,
            0, 1, 3,
            3, 2, 0,
            7, 5, 4,
            4, 6, 7,
            2, 3, 7,
            7, 6, 2,
            1, 0, 4,
            4, 5, 1
        };
        GLuint gbo[2];
        glGenBuffers(2, gbo);
        GLuint vertexdat = gbo[0];
        GLuint indexdat = gbo[1];
        glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
        // used in glDrawElement()
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexdat);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), indices, GL_STATIC_DRAW);
        // Setup Vertex Array Object
        glGenVertexArrays(1, &vertexBuffer);
        // vao like a closure binding 3 buffer object: verlocdat vercoldat and veridxdat
        glBindVertexArray(vertexBuffer);
        glEnableVertexAttribArray(0); // for vertexloc
        glEnableVertexAttribArray(1); // for vertexcol
        // the vertex location is the same as the vertex color
        glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexdat);
    } catch (...) {
        return false;
    }

    return true;
}

void RawDataModel::render()
{
    if (isLoaded) {
        // render cube back face for exit points
        renderBackFace();
        // render front face and volume with ray casting technique
        renderVolumeRayCasting();
    }
}

void RawDataModel::renderCubeFace(GLenum gCullFace)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(gCullFace);
    glBindVertexArray(vertexBuffer);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
    glDisable(GL_CULL_FACE);
}

bool RawDataModel::createBackFaceTexture()
{
    try {
        if (backFaceTexture > 0) return true;

        // Backface Texture
        glGenTextures(1, &backFaceTexture);
        glBindTexture(GL_TEXTURE_2D, backFaceTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MainData::rootWindow->getSize().x, MainData::rootWindow->getSize().y, 0, GL_RGBA, GL_FLOAT,
                     NULL);
    } catch (...) {
        return false;
    }

    return true;
}

bool RawDataModel::loadVolumeFromFile8(const char *pszFilepath, int width, int height, int numCuts)
{
    FILE *fp;
    size_t size = width * height * numCuts;

    if (dataScalars) {
        delete []dataScalars;
        dataScalars = nullptr;
        glDeleteTextures(1, &volumeTexture);
    }

    GLubyte *data = new GLubyte[size]; // 8bit

    if (!(fp = fopen(pszFilepath, "rb"))) {
        std::cout << "Error: opening " << pszFilepath << " file failed: " << std::endl;
        perror("fopen");
        return false;
    } else {
        std:: cout << "OK: opening " << pszFilepath << " file successed" << std::endl;
    }

    if (fread(data, sizeof(GLubyte), size, fp) != size) {
        std::cout << "Error: reading " << pszFilepath << " file failed" << std::endl;
        fclose(fp);
        return false;
    } else {
        std::cout << "OK: reading " << pszFilepath << " file successed" << std::endl;
    }

    dataScalars = new float[size];

    for (unsigned int i = 0; i < size; i++) {
        dataScalars[i] = (float)data[i] / std::numeric_limits<byte>::max();
    }

    fclose(fp);
    create3DTexture(width, height, numCuts);
    std::cout << "volume texture created" << std::endl;
    return true;
}

bool RawDataModel::loadVolumeFromFile16(const char *pszFilepath, int width, int height, int numCuts)
{
    FILE *fp;
    size_t size = width * height * numCuts;

    if (dataScalars) {
        delete[]dataScalars;
        dataScalars = nullptr;
    }

    unsigned short *data = new unsigned short[size]; // 16 bits

    if (!(fp = fopen(pszFilepath, "rb"))) {
        std::cout << "Error: opening " << pszFilepath << " file failed: " << std::endl;
        perror("fopen");
        return false;
    } else {
        std::cout << "OK: opening " << pszFilepath << " file successed" << std::endl;
    }

    if (fread(data, sizeof(unsigned short), size, fp) != size) {
        std::cout << "Error: reading " << pszFilepath << " file failed" << std::endl;
        fclose(fp);
        return false;
    } else {
        std::cout << "OK: reading " << pszFilepath << " file successed" << std::endl;
    }

    dataScalars = new float[size];

    for (unsigned int i = 0; i < size; i++) {
        dataScalars[i] = (float)data[i] / std::numeric_limits<unsigned short>::max();
    }

    fclose(fp);
    create3DTexture(width, height, numCuts);
    std::cout << "volume texture created" << std::endl;
    return true;
}


bool RawDataModel::createFrameBuffer()
{
    if (frameBuffer > 0) return true;

    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, MainData::rootWindow->getSize().x, MainData::rootWindow->getSize().y);
    // attach the texture and the depth buffer to the framebuffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backFaceTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);
    GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (complete != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "framebuffer is not complete" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void RawDataModel::createTransferFunctionTexture()
{
    glGenTextures(1, &transferFunctionTexture);
    glBindTexture(GL_TEXTURE_1D, transferFunctionTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_FLOAT, transferFunc);
}

void RawDataModel::updateTransferFunctionTexture()
{
    //TransferFunction::getLinearFunction(this->transferFunc);
    //glBindTexture(GL_TEXTURE_1D, transferFunctionTexture);
    //glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_FLOAT, transferFunc);
    stf.updateIndexFunctionTexture();
    stf.updateTransferFunctionTexture();
}

void RawDataModel::create3DTexture(int width, int height, int numCuts)
{
    glGenTextures(1, &volumeTexture);
    glBindTexture(GL_TEXTURE_3D, volumeTexture);							// bind 3D texture target
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, width, height, numCuts, 0, GL_LUMINANCE, GL_FLOAT, dataScalars);
}

void RawDataModel::setupVolumeShaders()
{
    Shader *backVert = new Shader(Shader::Vertex, "Shaders/backface.vert", true);
    Shader *backFrag = new Shader(Shader::Fragment, "Shaders/backface.Frag", true);
    backFrag->compile();
    backVert->compile();
    this->backFaceShader.attachShader(backVert);
    this->backFaceShader.attachShader(backFrag);
    this->backFaceShader.link();
    this->backFaceShader.addUniform("MVP");
    Shader *rayCastVert = new Shader(Shader::Vertex, "Shaders/raycasting.vert", true);
    Shader *rayCastFrag = new Shader(Shader::Fragment, "Shaders/raycasting.Frag", true);
    rayCastVert->compile();
    rayCastFrag->compile();
    this->rayCastShader.attachShader(rayCastVert);
    this->rayCastShader.attachShader(rayCastFrag);
    this->rayCastShader.link();
    this->rayCastShader.addUniform("MVP");
    this->rayCastShader.addUniform("VolumeTex");
    this->rayCastShader.addUniform("ExitPoints");
    this->rayCastShader.addUniform("TransferFunc");
    this->rayCastShader.addUniform("transferFunctionTexture");
    this->rayCastShader.addUniform("indexFunctionTexture");
    this->rayCastShader.addUniform("styleTransferTexture");
    this->rayCastShader.addUniform("StepSize");
    this->rayCastShader.addUniform("ViewMatrix");
    this->rayCastShader.addUniform("ScreenSize");
    this->rayCastShader.addUniform("NormalMatrix");
}

void RawDataModel::renderVolumeRayCasting()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, MainData::rootWindow->getSize().x, MainData::rootWindow->getSize().y);
    this->rayCastShader.use();
    this->rayCastShader.setUniform("MVP", this->modelViewProjection);
    this->rayCastShader.setUniform("ViewMatrix", this->view);
    this->rayCastShader.setUniform("NormalMatrix", this->normalMatrix);
    this->rayCastShader.setUniform("StepSize", this->stepSize);
    this->rayCastShader.setUniform("Threshold", this->threshold);
    this->rayCastShader.setUniform("ScreenSize", (float)MainData::rootWindow->getSize().x, (float)MainData::rootWindow->getSize().y);
    // style transfer function
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, this->stf.transferFunctionTexture);
    this->rayCastShader.setUniform("transferFunctionTexture", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, this->stf.indexFunctionTexture);
    this->rayCastShader.setUniform("indexFunctionTexture", 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->stf.styleFunctionTexture);
    this->rayCastShader.setUniform("styleTransferTexture", 3);
    // back face and volume
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, this->backFaceTexture);
    this->rayCastShader.setUniform("ExitPoints", 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_3D, this->volumeTexture);
    this->rayCastShader.setUniform("VolumeTex", 5);
    //glActiveTexture(GL_TEXTURE6);
    //glBindTexture(GL_TEXTURE_1D, this->transferFunctionTexture);
    //this->rayCastShader.setUniform("TransferFunc", 6);
    renderCubeFace(GL_BACK);
}

void RawDataModel::renderBackFace()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, MainData::rootWindow->getSize().x, MainData::rootWindow->getSize().y);
    this->backFaceShader.use();
    this->backFaceShader.setUniform("MVP", this->modelViewProjection);
    renderCubeFace(GL_FRONT);
}

void RawDataModel::generateGradients(int sampleSize)
{
    int n = sampleSize;
    glm::vec3 normal = glm::vec3(0.f);
    glm::vec3 s1, s2;
    int index = 0;

    for (int z = 0; z < numCuts; z++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                s1 = glm::vec3(sampleVolume(x - n, y, z),
                               sampleVolume(x, y - n, z),
                               sampleVolume(x, y, z - n));
                s2 = glm::vec3(sampleVolume(x + n, y, z),
                               sampleVolume(x, y + n, z),
                               sampleVolume(x, y, z + n));
                gradients[index++] = glm::normalize(s2 - s1);

                if (std::isnan(gradients[index - 1].x)) {
                    gradients[index - 1] = glm::vec3(0.f);
                }
            }
        }
    }
}

float RawDataModel::sampleVolume(int x, int y, int z)
{
    x = (int)glm::clamp(x, 0, width - 1);
    y = (int)glm::clamp(y, 0, height - 1);
    y = (int)glm::clamp(z, 0, numCuts - 1);
    return (float)dataScalars[x + (y * width) + (z * width * height)];
}

void RawDataModel::filterNxNxN(int sampleSize)
{
    int index = 0;

    for (int z = 0; z < numCuts; z++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                gradients[index++] = sampleNxNxN(x, y, z, sampleSize);
            }
        }
    }
}

glm::vec3 &RawDataModel::sampleNxNxN(int x, int y, int z, int n)
{
    n = (n - 1) / 2;
    glm::vec3 average = glm::vec3(0.f);
    int num = 0;

    for (int k = z - n; k <= z + n; k++) {
        for (int j = y - n; j <= y + n; j++) {
            for (int i = x - n; i <= x + n; i++) {
                if (isInBounds(i, j, k)) {
                    average += sampleGradients(i, j, k);
                    num++;
                }
            }
        }
    }

    average /= (float)num;

    if (average.x != 0.0f && average.y != 0.0f && average.z != 0.0f) {
        glm::normalize(average);
    }

    return average;
}

bool RawDataModel::isInBounds(int x, int y, int z)
{
    return ((x >= 0 && x < width) &&
            (y >= 0 && y < height) &&
            (z >= 0 && z < numCuts));
}

glm::vec3 &RawDataModel::sampleGradients(int x, int y, int z)
{
    return gradients[x + (y * width) + (z * width * height)];
}

#pragma once
#include "Transform.h"
#include "Commons.h"
#include "MainData.h"
#include "ShaderProgram.h"
#include "StyleTransfer.h"

class RawDataModel {
    private:
        GLuint backFaceTexture;
        GLuint depthRenderBuffer;
        GLuint frameBuffer;
        GLuint vertexBuffer;
        GLuint transferFunctionTexture;
        GLuint volumeTexture;
        int _heightP;
        int _numCutsP;
        int _widthP;

        bool createBackFaceTexture();
        bool createFrameBuffer();
        bool createVertexBuffer();
        bool loadVolumeFromFile16(const char *pszFilepath, int width, int height, int numCuts);
        bool loadVolumeFromFile8(const char *pszFilepath, int width, int height, int numCuts);
        void create3DTexture(int width, int height, int numCuts);
        void createTransferFunctionTexture();
        void renderBackFace();
        void renderCubeFace(GLenum gCullFace);
        void renderVolumeRayCasting();
        void setupVolumeShaders();
        void generateGradients(int sampleSize);
        void filterNxNxN(int sampleSize);
        glm::vec3 &sampleNxNxN(int x, int y, int z, int n);
        float sampleVolume(int x, int y, int z);
        glm::vec3 &sampleGradients(int x, int y, int z);
        bool isInBounds(int x, int y, int z);

    public:
        float *dataScalars;
        glm::vec3 *gradients;
        glm::vec4 transferFunc[256];

        StyleTransfer stf;

        // cube face width height depth
        glm::vec3 cubeSizes;
        // volume transform
        Transform transform;

        // rendering shaders
        ShaderProgram backFaceShader;
        ShaderProgram rayCastShader;

        // render matrices
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 viewProjection;
        glm::mat4 modelViewProjection;
        glm::mat4 normalMatrix;

        bool isLoaded;
        char *sModelName;
        float stepSize;
        float threshold;

        int height;
        int numCuts;
        int width;

        void load(const char *pszFilepath, int width, int height, int numCuts);
        void render();

        RawDataModel(void);
        ~RawDataModel(void);
        void updateTransferFunctionTexture();
};


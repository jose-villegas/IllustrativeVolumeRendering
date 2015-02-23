#pragma once
#include "Commons.h"

class StyleTransfer {

    public:
        static const unsigned int AVAILABLE_STYLE_COUNT = 34;

        BYTE *wholeData;
        static const std::string styleTextList;
        glm::vec4 transferFunctionValues[256];

        bool stylesLoaded;
        unsigned int transferFunctionTexture;
        unsigned int indexFunctionTexture;
        unsigned int styleFunctionTexture;

        void createIndexFunctionTexture();
        void createTransferFunctionTexture();
        void createStyleFunctionTexture();

    public:
        unsigned int availableStyles[AVAILABLE_STYLE_COUNT];
        float *indexFunction;
        StyleTransfer();
        ~StyleTransfer();
        void loadStyles();

        void updateIndexFunctionTexture();
        void updateTransferFunctionTexture();

        bool StylesLoaded() const
        {
            return stylesLoaded;
        }
};


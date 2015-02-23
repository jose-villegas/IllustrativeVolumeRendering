#include "StyleTransfer.h"
#include "FreeImage.h"
#include "TransferFunction.h"

StyleTransfer::StyleTransfer() : stylesLoaded(false)
{
    // call this ONLY when linking with FreeImage as a static library
    for (int i = 0; i < AVAILABLE_STYLE_COUNT; i++) availableStyles[i] = 0;

    #ifdef FREEIMAGE_LIB
    FreeImage_Initialise();
    #endif
}

StyleTransfer::~StyleTransfer()
{
}

void StyleTransfer::createTransferFunctionTexture()
{
    glGenTextures(1, &transferFunctionTexture);
    glBindTexture(GL_TEXTURE_1D, transferFunctionTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void StyleTransfer::createStyleFunctionTexture()
{
    // load raw data from texture folder
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    //pointer to the image, once loaded
    FIBITMAP *dib = nullptr;
    //pointer to the image data
    wholeData = new uint8_t[256 * 256 * AVAILABLE_STYLE_COUNT * 4];
    //image width and height
    unsigned int width = 0, height = 0, bitsPerPixel;

    for (int i = 0; i < AVAILABLE_STYLE_COUNT; i++) {
        //check the file signature and deduce its format
        fif = FreeImage_GetFileType(("resources/materials/litsphere (" + std::to_string(i + 1) + ").png").c_str(), 0);

        //if still unknown, try to guess the file format from the file extension
        if (fif == FIF_UNKNOWN)
            fif = FreeImage_GetFIFFromFilename(("resources/materials/litsphere (" + std::to_string(i + 1) + ").png").c_str());

        //if still unkown, return failure
        if (fif == FIF_UNKNOWN) return;

        //check that the plugin has reading capabilities and load the file
        if (FreeImage_FIFSupportsReading(fif))
            dib = FreeImage_Load(fif, ("resources/materials/litsphere (" + std::to_string(i + 1) + ").png").c_str());

        //if the image failed to load, return failure
        if (!dib) return;

        // always convert to 32
        dib = FreeImage_ConvertTo32Bits(dib);
        //retrieve the image data
        BYTE *bits = FreeImage_GetBits(dib);
        //get the image width and height
        width = FreeImage_GetWidth(dib);
        height = FreeImage_GetHeight(dib);
        bitsPerPixel = FreeImage_GetBPP(dib);

        //if this somehow one of these failed (they shouldn't), return failure
        if ((bits == 0) || (width == 0) || (height == 0) || width != 256 || height != 256) return;

        // copy raw data
        memcpy(&wholeData[i * 256 * 256 * 4], bits, 256 * 256 * 4);
        //Free FreeImage's copy of the data
        FreeImage_Unload(dib);
    }

    glGenTextures(1, &styleFunctionTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, styleFunctionTexture);
    // set reasonable texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, 256, 256, AVAILABLE_STYLE_COUNT, 0, GL_BGRA, GL_UNSIGNED_BYTE, wholeData);
}

void StyleTransfer::createIndexFunctionTexture()
{
    glGenTextures(1, &indexFunctionTexture);
    glBindTexture(GL_TEXTURE_1D, indexFunctionTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void StyleTransfer::updateIndexFunctionTexture()
{
    glBindTexture(GL_TEXTURE_1D, indexFunctionTexture);
    indexFunction = new float[TransferFunction::getControlPoints().size()];

    for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
        indexFunction[i] = (float)availableStyles[i] / (AVAILABLE_STYLE_COUNT - 1);
    }

    glTexImage1D(GL_TEXTURE_1D, 0, GL_INTENSITY, TransferFunction::getControlPoints().size(), 0, GL_LUMINANCE, GL_FLOAT, indexFunction);
}

void StyleTransfer::updateTransferFunctionTexture()
{
    int controlPointsSize = TransferFunction::getControlPoints().size();

    for (int i = 0; i < controlPointsSize; i++) {
        TransferFunction::getControlPointColors(i)[0] = i / (controlPointsSize - 1);
    }

    TransferFunction::getLinearFunction(this->transferTexture);
    glBindTexture(GL_TEXTURE_1D, transferFunctionTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RG8, 256, 0, GL_RG, GL_FLOAT, this->transferTexture);
}

void StyleTransfer::loadStyles()
{
    if (stylesLoaded) return;

    createIndexFunctionTexture();
    createTransferFunctionTexture();
    createStyleFunctionTexture();
    stylesLoaded = true;
}

const std::string StyleTransfer::styleTextList = "Default,Plastic Red,Green Shin,Ceramic Yellow,Aniso Metal,Sea Pebble,Marble,Yellow Wax,Shin Orange,Aniso Red,Beige Ceramic,Diffuse,Crest,Green Marble,Pink Plastic,Shin Aquamarine,Blue Spec,Green Pea,Gray,Brown,Green,Dark Glass,Gray Metal,Wax Yellow,Shinny Green,Ceramic Brown,Polished Wood,Fire,Coral,Rough Metal,Shinny Marble,Spec Grey,Shinny Grey,Border Grey";

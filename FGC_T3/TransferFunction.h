#pragma
#include "commons.h"
#include "Spline.h"

class ControlPoint {
    public:
        float rgba[4];
        int isoValue;

        void create(int r, int g, int b, int alpha, int isovalue);
        friend bool operator<(ControlPoint const &a, ControlPoint const &b);
};

class TransferFunction {
    private:
        static std::vector<ControlPoint> controlPoints;
    public:
        static void addControlPoint(int r, int g, int b, int alpha, int isovalue);
        static void deleteAlphaControlPoint(unsigned const int index);
        static void getSmoothFunction(glm::vec4 *dst[256]);
        static void getLinearFunction(glm::vec4 dst[256]);
        static void getLinearFunction(glm::vec2 dst[256]);
        static float *getControlPointColors(unsigned const int index);

        static const std::vector<ControlPoint> &getControlPoints()
        {
            return controlPoints;
        }

};
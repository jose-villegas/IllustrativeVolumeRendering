#include "TransferFunction.h"

std::vector<ControlPoint> TransferFunction::controlPoints;

void ControlPoint::create(int r, int g, int b, int alpha, int isovalue)
{
    this->rgba[0] = (float)r / 255.0;
    this->rgba[1] = (float)g / 255.0;
    this->rgba[2] = (float)b / 255.0;
    this->rgba[3] = (float)alpha / 255.0;
    this->isoValue = isovalue;
}

void TransferFunction::addControlPoint(int r, int g, int b, int alpha, int isovalue)
{
    if (alpha < 0) {
        alpha = 0;
    }

    if (isovalue < 0) {
        isovalue = 0;
    }

    if (alpha > 255) {
        alpha = 255;
    }

    if (isovalue > 255) {
        isovalue = 255;
    }

    ControlPoint nControlPoint;
    nControlPoint.create(r, g, b, alpha, isovalue);
    auto it = std::lower_bound(controlPoints.begin(), controlPoints.end(), nControlPoint);

    if (!controlPoints.empty() && it != controlPoints.end()) { // Real bad code to force isoValue uniqueness
        if ((it)->isoValue == nControlPoint.isoValue) {
            nControlPoint.isoValue--;

            if (nControlPoint.isoValue < 1) {
                nControlPoint.isoValue = 1;
                it++;
            }
        }

        auto uniqItLeft = it;
        auto uniqItRight = it;
        int previusVal = nControlPoint.isoValue;

        while (uniqItLeft != controlPoints.begin()) {
            if ((uniqItLeft - 1)->isoValue == previusVal) {
                (uniqItLeft - 1)->isoValue--;
            }

            previusVal = (uniqItLeft - 1)->isoValue;
            uniqItLeft--;
        }

        while (uniqItRight != controlPoints.end() - 1) {
            if ((uniqItRight + 1)->isoValue == previusVal) {
                (uniqItRight + 1)->isoValue++;
            }

            previusVal = (uniqItRight)->isoValue;
            uniqItRight++;
        }
    }

    controlPoints.insert(it, nControlPoint);
}

void TransferFunction::getSmoothFunction(glm::vec4 *dst[256])
{
    // TODO
}

void TransferFunction::getLinearFunction(glm::vec4 dst[256])
{
    std::vector<double> channel[5];
    tk::Spline channelSpline[4];

    // Control Points
    for (int i = 0; i < controlPoints.size(); i++) {
        channel[0].push_back(controlPoints[i].rgba[0]);
        channel[1].push_back(controlPoints[i].rgba[1]);
        channel[2].push_back(controlPoints[i].rgba[2]);
        channel[3].push_back(controlPoints[i].rgba[3]);
        channel[4].push_back(controlPoints[i].isoValue);
    }

    for (int i = 0; i < 4; i++) {
        // channelSplineThread(channelSpline[i], channel[4], &channel[i], dst);
        channelSpline[i].set_points(channel[4], channel[i], false);
        double min = std::numeric_limits<double>::infinity(), max = 0, current = 0;
        channel[i].clear();

        for (int k = 0; k < 256; k++) {
            current = channelSpline[i](k);
            // Max Min Value
            max = current > max ? current : max;
            min = current < min ? current : min;
            channel[i].push_back(current);
        }
    }

    for (int i = 0; i < 256; i++) {
        dst[i] = glm::vec4(channel[0][i], channel[1][i], channel[2][i], channel[3][i]);
    }
}

void TransferFunction::deleteAlphaControlPoint(unsigned const int index)
{
    controlPoints.erase(controlPoints.begin() + index);
}

float *TransferFunction::getControlPointColors(unsigned const int index)
{
    return controlPoints[index].rgba;
}

bool operator<(ControlPoint const &a, ControlPoint const &b)
{
    return a.isoValue < b.isoValue;
}


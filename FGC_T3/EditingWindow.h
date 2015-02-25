#pragma once
#include "Commons.h"
#include "RawDataModel.h"
#include "UIBuilder.h"
#include "TransferFunction.h"
#define DRAG_TOLERANCE 7.5f

class EditingWindow {
    private:
        sf::CircleShape circle;
        sf::RectangleShape line;
        sf::RectangleShape indicator;
        RawDataModel *rawModel;
        std::array<float, 256> histogram;
        bool isHistLoaded;
        bool dragStarted;
        int mouseOverIndex;

        bool isMouseOver();
        void drawControlPointCircles(int i);
        void drawHistogram();
        void drawTransferFuncPlot(int i);
        void initRenderContext();
        void updateTransferFunction();
        static void windowRender(EditingWindow *eWin);

    public:

        std::thread *windowThread;
        sf::RenderWindow *parent;
        sf::RenderWindow *window;

        static bool frameDone;
        bool controlPointChanged;
        bool stop;
        void initOnSeparateThread(sf::RenderWindow *parent, RawDataModel *rawModel);
        void eventHandler(sf::Event event);
        void loadHistogram();
        EditingWindow(void);
        ~EditingWindow(void);

        void drawHistogramAndTransferFunc();
        void RawModel(RawDataModel *val)
        {
            rawModel = val;
        }
};


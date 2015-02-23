#pragma once
#include "Commons.h"

class MainData {
    public:
        //static RawDataModel * currentModel;
        static sf::RenderWindow *rootWindow;
        static sf::Clock *mainClock;
        static sf::Clock *frameClock;
        static const int AVAILABLE_CORES;
};


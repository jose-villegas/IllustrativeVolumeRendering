#include "MainData.h"


sf::RenderWindow *MainData::rootWindow;
sf::Clock *MainData::mainClock = new sf::Clock();
sf::Clock *MainData::frameClock = new sf::Clock();
const int MainData::AVAILABLE_CORES = std::thread::hardware_concurrency();

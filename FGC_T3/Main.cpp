#include "Commons.h"
#include "RawDataModel.h"
#include "MainData.h"
#include "EditingWindow.h"

float deltaTime();
// SFML Context Settings for OpenGL Rendering
sf::ContextSettings openglWindowContext();
// Setup AntTweakBar
void guiSetup(sf::Window &window, UIBuilder &gui);
// GLEW Initializer
void initGlew();
// Main Render-Logic Loop
void Render(sf::RenderWindow &window, sf::Clock &frameClock);
// Active Volume Data
RawDataModel *rawModel;
UIBuilder gui;
EditingWindow eWindow;

glm::vec3 mousePositionNormalized(0.f, 0.f, 0.f);
glm::vec2 initialAngle(0.f);
glm::vec2 currentAngle(0.f);
unsigned int testValue;
unsigned int controlPointCount = 0;
bool arcBallOn = false;

int main()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    desktop.width = 1440;
    desktop.height = 900;
    sf::RenderWindow window(desktop, "Volume Rendering - Style Transfer Function", sf::Style::Default, openglWindowContext());
    // set active main window
    window.setKeyRepeatEnabled(true);
    window.setActive(true);
    // Initialize GLEW
    initGlew();
    // Setup MainEngine to hold important shader data
    rawModel = new RawDataModel();
    MainData::rootWindow = &window;
    // output available cores
    std::cout << "--- Available CPU Cores: " << MainData::AVAILABLE_CORES << std::endl;
    // Control Points
    TransferFunction::addControlPoint(0, 0, 0, 0, 0);
    TransferFunction::addControlPoint(255, 255, 255, 255, 255);
    controlPointCount = TransferFunction::getControlPoints().size();
    // start editing window
    gui.setHwnd(window.getSystemHandle());
    guiSetup(window, gui);
    // start editing window
    eWindow.initOnSeparateThread(&window, rawModel);
    // Initialze Main Loop
    Render(window, *MainData::frameClock);
    // Free Memory
    eWindow.windowThread->join();
    delete rawModel;
    return 0;
}

struct Callbacks {
    static void TW_CALL loadModelClick(void *clientData)
    {
        rawModel->load(rawModel->sModelName, rawModel->width, rawModel->height, rawModel->numCuts);

        if (rawModel->isLoaded) {
            eWindow.loadHistogram();
        }
    }
};

void guiSetup(sf::Window &window, UIBuilder &gui)
{
    gui.init(window.getSize().x, window.getSize().y);
    // Model Loading
    gui.addBar("Archivo");
    gui.setBarPosition("Archivo", 5, window.getSize().y - 155);
    gui.setBarSize("Archivo", 200, 130);
    gui.addFileDialogButton("Archivo", "Seleccionar .RAW", rawModel->sModelName, "");
    gui.addTextfield("Archivo", "Modelo: ", &rawModel->sModelName, "");
    gui.addIntegerNumber("Archivo", "Ancho", &rawModel->width, "");
    gui.addIntegerNumber("Archivo", "Largo", &rawModel->height, "");
    gui.addIntegerNumber("Archivo", "Cortes", &rawModel->numCuts, "");
    gui.addButton("Archivo", "Cargar Modelo Seleccionado", Callbacks::loadModelClick, NULL, "");
    //transfer func
    gui.addBar("Funcion de Transferencia");
    gui.setBarSize("Funcion de Transferencia", 200, 500);
    gui.setBarPosition("Funcion de Transferencia", 5, 5);

    for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
        // gui.addColorControls("Funcion de Transferencia", "Punto " + std::to_string(i + 1), TransferFunction::getControlPointColors(i), "");
        gui.addTextList("Funcion de Transferencia", "Punto " + std::to_string(i + 1), StyleTransfer::styleTextList, &rawModel->stf.availableStyles[i], "");
    }
}

sf::ContextSettings openglWindowContext()
{
    sf::ContextSettings settings;
    settings.depthBits = 32;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 0;
    settings.majorVersion = 3;
    settings.minorVersion = 0;
    return settings;
}

void initGlew()
{
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return;
    }
}

glm::vec3 getArcBallVector(int x, int y)
{
    glm::vec3 P = glm::vec3(1.0 * x / MainData::rootWindow->getSize().x * 2 - 1.0,
                            1.0 * y / MainData::rootWindow->getSize().y * 2 - 1.0,
                            0);
    P.y = -P.y;
    float OP_squared = P.x * P.x + P.y * P.y;

    if (OP_squared <= 1 * 1)
        P.z = sqrt(1 * 1 - OP_squared);  // Pythagore
    else
        P = glm::normalize(P);  // nearest point

    return P;
}

float deltaTime()
{
    return MainData::frameClock->getElapsedTime().asSeconds();
}

void eventHandler(sf::Event &e, sf::RenderWindow &window)
{
    while (window.pollEvent(e)) {
        // Send event to AntTweakBar
        int handled = TwEventSFML(&e, 1, 6);

        if (handled) return;

        if (e.type == sf::Event::Closed) {
            rawModel->isLoaded = false;
            window.close();
        }

        if (e.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            arcBallOn = true;
            initialAngle.x = currentAngle.x = sf::Mouse::getPosition(*MainData::rootWindow).x;
            initialAngle.y = currentAngle.y = sf::Mouse::getPosition(*MainData::rootWindow).y;
        }

        if (e.type == sf::Event::MouseWheelMoved) {
            rawModel->view = glm::translate(rawModel->view, glm::vec3(0.f, 0.f, -e.mouseWheel.delta * deltaTime() * 150.f));
            // recalculate matrices with new values
            rawModel->normalMatrix = glm::inverse(glm::transpose(rawModel->view * rawModel->model));
            rawModel->viewProjection = rawModel->projection * rawModel->view;
            rawModel->modelViewProjection = rawModel->viewProjection * rawModel->model;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            currentAngle.x = sf::Mouse::getPosition(*MainData::rootWindow).x;
            currentAngle.y = sf::Mouse::getPosition(*MainData::rootWindow).y;
        }

        if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
            arcBallOn = false;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            currentAngle.x = currentAngle.x + 35;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            currentAngle.x = currentAngle.x - 35;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            currentAngle.y = currentAngle.y - 35;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            currentAngle.y = currentAngle.y + 35;
        }
    }

    if (controlPointCount != TransferFunction::getControlPoints().size()) {
        TwRemoveAllVars(gui.getBar("Funcion de Transferencia"));

        for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
            // gui.addColorControls("Funcion de Transferencia", "Punto " + std::to_string(i + 1), TransferFunction::getControlPointColors(i), "");
            gui.addTextList("Funcion de Transferencia", "Punto " + std::to_string(i + 1), StyleTransfer::styleTextList, &rawModel->stf.availableStyles[i], "");
        }

        controlPointCount = TransferFunction::getControlPoints().size();
    }

    if (currentAngle != initialAngle) {
        // get rotation angle
        glm::vec3 va = getArcBallVector(initialAngle.x, initialAngle.y);
        glm::vec3 vb = getArcBallVector(currentAngle.x, currentAngle.y);
        float angle = std::acos(std::min(1.0f, glm::dot(va, vb)));
        glm::vec3 axisCameraCoords = glm::cross(va, vb);
        glm::mat3 cameraToObject = glm::inverse(glm::mat3(rawModel->viewProjection) * glm::mat3(rawModel->modelViewProjection));
        glm::vec3 axisObjectCoords = cameraToObject * axisCameraCoords;
        // rotate around center
        rawModel->model = glm::translate(rawModel->model, rawModel->cubeSizes / 2.f);
        rawModel->model = glm::rotate(rawModel->model, -angle * 0.5f, axisObjectCoords);
        rawModel->model = glm::translate(rawModel->model, -rawModel->cubeSizes / 2.f);
        // recalculate matrices with new values
        rawModel->modelViewProjection = rawModel->viewProjection * rawModel->model;
        rawModel->normalMatrix = glm::inverse(glm::transpose(rawModel->view * rawModel->model));
        // reset
        currentAngle = initialAngle;
    }
}

void Render(sf::RenderWindow &window, sf::Clock &frameClock)
{
    while (window.isOpen()) {
        // handle input events
        eventHandler(sf::Event(), window);
        // clear previous drawings
        window.clear();
        // Render OpenGL
        rawModel->render();
        // draw ui
        TwDraw();
        // End Frame
        window.display();
        // frame time
        frameClock.restart();
    }
}
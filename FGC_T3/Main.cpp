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
    // eWindow.initOnSeparateThread(&window, rawModel);
    eWindow.window = &window;
    eWindow.RawModel(rawModel);
    // Initialze Main Loop
    Render(window, *MainData::frameClock);
    // Free Memory
    // eWindow.windowThread->join();
    delete rawModel;
    return 0;
}

struct FileDialog {
    static void Save(char *sFilePath)
    {
        OPENFILENAME ofn;       // common dialog box structure
        char szFile[1024] = { "" };       // buffer for file name
        HWND hwnd = nullptr;              // owner window
        HANDLE hf;              // file handle
        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szFile;
        // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
        // use the contents of szFile to initialize itself.
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "TF\0*.tf\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        // Display the Open dialog box.
        if (GetSaveFileName(&ofn) == TRUE) {
            std::string filename = std::string(szFile) + ".tf";
            const size_t last_slash_idx = filename.find_last_of("\\/");
            memcpy(sFilePath, &szFile[last_slash_idx + 1], 1024);
        }
    }

    static void Open(char *sFilePath)
    {
        OPENFILENAME ofn;       // common dialog box structure
        char szFile[1024] = { "" };       // buffer for file name
        HWND hwnd = nullptr;              // owner window
        HANDLE hf;              // file handle
        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szFile;
        // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
        // use the contents of szFile to initialize itself.
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "TF\0*.tf\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        // Display the Open dialog box.
        if (GetOpenFileName(&ofn) == TRUE) {
            std::string filename = std::string(szFile);
            const size_t last_slash_idx = filename.find_last_of("\\/");
            memcpy(sFilePath, &szFile[last_slash_idx + 1], 1024);
        }
    }
};

struct Callbacks {
    static void TW_CALL loadModelClick(void *clientData)
    {
        rawModel->load(rawModel->sModelName, rawModel->width, rawModel->height, rawModel->numCuts);

        if (rawModel->isLoaded) {
            eWindow.loadHistogram();
        }
    }

    static void TW_CALL loadTransferFunction(void *clientData)
    {
        char filename[1024] = {};
        // show save file dialog
        std::thread dialogThread(FileDialog::Open, ((char *)filename));
        dialogThread.join();

        if (!filename[0]) return;

        eWindow.stop = true;
        jsoncons::json inFile = jsoncons::json::parse_file(filename);
        jsoncons::json controlPoints = inFile["Control Points"];
        TransferFunction::Clear();

        for (int i = 0; i < controlPoints.size(); i++) {
            try {
                jsoncons::json &controlPoint = controlPoints[i];
                int opacity = controlPoint["Opacity"].as<int>();
                int isoValue = controlPoint["IsoValue"].as<int>();
                int style = controlPoint["Style"].as<int>();
                // save values
                rawModel->stf.availableStyles[i] = style;
                TransferFunction::addControlPoint(opacity, opacity, opacity, opacity, isoValue);
            } catch (const jsoncons::json_exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }

        TwRemoveAllVars(gui.getBar("Control Points"));

        for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
            // gui.addColorControls("Funcion de Transferencia", "Punto " + std::to_string(i + 1), TransferFunction::getControlPointColors(i), "");
            gui.addTextList("Control Points", "Point " + std::to_string(i + 1), StyleTransfer::styleTextList, &rawModel->stf.availableStyles[i], "");
        }

        eWindow.stop = false;
    }

    static void TW_CALL saveTransferFunction(void *clientData)
    {
        char filename[1024] = {};
        // show save file dialog
        std::thread dialogThread(FileDialog::Save, ((char *)filename));
        dialogThread.join();

        if (!filename[0]) return;

        jsoncons::json outFile;
        jsoncons::json stf(jsoncons::json::an_array);
        eWindow.stop = true;

        for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
            jsoncons::json controlPoint;
            controlPoint["Opacity"] = (int)(TransferFunction::getControlPoint(i).rgba[3] * 255);
            controlPoint["IsoValue"] = (int)TransferFunction::getControlPoint(i).isoValue;
            controlPoint["Style"] = (int)rawModel->stf.availableStyles[i];
            // add to final json
            stf.add(controlPoint);
        }

        outFile["Control Points"] = stf;
        // save output to file
        std::ofstream outfile(filename);
        outfile << jsoncons::pretty_print(outFile);
        std::cout << jsoncons::pretty_print(outFile) << std::endl;
        outfile.close();
        eWindow.stop = false;
    }
};

void guiSetup(sf::Window &window, UIBuilder &gui)
{
    gui.init(window.getSize().x, window.getSize().y);
    // Model Loading
    gui.addBar("Volumetric Data");
    gui.setBarPosition("Volumetric Data", 5, window.getSize().y - 155);
    gui.setBarSize("Volumetric Data", 200, 130);
    gui.addFileDialogButton("Volumetric Data", "Load from .RAW", rawModel->sModelName, "");
    gui.addTextfield("Volumetric Data", "Model name: ", &rawModel->sModelName, "");
    gui.addIntegerNumber("Volumetric Data", "Width", &rawModel->width, "");
    gui.addIntegerNumber("Volumetric Data", "Height", &rawModel->height, "");
    gui.addIntegerNumber("Volumetric Data", "Depth", &rawModel->numCuts, "");
    gui.addButton("Volumetric Data", "Load selected model", Callbacks::loadModelClick, NULL, "");
    // transfer func save-load
    gui.addBar("Transfer Function");
    gui.setBarSize("Transfer Function", 200, 80);
    gui.setBarPosition("Transfer Function", 5, window.getSize().y - 155 - 80 - 5);
    gui.addButton("Transfer Function", "Cargar de .TF", Callbacks::loadTransferFunction, NULL, "");
    gui.addButton("Transfer Function", "Guardar en .TF", Callbacks::saveTransferFunction, NULL, "");
    //transfer func
    gui.addBar("Control Points");
    gui.setBarSize("Control Points", 150, 130 + 80 + 5);
    gui.setBarPosition("Control Points", 210, window.getSize().y - 155 - 80 - 5);

    for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
        // gui.addColorControls("Funcion de Transferencia", "Punto " + std::to_string(i + 1), TransferFunction::getControlPointColors(i), "");
        gui.addTextList("Control Points", "Point " + std::to_string(i + 1), StyleTransfer::styleTextList, &rawModel->stf.availableStyles[i], "");
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

        eWindow.eventHandler(e);

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
        TwRemoveAllVars(gui.getBar("Control Points"));

        for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
            // gui.addColorControls("Funcion de Transferencia", "Punto " + std::to_string(i + 1), TransferFunction::getControlPointColors(i), "");
            gui.addTextList("Control Points", "Point " + std::to_string(i + 1), StyleTransfer::styleTextList, &rawModel->stf.availableStyles[i], "");
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
        sf::Event e;
        // handle input events
        eventHandler(e, window);
        // clear previous drawings
        window.clear();
        // Render OpenGL
        rawModel->render();
        eWindow.drawHistogramAndTransferFunc();
        // draw ui
        TwDraw();
        // End Frame
        window.display();
        // frame time
        frameClock.restart();
    }
}
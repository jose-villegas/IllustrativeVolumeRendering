#include "EditingWindow.h"

EditingWindow::EditingWindow(void)
{
    histogram.fill(0);
    dragStarted = false;
    isHistLoaded = false;
    mouseOverIndex = -1;
    rawModel = NULL;
    windowThread = NULL;
    parent = window = NULL;
    this->indicator = sf::RectangleShape();
    this->line = sf::RectangleShape();
    this->circle = sf::CircleShape(4);
    // Setup Figures
    this->circle.setOutlineThickness(1);
    this->line.setFillColor(sf::Color::Red);
    this->line.rotate(270);
}

EditingWindow::~EditingWindow(void)
{
}

void EditingWindow::initOnSeparateThread(sf::RenderWindow *parent, RawDataModel *rawModel)
{
    this->parent = parent;
    this->rawModel = rawModel;
    windowThread = new std::thread(&EditingWindow::windowRender, this);
}

void EditingWindow::windowRender(EditingWindow *eWin)
{
    // Drawing Figures
    sf::Event event;
    // AntTweakBar Handling
    UIBuilder ui;
    // Dragging Control Variables
    eWin->initRenderContext();
    // Histogram Drawing and Control
    int index = 0;

    while (eWin->parent->isOpen() && eWin->window->isOpen()) {
        eWin->controlPointChanged = false;

        while (eWin->window->pollEvent(event)) {
            if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
                eWin->updateTransferFunction();
            }
        }

        eWin->window->clear(sf::Color::Color(20, 20, 20, 255));
        eWin->drawHistogramAndTransferFunc();
        // draw
        eWin->window->display();
    }

    return;
}

void EditingWindow::loadHistogram()
{
    isHistLoaded = false;
    this->histogram.fill(0);
    int max = 0;

    for (int i = 0; i < rawModel->width * rawModel->height * rawModel->numCuts; i++) {
        unsigned int index = (int)(rawModel->dataScalars[i] * 255.f);
        histogram[index] = histogram[index] + 1;
        max < histogram[index] ? max = histogram[index] : 0;
    }

    for (int i = 0; i < 256;  i++) {
        histogram[i] /= max;
        histogram[i] = std::log(histogram[i] + 1) * (1.f / log(2)); // scale
    }

    isHistLoaded = true;
}

bool EditingWindow::isMouseOver()
{
    sf::Vector2f mousePos(sf::Mouse::getPosition(*this->window));

    if (mousePos.x > circle.getPosition().x - DRAG_TOLERANCE &&
            mousePos.x <= circle.getPosition().x + circle.getGlobalBounds().width + DRAG_TOLERANCE &&
            mousePos.y > circle.getPosition().y - DRAG_TOLERANCE &&
            mousePos.y <= circle.getPosition().y + circle.getGlobalBounds().height + DRAG_TOLERANCE
       ) {
        return true;
    } else {
        return false;
    }
}

void EditingWindow::updateTransferFunction()
{
    if (sf::Mouse::getPosition(*this->window).y > 2 && sf::Mouse::getPosition(*this->window).x > 0 &&
            sf::Mouse::getPosition(*this->window).x < 769 && sf::Mouse::getPosition(*this->window).y < 259) {
        TransferFunction::addControlPoint(255, 255, 255, 255 - sf::Mouse::getPosition(*this->window).y + 3,
                                          sf::Mouse::getPosition(*this->window).x / 3);
        this->controlPointChanged = true;
    }
}

void EditingWindow::initRenderContext()
{
    sf::ContextSettings settings;
    settings.depthBits = 32;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 8;
    settings.majorVersion = 4;
    settings.minorVersion = 0;
    this->window = new sf::RenderWindow(sf::VideoMode(775, 285, 32), "Funcion de Transferencia", sf::Style::Titlebar, settings);
    this->window->setPosition(sf::Vector2i(0, 0));
}

void EditingWindow::drawHistogram()
{
    if (!this->isHistLoaded) return;

    for (int i = 0; i < 256; i++) {
        // Histogram Values
        this->line.setSize(sf::Vector2f(this->histogram[i] * 256.0f, 2));
        this->line.setPosition(5 + i * 3, 260);
        this->window->draw(this->line);
        /// Transfer Function Result
        glm::vec4 color = this->rawModel->transferFunc[i];
        this->indicator.setSize(sf::Vector2f(3, 10));
        this->indicator.setPosition(5 + i * 3, 273);
        // this->indicator.setFillColor(sf::Color((int)(color.r * 255.f), (int)(color.g * 255.f), (int)(color.b * 255.f), (int)(color.a * 255.f)));
        this->indicator.setFillColor(sf::Color((int)(color.a * 255.f), (int)(color.a * 255.f), (int)(color.a * 255.f), (int)(color.a * 255.f)));
        this->window->draw(this->indicator);
        // Iso Value Indicators
        this->indicator.setSize(sf::Vector2f(2, 10));
        this->indicator.setPosition(5 + i * 3, 260);
        this->indicator.setFillColor(sf::Color(i, i, i, i));
        this->window->draw(this->indicator);
    }
}

void EditingWindow::drawControlPointCircles(int i)
{
    // Circles for Controls Points
    sf::Color rgba;
    rgba.r = TransferFunction::getControlPoints()[i].rgba[0] * 255;
    rgba.g = TransferFunction::getControlPoints()[i].rgba[1] * 255;
    rgba.b = TransferFunction::getControlPoints()[i].rgba[2] * 255;
    rgba.a = TransferFunction::getControlPoints()[i].rgba[3] * 255;
    this->circle.setOutlineThickness(1);
    this->circle.setFillColor(rgba);
    this->circle.setOutlineColor(sf::Color::Cyan);
    this->circle.setPosition(TransferFunction::getControlPoints()[i].isoValue * 3 - 3, 255 - TransferFunction::getControlPoints()[i].rgba[3] * 255);

    if (isMouseOver()) {
        this->circle.setOutlineColor(sf::Color::Green);

        if (this->mouseOverIndex == -1) {
            this->mouseOverIndex = i;
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && i == this->mouseOverIndex) {
            this->controlPointChanged = true;

            if (!this->dragStarted) {
                this->dragStarted = true;
            } else {
                this->circle.setOutlineThickness(2);
                int finalIsoValue = sf::Mouse::getPosition(*this->window).x / 3;
                int finalAlphaValue = 255 - sf::Mouse::getPosition(*this->window).y + 3;
                TransferFunction::deleteAlphaControlPoint(i);
                finalIsoValue = i == 0 ? 0 : i == TransferFunction::getControlPoints().size() ? 255 : finalIsoValue;
                TransferFunction::addControlPoint(rgba.r, rgba.g, rgba.b, finalAlphaValue, finalIsoValue);
            }
        } else if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && i == this->mouseOverIndex) {
            if (i > 0 && i < TransferFunction::getControlPoints().size() - 1) {
                TransferFunction::deleteAlphaControlPoint(i);
            }
        } else {
            this->mouseOverIndex = -1;
            this->dragStarted = false;
        }
    }

    this->window->draw(this->circle);
}

void EditingWindow::drawTransferFuncPlot(int i)
{
    // Plotting lines
    if (i < TransferFunction::getControlPoints().size() - 1) {
        sf::RectangleShape plotLine;
        sf::Vector2f nextPos = sf::Vector2f(TransferFunction::getControlPoints().at(i + 1).isoValue * 3 - 3 + 4,
                                            255 - TransferFunction::getControlPoints().at(i + 1).rgba[3] * 255 + 4);
        sf::Vector2f currentPos = sf::Vector2f(this->circle.getPosition().x + 4, this->circle.getPosition().y + 4);
        float xDiff = currentPos.x - nextPos.x;
        float yDiff = currentPos.y - nextPos.y;
        float angle = atan2(yDiff, xDiff) * (180 / 3.14);
        float distSize = sqrt(pow(xDiff, 2) + pow(yDiff, 2));
        plotLine.rotate(90 + angle);
        plotLine.setPosition(currentPos);
        plotLine.setSize(sf::Vector2f(1, distSize));
        this->window->draw(plotLine);
    }
}

void EditingWindow::drawHistogramAndTransferFunc()
{
    drawHistogram();

    for (int i = 0; i < TransferFunction::getControlPoints().size(); i++) {
        drawControlPointCircles(i);
        drawTransferFuncPlot(i);
    }

    // Update Transfer Function Real-Time
    TransferFunction::getLinearFunction(this->rawModel->transferFunc);
    this->rawModel->updateTransferFunctionTexture();
}

bool EditingWindow::frameDone = false;

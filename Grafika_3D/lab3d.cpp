#include "pch.h"
#include "Element.h"
#include "Board.h"

// 2 < size < 8
#define SIZE 4
// 0 < bomb count < size*size*6
#define BOMBCOUNT 10

typedef sf::Event sfe;
typedef sf::Keyboard sfk;

void initOpenGL(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void reshapeScreen(sf::Vector2u size)
{
    glViewport(0, 0, (GLsizei)size.x, (GLsizei)size.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLdouble)size.x / (GLdouble)size.y, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawScene(Spherical camera)
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    Spherical north_of_camera(camera.distance, camera.theta, camera.phi + 0.01f);
    gluLookAt(camera.getX(), camera.getY(), camera.getZ(),
        0.0, 0.0, 0.0,
        north_of_camera.getX(), north_of_camera.getY(), north_of_camera.getZ());
}

int main()
{
    bool running = true;
    sf::ContextSettings context(24, 0, 0, 4, 5);
    sf::RenderWindow window(sf::VideoMode(1200, 700), "Saper 3D", 7U, context);
    sf::Clock deltaClock;
    sf::Vector2i mouse_last_position(0, 0);
    Spherical camera(SIZE * 2 + 5, 1.0f, 0.2f);

    ImGui::SFML::Init(window);

    window.setVerticalSyncEnabled(true);

    reshapeScreen(window.getSize());
    initOpenGL();
    
    std::vector<int> numbers;
    Board *board = new Board(SIZE, BOMBCOUNT);
    board->logBoard();

    while (running)
    {
        sfe event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sfe::Closed || (event.type == sfe::KeyPressed && event.key.code == sfk::Escape)) running = false;
            if (event.type == sfe::Resized) reshapeScreen(window.getSize());
            if (event.type == sfe::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                mouse_last_position = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
            }
            if (event.type == sfe::MouseMoved && sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                camera.theta += 2.0f / window.getSize().x * (event.mouseMove.x - mouse_last_position.x);
                camera.phi += 2.0f / window.getSize().y * (event.mouseMove.y - mouse_last_position.y);
                mouse_last_position = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
            }

            if (event.type == sfe::KeyPressed) {
                if (event.key.code >= sfk::Num0 && event.key.code <= sfk::Num9) {
                    int digit = event.key.code - sfk::Num0;
                    numbers.push_back(digit);
                    std::cout << "dodano liczbe: " << digit << std::endl;

                    if (numbers.size() == 3) {
                        bool goodNumbers = true;
                        if (numbers[0] > 6 || numbers[0] < 1) {
                            goodNumbers = false;
                            std::cout << "nieprawidlowe dane";
                        }
                        if (numbers[1] > SIZE || numbers[1] < 1) {
                            goodNumbers = false;
                            std::cout << "nieprawidlowe dane";
                        }
                        if (numbers[2] > SIZE || numbers[2] < 1) {
                            goodNumbers = false;
                            std::cout << "nieprawidlowe dane";
                        }
                        std::string boardName;
                        switch (numbers[0]) {
                        case 1:
                            boardName = "front";
                            break;
                        case 2:
                            boardName = "back";
                            break;
                        case 3:
                            boardName = "top";
                            break;
                        case 4:
                            boardName = "bottom";
                            break;
                        case 5:
                            boardName = "left";
                            break;
                        case 6:
                            boardName = "right";
                            break;
                        }
                        if (goodNumbers) {
                            Element* element = board->getElementFromCoordinates(boardName, numbers[1] - 1, numbers[2] - 1);
                            if (!element->getIsShowed()) {
                                std::cout << "zmieniono stan" << std::endl;
                                element->setShowed(true);
                            }
                        }
                        numbers.clear();
                        std::cout << std::endl;
                    }
                }
                else if (event.key.code == sfk::Backspace) {
                    numbers.clear();
                    std::cout << "reset liczb" << std::endl;
                }
            }
        }

        if (sfk::isKeyPressed(sfk::Left)) { camera.theta -= 0.0174533f; if (camera.theta < 0.0f) camera.theta = 0.0f; }
        if (sfk::isKeyPressed(sfk::Right)) { camera.theta += 0.0174533f; if (camera.theta > (float)(2.0 * std::numbers::pi)) camera.theta = (float)(2.0 * std::numbers::pi); }
        if (sfk::isKeyPressed(sfk::Up)) { camera.phi += 0.0174533f;  if (camera.phi > (float)std::numbers::pi) camera.phi = (float)std::numbers::pi; }
        if (sfk::isKeyPressed(sfk::Down)) { camera.phi -= 0.0174533f; if (camera.phi < 0.0f) camera.phi = 0.0f; }

        drawScene(camera);
        board->drawCube(1.0);

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return 0;
}

#include "engine.h"
#include "basicWorld.h"
#include "characters.h"
// handles world gen / character selection, and starts the main loop
int main(void) {

    const std::array<WorldFunction, 3> worlds = {desertWorld, spaceWorld, volcanoWorld};
    const std::array<CharacterFunction, 3> characters = {makeCowboy, makeAstronaut, makeRobot};

    std::cout << "Select the map:\n1. Desert\n2. Space\n3. Volcano" << std::endl;

    int map = 0;
    std::cin >> map;
    while (map < 1 || map > worlds.size() || std::cin.fail()) {
        std::cout << "Please choose a number between 1 and " << worlds.size() <<  std::endl;

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cin >> map;
    }

    std::cout << "Select character for player one (KB/mouse):\n1. Cowboy\n2. Astronaut\n3. Robot" << std::endl;

    int p1 = 0;
    std::cin >> p1;
    while (p1 < 1 || p1 > characters.size() || std::cin.fail()) {
        std::cout << "Please choose a number between 1 and " << characters.size() << std::endl;

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cin >> p1;
    }

    std::cout << "Select character for player one (KB/mouse):\n1. Cowboy\n2. Astronaut\n3. Robot" << std::endl;

    int p2 = 0;
    std::cin >> p2;
    while (p2 < 1 || p2 > characters.size() || std::cin.fail()) {
        std::cout << "Please choose a number between 1 and " << characters.size()<< std::endl;

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cin >> p2;
    }


    Engine engine(800, 800);

    std::vector<CharacterFunction> playerFuncs;
    playerFuncs.push_back(characters[p1 - 1]);
    playerFuncs.push_back(characters[p2 - 1]);
    worlds[map - 1](engine, playerFuncs);

    engine.doLoopForever();

    return 0;
}
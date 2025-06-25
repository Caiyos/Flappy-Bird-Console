#include <iostream>
#include <fcntl.h>
#include <io.h>

#include "game.hpp"

using namespace std;

//
int main()
{
    _setmode(_fileno(stdout), _O_U8TEXT);

    game::run();

    return 0;
}

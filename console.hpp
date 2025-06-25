#ifndef CONSOLE_HPP_INCLUDED
#define CONSOLE_HPP_INCLUDED

#include <windows.h>

namespace console {
    // Função para definir o tamanho da janela do console
    void SetConsoleWindowSize(int width, int height);

    // Função para esconder o cursor do console
    void HideConsoleCursor();

    // Função para copiar o buffer para o console
    void WriteBufferToConsole(int width, int height, CHAR_INFO buffer[]);
}

#endif // CONSOLE_HPP_INCLUDED

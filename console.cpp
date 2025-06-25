#include <iostream>

#include "console.hpp"

namespace console {

    // Função para definir o tamanho da janela do console
    void SetConsoleWindowSize(int width, int height)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole == INVALID_HANDLE_VALUE)
        {
            std::cerr << "Erro ao obter o handle do console!" << std::endl;
            return;
        }

        // Define as dimensões da janela do console
        SMALL_RECT rect;
        rect.Left = 0;
        rect.Top = 0;
        rect.Right = width - 1;
        rect.Bottom = height - 1;
        SetConsoleWindowInfo(hConsole, TRUE, &rect);

        // Define o buffer de tela para o tamanho desejado
        COORD coord;
        coord.X = width;
        coord.Y = height;
        SetConsoleScreenBufferSize(hConsole, coord);
    }

    // Função para esconder o cursor do console
    void HideConsoleCursor()
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

    // Função para copiar o buffer para o console
    void WriteBufferToConsole(int width, int height, CHAR_INFO buffer[])
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        // Define o Console para exibir simbolos Unicode
        SetConsoleOutputCP(CP_UTF8);

        // Define a área a ser escrita
        COORD bufferSize = {width, height};
        COORD bufferCoord = {0, 0};
        SMALL_RECT writeRegion = {0, 0, width - 1, height - 1};

        // Escreve o buffer no console
        WriteConsoleOutputW(hConsole, buffer, bufferSize, bufferCoord, &writeRegion);
    }

}

#include <iostream>
#include <conio.h>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>

#include "game.hpp"
#include "console.hpp"

using namespace std;

namespace game
{
    typedef struct _BIRD
    {
        float x;
        float y;
        short state;
        float velocityY;
        double deltaY;
    } BIRD;
    BIRD bird;

    typedef struct _GROUND
    {
        float x;
        float y;
        short height;
        float velocityX;
        CHAR_INFO texture[6];
        int width;
    } GROUND;
    GROUND ground;

    typedef struct _PIPE
    {
        float x;
        float y;
        short height;
        float velocityX;
        CHAR_INFO *texture;
        int width;
        bool scored;
    } PIPE;
    vector<PIPE> pipes;

    const short GAP_SIZE = 5;

    CHAR_INFO *pipeTexture;

    // Definir a cores usadas no código
    const WORD COLOR_WHITE  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD COLOR_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;
    const WORD COLOR_GREEN  = FOREGROUND_GREEN;

    // Controlar os estados do jogo
    bool isRunning = true;
    bool isWatting = true;
    bool isGameOver = false;

    int FPS = 0;
    float score = 0.f;

    // Controlar a força gravitacional do jogo
    const float gravity = 1.f;
    const float velocity = -6.f;

    // Buffer para renderizar o jogo na tela do console
    CHAR_INFO buffer[SCREEN_WIDTH * SCREEN_HEIGHT];



    void setup()
    {
        console::SetConsoleWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        console::HideConsoleCursor();

        srand((unsigned)time(NULL));

        ground.texture[0].Char.UnicodeChar = L'▓';
        ground.texture[0].Attributes = COLOR_WHITE;
        ground.texture[1].Char.UnicodeChar = L'▒';
        ground.texture[1].Attributes = COLOR_WHITE;
        ground.texture[2].Char.UnicodeChar = L'▒';
        ground.texture[2].Attributes = COLOR_WHITE;
        ground.texture[3].Char.UnicodeChar = L'▒';
        ground.texture[3].Attributes = COLOR_WHITE;
        ground.texture[4].Char.UnicodeChar = L'▒';
        ground.texture[4].Attributes = COLOR_WHITE;
        ground.texture[5].Char.UnicodeChar = L'▒';
        ground.texture[5].Attributes = COLOR_WHITE;
        ground.height = 3;
        ground.x = 0.f;
        ground.y = SCREEN_HEIGHT - ground.height;
        ground.velocityX = 6.f;
        ground.width = 3;

        // Alocar memória para textura
        pipeTexture = new CHAR_INFO[4];
//        memset(pipeTexture, 0, sizeof(CHAR_INFO) * 4);

        pipeTexture[0].Char.UnicodeChar = L'▓';
        pipeTexture[0].Attributes = COLOR_GREEN;
        pipeTexture[1].Char.UnicodeChar = L'▒';
        pipeTexture[1].Attributes = COLOR_GREEN;
        pipeTexture[2].Char.UnicodeChar = L'▒';
        pipeTexture[2].Attributes = COLOR_GREEN;
        pipeTexture[3].Char.UnicodeChar = L'░';
        pipeTexture[3].Attributes = COLOR_GREEN;

        bird.x = 10.f;
        bird.y = 10.f;
        bird.state = 0;
        bird.velocityY = 0.f;
        bird.deltaY = 0.f;
    }

    void drawChar(int x, int y, WCHAR c, WORD color)
    {
        if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
            return;

        buffer[y * SCREEN_WIDTH + x].Char.UnicodeChar = c;
        buffer[y * SCREEN_WIDTH + x].Attributes = color;
    }

    void drawString(int x, int y, std::wstring c, WORD color = 0x000F)
    {
        for (size_t i = 0; i < c.size(); i++)
        {
            buffer[y * SCREEN_WIDTH + x + i].Char.UnicodeChar = c[i];
            buffer[y * SCREEN_WIDTH + x + i].Attributes = color;
        }
    }

    void drawGround()
    {
        int dx0 = (int)ground.x  % ground.width;
        int dx1 = (int)(dx0 + 1) % ground.width;
        int dx2 = (int)(dx0 + 2) % ground.width;

        for (int i = 0; i < SCREEN_WIDTH - 2;)
        {
            // Linha 1
            drawChar(i, ground.y, ground.texture[dx0].Char.UnicodeChar, ground.texture[dx0].Attributes);
            drawChar(i + 1, ground.y, ground.texture[dx1].Char.UnicodeChar, ground.texture[dx1].Attributes);
            drawChar(i + 2, ground.y, ground.texture[dx2].Char.UnicodeChar, ground.texture[dx2].Attributes);
            // Linha 2
            drawChar(i, ground.y + 1, ground.texture[3].Char.UnicodeChar, ground.texture[3].Attributes);
            drawChar(i + 1, ground.y + 1, ground.texture[4].Char.UnicodeChar, ground.texture[4].Attributes);
            drawChar(i + 2, ground.y + 1, ground.texture[5].Char.UnicodeChar, ground.texture[5].Attributes);
            // Duplicar a linha 2
            drawChar(i, ground.y + 2, ground.texture[3].Char.UnicodeChar, ground.texture[3].Attributes);
            drawChar(i + 1, ground.y + 2, ground.texture[4].Char.UnicodeChar, ground.texture[4].Attributes);
            drawChar(i + 2, ground.y + 2, ground.texture[5].Char.UnicodeChar, ground.texture[5].Attributes);
            i += 3;
        }
    }

    void drawPipes()
    {
        for (auto& pipe : pipes)
        {
            for (int y = pipe.y; y < pipe.y + pipe.height; y++)
            {
                drawChar(pipe.x, y, pipe.texture[0].Char.UnicodeChar, pipe.texture[0].Attributes);
                drawChar(pipe.x + 1, y, pipe.texture[1].Char.UnicodeChar, pipe.texture[1].Attributes);
                drawChar(pipe.x + 2, y, pipe.texture[2].Char.UnicodeChar, pipe.texture[2].Attributes);
                drawChar(pipe.x + 3, y, pipe.texture[3].Char.UnicodeChar, pipe.texture[3].Attributes);
            }
        }
    }

    void drawBird()
    {
        drawChar(bird.x, bird.y, L'►', COLOR_YELLOW);
    }

    // Função para processar interação com o jogador
    void processInput()
    {
        if (kbhit())
        {
            char ch = getch();
            if (ch == 32) // Verifica se o usuário pressionou a barra de espaço
            {
                if (isWatting) isWatting = false;

                if (bird.y > 2)
                {
    //                    bird.y -= 2;
    //                    if (bird.velocityY > 0)
                    bird.velocityY = -0.35;
                }
            }
            if (ch == 27) // Verifica se o usuário pressionou alguma tecla para sair
            {
                isRunning = false;
            }
        }
    }

    // Gerar novos pares de tubos
    void addPipes(double deltaTime)
    {
        static double dt;

        dt += deltaTime;

        if (dt > 5)
        {
            short gapPosition = 3 + rand() % 14;

            PIPE pipeUp;
            pipeUp.texture = pipeTexture;
            pipeUp.height = gapPosition;
            pipeUp.x = SCREEN_WIDTH;
            pipeUp.y = 0;
            pipeUp.velocityX = velocity;
            pipeUp.width = 4;
            pipeUp.scored = false;

            pipes.push_back(pipeUp);

            PIPE pipeDown;
            pipeDown.texture = pipeTexture;
            pipeDown.height = SCREEN_HEIGHT - ground.height - gapPosition - GAP_SIZE;
            pipeDown.x = SCREEN_WIDTH;
            pipeDown.y = ground.y - pipeDown.height;
            pipeDown.velocityX = velocity;
            pipeDown.width = 4;
            pipeDown.scored = false;

            pipes.push_back(pipeDown);

            dt = 0.f;
        }
    }

    // Remover pares de tubos fora da tela
    void removePipes()
    {
        pipes.erase(std::remove_if(pipes.begin(), pipes.end(), [](PIPE pipe)
        {
            return pipe.x < -4;
        }), pipes.end());
    //        for (auto it = pipes.begin(); it != pipes.end(); ) {
    //            if (pipes.at(it).x < -4) {
    //                it = pipes.erase(it);
    //            } else {
    //                ++it;
    //            }
    //        }
    }

    // Verificar colisão com o chão e tubos
    bool collision() {
        if (bird.y >= ground.y)
        {
            bird.y = ground.y - 1;
            bird.velocityY = 0.f;

            return true;
        }

        for (auto& pipe : pipes)
        {
            if (!pipe.scored && pipe.x <= bird.x)
            {
                if (bird.y >= pipe.y && bird.y <= (pipe.y + pipe.height))
                {
                    return true;
                }
            }
        }

        return false;
    }

    // Função para atualizar a lógica do jogo
    void update(double deltaTime)
    {
        // BIRD
        bird.velocityY += gravity * deltaTime;
        bird.y += bird.velocityY;

        // GROUND
        ground.x += ground.velocityX * deltaTime;

        // PIPES
        removePipes();

        for (auto& pipe : pipes)
        {
            pipe.x += pipe.velocityX * deltaTime;

            // SCORE
            if (!pipe.scored && pipe.x < 10 - 4)
            {
                score += .5;
                pipe.scored = true;
            }
        }

        addPipes(deltaTime);

        // GAMEOVER?
        if (collision())
            isGameOver = true;
    }

    // Função para limpar o buffer de tela
    void clearBuffer()
    {
        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                buffer[y * SCREEN_WIDTH + x].Char.UnicodeChar = L' ';
                buffer[y * SCREEN_WIDTH + x].Attributes = 0xFFFF;
            }
        }
    }

    // Função para desenhar a tela do jogo
    void render()
    {
        // Limpa o buffer de tela
        clearBuffer();

        drawGround();
        drawPipes();
        drawBird();

        drawString(1, 1, L"FPS: " + to_wstring(FPS));
        drawString(80, 1, L"SCORE: " + to_wstring((int)score));

        if (isGameOver)
        {
            drawString(41, 11, L"-- GAMEOVER --");
            drawString(36, 13, L"PRESSIONE ESC PARA SAIR");
            drawString(29, 14, L"OU QUALQUER OUTRA TECLA PARA RECOMEÇAR");
        }

        if (isWatting)
            drawString(25, 12, L"-- PRESSIONE A BARRA DE SPAÇO PARA COMEÇAR --");

        console::WriteBufferToConsole(SCREEN_WIDTH, SCREEN_HEIGHT, buffer);
    }

    void calculateFramesPerSecond(double deltaTime)
    {
        static int frames;
        static double dt;

        frames++;
        dt += deltaTime;

        if (dt >= 1.0f)
        {
            FPS = frames;

            frames = 0;
            dt = 0;
        }
    }

    //
    int run()
    {
        // Setup inicial do jogo
        setup();

        using clock = chrono::high_resolution_clock;

        auto lastTime = clock::now();
        double frameTime = 1.0 / 60.0; // Aproximadamente 60 FPS

        // Loop do jogo
        while (isRunning)
        {
            auto current = clock::now();
            chrono::duration<double> elapsed = current - lastTime;
            lastTime = current;
            double deltaTime = elapsed.count();

            // Processa interação com o jogador
            processInput();

            // Atualiza a lógica do jogo
            if (!isWatting && !isGameOver)
                update(frameTime); // Como o FPS é constante, deve passar o frameTime fixo

            // Renderiza a tela do jogo
            render();

            // Calcula o tempo necessário para manter uma taxa de quadros constante
            if (deltaTime < frameTime)
                this_thread::sleep_for(chrono::duration<double>(frameTime - deltaTime));

            current = clock::now();
            elapsed = current - lastTime;
            deltaTime = elapsed.count();

            calculateFramesPerSecond(deltaTime);
        }

        delete[] pipeTexture;

        return 0;
    }
}

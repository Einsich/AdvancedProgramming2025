#include <SDL3/SDL.h>
#include <iostream>
#include <vector>
#include "optick.h"
#include "world.h"
#include <stacktrace>
#include <optional>
#include <thread>
#include "network.h"

void init_world(SDL_Renderer* renderer, World& world);
void render_world(SDL_Window* window, SDL_Renderer* renderer, World& world);

int main(int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL could not initialize! SDL_Error: "
                  << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Advanced Programming Course(Last Name/First Name)",
        1600, 1200,
        SDL_WINDOW_RESIZABLE // вместо SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: "
                  << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: "
                  << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    PlayerId playerId = PlayerId::Invalid;
    PlayerId teammateId = PlayerId::Invalid;
    int userID = -1;
    for (int i = 1; i < argc; i++) {
        int value;
        if (sscanf(argv[i], "--player_id=%d", &value) == 1) {
            std::cout << "--player_id=" << value << std::endl;
            if (value == 1)
            {
                playerId = PlayerId::Player1;
                teammateId = PlayerId::Player2;
            }
            else if (value == 2)
            {
                playerId = PlayerId::Player2;
                teammateId = PlayerId::Player1;
            }
        }
    }
    std::optional<Network> network;
    std::thread recvThread;
    bool handshakeSent = false;
    bool handshakeReceived = false;
    if (playerId != PlayerId::Invalid) {
        network = Network();
        if (!network->Init(GetPortForPlayer(playerId))) {
            std::cerr << "Failed to initialize network\n";
            return 1;
        }

        recvThread = std::thread([&]()
        {
            char buffer[1024];

            while (true)
            {
                size_t bytes;

                if (network->Receive(buffer, sizeof(buffer), bytes))
                {
                    std::cout << std::string_view(buffer, buffer + bytes) << std::endl;
                    handshakeReceived = true;
                }
            }
        });
    }
    std::string text;
    std::getline(std::cin, text); // flush

    while (network && !handshakeSent)
    {
        std::cout << "> ";
        std::getline(std::cin, text);

        if (text == "exit")
            break;

        network->Send(text.data(), text.size(), "127.0.0.1", GetPortForPlayer(teammateId));
        handshakeSent = true;
    }
    while (!handshakeReceived)
    {
        SDL_Delay(100);
    }

    {
        auto world = std::make_shared<World>();

        {
            OPTICK_EVENT("world.init");
            init_world(renderer, *world);
        }

        bool quit = false;
        SDL_Event e;
        Uint64 lastTicks = SDL_GetTicks();

        while (!quit) {

	        OPTICK_FRAME("MainThread");
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
            }
            Uint64 now = SDL_GetTicks();
            float deltaTime = (now - lastTicks) / 1000.0f;
            lastTicks = now;
            {
                OPTICK_EVENT("world.update");
                world->update(deltaTime);
            }

            // Теперь сразу цвет внутри Clear
            SDL_SetRenderDrawColor(renderer, 50, 50, 150, 255);
            SDL_RenderClear(renderer);
            {
                OPTICK_EVENT("world.render");
                // Отрисовка всех игровых объектов
                render_world(window, renderer, *world);
            }

            SDL_RenderPresent(renderer);
        }
    }
    network.reset();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
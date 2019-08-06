#include <iostream>
#include <cstdlib>
#include "spdlog/spdlog.h"
#include "WindowManager.h"

using ::std::unique_ptr;

int main() {
    spdlog::info("Started WindowManager!");

    unique_ptr<WindowManager> wm = WindowManager::Create();
    if (!wm){
        spdlog::error("Failed to initialize window manager.");
        return EXIT_FAILURE;
    }

    wm->Run();
    spdlog::info("Its OK");

    return EXIT_SUCCESS;
}
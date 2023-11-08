#include "Bretema/bm/app.hpp"

BM_FORCE_DISCRETE_GPU;

int main(int argc, char *argv[])
{
    bm::App app { "Bretema Engine", bm::RenderAPI::Vulkan };
    app.runLoop();

    return 0;
}
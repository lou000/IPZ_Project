#include "game.h"
int main()
{
    Game demo;
    if (demo.Construct(680, 340, 2, 2))
        demo.Start();

    return 0;
}

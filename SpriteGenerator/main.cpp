#include "uifunctions.h"


int main()
{
    setinitmode(0);
    JigsawView view;
    view.init(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    setrendermode(RENDER_MANUAL);
    setbkmode(TRANSPARENT);
    setcaption("SpriteGenerator V1.2 by wang yang");
    setfont(20, 10, "ËÎÌו");
    for(view.render(); is_run(); )
    {
        view.update();
        if(!view.getSmoothMode())
            delay_fps(60);
        else Sleep(1);
    }
    return 0;
}
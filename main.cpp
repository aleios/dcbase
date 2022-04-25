#include <kos.h>
#include <gldc/GL/gl.h>
#include <gldc/GL/glkos.h>

#define STBI_ONLY_PNG
#include "stb_image.h"

KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);
extern uint8_t romdisk[];
KOS_INIT_ROMDISK(romdisk);

void setupGL(int w, int h)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
}

void resizeGl(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
}

void update()
{

}

void draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glKosSwapBuffers();
}

int main(int argc, char** argv)
{
    glKosInit();
    setupGL(640, 480);

    int done = 0;
    maple_device_t* p1Controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    while(!done)
    {
        auto* p1ControllerState = static_cast<cont_state_t*>(maple_dev_status(p1Controller));
        if(p1ControllerState->buttons & CONT_START)
            done = 1;

        update();
        draw();
    }

    return 0;
}
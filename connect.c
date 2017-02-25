#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <EGL/egl.h>
#include <GL/gl.h>

void render()
{
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

int main(int argc, char* argv[])
{
    EGLint i;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLint numConfigs;
    EGLint maxConfigs = 10;
    EGLConfig configs[maxConfigs];
    EGLint attribList[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };
    EGLint windowAttribList[] =
    {
        EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
        EGL_NONE
    };
    EGLint eglContextAttribList[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint queryList[] =
    {
        EGL_CLIENT_APIS,
        EGL_EXTENSIONS,
        EGL_VENDOR,
        EGL_VERSION,
        EGL_NONE
    };

    EGLContext context;
    EGLSurface surface;

    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t events[] = { XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS };

    xcb_connection_t *c = xcb_connect(NULL, NULL);
    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
    xcb_window_t window = xcb_generate_id(c);

    xcb_create_window(c, XCB_COPY_FROM_PARENT, window, screen->root, 100, 100, 500, 500,
            10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, events);

    xcb_map_window(c, window);
    xcb_flush(c);

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
    {
        fprintf(stderr, "Unable to get eglDisplay\n");
        exit(0);
    }

    if (!eglInitialize(display, &majorVersion, &minorVersion))
    {
        fprintf(stderr, "eglInitialize failed\n");
        exit(0);
    }

    for (int it = 0; ; it++)
    {
        const char* buffer;
        if (queryList[it] == EGL_NONE)
            break;

        buffer = eglQueryString(display, queryList[it]);
        fprintf(stderr, "%d: %s\n", it, buffer);
    }


    fprintf(stdout, "EGL version is %d.%d\n", majorVersion, minorVersion);

    if (!eglChooseConfig(display, attribList, configs, maxConfigs, &numConfigs))
    {
        fprintf(stderr, "chooseConfig failed: %d\n", eglGetError());
        exit(0);
    }

    fprintf(stdout, "Found %d configs\n", numConfigs);

    for (i = 0; i < numConfigs; i++)
    {
        surface = eglCreateWindowSurface(display, configs[i], window, windowAttribList);
        if (surface == EGL_NO_SURFACE)
        {
            switch(eglGetError())
            {
                case EGL_BAD_MATCH:
                    fprintf(stderr, "EGL_BAD_MATCH\n");
                    break;

                case EGL_BAD_CONFIG:
                    fprintf(stderr, "EGL_BAD_CONFIG\n");
                    break;

               case EGL_BAD_NATIVE_WINDOW:
                    fprintf(stderr, "EGL_BAD_NATIVE_WINDOW\n");
                    break;

               case EGL_BAD_ALLOC:
                    fprintf(stderr, "EGL_BAD_ALLOC\n");
                    break;
            }
            continue;
        }
        fprintf(stdout, "created surface with config %d\n", i);
        break;
    }

    context = eglCreateContext(display, configs[i], EGL_NO_CONTEXT, eglContextAttribList);
    if (context == EGL_NO_CONTEXT)
    {
        fprintf(stderr, "Failed to create eglContext\n");
        exit(0);
    }

    if (!eglMakeCurrent(display, surface, surface, context))
    {
        fprintf(stderr, "Failed to make the context current\n");
        exit(0);
    }

    while (1)
    {
        xcb_generic_event_t* e;

        render();
        eglSwapBuffers(display, surface);
        e = xcb_wait_for_event(c);

        if (!e)
        {
            eglTerminate(display);
            exit(0);
        }

        switch (e->response_type & ~0x80)
        {
            case XCB_EXPOSE:
            {
                xcb_expose_event_t* expose = (xcb_expose_event_t*)e;
                glViewport(0, 0, expose->width, expose->height);
            }
            break;

            case XCB_BUTTON_PRESS:
                eglTerminate(display);
                exit(0);
                break;
        }
    }

    sleep(100);
    return 0;
}


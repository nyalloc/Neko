#include <Neko/Sample.h>
#include <Neko/Neko.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <cassert>

struct NkSampleApp_ {
    SDL_Window* window;
    NkSampleAppState state;
    NkSampleAppResize resize;
    SDL_Event event;
};

NkSampleApp nkCreateSampleApp(const NkSampleAppInfo* desc) {
    struct NkSampleApp_* newSample = (NkSampleApp)malloc(sizeof(struct NkSampleApp_));
    assert(newSample);

    Uint32 flags = 0;

#ifdef NK_BACKEND_VULKAN
    flags |= SDL_WINDOW_VULKAN;
#endif

    newSample->window = SDL_CreateWindow(
        desc->title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        desc->width,
        desc->height,
        flags
    );

    assert(newSample->window);

    return newSample;
}

void nkDestroySampleApp(NkSampleApp sample) {
    SDL_DestroyWindow(sample->window);
    free(sample);
}

NkBool nkSampleAppProcessEvents(NkSampleApp sample, NkSampleAppState* state) {
    sample->state.resize = NULL;

    while (SDL_PollEvent(&sample->event)) {
        if (sample->event.type == SDL_QUIT) {
            return NkFalse;
        }
    }

    if (state) {
        *state = sample->state;
    }

    return NkTrue;
}

NkNativeSurface nkSampleAppGetNativeSurface(NkSampleApp sample) {
    NkNativeSurface nativeSurface;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sample->window, &wmInfo);
    nativeSurface.hwnd = wmInfo.info.win.window;
    nativeSurface.hinstance = wmInfo.info.win.hinstance;

    return nativeSurface;
}

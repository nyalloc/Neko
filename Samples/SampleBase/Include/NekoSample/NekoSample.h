#ifndef NK_SAMPLE_WINDOW_INCLUDE_GUARD
#define NK_SAMPLE_WINDOW_INCLUDE_GUARD

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NkSampleApp_* NkSampleApp;
typedef struct NkSampleAppInfo {
    uint32_t width;
    uint32_t height;
    const char* title;
} NkSampleAppInfo;

typedef struct NkSampleAppResize {
    uint32_t width;
    uint32_t height;
} NkSampleAppResize;

typedef struct NkSampleAppState {
    const NkSampleAppResize* resize;
} NkSampleAppState;

typedef struct NkNativeSurface NkNativeSurface;
typedef enum NkBool NkBool;
typedef enum NkResult NkResult;

NkSampleApp nkCreateSampleApp(const NkSampleAppInfo* desc);
void nkDestroySampleApp(NkSampleApp sample);
NkBool nkSampleAppProcessEvents(NkSampleApp sample, NkSampleAppState* state);
NkNativeSurface nkSampleAppGetNativeSurface(NkSampleApp sample);

#ifdef __cplusplus
}
#endif

#endif /* NK_SAMPLE_WINDOW_INCLUDE_GUARD  */

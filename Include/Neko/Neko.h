#pragma once

#ifndef NEKO_H
#define NEKO_H

/*
    The latest version of this library is available on GitHub;
    https://github.com/nyalloc/neko

    Neko is a graphics API abstraction layer. It intends to sit on top of Vulkan, D3D12 and Metal and bring
    a higher-level Metal / WebGPU style interface. The intended users are developers who want to quickly
    prototype or work on small game engines. I intend to bring the most important capabilities of recent
    APIs, but restore some of the joy working with higher-level APIs.
    https://twitter.com/aras_p/status/1313692668095012875

    Largely inspired by Andre Weissflog's sokol-gfx, but intends to bring more of the current-gen graphics
    API features.
    https://twitter.com/FlohOfWoe/status/1328683854195003392

    Neko is under construction. It's gonna be nasty for a while, and the API will 100% change. Bear with me!

    TODOs:

    There are hacks here, and most of them are in the interest of getting things done while I'map still sane.
    I fully intend to go through all these issues and iron over the issues and flesh out the oversimplifications.
    However I am but one person and this is largely experimental. Set your expectations accordingly.

    * Handles
        Right now each neko object is an opaque pointer handle. These need to be individually allocated, which isn't
        great. The user can provide their own malloc through the NK_MALLOC macro, but most people won't. So I wonder
        if we can do something to improve the default performance here. We can replace the opaque pointers with integer
        ids, using sparse sets to maintain a list of unique ids, and each external handle is just an index into an
        internal array. If we continue using opaque pointers, we can use a small object allocator.
    
    * Device selection
        To make things simple for myself, I'map avoiding exposing an API for in-depth device selection until I've got
        more interesting problems solved. Right now, the CreateDevice() function will simply ensure that the device
        being used is correct for the backend and is compatible with a given surface. Maybe I'll borrow from SYCL and
        expose device selector functions that let users supply a function. I think most people find the Adapter /
        PhysicalDevice API style kind of confusing to begin with.

    * User-specified swapchain format
        Right now the swapchain format is chosen by the backend for simplicity. The user might want to do that themselves.

    * Custom allocator support
        Allocator structs to every create function that can allocate. Just use malloc / free until everything is working.
        Only after that will I refactor to make functions allocator-friendly.

    * Recoverable errors
        Right now, errors are not reported back to the user in an actionable way. They're driven through assertions.
        Introduce "Try" variants of functions that can fail. They will return error codes and return their values via
        arguments. E.g. NkInstance nkCreateInstance() -> NkResult nkTryCreateInstance(NkInstance* instance). To prevent
        code duplication, the old versions of the functions can invoke the Try* version and assert that the result is
        a success.

    * Configurable present modes (FIFO, MAILBOX...)

    * Different backends
        Focusing on Vulkan first. D3D12, Metal and WebGPU as pipedream goals. Not interested in OpenGL support, mapping
        OpenGL to this API sounds like a lot of work with little to no benefit.

    * Runtime backend selection
        Not sure if this is practically useful yet, but it could be interesting to let multiple backends be used at
        runtime or let the user decide which backend to use at runtime. Not worth investing effort into until there is
        a lot more done.

    * C++ wrapper
        C is good for libraries, but I think the majority of people will be using C libraries from a C++ application.
        I'd like to introduce a very thin header-only C++ API wrapper for this library, like Vulkan-Hpp, or mtlpp.
*/

#if defined(_WIN32)
#    define NK_ALIGN_OF(x) __alignof(x)
#    define NK_ALIGN_AS(x) __declspec(align(x))
#    if defined(NK_SHARED_LIBRARY)
#        if defined(NK_IMPLEMENTATION)
#            define NK_EXPORT __declspec(dllexport)
#        else
#            define NK_EXPORT __declspec(dllimport)
#        endif
#    else
#        define NK_EXPORT
#    endif
typedef struct HINSTANCE__* HINSTANCE;
typedef struct HWND__* HWND;
#else
#    define NK_ALIGN_OF(x) __alignof__(x)
#    define NK_ALIGN_AS(x) __attribute__ ((__aligned__(x)))
#    if defined(NK_SHARED_LIBRARY)
#        if defined(NK_IMPLEMENTATION)
#            define NK_EXPORT __attribute__((visibility("default")))
#        else
#            define NK_EXPORT
#        endif
#    else
#        define NK_EXPORT
#    endif
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint32_t NkFlags;

typedef struct NkBindGroupImpl* NkBindGroup;
typedef struct NkBindGroupLayoutImpl* NkBindGroupLayout;
typedef struct NkBufferImpl* NkBuffer;
typedef struct NkCommandBufferImpl* NkCommandBuffer;
typedef struct NkCommandEncoderImpl* NkCommandEncoder;
typedef struct NkComputePassEncoderImpl* NkComputePassEncoder;
typedef struct NkComputePipelineImpl* NkComputePipeline;
typedef struct NkDeviceImpl* NkDevice;
typedef struct NkFenceImpl* NkFence;
typedef struct NkInstanceImpl* NkInstance;
typedef struct NkPipelineLayoutImpl* NkPipelineLayout;
typedef struct NkQuerySetImpl* NkQuerySet;
typedef struct NkQueueImpl* NkQueue;
typedef struct NkRenderBundleImpl* NkRenderBundle;
typedef struct NkRenderBundleEncoderImpl* NkRenderBundleEncoder;
typedef struct NkRenderPassEncoderImpl* NkRenderPassEncoder;
typedef struct NkRenderPipelineImpl* NkRenderPipeline;
typedef struct NkSamplerImpl* NkSampler;
typedef struct NkShaderModuleImpl* NkShaderModule;
typedef struct NkSurfaceImpl* NkSurface;
typedef struct NkSwapChainImpl* NkSwapChain;
typedef struct NkTextureImpl* NkTexture;
typedef struct NkTextureViewImpl* NkTextureView;

typedef enum NkBool {
    NkFalse,
    NkTrue
} NkBool;

typedef enum NkDeviceType {
    NkDeviceType_DiscreteGPU = 0x00000000,
    NkDeviceType_IntegratedGPU = 0x00000001,
    NkDeviceType_CPU = 0x00000002,
    NkDeviceType_Unknown = 0x00000003,
    NkDeviceType_Force32 = 0x7FFFFFFF
} NkDeviceType;

typedef enum NkAddressMode {
    NkAddressMode_Repeat = 0x00000000,
    NkAddressMode_MirrorRepeat = 0x00000001,
    NkAddressMode_ClampToEdge = 0x00000002,
    NkAddressMode_Force32 = 0x7FFFFFFF
} NkAddressMode;

typedef enum NkBindingType {
    NkBindingType_UniformBuffer = 0x00000000,
    NkBindingType_StorageBuffer = 0x00000001,
    NkBindingType_ReadonlyStorageBuffer = 0x00000002,
    NkBindingType_Sampler = 0x00000003,
    NkBindingType_ComparisonSampler = 0x00000004,
    NkBindingType_SampledTexture = 0x00000005,
    NkBindingType_MultisampledTexture = 0x00000006,
    NkBindingType_ReadonlyStorageTexture = 0x00000007,
    NkBindingType_WriteonlyStorageTexture = 0x00000008,
    NkBindingType_Force32 = 0x7FFFFFFF
} NkBindingType;

typedef enum NkBlendFactor {
    NkBlendFactor_Zero = 0x00000000,
    NkBlendFactor_One = 0x00000001,
    NkBlendFactor_SrcColor = 0x00000002,
    NkBlendFactor_OneMinusSrcColor = 0x00000003,
    NkBlendFactor_SrcAlpha = 0x00000004,
    NkBlendFactor_OneMinusSrcAlpha = 0x00000005,
    NkBlendFactor_DstColor = 0x00000006,
    NkBlendFactor_OneMinusDstColor = 0x00000007,
    NkBlendFactor_DstAlpha = 0x00000008,
    NkBlendFactor_OneMinusDstAlpha = 0x00000009,
    NkBlendFactor_SrcAlphaSaturated = 0x0000000A,
    NkBlendFactor_BlendColor = 0x0000000B,
    NkBlendFactor_OneMinusBlendColor = 0x0000000C,
    NkBlendFactor_Force32 = 0x7FFFFFFF
} NkBlendFactor;

typedef enum NkBlendOperation {
    NkBlendOperation_Add = 0x00000000,
    NkBlendOperation_Subtract = 0x00000001,
    NkBlendOperation_ReverseSubtract = 0x00000002,
    NkBlendOperation_Min = 0x00000003,
    NkBlendOperation_Max = 0x00000004,
    NkBlendOperation_Force32 = 0x7FFFFFFF
} NkBlendOperation;

typedef enum NkBufferMapAsyncStatus {
    NkBufferMapAsyncStatus_Success = 0x00000000,
    NkBufferMapAsyncStatus_Error = 0x00000001,
    NkBufferMapAsyncStatus_Unknown = 0x00000002,
    NkBufferMapAsyncStatus_DeviceLost = 0x00000003,
    NkBufferMapAsyncStatus_DestroyedBeforeCallback = 0x00000004,
    NkBufferMapAsyncStatus_UnmappedBeforeCallback = 0x00000005,
    NkBufferMapAsyncStatus_Force32 = 0x7FFFFFFF
} NkBufferMapAsyncStatus;

typedef enum NkCompareFunction {
    NkCompareFunction_Undefined = 0x00000000,
    NkCompareFunction_Never = 0x00000001,
    NkCompareFunction_Less = 0x00000002,
    NkCompareFunction_LessEqual = 0x00000003,
    NkCompareFunction_Greater = 0x00000004,
    NkCompareFunction_GreaterEqual = 0x00000005,
    NkCompareFunction_Equal = 0x00000006,
    NkCompareFunction_NotEqual = 0x00000007,
    NkCompareFunction_Always = 0x00000008,
    NkCompareFunction_Force32 = 0x7FFFFFFF
} NkCompareFunction;

typedef enum NkCreateReadyPipelineStatus {
    NkCreateReadyPipelineStatus_Success = 0x00000000,
    NkCreateReadyPipelineStatus_Error = 0x00000001,
    NkCreateReadyPipelineStatus_DeviceLost = 0x00000002,
    NkCreateReadyPipelineStatus_DeviceDestroyed = 0x00000003,
    NkCreateReadyPipelineStatus_Unknown = 0x00000004,
    NkCreateReadyPipelineStatus_Force32 = 0x7FFFFFFF
} NkCreateReadyPipelineStatus;

typedef enum NkCullMode {
    NkCullMode_None = 0x00000000,
    NkCullMode_Front = 0x00000001,
    NkCullMode_Back = 0x00000002,
    NkCullMode_Force32 = 0x7FFFFFFF
} NkCullMode;

typedef enum NkErrorFilter {
    NkErrorFilter_None = 0x00000000,
    NkErrorFilter_Validation = 0x00000001,
    NkErrorFilter_OutOfMemory = 0x00000002,
    NkErrorFilter_Force32 = 0x7FFFFFFF
} NkErrorFilter;

typedef enum NkErrorType {
    NkErrorType_NoError = 0x00000000,
    NkErrorType_Validation = 0x00000001,
    NkErrorType_OutOfMemory = 0x00000002,
    NkErrorType_Unknown = 0x00000003,
    NkErrorType_DeviceLost = 0x00000004,
    NkErrorType_Force32 = 0x7FFFFFFF
} NkErrorType;

typedef enum NkFenceCompletionStatus {
    NkFenceCompletionStatus_Success = 0x00000000,
    NkFenceCompletionStatus_Error = 0x00000001,
    NkFenceCompletionStatus_Unknown = 0x00000002,
    NkFenceCompletionStatus_DeviceLost = 0x00000003,
    NkFenceCompletionStatus_Force32 = 0x7FFFFFFF
} NkFenceCompletionStatus;

typedef enum NkFilterMode {
    NkFilterMode_Nearest = 0x00000000,
    NkFilterMode_Linear = 0x00000001,
    NkFilterMode_Force32 = 0x7FFFFFFF
} NkFilterMode;

typedef enum NkFrontFace {
    NkFrontFace_CCW = 0x00000000,
    NkFrontFace_CW = 0x00000001,
    NkFrontFace_Force32 = 0x7FFFFFFF
} NkFrontFace;

typedef enum NkIndexFormat {
    NkIndexFormat_Undefined = 0x00000000,
    NkIndexFormat_Uint16 = 0x00000001,
    NkIndexFormat_Uint32 = 0x00000002,
    NkIndexFormat_Force32 = 0x7FFFFFFF
} NkIndexFormat;

typedef enum NkInputStepMode {
    NkInputStepMode_Vertex = 0x00000000,
    NkInputStepMode_Instance = 0x00000001,
    NkInputStepMode_Force32 = 0x7FFFFFFF
} NkInputStepMode;

typedef enum NkLoadOp {
    NkLoadOp_Clear = 0x00000000,
    NkLoadOp_Load = 0x00000001,
    NkLoadOp_Force32 = 0x7FFFFFFF
} NkLoadOp;

typedef enum NkPipelineStatisticName {
    NkPipelineStatisticName_VertexShaderInvocations = 0x00000000,
    NkPipelineStatisticName_ClipperInvocations = 0x00000001,
    NkPipelineStatisticName_ClipperPrimitivesOut = 0x00000002,
    NkPipelineStatisticName_FragmentShaderInvocations = 0x00000003,
    NkPipelineStatisticName_ComputeShaderInvocations = 0x00000004,
    NkPipelineStatisticName_Force32 = 0x7FFFFFFF
} NkPipelineStatisticName;

typedef enum NkPrimitiveTopology {
    NkPrimitiveTopology_PointList = 0x00000000,
    NkPrimitiveTopology_LineList = 0x00000001,
    NkPrimitiveTopology_LineStrip = 0x00000002,
    NkPrimitiveTopology_TriangleList = 0x00000003,
    NkPrimitiveTopology_TriangleStrip = 0x00000004,
    NkPrimitiveTopology_Force32 = 0x7FFFFFFF
} NkPrimitiveTopology;

typedef enum NkQueryType {
    NkQueryType_Occlusion = 0x00000000,
    NkQueryType_PipelineStatistics = 0x00000001,
    NkQueryType_Timestamp = 0x00000002,
    NkQueryType_Force32 = 0x7FFFFFFF
} NkQueryType;

typedef enum NkStencilOperation {
    NkStencilOperation_Keep = 0x00000000,
    NkStencilOperation_Zero = 0x00000001,
    NkStencilOperation_Replace = 0x00000002,
    NkStencilOperation_Invert = 0x00000003,
    NkStencilOperation_IncrementClamp = 0x00000004,
    NkStencilOperation_DecrementClamp = 0x00000005,
    NkStencilOperation_IncrementWrap = 0x00000006,
    NkStencilOperation_DecrementWrap = 0x00000007,
    NkStencilOperation_Force32 = 0x7FFFFFFF
} NkStencilOperation;

typedef enum NkStoreOp {
    NkStoreOp_Store = 0x00000000,
    NkStoreOp_Clear = 0x00000001,
    NkStoreOp_Force32 = 0x7FFFFFFF
} NkStoreOp;

typedef enum NkTextureAspect {
    NkTextureAspect_All = 0x00000000,
    NkTextureAspect_StencilOnly = 0x00000001,
    NkTextureAspect_DepthOnly = 0x00000002,
    NkTextureAspect_Force32 = 0x7FFFFFFF
} NkTextureAspect;

typedef enum NkTextureComponentType {
    NkTextureComponentType_Float = 0x00000000,
    NkTextureComponentType_Sint = 0x00000001,
    NkTextureComponentType_Uint = 0x00000002,
    NkTextureComponentType_DepthComparison = 0x00000003,
    NkTextureComponentType_Force32 = 0x7FFFFFFF
} NkTextureComponentType;

typedef enum NkTextureDimension {
    NkTextureDimension_1D = 0x00000000,
    NkTextureDimension_2D = 0x00000001,
    NkTextureDimension_3D = 0x00000002,
    NkTextureDimension_Force32 = 0x7FFFFFFF
} NkTextureDimension;

typedef enum NkTextureFormat {
    NkTextureFormat_Undefined = 0x00000000,
    NkTextureFormat_R8Unorm = 0x00000001,
    NkTextureFormat_R8Snorm = 0x00000002,
    NkTextureFormat_R8Uint = 0x00000003,
    NkTextureFormat_R8Sint = 0x00000004,
    NkTextureFormat_R16Uint = 0x00000005,
    NkTextureFormat_R16Sint = 0x00000006,
    NkTextureFormat_R16Float = 0x00000007,
    NkTextureFormat_RG8Unorm = 0x00000008,
    NkTextureFormat_RG8Snorm = 0x00000009,
    NkTextureFormat_RG8Uint = 0x0000000A,
    NkTextureFormat_RG8Sint = 0x0000000B,
    NkTextureFormat_R32Float = 0x0000000C,
    NkTextureFormat_R32Uint = 0x0000000D,
    NkTextureFormat_R32Sint = 0x0000000E,
    NkTextureFormat_RG16Uint = 0x0000000F,
    NkTextureFormat_RG16Sint = 0x00000010,
    NkTextureFormat_RG16Float = 0x00000011,
    NkTextureFormat_RGBA8Unorm = 0x00000012,
    NkTextureFormat_RGBA8UnormSrgb = 0x00000013,
    NkTextureFormat_RGBA8Snorm = 0x00000014,
    NkTextureFormat_RGBA8Uint = 0x00000015,
    NkTextureFormat_RGBA8Sint = 0x00000016,
    NkTextureFormat_BGRA8Unorm = 0x00000017,
    NkTextureFormat_BGRA8UnormSrgb = 0x00000018,
    NkTextureFormat_RGB10A2Unorm = 0x00000019,
    NkTextureFormat_RG11B10Ufloat = 0x0000001A,
    NkTextureFormat_RGB9E5Ufloat = 0x0000001B,
    NkTextureFormat_RG32Float = 0x0000001C,
    NkTextureFormat_RG32Uint = 0x0000001D,
    NkTextureFormat_RG32Sint = 0x0000001E,
    NkTextureFormat_RGBA16Uint = 0x0000001F,
    NkTextureFormat_RGBA16Sint = 0x00000020,
    NkTextureFormat_RGBA16Float = 0x00000021,
    NkTextureFormat_RGBA32Float = 0x00000022,
    NkTextureFormat_RGBA32Uint = 0x00000023,
    NkTextureFormat_RGBA32Sint = 0x00000024,
    NkTextureFormat_Depth32Float = 0x00000025,
    NkTextureFormat_Depth24Plus = 0x00000026,
    NkTextureFormat_Depth24PlusStencil8 = 0x00000027,
    NkTextureFormat_Stencil8 = 0x00000028,
    NkTextureFormat_BC1RGBAUnorm = 0x00000029,
    NkTextureFormat_BC1RGBAUnormSrgb = 0x0000002A,
    NkTextureFormat_BC2RGBAUnorm = 0x0000002B,
    NkTextureFormat_BC2RGBAUnormSrgb = 0x0000002C,
    NkTextureFormat_BC3RGBAUnorm = 0x0000002D,
    NkTextureFormat_BC3RGBAUnormSrgb = 0x0000002E,
    NkTextureFormat_BC4RUnorm = 0x0000002F,
    NkTextureFormat_BC4RSnorm = 0x00000030,
    NkTextureFormat_BC5RGUnorm = 0x00000031,
    NkTextureFormat_BC5RGSnorm = 0x00000032,
    NkTextureFormat_BC6HRGBUfloat = 0x00000033,
    NkTextureFormat_BC6HRGBFloat = 0x00000034,
    NkTextureFormat_BC7RGBAUnorm = 0x00000035,
    NkTextureFormat_BC7RGBAUnormSrgb = 0x00000036,
    NkTextureFormat_Force32 = 0x7FFFFFFF
} NkTextureFormat;

typedef enum NkTextureViewDimension {
    NkTextureViewDimension_Undefined = 0x00000000,
    NkTextureViewDimension_1D = 0x00000001,
    NkTextureViewDimension_2D = 0x00000002,
    NkTextureViewDimension_2DArray = 0x00000003,
    NkTextureViewDimension_Cube = 0x00000004,
    NkTextureViewDimension_CubeArray = 0x00000005,
    NkTextureViewDimension_3D = 0x00000006,
    NkTextureViewDimension_Force32 = 0x7FFFFFFF
} NkTextureViewDimension;

typedef enum NkVertexFormat {
    NkVertexFormat_UChar2 = 0x00000000,
    NkVertexFormat_UChar4 = 0x00000001,
    NkVertexFormat_Char2 = 0x00000002,
    NkVertexFormat_Char4 = 0x00000003,
    NkVertexFormat_UChar2Norm = 0x00000004,
    NkVertexFormat_UChar4Norm = 0x00000005,
    NkVertexFormat_Char2Norm = 0x00000006,
    NkVertexFormat_Char4Norm = 0x00000007,
    NkVertexFormat_UShort2 = 0x00000008,
    NkVertexFormat_UShort4 = 0x00000009,
    NkVertexFormat_Short2 = 0x0000000A,
    NkVertexFormat_Short4 = 0x0000000B,
    NkVertexFormat_UShort2Norm = 0x0000000C,
    NkVertexFormat_UShort4Norm = 0x0000000D,
    NkVertexFormat_Short2Norm = 0x0000000E,
    NkVertexFormat_Short4Norm = 0x0000000F,
    NkVertexFormat_Half2 = 0x00000010,
    NkVertexFormat_Half4 = 0x00000011,
    NkVertexFormat_Float = 0x00000012,
    NkVertexFormat_Float2 = 0x00000013,
    NkVertexFormat_Float3 = 0x00000014,
    NkVertexFormat_Float4 = 0x00000015,
    NkVertexFormat_UInt = 0x00000016,
    NkVertexFormat_UInt2 = 0x00000017,
    NkVertexFormat_UInt3 = 0x00000018,
    NkVertexFormat_UInt4 = 0x00000019,
    NkVertexFormat_Int = 0x0000001A,
    NkVertexFormat_Int2 = 0x0000001B,
    NkVertexFormat_Int3 = 0x0000001C,
    NkVertexFormat_Int4 = 0x0000001D,
    NkVertexFormat_Force32 = 0x7FFFFFFF
} NkVertexFormat;

typedef enum NkBufferUsage {
    NkBufferUsage_None = 0x00000000,
    NkBufferUsage_MapRead = 0x00000001,
    NkBufferUsage_MapWrite = 0x00000002,
    NkBufferUsage_CopySrc = 0x00000004,
    NkBufferUsage_CopyDst = 0x00000008,
    NkBufferUsage_Index = 0x00000010,
    NkBufferUsage_Vertex = 0x00000020,
    NkBufferUsage_Uniform = 0x00000040,
    NkBufferUsage_Storage = 0x00000080,
    NkBufferUsage_Indirect = 0x00000100,
    NkBufferUsage_QueryResolve = 0x00000200,
    NkBufferUsage_Force32 = 0x7FFFFFFF
} NkBufferUsage;
typedef NkFlags NkBufferUsageFlags;

typedef enum NkColorWriteMask {
    NkColorWriteMask_None = 0x00000000,
    NkColorWriteMask_Red = 0x00000001,
    NkColorWriteMask_Green = 0x00000002,
    NkColorWriteMask_Blue = 0x00000004,
    NkColorWriteMask_Alpha = 0x00000008,
    NkColorWriteMask_All = 0x0000000F,
    NkColorWriteMask_Force32 = 0x7FFFFFFF
} NkColorWriteMask;
typedef NkFlags NkColorWriteMaskFlags;

typedef enum NkMapMode {
    NkMapMode_Read = 0x00000001,
    NkMapMode_Write = 0x00000002,
    NkMapMode_Force32 = 0x7FFFFFFF
} NkMapMode;
typedef NkFlags NkMapModeFlags;

typedef enum NkShaderStage {
    NkShaderStage_None = 0x00000000,
    NkShaderStage_Vertex = 0x00000001,
    NkShaderStage_Fragment = 0x00000002,
    NkShaderStage_Compute = 0x00000004,
    NkShaderStage_Force32 = 0x7FFFFFFF
} NkShaderStage;
typedef NkFlags NkShaderStageFlags;

typedef enum NkTextureUsage {
    NkTextureUsage_None = 0x00000000,
    NkTextureUsage_CopySrc = 0x00000001,
    NkTextureUsage_CopyDst = 0x00000002,
    NkTextureUsage_Sampled = 0x00000004,
    NkTextureUsage_Storage = 0x00000008,
    NkTextureUsage_RenderAttachment = 0x00000010,
    NkTextureUsage_Force32 = 0x7FFFFFFF
} NkTextureUsage;
typedef NkFlags NkTextureUsageFlags;

typedef struct NkBindGroupEntry {
    uint32_t binding;
    NkBuffer buffer;
    uint64_t offset;
    uint64_t size;
    NkSampler sampler;
    NkTextureView textureView;
} NkBindGroupEntry;

typedef struct NkBindGroupLayoutEntry {
    uint32_t binding;
    NkShaderStageFlags visibility;
    NkBindingType type;
    NkBool hasDynamicOffset;
    uint64_t minBufferBindingSize;
    NkBool multisampled;
    NkTextureViewDimension viewDimension;
    NkTextureComponentType textureComponentType;
    NkTextureFormat storageTextureFormat;
} NkBindGroupLayoutEntry;

typedef struct NkBlendInfo {
    NkBlendOperation operation;
    NkBlendFactor srcFactor;
    NkBlendFactor dstFactor;
} NkBlendInfo;

typedef struct NkBufferInfo {
    NkBufferUsageFlags usage;
    uint64_t size;
    NkBool mappedAtCreation;
} NkBufferInfo;

typedef struct NkFloat3 {
    float x;
    float y;
    float z;
} NkFloat3;

typedef struct NkFloat4 {
    float x;
    float y;
    float z;
    float w;
} NkFloat4;

typedef struct NkColor {
    float r;
    float g;
    float b;
    float a;
} NkColor;

typedef struct NkExtent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} NkExtent3D;

typedef struct NkFenceInfo {
    uint64_t initialValue;
} NkFenceInfo;

typedef struct NkOrigin3D {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} NkOrigin3D;

typedef struct NkPipelineLayoutInfo {
    uint32_t bindGroupLayoutCount;
    const NkBindGroupLayout* bindGroupLayouts;
} NkPipelineLayoutInfo;

typedef struct NkProgrammableStageInfo {
    NkShaderModule module;
    const char* entryPoint;
} NkProgrammableStageInfo;

typedef struct NkQuerySetInfo {
    NkQueryType type;
    uint32_t count;
    const NkPipelineStatisticName* pipelineStatistics;
    uint32_t pipelineStatisticsCount;
} NkQuerySetInfo;

typedef struct NkRasterizationStateInfo {
    NkFrontFace frontFace;
    NkCullMode cullMode;
    int32_t depthBias;
    float depthBiasSlopeScale;
    float depthBiasClamp;
    NkBool clampDepth;
} NkRasterizationStateInfo;

typedef struct NkRenderBundleEncoderInfo {
    uint32_t colorFormatsCount;
    const NkTextureFormat* colorFormats;
    NkTextureFormat depthStencilFormat;
    uint32_t sampleCount;
} NkRenderBundleEncoderInfo;

typedef struct NkRenderPassDepthStencilAttachmentInfo {
    NkTextureView attachment;
    NkLoadOp depthLoadOp;
    NkStoreOp depthStoreOp;
    float clearDepth;
    NkBool depthReadOnly;
    NkLoadOp stencilLoadOp;
    NkStoreOp stencilStoreOp;
    uint32_t clearStencil;
    NkBool stencilReadOnly;
} NkRenderPassDepthStencilAttachmentInfo;

typedef struct NkSamplerInfo {
    NkAddressMode addressModeU;
    NkAddressMode addressModeV;
    NkAddressMode addressModeW;
    NkFilterMode magFilter;
    NkFilterMode minFilter;
    NkFilterMode mipmapFilter;
    float lodMinClamp;
    float lodMaxClamp;
    NkCompareFunction compare;
    uint32_t maxAnisotropy;
} NkSamplerInfo;

typedef struct NkShaderModuleInfo {
    uint32_t size;      // size of the shader source in bytes
    const void* source; // pointer to the shader source
} NkShaderModuleInfo;

typedef struct NkStencilStateFaceInfo {
    NkCompareFunction compare;
    NkStencilOperation failOp;
    NkStencilOperation depthFailOp;
    NkStencilOperation passOp;
} NkStencilStateFaceInfo;

typedef struct NkNativeSurface {
#if defined(_WIN32) 
    HINSTANCE hinstance;
    HWND hwnd;
#endif
} NkNativeSurface;

typedef struct NkSurfaceInfo {
    NkNativeSurface native;
} NkSurfaceInfo;

typedef struct NkSwapChainInfo {
    uint32_t width;
    uint32_t height;
} NkSwapChainInfo;

typedef struct NkTextureDataLayout {
    uint64_t offset;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
} NkTextureDataLayout;

typedef struct NkTextureViewInfo {
    NkTextureFormat format;
    NkTextureViewDimension dimension;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerCount;
    NkTextureAspect aspect;
} NkTextureViewInfo;

typedef struct NkVertexAttributeInfo {
    NkVertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
} NkVertexAttributeInfo;

typedef struct NkBindGroupInfo {
    NkBindGroupLayout layout;
    uint32_t entryCount;
    const NkBindGroupEntry* entries;
} NkBindGroupInfo;

typedef struct NkBindGroupLayoutInfo {
    uint32_t entryCount;
    const NkBindGroupLayoutEntry* entries;
} NkBindGroupLayoutInfo;

typedef struct NkBufferCopyView {
    NkTextureDataLayout layout;
    NkBuffer buffer;
} NkBufferCopyView;

typedef struct NkColorStateInfo {
    NkTextureFormat format;
    NkBlendInfo alphaBlend;
    NkBlendInfo colorBlend;
    NkColorWriteMaskFlags writeMask;
} NkColorStateInfo;

typedef struct NkComputePipelineInfo {
    NkPipelineLayout layout;
    NkProgrammableStageInfo computeStage;
} NkComputePipelineInfo;

typedef struct NkDepthStencilStateInfo {
    NkTextureFormat format;
    NkBool depthWriteEnabled;
    NkCompareFunction depthCompare;
    NkStencilStateFaceInfo stencilFront;
    NkStencilStateFaceInfo stencilBack;
    uint32_t stencilReadMask;
    uint32_t stencilWriteMask;
} NkDepthStencilStateInfo;

typedef struct NkRenderPassColorAttachmentInfo {
    NkTextureView attachment;
    NkTextureView resolveTarget;
    NkLoadOp loadOp;
    NkStoreOp storeOp;
    NkColor clearColor;
} NkRenderPassColorAttachmentInfo;

typedef struct NkTextureCopyView {
    NkTexture texture;
    uint32_t mipLevel;
    NkOrigin3D origin;
} NkTextureCopyView;

typedef struct NkTextureInfo {
    NkTextureUsageFlags usage;
    NkTextureDimension dimension;
    NkExtent3D size;
    NkTextureFormat format;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
} NkTextureInfo;

typedef struct NkVertexBufferLayoutInfo {
    uint64_t arrayStride;
    NkInputStepMode stepMode;
    uint32_t attributeCount;
    const NkVertexAttributeInfo* attributes;
} NkVertexBufferLayoutInfo;

typedef struct NkRenderPassInfo {
    uint32_t colorAttachmentCount;
    const NkRenderPassColorAttachmentInfo* colorAttachments;
    const NkRenderPassDepthStencilAttachmentInfo* depthStencilAttachment;
    NkQuerySet occlusionQuerySet;
} NkRenderPassInfo;

typedef struct NkVertexStateInfo {
    NkIndexFormat indexFormat;
    uint32_t vertexBufferCount;
    const NkVertexBufferLayoutInfo* vertexBuffers;
} NkVertexStateInfo;

typedef struct NkRenderPipelineInfo {
    NkPipelineLayout layout;
    NkProgrammableStageInfo vertexStage;
    NkProgrammableStageInfo fragmentStage;
    const NkVertexStateInfo* vertexState;
    NkPrimitiveTopology primitiveTopology;
    const NkRasterizationStateInfo* rasterizationState;
    uint32_t sampleCount;
    const NkDepthStencilStateInfo* depthStencilState;
    uint32_t colorStateCount;
    const NkColorStateInfo* colorStates;
    uint32_t sampleMask;
    NkBool alphaToCoverageEnabled;
} NkRenderPipelineInfo;

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*NkDeviceLostCallback)(const char* message, void* userdata);
typedef void (*NkErrorCallback)(NkErrorType type, const char* message, void* userdata);
typedef void (*NkFenceOnCompletionCallback)(NkFenceCompletionStatus status, void* userdata);

NK_EXPORT NkInstance nkCreateInstance();

// Methods of Buffer
NK_EXPORT void nkDestroyBuffer(NkBuffer buffer);
NK_EXPORT const void* nkBufferGetConstMappedRange(NkBuffer buffer, size_t offset, size_t size);
NK_EXPORT void* nkBufferGetMappedRange(NkBuffer buffer, size_t offset, size_t size);
NK_EXPORT NkBufferMapAsyncStatus nkBufferMap(NkBuffer buffer, NkMapModeFlags mode, size_t offset, size_t size);
NK_EXPORT void nkBufferUnmap(NkBuffer buffer);

// Methods of CommandEncoder
NK_EXPORT NkComputePassEncoder nkCommandEncoderBeginComputePass(NkCommandEncoder commandEncoder);
NK_EXPORT NkRenderPassEncoder nkCommandEncoderBeginRenderPass(NkCommandEncoder commandEncoder, const NkRenderPassInfo* descriptor);
NK_EXPORT void nkCommandEncoderCopyBufferToBuffer(NkCommandEncoder commandEncoder, NkBuffer source, uint64_t sourceOffset, NkBuffer destination, uint64_t destinationOffset, uint64_t size);
NK_EXPORT void nkCommandEncoderCopyBufferToTexture(NkCommandEncoder commandEncoder, const NkBufferCopyView* source, const NkTextureCopyView* destination, const NkExtent3D* copySize);
NK_EXPORT void nkCommandEncoderCopyTextureToBuffer(NkCommandEncoder commandEncoder, const NkTextureCopyView* source, const NkBufferCopyView* destination, const NkExtent3D* copySize);
NK_EXPORT void nkCommandEncoderCopyTextureToTexture(NkCommandEncoder commandEncoder, const NkTextureCopyView* source, const NkTextureCopyView* destination, const NkExtent3D* copySize);
NK_EXPORT NkCommandBuffer nkCommandEncoderFinish(NkCommandEncoder commandEncoder);
NK_EXPORT void nkCommandEncoderInsertDebugMarker(NkCommandEncoder commandEncoder, const char* markerLabel);
NK_EXPORT void nkCommandEncoderPopDebugGroup(NkCommandEncoder commandEncoder);
NK_EXPORT void nkCommandEncoderPushDebugGroup(NkCommandEncoder commandEncoder, const char* groupLabel);
NK_EXPORT void nkCommandEncoderResolveQuerySet(NkCommandEncoder commandEncoder, NkQuerySet querySet, uint32_t firstQuery, uint32_t queryCount, NkBuffer destination, uint64_t destinationOffset);
NK_EXPORT void nkCommandEncoderWriteTimestamp(NkCommandEncoder commandEncoder, NkQuerySet querySet, uint32_t queryIndex);

// Methods of ComputePassEncoder
NK_EXPORT void nkComputePassEncoderBeginPipelineStatisticsQuery(NkComputePassEncoder computePassEncoder, NkQuerySet querySet, uint32_t queryIndex);
NK_EXPORT void nkComputePassEncoderDispatch(NkComputePassEncoder computePassEncoder, uint32_t x, uint32_t y, uint32_t z);
NK_EXPORT void nkComputePassEncoderDispatchIndirect(NkComputePassEncoder computePassEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset);
NK_EXPORT void nkComputePassEncoderEndPass(NkComputePassEncoder computePassEncoder);
NK_EXPORT void nkComputePassEncoderEndPipelineStatisticsQuery(NkComputePassEncoder computePassEncoder);
NK_EXPORT void nkComputePassEncoderInsertDebugMarker(NkComputePassEncoder computePassEncoder, const char* markerLabel);
NK_EXPORT void nkComputePassEncoderPopDebugGroup(NkComputePassEncoder computePassEncoder);
NK_EXPORT void nkComputePassEncoderPushDebugGroup(NkComputePassEncoder computePassEncoder, const char* groupLabel);
NK_EXPORT void nkComputePassEncoderSetBindGroup(NkComputePassEncoder computePassEncoder, uint32_t groupIndex, NkBindGroup group, uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets);
NK_EXPORT void nkComputePassEncoderSetPipeline(NkComputePassEncoder computePassEncoder, NkComputePipeline pipeline);
NK_EXPORT void nkComputePassEncoderWriteTimestamp(NkComputePassEncoder computePassEncoder, NkQuerySet querySet, uint32_t queryIndex);

// Methods of ComputePipeline
NK_EXPORT NkBindGroupLayout nkComputePipelineGetBindGroupLayout(NkComputePipeline computePipeline, uint32_t groupIndex);

// Methods of Device
NK_EXPORT void nkDestroyDevice(NkDevice device);
NK_EXPORT NkBindGroup nkCreateBindGroup(NkDevice device, const NkBindGroupInfo* descriptor);
NK_EXPORT NkBindGroupLayout nkCreateBindGroupLayout(NkDevice device, const NkBindGroupLayoutInfo* descriptor);
NK_EXPORT NkBuffer nkCreateBuffer(NkDevice device, const NkBufferInfo* descriptor);
NK_EXPORT NkCommandEncoder nkCreateCommandEncoder(NkDevice device);
NK_EXPORT NkComputePipeline nkCreateComputePipeline(NkDevice device, const NkComputePipelineInfo* descriptor);
NK_EXPORT NkPipelineLayout nkCreatePipelineLayout(NkDevice device, const NkPipelineLayoutInfo* descriptor);
NK_EXPORT NkQuerySet nkCreateQuerySet(NkDevice device, const NkQuerySetInfo* descriptor);
NK_EXPORT NkRenderBundleEncoder nkCreateRenderBundleEncoder(NkDevice device, const NkRenderBundleEncoderInfo* descriptor);
NK_EXPORT NkRenderPipeline nkCreateRenderPipeline(NkDevice device, const NkRenderPipelineInfo* descriptor);
NK_EXPORT NkSampler nkCreateSampler(NkDevice device, const NkSamplerInfo* descriptor);
NK_EXPORT NkSwapChain nkCreateSwapChain(NkDevice device, NkSurface surface, const NkSwapChainInfo* descriptor);
NK_EXPORT NkTexture nkCreateTexture(NkDevice device, const NkTextureInfo* descriptor);
NK_EXPORT NkQueue nkDeviceGetDefaultQueue(NkDevice device);
NK_EXPORT NkBool nkDevicePopErrorScope(NkDevice device, NkErrorCallback callback, void* userdata);
NK_EXPORT void nkDevicePushErrorScope(NkDevice device, NkErrorFilter filter);
NK_EXPORT void nkDeviceSetDeviceLostCallback(NkDevice device, NkDeviceLostCallback callback, void* userdata);
NK_EXPORT void nkDeviceSetUncapturedErrorCallback(NkDevice device, NkErrorCallback callback, void* userdata);

NK_EXPORT NkShaderModule nkCreateShaderModule(NkDevice device, const NkShaderModuleInfo* descriptor);
NK_EXPORT void nkDestroyShaderModule(NkShaderModule shaderModule);

// Methods of Fence
NK_EXPORT void nkDeviceFence(NkFence fence);
NK_EXPORT uint64_t nkFenceGetCompletedValue(NkFence fence);
NK_EXPORT void nkFenceOnCompletion(NkFence fence, uint64_t value, NkFenceOnCompletionCallback callback, void* userdata);

// Methods of Instance
NK_EXPORT void nkDestroyInstance(NkInstance instance);
NK_EXPORT NkSurface nkCreateSurface(NkInstance instance, const NkSurfaceInfo* descriptor);
NK_EXPORT NkDevice nkCreateDevice(NkInstance instance, NkSurface surface);

// Methods of QuerySet
NK_EXPORT void nkDestroyQuerySet(NkQuerySet querySet);

// Methods of Queue
NK_EXPORT void nkDestroyQueue(NkQueue queue);
NK_EXPORT NkFence nkCreateFence(NkQueue queue, const NkFenceInfo* descriptor);
NK_EXPORT void nkQueueSignal(NkQueue queue, NkFence fence, uint64_t signalValue);
NK_EXPORT void nkQueueSubmit(NkQueue queue, uint32_t commandCount, const NkCommandBuffer* commands);
NK_EXPORT void nkQueueWriteBuffer(NkQueue queue, NkBuffer buffer, uint64_t bufferOffset, const void* data, size_t size);
NK_EXPORT void nkQueueWriteTexture(NkQueue queue, const NkTextureCopyView* destination, const void* data, size_t dataSize, const NkTextureDataLayout* dataLayout, const NkExtent3D* writeSize);

// Methods of RenderBundleEncoder
NK_EXPORT void nkRenderBundleEncoderDraw(NkRenderBundleEncoder renderBundleEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
NK_EXPORT void nkRenderBundleEncoderDrawIndexed(NkRenderBundleEncoder renderBundleEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);
NK_EXPORT void nkRenderBundleEncoderDrawIndexedIndirect(NkRenderBundleEncoder renderBundleEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset);
NK_EXPORT void nkRenderBundleEncoderDrawIndirect(NkRenderBundleEncoder renderBundleEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset);
NK_EXPORT NkRenderBundle nkRenderBundleEncoderFinish(NkRenderBundleEncoder renderBundleEncoder);
NK_EXPORT void nkRenderBundleEncoderInsertDebugMarker(NkRenderBundleEncoder renderBundleEncoder, const char* markerLabel);
NK_EXPORT void nkRenderBundleEncoderPopDebugGroup(NkRenderBundleEncoder renderBundleEncoder);
NK_EXPORT void nkRenderBundleEncoderPushDebugGroup(NkRenderBundleEncoder renderBundleEncoder, const char* groupLabel);
NK_EXPORT void nkRenderBundleEncoderSetBindGroup(NkRenderBundleEncoder renderBundleEncoder, uint32_t groupIndex, NkBindGroup group, uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets);
NK_EXPORT void nkRenderBundleEncoderSetIndexBuffer(NkRenderBundleEncoder renderBundleEncoder, NkBuffer buffer, NkIndexFormat format, uint64_t offset, uint64_t size);
NK_EXPORT void nkRenderBundleEncoderSetPipeline(NkRenderBundleEncoder renderBundleEncoder, NkRenderPipeline pipeline);
NK_EXPORT void nkRenderBundleEncoderSetVertexBuffer(NkRenderBundleEncoder renderBundleEncoder, uint32_t slot, NkBuffer buffer, uint64_t offset, uint64_t size);

// Methods of RenderPassEncoder
NK_EXPORT void nkRenderPassEncoderBeginOcclusionQuery(NkRenderPassEncoder renderPassEncoder, uint32_t queryIndex);
NK_EXPORT void nkRenderPassEncoderBeginPipelineStatisticsQuery(NkRenderPassEncoder renderPassEncoder, NkQuerySet querySet, uint32_t queryIndex);
NK_EXPORT void nkRenderPassEncoderDraw(NkRenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
NK_EXPORT void nkRenderPassEncoderDrawIndexed(NkRenderPassEncoder renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);
NK_EXPORT void nkRenderPassEncoderDrawIndexedIndirect(NkRenderPassEncoder renderPassEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset);
NK_EXPORT void nkRenderPassEncoderDrawIndirect(NkRenderPassEncoder renderPassEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset);
NK_EXPORT void nkRenderPassEncoderEndOcclusionQuery(NkRenderPassEncoder renderPassEncoder);
NK_EXPORT void nkRenderPassEncoderEndPass(NkRenderPassEncoder renderPassEncoder);
NK_EXPORT void nkRenderPassEncoderEndPipelineStatisticsQuery(NkRenderPassEncoder renderPassEncoder);
NK_EXPORT void nkRenderPassEncoderExecuteBundles(NkRenderPassEncoder renderPassEncoder, uint32_t bundlesCount, const NkRenderBundle* bundles);
NK_EXPORT void nkRenderPassEncoderInsertDebugMarker(NkRenderPassEncoder renderPassEncoder, const char* markerLabel);
NK_EXPORT void nkRenderPassEncoderPopDebugGroup(NkRenderPassEncoder renderPassEncoder);
NK_EXPORT void nkRenderPassEncoderPushDebugGroup(NkRenderPassEncoder renderPassEncoder, const char* groupLabel);
NK_EXPORT void nkRenderPassEncoderSetBindGroup(NkRenderPassEncoder renderPassEncoder, uint32_t groupIndex, NkBindGroup group, uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets);
NK_EXPORT void nkRenderPassEncoderSetBlendColor(NkRenderPassEncoder renderPassEncoder, const NkColor* color);
NK_EXPORT void nkRenderPassEncoderSetIndexBuffer(NkRenderPassEncoder renderPassEncoder, NkBuffer buffer, uint64_t offset, uint64_t size);
NK_EXPORT void nkRenderPassEncoderSetPipeline(NkRenderPassEncoder renderPassEncoder, NkRenderPipeline pipeline);
NK_EXPORT void nkRenderPassEncoderSetScissorRect(NkRenderPassEncoder renderPassEncoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
NK_EXPORT void nkRenderPassEncoderSetStencilReference(NkRenderPassEncoder renderPassEncoder, uint32_t reference);
NK_EXPORT void nkRenderPassEncoderSetVertexBuffer(NkRenderPassEncoder renderPassEncoder, uint32_t slot, NkBuffer buffer, uint64_t offset, uint64_t size);
NK_EXPORT void nkRenderPassEncoderSetViewport(NkRenderPassEncoder renderPassEncoder, float x, float y, float width, float height, float minDepth, float maxDepth);
NK_EXPORT void nkRenderPassEncoderWriteTimestamp(NkRenderPassEncoder renderPassEncoder, NkQuerySet querySet, uint32_t queryIndex);

// Methods of RenderPipeline
NK_EXPORT void nkDestroyRenderPipeline(NkRenderPipeline renderPipeline);
NK_EXPORT NkBindGroupLayout nkRenderPipelineGetBindGroupLayout(NkRenderPipeline renderPipeline, uint32_t groupIndex);

// Methods of Surface
NK_EXPORT void nkDestroySurface(NkSurface surface);

// Methods of SwapChain
NK_EXPORT void nkDestroySwapChain(NkSwapChain swapChain);
NK_EXPORT NkTextureView nkSwapChainGetCurrentTextureView(NkSwapChain swapChain);
NK_EXPORT void nkSwapChainPresent(NkSwapChain swapChain);

// Methods of Texture
NK_EXPORT void nkDestroyTexture(NkTexture texture);
NK_EXPORT NkTextureView nkCreateTextureView(NkTexture texture, const NkTextureViewInfo* descriptor);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef NK_IMPLEMENTATION

/*
    Encoder API design considerations.

    BGFX and Dawn take the approach of recording commands on the CPU to, and then
    deferring recording to the backend command API until you actually submit the work.
    The WebGPU specification seems to suggest that this approach is intended for all
    implementations of WebGPU. This hides a bunch of complexity from the user, but it
    requires a bit of work from the implementation.

    https://github.com/gpuweb/gpuweb/blob/main/design/CommandSubmission.md
    
    I think this is a good idea and means the Encoder API is decoupled from the backends,
    meaning we could potentially have a shared Encoder API implementation. Freeing the
    encoder implementation from the backend also means we are in control of the implementation,
    and can focus on making it nice and simple.

    I think to avoid command allocation getting slow, we should probably use a
    linear allocator.
 */

#if defined(__cplusplus)
#define NK_CAST(type, x) static_cast<type>(x)
#define NK_PTR_CAST(type, x) reinterpret_cast<type>(x)
#define NK_NULL nullptr
#else
#define NK_CAST(type, x) ((type)x)
#define NK_PTR_CAST(type, x) ((type)x)
#define NK_NULL 0
#endif

#ifndef NK_ASSERT
#include <assert.h>
#define NK_ASSERT(c) assert(c)
#endif

#ifndef NK_MALLOC
#include <stdlib.h>
#define NK_MALLOC(size) malloc(size)
#define NK_CALLOC(num, size) calloc(num, size)
#define NK_FREE(pointer) free(pointer)
#endif

#ifndef NK_DEBUG
#ifndef NDEBUG
#define NK_DEBUG (1)
#endif
#endif

#ifndef NK_LOG
#ifdef NK_DEBUG
#include <stdio.h>
#define NK_LOG(...) { printf(__VA_ARGS__, __FILE__, __LINE__); }
#else
#define NK_LOG(...)
#endif
#endif

#define NK_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define NK_MIN(x, y) (((x) < (y)) ? (x) : (y))

// Initial allocator is super simple linear allocator.
// In the future it should be backed by a pool of memory blocks to let the allocator expand.

typedef struct NkCommandAllocator {
    uintptr_t buffer;
    uint32_t bufferSize;
    uint32_t allocatedSize;
    uint32_t lastAllocationSize;
} NkCommandAllocator;

NkCommandAllocator nkCreateCommandAllocator(uint32_t size) {

    NkCommandAllocator allocator;
    {
        allocator.buffer = NK_PTR_CAST(uintptr_t, NK_MALLOC(size));
        NK_ASSERT(allocator.buffer);
        allocator.bufferSize = size;
        allocator.allocatedSize = 0;
        allocator.lastAllocationSize = 0;
    }
    return allocator;
}

void nkCommandAllocatorReset(NkCommandAllocator* const allocator) {

    NK_ASSERT(allocator);
    allocator->buffer = 0;
    allocator->bufferSize = 0;
    allocator->allocatedSize = 0;
}

#define NK_IS_POWER_OF_TWO(value) (value != 0 && (value & (value - 1)) == 0)

#define NK_ALIGN_TO(type, value, alignment) (NK_CAST(type, ((NK_CAST(size_t, value) + (alignment - 1)) & ~(alignment - 1))))

#define NK_PTR_ALIGN_TO(type, value, alignment) NK_PTR_CAST(type*, (NK_ALIGN_TO(NK_PTR_CAST(uintptr_t, value), alignment)))

#define NK_IS_PTR_ALIGNED(ptr, alignment) !(NK_PTR_CAST(uintptr_t, ptr) % alignment)

NkBool nkCanSatisfyAllocation(uintptr_t buffer, uint32_t bufferSize, uint32_t allocatedSize, uint32_t size, uint32_t alignment) {

    uintptr_t bufferHead = buffer + allocatedSize;
    uintptr_t allocStart = NK_ALIGN_TO(uintptr_t, bufferHead, alignment);

    NK_ASSERT(allocStart >= bufferHead);
    if (allocStart < bufferHead)
    {
        // Alignment made us overflow
        return false;
    }

    uintptr_t allocEnd = allocStart + size;

    NK_ASSERT(allocEnd > allocStart);
    if (allocEnd <= allocStart)
    {
        // Requested size made us overflow
        return false;
    }

    uintptr_t allocSize = allocEnd - bufferHead;
    uint32_t newAllocatedSize = allocatedSize + allocSize;

    NK_ASSERT(newAllocatedSize <= bufferSize);
    if (newAllocatedSize <= bufferSize)
    {
        // Still has free space, we fit
        return true;
    }

    // Not enough space
    return false;
}

void* nkAllocateFromBuffer(uintptr_t buffer, uint32_t bufferSize, uint32_t* allocatedSize, size_t size, size_t alignment, uint32_t* const outAllocationOffset) {

    NK_ASSERT(allocatedSize);

    uintptr_t bufferHead = buffer + *allocatedSize;
    uintptr_t allocStart = NK_ALIGN_TO(uintptr_t, bufferHead, alignment);
    NK_ASSERT(allocStart >= bufferHead);

    uintptr_t allocEnd = allocStart + size;
    NK_ASSERT(allocEnd > allocStart);

    uintptr_t allocSize = allocEnd - bufferHead;
    uint32_t newAllocatedSize = *allocatedSize + allocSize;
    NK_ASSERT(newAllocatedSize <= bufferSize);

    *allocatedSize = newAllocatedSize;

    if (outAllocationOffset) {
        *outAllocationOffset = NK_CAST(uint32_t, (allocStart - buffer));
    }

    return NK_PTR_CAST(void*, allocStart);
}

void* nkCommandAllocatorAllocate(NkCommandAllocator* const allocator, uint32_t size, uint32_t alignment) {

    NK_ASSERT(allocator);
    NK_ASSERT(allocator->buffer);
    NK_ASSERT(size > 0);
    NK_ASSERT(NK_IS_POWER_OF_TWO(alignment));
    NK_ASSERT(nkCanSatisfyAllocation(allocator->buffer, allocator->bufferSize, allocator->allocatedSize, size, alignment));
    return nkAllocateFromBuffer(allocator->buffer, allocator->bufferSize, &allocator->allocatedSize, size, alignment, NK_NULL);
}

struct NkCommandEncoderImpl {
    NkCommandAllocator allocator;
};

struct NkRenderPassEncoderImpl {
    NkCommandAllocator* allocator;
};

struct NkComputePassEncoderImpl {
    NkCommandAllocator* allocator;
};

#define NK_COMMAND_ALLOCATOR_SIZE 16384 // this is a hack that will be removed when the command allocator is smarter

NkCommandEncoder nkCreateCommandEncoder(NkDevice device) {

    NkCommandEncoder commandEncoder = NK_PTR_CAST(NkCommandEncoder, NK_MALLOC(sizeof(struct NkCommandEncoderImpl)));
    NK_ASSERT(commandEncoder);
    commandEncoder->allocator = nkCreateCommandAllocator(NK_COMMAND_ALLOCATOR_SIZE);
}

typedef enum NkCommandType {
    NkCommandType_RenderPass
} NkCommandType;

typedef struct NkRenderPassEncoderBeginCommand {
    NkCommandType type;
} NkRenderPassEncoderBeginCommand;

// Methods of CommandEncoder
NkComputePassEncoder nkCommandEncoderBeginComputePass(NkCommandEncoder commandEncoder) {

}


NkRenderPassEncoder nkCommandEncoderBeginRenderPass(NkCommandEncoder commandEncoder, const NkRenderPassInfo* descriptor) {

    NK_ASSERT(commandEncoder);
    NK_ASSERT(descriptor);

    NkRenderPassEncoderBeginCommand* command = NK_PTR_CAST(NkRenderPassEncoderBeginCommand*, nkCommandAllocatorAllocate(&commandEncoder->allocator, sizeof(NkRenderPassEncoderBeginCommand), NK_ALIGN_OF(NkRenderPassEncoderBeginCommand)));
    NK_ASSERT(command);

    command->type = NkCommandType_RenderPass;

    NkRenderPassEncoder passEncoder = NK_PTR_CAST(NkRenderPassEncoder, NK_MALLOC(sizeof(struct NkRenderPassEncoderImpl)));
    NK_ASSERT(passEncoder);

    passEncoder->allocator = &commandEncoder->allocator;

    return passEncoder;
}

void nkCommandEncoderCopyBufferToBuffer(NkCommandEncoder commandEncoder, NkBuffer source, uint64_t sourceOffset, NkBuffer destination, uint64_t destinationOffset, uint64_t size) {

}

void nkCommandEncoderCopyBufferToTexture(NkCommandEncoder commandEncoder, const NkBufferCopyView* source, const NkTextureCopyView* destination, const NkExtent3D* copySize) {

}

void nkCommandEncoderCopyTextureToBuffer(NkCommandEncoder commandEncoder, const NkTextureCopyView* source, const NkBufferCopyView* destination, const NkExtent3D* copySize) {

}

void nkCommandEncoderCopyTextureToTexture(NkCommandEncoder commandEncoder, const NkTextureCopyView* source, const NkTextureCopyView* destination, const NkExtent3D* copySize) {

}

NkCommandBuffer nkCommandEncoderFinish(NkCommandEncoder commandEncoder) {

    NK_ASSERT(commandEncoder);
}

void nkCommandEncoderInsertDebugMarker(NkCommandEncoder commandEncoder, const char* markerLabel) {

}

void nkCommandEncoderPopDebugGroup(NkCommandEncoder commandEncoder) {

}

void nkCommandEncoderPushDebugGroup(NkCommandEncoder commandEncoder, const char* groupLabel) {

}

void nkCommandEncoderResolveQuerySet(NkCommandEncoder commandEncoder, NkQuerySet querySet, uint32_t firstQuery, uint32_t queryCount, NkBuffer destination, uint64_t destinationOffset) {

}

void nkCommandEncoderWriteTimestamp(NkCommandEncoder commandEncoder, NkQuerySet querySet, uint32_t queryIndex) {

}

// Methods of ComputePassEncoder
void nkComputePassEncoderBeginPipelineStatisticsQuery(NkComputePassEncoder computePassEncoder, NkQuerySet querySet, uint32_t queryIndex) {

}

void nkComputePassEncoderDispatch(NkComputePassEncoder computePassEncoder, uint32_t x, uint32_t y, uint32_t z) {

}

void nkComputePassEncoderDispatchIndirect(NkComputePassEncoder computePassEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset) {

}

void nkComputePassEncoderEndPass(NkComputePassEncoder computePassEncoder) {

}

void nkComputePassEncoderEndPipelineStatisticsQuery(NkComputePassEncoder computePassEncoder) {

}

void nkComputePassEncoderInsertDebugMarker(NkComputePassEncoder computePassEncoder, const char* markerLabel) {

}

void nkComputePassEncoderPopDebugGroup(NkComputePassEncoder computePassEncoder) {

}

void nkComputePassEncoderPushDebugGroup(NkComputePassEncoder computePassEncoder, const char* groupLabel) {

}

void nkComputePassEncoderSetBindGroup(NkComputePassEncoder computePassEncoder, uint32_t groupIndex, NkBindGroup group, uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets) {

}

void nkComputePassEncoderSetPipeline(NkComputePassEncoder computePassEncoder, NkComputePipeline pipeline) {

}

void nkComputePassEncoderWriteTimestamp(NkComputePassEncoder computePassEncoder, NkQuerySet querySet, uint32_t queryIndex) {

}

// Methods of RenderPassEncoder
void nkRenderPassEncoderBeginOcclusionQuery(NkRenderPassEncoder renderPassEncoder, uint32_t queryIndex) {

}

void nkRenderPassEncoderBeginPipelineStatisticsQuery(NkRenderPassEncoder renderPassEncoder, NkQuerySet querySet, uint32_t queryIndex) {

}

void nkRenderPassEncoderDraw(NkRenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {

}

void nkRenderPassEncoderDrawIndexed(NkRenderPassEncoder renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) {

}

void nkRenderPassEncoderDrawIndexedIndirect(NkRenderPassEncoder renderPassEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset) {

}

void nkRenderPassEncoderDrawIndirect(NkRenderPassEncoder renderPassEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset) {

}

void nkRenderPassEncoderEndOcclusionQuery(NkRenderPassEncoder renderPassEncoder) {

}

void nkRenderPassEncoderEndPass(NkRenderPassEncoder renderPassEncoder) {

}

void nkRenderPassEncoderEndPipelineStatisticsQuery(NkRenderPassEncoder renderPassEncoder) {

}

void nkRenderPassEncoderExecuteBundles(NkRenderPassEncoder renderPassEncoder, uint32_t bundlesCount, const NkRenderBundle* bundles) {

}

void nkRenderPassEncoderInsertDebugMarker(NkRenderPassEncoder renderPassEncoder, const char* markerLabel) {

}

void nkRenderPassEncoderPopDebugGroup(NkRenderPassEncoder renderPassEncoder) {

}

void nkRenderPassEncoderPushDebugGroup(NkRenderPassEncoder renderPassEncoder, const char* groupLabel) {

}

void nkRenderPassEncoderSetBindGroup(NkRenderPassEncoder renderPassEncoder, uint32_t groupIndex, NkBindGroup group, uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets) {

}

void nkRenderPassEncoderSetBlendColor(NkRenderPassEncoder renderPassEncoder, const NkColor* color) {

}

void nkRenderPassEncoderSetIndexBuffer(NkRenderPassEncoder renderPassEncoder, NkBuffer buffer, uint64_t offset, uint64_t size) {

}

void nkRenderPassEncoderSetPipeline(NkRenderPassEncoder renderPassEncoder, NkRenderPipeline pipeline) {

}

void nkRenderPassEncoderSetScissorRect(NkRenderPassEncoder renderPassEncoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {

}

void nkRenderPassEncoderSetStencilReference(NkRenderPassEncoder renderPassEncoder, uint32_t reference) {

}

void nkRenderPassEncoderSetVertexBuffer(NkRenderPassEncoder renderPassEncoder, uint32_t slot, NkBuffer buffer, uint64_t offset, uint64_t size) {

}

void nkRenderPassEncoderSetViewport(NkRenderPassEncoder renderPassEncoder, float x, float y, float width, float height, float minDepth, float maxDepth) {

}

void nkRenderPassEncoderWriteTimestamp(NkRenderPassEncoder renderPassEncoder, NkQuerySet querySet, uint32_t queryIndex) {

}

#ifdef NK_VULKAN_IMPLEMENTATION

#include <vulkan/vulkan.h>
#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

const char* NkVkErrorString(VkResult errorCode)
{
    switch (errorCode)
    {
#define STR(r) case VK_ ##r: return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
#undef STR
    default:
        return "UNKNOWN_ERROR";
    }
}

#define NK_CHECK_VK(x)                                                                              \
    do {                                                                                            \
        VkResult err = x;                                                                           \
        if (err) {                                                                                  \
            NK_LOG("ERROR: Detected Vulkan error %string at %string:%d.\n", NkVkErrorString(err));  \
            abort();                                                                                \
        }                                                                                           \
    } while (0)

#define NK_ASSERT_VK_HANDLE(handle)                             \
    do {                                                        \
        if ((handle) == VK_NULL_HANDLE) {                       \
            NK_LOG("ERROR: Handle is NULL at %string:%d.\n");   \
            abort();                                            \
        }                                                       \
    } while (0)

static const char* NkDeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static uint32_t NkDeviceExtensionCount
    = sizeof(NkDeviceExtensions) / sizeof(NkDeviceExtensions[0]);

static const char* NkInstanceExtensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(_WIN32)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

static uint32_t NkInstanceExtensionCount 
    = sizeof(NkInstanceExtensions) / sizeof(NkInstanceExtensions[0]);

static const char* NkValidationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

static uint32_t NkValidationLayerCount
    = sizeof(NkValidationLayers) / sizeof(NkValidationLayers[0]);

#ifdef NDEBUG
const NkBool NkEnableValidationLayers = NkFalse;
#else
const NkBool NkEnableValidationLayers = NkTrue;
#endif

struct NkAdapterImpl {
    int32_t foo;
};

struct NkBindGroupImpl {
    int32_t foo;
};

struct NkBindGroupLayoutImpl {
    int32_t foo;
};

struct NkBufferImpl {
    int32_t foo;
};

struct NkCommandBufferImpl {
    int32_t foo;
};

struct NkComputePipelineImpl {
    int32_t foo;
};

typedef struct NkVkQueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
} NkVkQueueFamilyIndices;

struct NkQueueImpl {
    VkQueue queue;
};

struct NkDeviceImpl {
    NkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    struct NkQueueImpl queue;
};

struct NkFenceImpl {
    int32_t foo;
};

struct NkInstanceImpl {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
};

struct NkPipelineLayoutImpl {
    int32_t foo;
};

struct NkQuerySetImpl {
    int32_t foo;
};

struct NkRenderBundleImpl {
    int32_t foo;
};

struct NkRenderBundleEncoderImpl {
    int32_t foo;
};

struct NkRenderPipelineImpl {
    int32_t foo;
};

struct NkSamplerImpl {
    int32_t foo;
};

struct NkShaderModuleImpl {
    VkShaderModule module;
};

struct NkSurfaceImpl {
    VkInstance instance;
    VkSurfaceKHR surface;
};

struct NkSwapChainImpl {
    VkDevice device;
    VkSwapchainKHR swapChain;
    VkImage* swapChainImages;
    uint32_t swapChainImageCount;
    struct NkTextureViewImpl* swapChainTextureViews;
    uint32_t currentFrame;
};

struct NkTextureImpl {
    int32_t foo;
};

struct NkTextureViewImpl {
    VkImageView imageView;
};

static NkBool nkVkCheckValidationLayerSupport() {

    uint32_t layerCount;
    NK_CHECK_VK(vkEnumerateInstanceLayerProperties(&layerCount, NK_NULL));

    if (layerCount != 0)
    {
        VkLayerProperties* availableLayers = NK_PTR_CAST(VkLayerProperties*, NK_MALLOC(sizeof(VkLayerProperties) * layerCount));
        NK_ASSERT(availableLayers);

        NK_CHECK_VK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers));

        for (uint32_t i = 0; i < NkValidationLayerCount; i++)
        {
            const char* layerName = NkValidationLayers[i];
            NkBool layerFound = NkFalse;

            for (uint32_t j = 0; j < layerCount; j++) {
                if (strcmp(layerName, availableLayers[j].layerName) == 0) {
                    layerFound = NkTrue;
                    break;
                }
            }

            if (!layerFound) {
                return NkFalse;
            }
        }

        return NkTrue;
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL nkVkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        NK_LOG(pCallbackData->pMessage);
    }
    return VK_FALSE;
}

static VkResult nkVkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

    NK_ASSERT_VK_HANDLE(instance);
    NK_ASSERT(pCreateInfo);
    NK_ASSERT(pDebugMessenger);

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NK_NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void nkVkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {

    NK_ASSERT_VK_HANDLE(instance);
    NK_ASSERT_VK_HANDLE(debugMessenger);

    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NK_NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

NkInstance nkCreateInstance() {

    if (NkEnableValidationLayers) {
        NK_ASSERT(nkVkCheckValidationLayerSupport());
    }

    NkInstance instance = NK_PTR_CAST(NkInstance, NK_MALLOC(sizeof(struct NkInstanceImpl)));
    NK_ASSERT(instance);

    VkApplicationInfo appInfo;
    {
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = NK_NULL;
        appInfo.pApplicationName = "Neko";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Neko";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
    }

    VkInstanceCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = NK_NULL;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = sizeof(NkInstanceExtensions) / sizeof(NkInstanceExtensions[0]);
        createInfo.ppEnabledExtensionNames = NkInstanceExtensions;
        if (NkEnableValidationLayers) {
            createInfo.enabledLayerCount = NkValidationLayerCount;
            createInfo.ppEnabledLayerNames = NkValidationLayers;
        }
        else {
            createInfo.enabledLayerCount = 0;
        }
    }

    NK_CHECK_VK(vkCreateInstance(&createInfo, NK_NULL, &instance->instance));

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
    {
        debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessengerCreateInfo.flags = 0;
        debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugMessengerCreateInfo.pfnUserCallback = nkVkDebugCallback;
        debugMessengerCreateInfo.pUserData = NK_NULL;
    }

    NK_CHECK_VK(nkVkCreateDebugUtilsMessengerEXT(instance->instance, &debugMessengerCreateInfo, NK_NULL, &instance->debugMessenger));

    return instance;
}

// Methods of Buffer
void nkDestroyBuffer(NkBuffer buffer) {

}

const void* nkBufferGetConstMappedRange(NkBuffer buffer, size_t offset, size_t size) {

}

void* nkBufferGetMappedRange(NkBuffer buffer, size_t offset, size_t size) {

}

NkBufferMapAsyncStatus nkBufferMap(NkBuffer buffer, NkMapModeFlags mode, size_t offset, size_t size) {

}

void nkBufferUnmap(NkBuffer buffer) {

}

// Methods of ComputePipeline
NkBindGroupLayout nkComputePipelineGetBindGroupLayout(NkComputePipeline computePipeline, uint32_t groupIndex) {

}

// Methods of Device
void nkDestroyDevice(NkDevice device) {

    NK_ASSERT(device);
    vkDestroyDevice(device->device, NK_NULL);
    NK_FREE(device);
}

NkBindGroup nkCreateBindGroup(NkDevice device, const NkBindGroupInfo* descriptor) {

}

NkBindGroupLayout nkCreateBindGroupLayout(NkDevice device, const NkBindGroupLayoutInfo* descriptor) {

}

NkBuffer nkCreateBuffer(NkDevice device, const NkBufferInfo* descriptor) {

}

NkComputePipeline nkCreateComputePipeline(NkDevice device, const NkComputePipelineInfo* descriptor) {

}

NkPipelineLayout nkCreatePipelineLayout(NkDevice device, const NkPipelineLayoutInfo* descriptor) {

}

NkQuerySet nkCreateQuerySet(NkDevice device, const NkQuerySetInfo* descriptor) {

}

NkRenderBundleEncoder nkCreateRenderBundleEncoder(NkDevice device, const NkRenderBundleEncoderInfo* descriptor) {

}

NkRenderPipeline nkCreateRenderPipeline(NkDevice device, const NkRenderPipelineInfo* descriptor) {

}

NkSampler nkCreateSampler(NkDevice device, const NkSamplerInfo* descriptor) {

}

NkShaderModule nkCreateShaderModule(NkDevice device, const NkShaderModuleInfo* descriptor) {

    NK_ASSERT(device);
    NK_ASSERT(descriptor);

    NkShaderModule shaderModule = NK_PTR_CAST(NkShaderModule, NK_MALLOC(sizeof(struct NkShaderModuleInfo)));
    NK_ASSERT(shaderModule);

    // SPIR-V code is passed to Vulkan as an array of uint32_t. Neko's interface is generalised so it takes IR
    // as a void pointer. Unfortunately that means that someone could feasibly feed it a byte buffer that is not
    // aligned correctly. This is unlikely to happen as I think most general allocators will make sure that the
    // data satisfies the worst-case alignment requirements, but just in case, we're just making sure the buffer
    // is suitably aligned before we cast the pointer to a uint32_t.
    NK_ASSERT(NK_IS_PTR_ALIGNED(descriptor->source, NK_ALIGN_OF(uint32_t)));

    VkShaderModuleCreateInfo shaderInfo;
    {
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.pNext = NK_NULL;
        shaderInfo.flags = 0;
        shaderInfo.codeSize = descriptor->size;
        shaderInfo.pCode = NK_PTR_CAST(const uint32_t*, descriptor->source);
    }

    NK_CHECK_VK(vkCreateShaderModule(device->device, &shaderInfo, NK_NULL, &shaderModule->module));

    return shaderModule;
}

void nkDestroyShaderModule(NkShaderModule shaderModule) {
}

typedef struct NkVkSurfaceSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t formatCount;
    VkPresentModeKHR* presentModes;
    uint32_t presentModeCount;
} NkVkSurfaceSupportDetails;

static NkVkSurfaceSupportDetails nkVkCreateSurfaceSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

    NK_ASSERT_VK_HANDLE(physicalDevice);
    NK_ASSERT_VK_HANDLE(surface);

    NkVkSurfaceSupportDetails details;
    {
        details.formats = NK_NULL;
        details.formatCount = 0;
        details.presentModes = NK_NULL;
        details.presentModeCount = 0;
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &details.formatCount, NK_NULL);

    if (details.formatCount != 0) {
        details.formats = NK_PTR_CAST(VkSurfaceFormatKHR*, NK_MALLOC(details.formatCount * sizeof(VkSurfaceFormatKHR)));
        NK_ASSERT(details.formats);

        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &details.formatCount, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &details.presentModeCount, NK_NULL);

    if (details.presentModeCount != 0) {
        details.presentModes = NK_PTR_CAST(VkPresentModeKHR*, NK_MALLOC(details.presentModeCount * sizeof(VkPresentModeKHR)));
        NK_ASSERT(details.presentModes);

        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &details.presentModeCount, details.presentModes);
    }

    return details;
}

static NkVkSurfaceSupportDetails nkVkDestroySurfaceSupportDetails(NkVkSurfaceSupportDetails* details) {

    NK_ASSERT(details);
    NK_FREE(details->formats);
    NK_FREE(details->presentModes);
}

static VkPresentModeKHR nkVkChooseSwapPresentMode(VkPresentModeKHR const* availablePresentModes, uint32_t availablePresentModeCount) {

    NK_ASSERT(availablePresentModes);
    NK_ASSERT(availablePresentModeCount != 0);

    for (size_t i = 0; i < availablePresentModeCount - 1; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkSurfaceFormatKHR nkVkChooseSwapSurfaceFormat(VkSurfaceFormatKHR const* availableFormats, uint32_t availableFormatCount) {

    NK_ASSERT(availableFormats);
    NK_ASSERT(availableFormatCount != 0);

    for (size_t i = 0; i < availableFormatCount - 1; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }

    return availableFormats[0];
}

static VkExtent2D nkVkChooseSwapExtent(VkSurfaceCapabilitiesKHR const* capabilities, const NkSwapChainInfo* info) {

    NK_ASSERT(capabilities);
    NK_ASSERT(info);

    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        VkExtent2D actualExtent = {
            info->width,
            info->height
        };

        actualExtent.width
            = NK_MAX(capabilities->minImageExtent.width, NK_MIN(capabilities->maxImageExtent.width, actualExtent.width));

        actualExtent.height
            = NK_MAX(capabilities->minImageExtent.height, NK_MIN(capabilities->maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

static NkVkQueueFamilyIndices nkVkFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {

    NK_ASSERT_VK_HANDLE(device);
    NK_ASSERT_VK_HANDLE(surface);

    NkVkQueueFamilyIndices indices;
    {
        indices.graphicsFamily = UINT32_MAX;
        indices.presentFamily = UINT32_MAX;
    }

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NK_NULL);

    VkQueueFamilyProperties* queueFamilies = NK_PTR_CAST(VkQueueFamilyProperties*, NK_MALLOC(sizeof(VkQueueFamilyProperties) * queueFamilyCount));
    NK_ASSERT(queueFamilies);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (uint32_t i = 0; i < queueFamilyCount; i++) {

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (surface != VK_NULL_HANDLE) {

            VkBool32 presentSupport = VK_FALSE;
            NK_CHECK_VK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport));

            if (presentSupport) {
                indices.presentFamily = i;
            }
        }

        if (indices.graphicsFamily != UINT32_MAX &&
            indices.presentFamily != UINT32_MAX) {
            break;
        }
    }

    NK_FREE(queueFamilies);

    return indices;
}

NkSwapChain nkCreateSwapChain(NkDevice device, NkSurface surface, const NkSwapChainInfo* info) {

    NK_ASSERT(device);
    NK_ASSERT(surface);
    NK_ASSERT(info);

    NkSwapChain swapChain = NK_PTR_CAST(NkSwapChain, NK_MALLOC(sizeof(struct NkSwapChainImpl)));
    NK_ASSERT(swapChain);

    swapChain->currentFrame = 0;

    NkVkSurfaceSupportDetails surfaceSupport = nkVkCreateSurfaceSupportDetails(device->physicalDevice, surface->surface);
    VkSurfaceFormatKHR surfaceFormat = nkVkChooseSwapSurfaceFormat(surfaceSupport.formats, surfaceSupport.formatCount);
    VkPresentModeKHR presentMode = nkVkChooseSwapPresentMode(surfaceSupport.presentModes, surfaceSupport.presentModeCount);
    VkExtent2D extent = nkVkChooseSwapExtent(&surfaceSupport.capabilities, info);

    uint32_t imageCount = surfaceSupport.capabilities.minImageCount + 1;
    if (surfaceSupport.capabilities.maxImageCount > 0 && imageCount > surfaceSupport.capabilities.maxImageCount) {
        imageCount = surfaceSupport.capabilities.maxImageCount;
    }

    NkVkQueueFamilyIndices indices =
        nkVkFindQueueFamilies(device->physicalDevice, surface->surface);

    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    VkSwapchainCreateInfoKHR createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.flags = 0;
        createInfo.pNext = NK_NULL;
        createInfo.surface = surface->surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = NK_NULL;
        }

        createInfo.preTransform = surfaceSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;
    }
    NK_CHECK_VK(vkCreateSwapchainKHR(device->device, &createInfo, NK_NULL, &swapChain->swapChain));

    swapChain->device = device->device;

    NK_CHECK_VK(vkGetSwapchainImagesKHR(device->device, swapChain->swapChain, &swapChain->swapChainImageCount, NK_NULL));

    swapChain->swapChainImages = NK_PTR_CAST(VkImage*, NK_MALLOC(sizeof(VkImage) * swapChain->swapChainImageCount));
    NK_ASSERT(swapChain->swapChainImages);

    NK_CHECK_VK(vkGetSwapchainImagesKHR(device->device, swapChain->swapChain, &swapChain->swapChainImageCount, swapChain->swapChainImages));

    nkVkDestroySurfaceSupportDetails(&surfaceSupport);

    swapChain->swapChainTextureViews = NK_PTR_CAST(struct NkTextureViewImpl*, NK_MALLOC(sizeof(struct NkTextureViewImpl) * swapChain->swapChainImageCount));
    NK_ASSERT(swapChain->swapChainTextureViews);

    for (size_t i = 0; i < swapChain->swapChainImageCount; i++) {
        VkImageViewCreateInfo imageViewCreateInfo;
        {
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.flags = 0;
            imageViewCreateInfo.pNext = NK_NULL;
            imageViewCreateInfo.image = swapChain->swapChainImages[i];
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = surfaceFormat.format;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
        }
        NK_CHECK_VK(vkCreateImageView(device->device, &imageViewCreateInfo, NK_NULL, &swapChain->swapChainTextureViews[i].imageView));
    }

    return swapChain;
}

NkTexture nkCreateTexture(NkDevice device, const NkTextureInfo* descriptor) {

}

NkQueue nkDeviceGetDefaultQueue(NkDevice device) {
    return &device->queue;
}

NkBool nkDevicePopErrorScope(NkDevice device, NkErrorCallback callback, void* userdata) {

}

void nkDevicePushErrorScope(NkDevice device, NkErrorFilter filter) {

}

void nkDeviceSetDeviceLostCallback(NkDevice device, NkDeviceLostCallback callback, void* userdata) {

}

void nkDeviceSetUncapturedErrorCallback(NkDevice device, NkErrorCallback callback, void* userdata) {

}

// Methods of Fence
void nkDeviceFence(NkFence fence) {

}

uint64_t nkFenceGetCompletedValue(NkFence fence) {

}

void nkFenceOnCompletion(NkFence fence, uint64_t value, NkFenceOnCompletionCallback callback, void* userdata) {

}

// Methods of Instance
void nkDestroyInstance(NkInstance instance) {

    NK_ASSERT(instance);

    if (NkEnableValidationLayers) {
        nkVkDestroyDebugUtilsMessengerEXT(instance->instance, instance->debugMessenger, NK_NULL);
    }
    vkDestroyInstance(instance->instance, NK_NULL);
    NK_FREE(instance);
}

NkSurface nkCreateSurface(NkInstance instance, const NkSurfaceInfo* descriptor) {

    NK_ASSERT(instance);
    NK_ASSERT(descriptor);

    NkSurface surface = NK_PTR_CAST(NkSurface, NK_MALLOC(sizeof(struct NkSurfaceImpl)));
    NK_ASSERT(surface);

    surface->instance = instance->instance;

#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext = NK_NULL;
        createInfo.flags = 0;
        createInfo.hwnd = descriptor->native.hwnd;
        createInfo.hinstance = descriptor->native.hinstance;
    }
    NK_CHECK_VK(vkCreateWin32SurfaceKHR(instance->instance, &createInfo, NK_NULL, &surface->surface));
#endif

    return surface;
}

static const char* NkVkDeviceEnabledExtensionNames[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static const uint32_t NkVkDeviceEnabledExtensionCount
    = sizeof(NkVkDeviceEnabledExtensionNames) / sizeof(NkVkDeviceEnabledExtensionNames[0]);

static NkBool nkVkCheckDeviceExtensionProperties(VkExtensionProperties* properties, uint32_t propertyCount) {

    NK_ASSERT(properties);
    NK_ASSERT(propertyCount != 0);

    for (uint32_t i = 0; i < NkVkDeviceEnabledExtensionCount; i++) {
        uint32_t enabledExtensionCount = 0;
        for (uint32_t j = 0; j < propertyCount - 1; j++) {
            if (strcmp(NkVkDeviceEnabledExtensionNames[i], properties[j].extensionName) == 0) {
                ++enabledExtensionCount;
            }
            if (enabledExtensionCount == NkVkDeviceEnabledExtensionCount) {
                return NkTrue;
            }
        }
    }
    return NkFalse;
}

static NkBool nkVkIsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

    NK_ASSERT_VK_HANDLE(physicalDevice);
    NK_ASSERT_VK_HANDLE(surface);

    uint32_t propertyCount = 0;

    NK_CHECK_VK(vkEnumerateDeviceExtensionProperties(physicalDevice, NK_NULL, &propertyCount, NK_NULL));

    VkExtensionProperties* properties = NK_PTR_CAST(VkExtensionProperties*, NK_MALLOC(sizeof(VkExtensionProperties) * propertyCount));
    NK_ASSERT(properties);

    NK_CHECK_VK(vkEnumerateDeviceExtensionProperties(physicalDevice, NK_NULL, &propertyCount, properties));

    NkVkQueueFamilyIndices queueFamilyIndices = nkVkFindQueueFamilies(physicalDevice, surface);

    NkBool indicesIsComplete = queueFamilyIndices.graphicsFamily != UINT32_MAX &&
                               queueFamilyIndices.presentFamily != UINT32_MAX;

    NkBool extensionsSupported = nkVkCheckDeviceExtensionProperties(properties, propertyCount);

    NkBool surfaceAdequate = NkFalse;
    if (extensionsSupported) {
        NkVkSurfaceSupportDetails details = nkVkCreateSurfaceSupportDetails(physicalDevice, surface);
        surfaceAdequate = details.formatCount != 0 && details.presentModeCount != 0;
        nkVkDestroySurfaceSupportDetails(&details);
    }

    NK_FREE(properties);

    return indicesIsComplete && extensionsSupported && surfaceAdequate;
}

NkDevice nkCreateDevice(NkInstance instance, NkSurface surface) {

    NK_ASSERT(instance);
    NK_ASSERT(surface);

    NkDevice device = NK_PTR_CAST(NkDevice, NK_MALLOC(sizeof(struct NkDeviceImpl)));
    NK_ASSERT(device);

    device->instance = instance;
    
    // select physical device

    uint32_t physicalDeviceCount = 0;
    NK_CHECK_VK(vkEnumeratePhysicalDevices(instance->instance, &physicalDeviceCount, NK_NULL));

    VkPhysicalDevice* physicalDevices =
        NK_PTR_CAST(VkPhysicalDevice*, NK_MALLOC(sizeof(VkPhysicalDevice) * physicalDeviceCount));
    NK_ASSERT(physicalDevices);

    vkEnumeratePhysicalDevices(instance->instance, &physicalDeviceCount, physicalDevices);

    VkSurfaceKHR vkSurface = surface->surface;

    device->physicalDevice = VK_NULL_HANDLE;
    for (size_t i = 0; i < physicalDeviceCount; i++) {
        if (nkVkIsPhysicalDeviceSuitable(physicalDevices[i], vkSurface)) {
            device->physicalDevice = physicalDevices[i];   
            break;
        }
    }
    NK_ASSERT_VK_HANDLE(device->physicalDevice);

    NK_FREE(physicalDevices);

    // select logical device

    NkVkQueueFamilyIndices queueFamilyIndices = 
        nkVkFindQueueFamilies(device->physicalDevice, vkSurface);

    VkDeviceQueueCreateInfo queueCreateInfos[2];
    uint32_t queueCount = 0;
    float queuePriority = 1.0f;

    for (size_t i = 0; i < 2; i++) {
        VkDeviceQueueCreateInfo info;
        {
            info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            info.pNext = NK_NULL;
            info.flags = 0;
            info.queueFamilyIndex = *(&queueFamilyIndices.graphicsFamily + i);
            info.queueCount = 1;
            info.pQueuePriorities = &queuePriority;
        }
        queueCreateInfos[i] = info;

        queueCount = i + 1;

        if (queueFamilyIndices.graphicsFamily == queueFamilyIndices.presentFamily) {
            break;
        }
    }

    VkDeviceCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = NK_NULL;
        createInfo.flags = 0;
        createInfo.queueCreateInfoCount = queueCount;
        createInfo.pQueueCreateInfos = queueCreateInfos;
        createInfo.enabledLayerCount = 0;       // deprecated and ignored
        createInfo.ppEnabledLayerNames = NK_NULL;  // deprecated and ignored
        createInfo.enabledExtensionCount = NkVkDeviceEnabledExtensionCount;
        createInfo.ppEnabledExtensionNames = NkVkDeviceEnabledExtensionNames;
        createInfo.pEnabledFeatures = NK_NULL;
    }

    NK_CHECK_VK(vkCreateDevice(device->physicalDevice, &createInfo, NK_NULL, &device->device));

    vkGetDeviceQueue(device->device, queueFamilyIndices.graphicsFamily, 0, &device->queue.queue);

    return device;
}

// Methods of QuerySet
void nkDestroyQuerySet(NkQuerySet querySet) {

}

// Methods of Queue
void nkDestroyQueue(NkQueue queue) {

}

NkFence nkCreateFence(NkQueue queue, const NkFenceInfo* descriptor) {

}

void nkQueueSignal(NkQueue queue, NkFence fence, uint64_t signalValue) {

}

void nkQueueSubmit(NkQueue queue, uint32_t commandCount, const NkCommandBuffer* commands) {

}

void nkQueueWriteBuffer(NkQueue queue, NkBuffer buffer, uint64_t bufferOffset, const void* data, size_t size) {

}

void nkQueueWriteTexture(NkQueue queue, const NkTextureCopyView* destination, const void* data, size_t dataSize, const NkTextureDataLayout* dataLayout, const NkExtent3D* writeSize) {

}

// Methods of RenderBundleEncoder
void nkRenderBundleEncoderDraw(NkRenderBundleEncoder renderBundleEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {

}

void nkRenderBundleEncoderDrawIndexed(NkRenderBundleEncoder renderBundleEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) {

}

void nkRenderBundleEncoderDrawIndexedIndirect(NkRenderBundleEncoder renderBundleEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset) {

}

void nkRenderBundleEncoderDrawIndirect(NkRenderBundleEncoder renderBundleEncoder, NkBuffer indirectBuffer, uint64_t indirectOffset) {

}

NkRenderBundle nkRenderBundleEncoderFinish(NkRenderBundleEncoder renderBundleEncoder) {

}

void nkRenderBundleEncoderInsertDebugMarker(NkRenderBundleEncoder renderBundleEncoder, const char* markerLabel) {

}

void nkRenderBundleEncoderPopDebugGroup(NkRenderBundleEncoder renderBundleEncoder) {

}

void nkRenderBundleEncoderPushDebugGroup(NkRenderBundleEncoder renderBundleEncoder, const char* groupLabel) {

}

void nkRenderBundleEncoderSetBindGroup(NkRenderBundleEncoder renderBundleEncoder, uint32_t groupIndex, NkBindGroup group, uint32_t dynamicOffsetCount, const uint32_t* dynamicOffsets) {

}

void nkRenderBundleEncoderSetIndexBuffer(NkRenderBundleEncoder renderBundleEncoder, NkBuffer buffer, NkIndexFormat format, uint64_t offset, uint64_t size) {

}

void nkRenderBundleEncoderSetPipeline(NkRenderBundleEncoder renderBundleEncoder, NkRenderPipeline pipeline) {

}

void nkRenderBundleEncoderSetVertexBuffer(NkRenderBundleEncoder renderBundleEncoder, uint32_t slot, NkBuffer buffer, uint64_t offset, uint64_t size) {

}

// Methods of RenderPipeline
void nkDestroyRenderPipeline(NkRenderPipeline renderPipeline) {

}

NkBindGroupLayout nkRenderPipelineGetBindGroupLayout(NkRenderPipeline renderPipeline, uint32_t groupIndex) {

}

// Methods of Surface
void nkDestroySurface(NkSurface surface) {

    NK_ASSERT(surface);

    vkDestroySurfaceKHR(surface->instance, surface->surface, NK_NULL);
    NK_FREE(surface);
}

// Methods of SwapChain
void nkDestroySwapChain(NkSwapChain swapChain) {

    NK_ASSERT(swapChain);

    for (uint32_t i = 0; i < swapChain->swapChainImageCount; i++) {
        vkDestroyImageView(swapChain->device, swapChain->swapChainTextureViews[i].imageView, NK_NULL);
    }
    NK_FREE(swapChain->swapChainTextureViews);
    vkDestroySwapchainKHR(swapChain->device, swapChain->swapChain, NK_NULL);
    NK_FREE(swapChain->swapChainImages);
    NK_FREE(swapChain);
}

NkTextureView nkSwapChainGetCurrentTextureView(NkSwapChain swapChain) {

    NK_ASSERT(swapChain);
    NK_ASSERT(swapChain->swapChainTextureViews);
    return &swapChain->swapChainTextureViews[swapChain->currentFrame];
}

void nkSwapChainPresent(NkSwapChain swapChain) {

}

// Methods of Texture
NkTextureView nkCreateTextureView(NkTexture texture, const NkTextureViewInfo* descriptor) {

}

void nkDestroyTexture(NkTexture texture) {

}

#endif // NK_VULKAN_IMPLEMENTATION

#endif // NK_IMPLEMENTATION

#endif // NEKO_H


#include <Neko/Neko.h>
#include <Neko/Shaders/Triangle.h>
#include <NekoSample/NekoSample.h>

typedef struct PositionColorVertex {
    NkFloat3 position;
    NkFloat4 color;
} PositionColorVertex;

static const PositionColorVertex vertices[] = {
    { .position = { 0.0f, 0.5f, 0.5f }, .color = { 1.0f, 0.0f, 0.0f, 1.0f } },
    { .position = { 0.5f,-0.5f, 0.5f }, .color = { 0.0f, 1.0f, 0.0f, 1.0f } },
    { .position = {-0.5f,-0.5f, 0.5f }, .color = { 0.0f, 0.0f, 1.0f, 1.0f } }
};

int main() {

    uint32_t windowHeight = 720;
    uint32_t windowWidth = 1280;

    const NkSampleApp sample = nkCreateSampleApp(&(NkSampleAppInfo) {
        .width  = windowWidth,
        .height = windowHeight,
        .title  = "Neko: Triangle",
    });

    const NkInstance instance = nkCreateInstance();

    const NkNativeSurface nativeSurface = nkSampleAppGetNativeSurface(sample);

    const NkSurface surface = nkCreateSurface(instance, &(NkSurfaceInfo) {
        .native = nativeSurface
    });

    const NkDevice device = nkCreateDevice(instance, surface);

    const NkQueue queue = nkDeviceGetDefaultQueue(device);

    const NkShaderModule triangleShader = nkCreateShaderModule(device, &(NkShaderModuleInfo) {
        .source = TriangleShaderSource,
        .size = sizeof(TriangleShaderSource)
    });

    const NkRenderPipeline renderPipeline = nkCreateRenderPipeline(device, &(NkRenderPipelineInfo) {
        .vertexStage   = { .module = triangleShader, .entryPoint = "vertexMain" },
        .fragmentStage = { .module = triangleShader, .entryPoint = "pixelMain"  },
        .primitiveTopology = NkPrimitiveTopology_TriangleList,
        .vertexState = &(NkVertexStateInfo) {
            .indexFormat = NkIndexFormat_Uint16,
            .vertexBuffers = &(NkVertexBufferLayoutInfo) {
                .arrayStride = sizeof(PositionColorVertex),
                .stepMode = NkInputStepMode_Vertex,
                .attributes = (NkVertexAttributeInfo[]) {
                    { .format = NkVertexFormat_Float4, .offset = 0, .shaderLocation = 0 },
                    { .format = NkVertexFormat_Float3, .offset = sizeof(NkFloat4), .shaderLocation = 1 }
                },
                .attributeCount = 2
            },
            .vertexBufferCount = 1
        }
    });

    const NkBuffer vertexBuffer = nkCreateBuffer(device, &(NkBufferInfo) {
        .usage = NkBufferUsage_CopyDst | NkBufferUsage_Vertex,
        .size = sizeof(vertices)
    });

    nkQueueWriteBuffer(queue, vertexBuffer, 0, vertices, sizeof(vertices));

    const NkSwapChain swapChain = nkCreateSwapChain(device, surface, &(NkSwapChainInfo) {
        .width  = windowWidth,
        .height = windowHeight
    });

    while (nkSampleAppProcessEvents(sample, NULL)) {
        const NkTextureView frame = nkSwapChainGetCurrentTextureView(swapChain);

        const NkCommandEncoder encoder = nkCreateCommandEncoder(device);

        const NkRenderPassEncoder renderPass = nkCommandEncoderBeginRenderPass(encoder, &(NkRenderPassInfo) {
            .colorAttachments = &(NkRenderPassColorAttachmentInfo) {
                .attachment = frame,
                .loadOp     = NkLoadOp_Clear,
                .storeOp    = NkStoreOp_Store,
                .clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }
            },
            .colorAttachmentCount = 1
        });

        nkRenderPassEncoderSetPipeline(renderPass, renderPipeline);
        nkRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0, 0);
        nkRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);

        const NkCommandBuffer commandBuffer = nkCommandEncoderFinish(encoder);
        nkQueueSubmit(queue, 1, &commandBuffer);
    }

    nkDestroyBuffer(vertexBuffer);
    nkDestroySwapChain(swapChain);
    nkDestroyRenderPipeline(renderPipeline);
    nkDestroyShaderModule(triangleShader);
    nkDestroyDevice(device);
    nkDestroySurface(surface);
    nkDestroyInstance(instance);

    nkDestroySampleApp(sample);
}

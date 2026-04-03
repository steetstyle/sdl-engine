#define SDL_MAIN_HANDLED
#include "sdl3webgpu.h"
#include <SDL3/SDL.h>
#include <webgpu.h>
#include <stdio.h>
#include <stdlib.h>

static WGPUAdapter adapter = NULL;
static WGPUDevice device = NULL;
static WGPUSurface surface = NULL;
static WGPUSurfaceConfiguration config = {0};
static int frame_count = 0;

static void handle_request_adapter(WGPURequestAdapterStatus status,
                                  WGPUAdapter a, WGPUStringView message,
                                  void *userdata, void *userdata2) {
    (void)userdata2;
    if (status == WGPURequestAdapterStatus_Success) {
        adapter = a;
        printf("OK: Adapter received\n");
    } else {
        printf("FAIL: request_adapter status=%d message=%.*s\n", status,
               (int)message.length, message.data);
        *(int*)userdata = 1;
    }
}

static void handle_request_device(WGPURequestDeviceStatus status,
                                  WGPUDevice d, WGPUStringView message,
                                  void *userdata, void *userdata2) {
    (void)userdata2;
    if (status == WGPURequestDeviceStatus_Success) {
        device = d;
        printf("OK: Device received\n");
    } else {
        printf("FAIL: request_device status=%d message=%.*s\n", status,
               (int)message.length, message.data);
        *(int*)userdata = 1;
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("=== SDL3 WebGPU Blue Screen Test ===\n");

    WGPUInstance instance = wgpuCreateInstance(NULL);
    if (!instance) {
        printf("FAIL: wgpuCreateInstance failed\n");
        return 1;
    }
    printf("OK: WebGPU instance created\n");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("FAIL: SDL_Init failed: %s\n", SDL_GetError());
        wgpuInstanceRelease(instance);
        return 1;
    }
    printf("OK: SDL_Init succeeded\n");

    SDL_Window *window = SDL_CreateWindow("SDL3 WebGPU Blue Screen", 640, 480, 0);
    if (!window) {
        printf("FAIL: SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        wgpuInstanceRelease(instance);
        return 1;
    }
    printf("OK: Window created (640x480)\n");

    surface = SDL_GetWGPUSurface(instance, window);
    if (!surface) {
        printf("FAIL: SDL_GetWGPUSurface failed\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        wgpuInstanceRelease(instance);
        return 1;
    }
    printf("OK: WebGPU surface created\n");

    int error = 0;
    wgpuInstanceRequestAdapter(instance,
                               &(const WGPURequestAdapterOptions){
                                   .compatibleSurface = surface,
                               },
                               (const WGPURequestAdapterCallbackInfo){
                                   .callback = handle_request_adapter,
                                   .userdata1 = &error
                               });
    
    if (!adapter) {
        printf("FAIL: No adapter\n");
        wgpuSurfaceRelease(surface);
        SDL_DestroyWindow(window);
        SDL_Quit();
        wgpuInstanceRelease(instance);
        return 1;
    }

    wgpuAdapterRequestDevice(adapter, NULL,
                             (const WGPURequestDeviceCallbackInfo){
                                 .callback = handle_request_device,
                                 .userdata1 = &error
                             });

    if (!device) {
        printf("FAIL: No device\n");
        wgpuAdapterRelease(adapter);
        wgpuSurfaceRelease(surface);
        SDL_DestroyWindow(window);
        SDL_Quit();
        wgpuInstanceRelease(instance);
        return 1;
    }

    WGPUSurfaceCapabilities capabilities = {0};
    wgpuSurfaceGetCapabilities(surface, adapter, &capabilities);
    printf("OK: Surface capabilities: %d formats\n", capabilities.formatCount);

    WGPUQueue queue = wgpuDeviceGetQueue(device);
    printf("OK: Queue obtained\n");

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    config = (const WGPUSurfaceConfiguration){
        .device = device,
        .usage = WGPUTextureUsage_RenderAttachment,
        .format = capabilities.formats[0],
        .presentMode = WGPUPresentMode_Fifo,
        .alphaMode = capabilities.alphaModes[0],
        .width = (uint32_t)width,
        .height = (uint32_t)height,
    };

    wgpuSurfaceConfigure(surface, &config);
    printf("OK: Surface configured\n");

    printf("Rendering blue screen...\n");

    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                printf("Quit event received\n");
                running = 0;
                break;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q) {
                    printf("Keyboard quit\n");
                    running = 0;
                    break;
                }
            }
        }

        if (!running) break;

        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);

        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
            surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
            printf("FAIL: get_current_texture status=%d\n", surfaceTexture.status);
            break;
        }

        WGPUTextureView frame = wgpuTextureCreateView(surfaceTexture.texture, NULL);

        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, NULL);

        WGPURenderPassColorAttachment colorAttachment = {
            .view = frame,
            .loadOp = WGPULoadOp_Clear,
            .storeOp = WGPUStoreOp_Store,
            .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
            .clearValue = (WGPUColor){0.0, 0.0, 1.0, 1.0},
        };

        WGPURenderPassDescriptor renderPassDesc = {
            .colorAttachmentCount = 1,
            .colorAttachments = &colorAttachment,
        };

        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
        wgpuRenderPassEncoderEnd(renderPass);

        WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, NULL);

        wgpuQueueSubmit(queue, 1, &commandBuffer);

        wgpuSurfacePresent(surface);

        wgpuTextureViewRelease(frame);
        wgpuCommandBufferRelease(commandBuffer);
        wgpuCommandEncoderRelease(encoder);
        wgpuTextureRelease(surfaceTexture.texture);

        frame_count++;
        if (frame_count % 60 == 0) {
            printf("Frame %d\n", frame_count);
        }
    }

    printf("Cleaning up...\n");
    wgpuQueueRelease(queue);
    wgpuSurfaceCapabilitiesFreeMembers(capabilities);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuSurfaceRelease(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
    wgpuInstanceRelease(instance);

    printf("=== TEST COMPLETE ===\n");
    return 0;
}

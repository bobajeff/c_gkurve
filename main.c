#include "create_surface.h"
#include "framework.h"
#include "webgpu.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <cglm/cglm.h>

typedef struct Vertex {
  vec4 pos;
  vec2 uv;
} Vertex;

// Simple triangle
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define TRIANGLE_SCALE 250.0
#define SQRT_0_75 0.8660254037844386 // sqrt(.75)
#define TRIANGLE_HEIGHT (TRIANGLE_SCALE * SQRT_0_75)
Vertex vertices[] = {
    { .pos = { (float)WINDOW_WIDTH / 2 + TRIANGLE_SCALE / 2, (float)WINDOW_HEIGHT / 2 + TRIANGLE_HEIGHT, 0, 1 }, .uv = { 0.5, 1 } },
    { .pos = { (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 2, 0, 1 }, .uv = { 0, 0 } },
    { .pos = { (float)WINDOW_WIDTH / 2 + TRIANGLE_SCALE, (float)WINDOW_HEIGHT / 2 + 0, 0, 1 }, .uv = { 1, 0 } },

    { .pos = { (float)WINDOW_WIDTH / 2 + TRIANGLE_SCALE / 2, (float)WINDOW_HEIGHT / 2, 0, 1 }, .uv = { 0.5, 1 } },
    { .pos = { (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 2 - TRIANGLE_HEIGHT, 0, 1 }, .uv = { 0, 0 } },
    { .pos = { (float)WINDOW_WIDTH / 2 + TRIANGLE_SCALE, (float)WINDOW_HEIGHT / 2 - TRIANGLE_HEIGHT, 0, 1 }, .uv = { 1, 0 } },

    { .pos = { (float)WINDOW_WIDTH / 2 - TRIANGLE_SCALE / 2, (float)WINDOW_HEIGHT / 2 + TRIANGLE_HEIGHT, 0, 1 }, .uv = { 0.5, 1 } },
    { .pos = { (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 2, 0, 1 }, .uv = { 0, 0 } },
    { .pos = { (float)WINDOW_WIDTH / 2 - TRIANGLE_SCALE, (float)WINDOW_HEIGHT / 2 + 0, 0, 1 }, .uv = { 1, 0 } },
};

size_t vertices_length = sizeof(vertices)/sizeof(Vertex);

// The uniform read by the vertex shader, it contains the matrix
// that will move vertices
typedef struct VertexUniform {
    mat4 mat;
} VertexUniform;

typedef struct FragUniform {
    // TODO use an enum? Remember that it will be casted to u32 in wgsl
    // type,
    uint32_t type;
    vec3 padding;
} FragUniform;

// TODO texture and sampler, create buffers and use an index field
// in FragUniform to tell which texture to read

int main(int argc, char *argv[]) {
  srand(time(NULL)); // seed random number generator

  initializeLog();

  WGPUInstance instance =
      wgpuCreateInstance(&(WGPUInstanceDescriptor){.nextInChain = NULL});

  WGPUAdapter adapter;
  wgpuInstanceRequestAdapter(instance, NULL, request_adapter_callback,
                             (void *)&adapter);

  WGPUDevice device;
  wgpuAdapterRequestDevice(adapter, NULL, request_device_callback,
                           (void *)&device);

  WGPUQueue queue = wgpuDeviceGetQueue(device);

  wgpuDeviceSetUncapturedErrorCallback(device, handle_uncaptured_error, NULL);

  // Create GLFW Window and use as WebGPU surface
  if (!glfwInit()) {
    printf("Cannot initialize glfw");
    return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "wgpu with glfw", NULL, NULL);

  if (!window) {
    printf("Cannot create window");
    return 1;
  }
  WGPUSurface surface = create_surface(instance, window);
  WGPUTextureFormat presentationFormat =
      wgpuSurfaceGetPreferredFormat(surface, adapter);

  WGPUShaderModuleDescriptor vert_shader_source = load_wgsl(
      RESOURCE_DIR "vert.wgsl");
  WGPUShaderModule vs_module = wgpuDeviceCreateShaderModule(device, &vert_shader_source);

  WGPUShaderModuleDescriptor frag_shader_source = load_wgsl(
      RESOURCE_DIR "frag.wgsl");
  WGPUShaderModule fs_module = wgpuDeviceCreateShaderModule(device, &frag_shader_source);

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(
      device,
      &(WGPURenderPipelineDescriptor){
          .label = "2 attributes",
          .vertex =
              (WGPUVertexState){
                  .module = vs_module,
                  .entryPoint = "main",
                  .bufferCount = 1,
                  .buffers =
                      (WGPUVertexBufferLayout[]){
                          {.arrayStride = sizeof(Vertex),
                           .attributes =
                               (WGPUVertexAttribute[]){
                                   {.shaderLocation = 0,
                                    .offset = offsetof(Vertex, pos),
                                    .format = WGPUVertexFormat_Float32x4},
                                   {.shaderLocation = 1,
                                    .offset = offsetof(Vertex, uv),
                                    .format = WGPUVertexFormat_Float32x2},
                               },
                           .attributeCount = 2,
                           .stepMode = WGPUVertexStepMode_Vertex},
                         },},
          .primitive =
              (WGPUPrimitiveState){
                  .topology = WGPUPrimitiveTopology_TriangleList,
                  .stripIndexFormat = WGPUIndexFormat_Undefined,
                  .frontFace = WGPUFrontFace_CCW,
                  .cullMode = WGPUCullMode_None
                  },
          .multisample =
              (WGPUMultisampleState){
                  .count = 1,
                  .mask = 0xFFFFFFFF,
                  .alphaToCoverageEnabled = false,
              },
          .fragment =
              &(WGPUFragmentState){
                  .module = fs_module,
                  .entryPoint = "main",
                  .targetCount = 1,
                  .targets =
                      &(WGPUColorTargetState){
                          .format = presentationFormat,
                          .blend =
                              &(WGPUBlendState){
                                  .color =
                                      (WGPUBlendComponent){
                                          .srcFactor = WGPUBlendFactor_One,
                                          .dstFactor = WGPUBlendFactor_Zero,
                                          .operation = WGPUBlendOperation_Add,
                                      },
                                  .alpha =
                                      (WGPUBlendComponent){
                                          .srcFactor = WGPUBlendFactor_One,
                                          .dstFactor = WGPUBlendFactor_Zero,
                                          .operation = WGPUBlendOperation_Add,
                                      }},
                          .writeMask = WGPUColorWriteMask_All,
                      },
              },
          .depthStencil = &(WGPUDepthStencilState){
            .nextInChain = NULL,
            .format = WGPUTextureFormat_Depth24Plus,
            .depthWriteEnabled = true,
            .depthCompare = WGPUCompareFunction_Less,
            .stencilFront = (WGPUStencilFaceState){
                .compare = WGPUCompareFunction_Never, // magick value needed 
                .failOp = WGPUStencilOperation_Keep,
                .depthFailOp = WGPUStencilOperation_Keep,
                .passOp = WGPUStencilOperation_Keep,
            },
            .stencilBack = (WGPUStencilFaceState){
                .compare = WGPUCompareFunction_Never, // magick value needed 
                .failOp = WGPUStencilOperation_Keep,
                .depthFailOp = WGPUStencilOperation_Keep,
                .passOp = WGPUStencilOperation_Keep,
            },
            .stencilReadMask = 0,
            .stencilWriteMask = 0,
            .depthBias = 0,
            .depthBiasSlopeScale = 0.0,
            .depthBiasClamp = 0.0
          },
      });

  
  const uint64_t vertex_uniform_buffer_size = sizeof(VertexUniform);
  WGPUBuffer vertex_uniform_buffer = wgpuDeviceCreateBuffer(
        device, &(WGPUBufferDescriptor){.label = "vertex uniform",
                                        .size = vertex_uniform_buffer_size,
                                        .usage = WGPUBufferUsage_Uniform |
                                                WGPUBufferUsage_CopyDst,
                                        .mappedAtCreation = false});
  const uint64_t frag_uniform_buffer_size = sizeof(FragUniform) * vertices_length / 3;
  WGPUBuffer frag_uniform_buffer = wgpuDeviceCreateBuffer(
        device, &(WGPUBufferDescriptor){.label = "frag uniform",
                                        .size = frag_uniform_buffer_size,
                                        .usage = WGPUBufferUsage_Storage,
                                        .mappedAtCreation = true});

  WGPUBindGroup bind_group = wgpuDeviceCreateBindGroup(
      device,
      &(WGPUBindGroupDescriptor){
          .label = "bind group for object",
          .layout = wgpuRenderPipelineGetBindGroupLayout(pipeline, 0),
          .entries = (WGPUBindGroupEntry[]){{.binding = 0,
                                             .buffer = vertex_uniform_buffer,
                                             .size = vertex_uniform_buffer_size,
                                             .offset = 0},
                                            {.binding = 1,
                                             .buffer = frag_uniform_buffer,
                                             .size = frag_uniform_buffer_size,
                                             .offset = 0}},
          .entryCount = 2});

  FragUniform * frag_uniform_mapped = wgpuBufferGetMappedRange(frag_uniform_buffer, 0, sizeof(FragUniform) *  vertices_length / 3);

  FragUniform tmp_frag_ubo[] = {{.type = 1}, {.type = 0}, {.type = 2}};

  memcpy(frag_uniform_mapped, tmp_frag_ubo, sizeof(tmp_frag_ubo));
  wgpuBufferUnmap(frag_uniform_buffer);

  VertexUniform ubos[] = {{.mat = {}}};
  mat4 proj = {};
  mat4 translation = {};
  glm_ortho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -100, 100, proj);
  glm_mat4_identity(translation);
  glm_translate(translation, (float[]){-1, -1, 0});
  glm_mat4_mul(proj, translation, ubos[0].mat);

  wgpuQueueWriteBuffer(queue, vertex_uniform_buffer, 0, ubos,
                       vertex_uniform_buffer_size);

  const WGPUBuffer vertex_buffer = wgpuDeviceCreateBuffer(
      device, &(WGPUBufferDescriptor){.label = "vertex buffer vertices",
                                      .size = sizeof(vertices),
                                      .usage = WGPUBufferUsage_Vertex |
                                               WGPUBufferUsage_CopyDst,
                                      .mappedAtCreation = false});
  wgpuQueueWriteBuffer(queue, vertex_buffer, 0, vertices,

                       sizeof(vertices));


  WGPUSwapChainDescriptor config = (WGPUSwapChainDescriptor){
      .nextInChain =
          (const WGPUChainedStruct *)&(WGPUSwapChainDescriptorExtras){
              .chain =
                  (WGPUChainedStruct){
                      .next = NULL,
                      .sType = (WGPUSType)WGPUSType_SwapChainDescriptorExtras,
                  },
              .alphaMode = WGPUCompositeAlphaMode_Auto,
              .viewFormatCount = 0,
              .viewFormats = NULL,
          },
      .usage = WGPUTextureUsage_RenderAttachment,
      .format = presentationFormat,
      .width = 0,
      .height = 0,
      .presentMode = WGPUPresentMode_Fifo,
  };

  glfwGetWindowSize(window, (int *)&config.width, (int *)&config.height);

  WGPUSwapChain swap_chain = wgpuDeviceCreateSwapChain(device, surface, &config);

  WGPUSupportedLimits limits = {};
  bool gotlimits = wgpuDeviceGetLimits(device, &limits);
  

  WGPUTexture depth_texture = NULL;
  
  while (!glfwWindowShouldClose(window)) {

    WGPUTextureView view = NULL;

    for (int attempt = 0; attempt < 2; attempt++) {
      uint32_t prev_width = config.width;
      uint32_t prev_height = config.height;
      glfwGetWindowSize(window, (int *)&config.width, (int *)&config.height);

      if (prev_width != config.width || prev_height != config.height) {
        swap_chain = wgpuDeviceCreateSwapChain(device, surface, &config);
      }
      // Get the current texture from the swapChain to use for rendering to by
      // the render pass
      view = wgpuSwapChainGetCurrentTextureView(swap_chain);
      if (attempt == 0 && !view) {
        printf("wgpuSwapChainGetCurrentTextureView() failed; trying to create "
               "a new swap chain...\n");
        config.width = 0;
        config.height = 0;
        continue;
      }

      break;
    }

    if (!view) {
      printf("Cannot acquire next swap chain texture\n");
      return 1;
    }

    // make a command encoder to start encoding commands
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(
        device, &(WGPUCommandEncoderDescriptor){.label = "our encoder"});

    int window_width, window_height;

    glfwGetWindowSize(window, (int *)&window_width, (int *)&window_height);

    window_width = (window_width < limits.limits.maxTextureDimension2D)
                       ? window_width
                       : limits.limits.maxTextureDimension2D;
    window_height = (window_height < limits.limits.maxTextureDimension2D)
                        ? window_height
                        : limits.limits.maxTextureDimension2D;

    window_width = (window_width > 1) ? window_width : 1;
    window_height = (window_height > 1) ? window_height : 1;

    // If we don't have a depth texture OR if its size is different
    // from the canvasTexture when make a new depth texture
    if (!depth_texture ||
        wgpuTextureGetWidth(depth_texture) != window_width ||
        wgpuTextureGetHeight(depth_texture) != window_height) {
      if (depth_texture) {
        wgpuTextureDestroy(depth_texture);
      }
      depth_texture = wgpuDeviceCreateTexture(device, &(WGPUTextureDescriptor){
        .nextInChain = NULL,
        .label = NULL,
        .usage = WGPUTextureUsage_RenderAttachment,
        .dimension = WGPUTextureDimension_2D,
        .size = (WGPUExtent3D){
            .width = window_width,
            .height = window_height,
            .depthOrArrayLayers = 1,
        },
        .format = WGPUTextureFormat_Depth24Plus,
        .mipLevelCount = 1,
        .sampleCount = 1,
        .viewFormatCount = 0,
        .viewFormats = (WGPUTextureFormat[1]){WGPUTextureFormat_Undefined},
      });
    }
   
   WGPUTextureView depth_stencil_attachment_view = wgpuTextureCreateView(depth_texture, NULL);

    WGPURenderPassDescriptor render_pass_descriptor = {
        .label = "our basic canvas renderPass",
        .colorAttachments =
            &(WGPURenderPassColorAttachment){
                .view = view, // texture from SwapChain
                .resolveTarget = NULL,
                .loadOp = WGPULoadOp_Clear,
                .storeOp = WGPUStoreOp_Store,
                .clearValue =
                    (WGPUColor){
                        .r = 0.3,
                        .g = 0.3,
                        .b = 0.3,
                        .a = 1.0,
                    },
            },
        .colorAttachmentCount = 1,
        .depthStencilAttachment = &(WGPURenderPassDepthStencilAttachment){
            .view = depth_stencil_attachment_view,
            .depthLoadOp = WGPULoadOp_Clear,
            .depthStoreOp = WGPUStoreOp_Store,
            .depthClearValue = 1.0,
            .depthReadOnly = false,
            .stencilLoadOp = WGPULoadOp_Clear, // magick value needed 
            .stencilStoreOp = WGPUStoreOp_Store, // magick value needed 
            .stencilClearValue = 0,
            .stencilReadOnly = false,
        },
    };

    // make a render pass encoder to encode render specific commands
    WGPURenderPassEncoder pass =
        wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_descriptor);
    wgpuRenderPassEncoderSetPipeline(pass, pipeline);
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertex_buffer, 0,
                                         sizeof(vertices));
    
    wgpuRenderPassEncoderSetBindGroup(pass, 0, bind_group, 0, NULL);
    wgpuRenderPassEncoderDraw(pass, vertices_length, 1, 0, 0);

    wgpuRenderPassEncoderEnd(pass);

    WGPUQueue queue = wgpuDeviceGetQueue(device);
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(
        encoder, &(WGPUCommandBufferDescriptor){.label = NULL});
    wgpuQueueSubmit(queue, 1, &commandBuffer);
    wgpuSwapChainPresent(swap_chain);

    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
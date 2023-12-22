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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "draw.h"
#include "atlas.h"

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
  GLFWwindow *window = glfwCreateWindow(640, 480, "wgpu with glfw", NULL, NULL);

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

int window_width_i, window_height_i;
glfwGetWindowSize(window, (int *)&window_width_i, (int *)&window_height_i);

int img_width, img_height, raw_image_channels;
unsigned char *img = stbi_load(RESOURCE_DIR "gotta-go-fast.png", &img_width,
                               &img_height, &raw_image_channels, 4);
size_t img_data_size = img_width * img_height * 4;

size_t white_tex_scale = 80;
size_t white_tex_data_size = white_tex_scale * white_tex_scale * 4;

unsigned char *white_texture_data = malloc(white_tex_data_size);
memset(white_texture_data, 0xff, white_tex_data_size);

ImageData imgs[2] = {
    {.width = img_width, .height = img_height, .data = img},
    {.width = white_tex_scale, .height = white_tex_scale, .data = white_texture_data}
};

Atlas atlas = atlasCreate(imgs, 2, 640);
stbi_image_free(img);
free(white_texture_data);

UVData img_uv_data = imgs[0].uv_data;
UVData white_texture_uv_data = imgs[1].uv_data;

App app = {};
float window_width = (float)window_width_i;
float window_height = (float)window_height_i;
float triangle_scale = 250.0;
// drawEquilateralTriangle(&app, (vec2){ window_width / 2, window_height / 2 }, triangle_scale, (FragUniform){.type = GkurveType_Filled, .blend_color = {1,1,1,1}}, img_uv_data);
// drawEquilateralTriangle(&app, (vec2){ window_width / 2, window_height / 2 - triangle_scale }, triangle_scale, (FragUniform){.type = GkurveType_Concave, .blend_color = {1,1,1,1}}, img_uv_data);
// drawEquilateralTriangle(&app, (vec2){ window_width / 2 - triangle_scale, window_height / 2 - triangle_scale / 2 }, triangle_scale, (FragUniform){ .type = GkurveType_Convex, .blend_color = {1,1,1,1} }, white_texture_uv_data);
// drawQuad(&app, (vec2){0, 0}, (vec2){200, 200}, (FragUniform){.type = GkurveType_Filled, .blend_color = {1,1,1,1}}, img_uv_data);
drawCircle(&app, (vec2){ window_width / 2, window_height / 2 }, window_height / 2 - 10, (vec4){ 0, 0.5, 0.75, 1.0 }, white_texture_uv_data);
// update_vertex_buffer
const WGPUBuffer vertex_buffer = wgpuDeviceCreateBuffer(
    device, &(WGPUBufferDescriptor){.label = "vertex buffer vertices",
                                    .size = app.vertices_size,
                                    .usage = WGPUBufferUsage_Vertex |
                                             WGPUBufferUsage_CopyDst,
                                    .mappedAtCreation = false});
wgpuQueueWriteBuffer(queue, vertex_buffer, 0, app.vertices, app.vertices_size);

size_t vertices_length = app.vertices_size / sizeof(Vertex);
// update_vertex_uniform_buffer
size_t vertex_uniform_buffer_size = sizeof(VertexUniform);
WGPUBuffer vertex_uniform_buffer = wgpuDeviceCreateBuffer(
    device, &(WGPUBufferDescriptor){.label = "vertex uniform",
                                    .size = vertex_uniform_buffer_size,
                                    .usage = WGPUBufferUsage_Uniform |
                                             WGPUBufferUsage_CopyDst,
                                    .mappedAtCreation = false});
VertexUniform ubos[] = {{.mat = {}}};
mat4 proj = {};
mat4 translation = {};
glm_ortho(0, (float)window_width_i, 0, (float)window_height_i, -100, 100, proj);
glm_mat4_identity(translation);
glm_translate(translation, (float[]){-1, -1, 0});
glm_mat4_mul(proj, translation, ubos[0].mat);
wgpuQueueWriteBuffer(queue, vertex_uniform_buffer, 0, ubos, vertex_uniform_buffer_size);
// update_frag_uniform_buffer
size_t frag_uniform_buffer_size = sizeof(FragUniform) * vertices_length / 3;
WGPUBuffer frag_uniform_buffer = wgpuDeviceCreateBuffer(
    device, &(WGPUBufferDescriptor){.label = "frag uniform",
                                    .size = frag_uniform_buffer_size,
                                    .usage = WGPUBufferUsage_Storage |
                                             WGPUBufferUsage_CopyDst,
                                    .mappedAtCreation = false});
wgpuQueueWriteBuffer(queue, frag_uniform_buffer, 0, app.fragment_uniform_list, frag_uniform_buffer_size);

WGPUSampler sampler = wgpuDeviceCreateSampler(
    device,
    &(WGPUSamplerDescriptor){
        .nextInChain = NULL,
        .label = NULL,
        .addressModeU = WGPUAddressMode_Repeat,
        .addressModeV = WGPUAddressMode_Repeat,
        .addressModeW = WGPUAddressMode_Repeat,
        .magFilter = WGPUFilterMode_Linear,
        .minFilter = WGPUFilterMode_Linear,
        .mipmapFilter = WGPUMipmapFilterMode_Nearest,
        .lodMinClamp = 0.0,
        .lodMaxClamp = 0.0,
        .compare = WGPUCompareFunction_Undefined,
        .maxAnisotropy = 1, //** mystery_setting ** - needs this value to work
    });


WGPUTexture texture = wgpuDeviceCreateTexture(
    device,
    &(WGPUTextureDescriptor){
        .nextInChain = NULL,
        .label = "atlas texture",
        .size =
            (WGPUExtent3D){
                .depthOrArrayLayers = 1,
                .width = atlas.texture_side_length,
                .height = atlas.texture_side_length},
        .format = WGPUTextureFormat_RGBA8Unorm,
        .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst |
                 WGPUTextureUsage_RenderAttachment,
        .dimension = WGPUTextureDimension_2D,
        .mipLevelCount = 1, //** mystery_setting ** - needs this value to work
        .sampleCount = 1,
        .viewFormatCount = 0,
        .viewFormats = NULL});

wgpuQueueWriteTexture(
    queue,
    &(WGPUImageCopyTexture){.texture = texture,
                            .nextInChain = NULL,
                            .aspect = WGPUTextureAspect_All,
                            .mipLevel = 0,
                            .origin = (WGPUOrigin3D){.x = 0, .y = 0, .z = 0}},
    atlas.texture_data, atlas.texture_data_size,
    &(WGPUTextureDataLayout){
        .bytesPerRow = atlas.texture_side_length * 4,
        .nextInChain = NULL,
        .offset = 0,
        .rowsPerImage = atlas.texture_side_length,
    },
    &(WGPUExtent3D){.depthOrArrayLayers =
                        1, // write one layer
                    .width = atlas.texture_side_length,
                    .height = atlas.texture_side_length});

WGPUBindGroup bind_group = wgpuDeviceCreateBindGroup(
    device,
    &(WGPUBindGroupDescriptor){
        .label = "bind group for object",
        .layout = wgpuRenderPipelineGetBindGroupLayout(pipeline, 0),
        .entries =
            (WGPUBindGroupEntry[]){
                {.binding = 0,
                 .buffer = vertex_uniform_buffer,
                 .size = vertex_uniform_buffer_size,
                 .offset = 0},
                {.binding = 1,
                 .buffer = frag_uniform_buffer,
                 .size = frag_uniform_buffer_size,
                 .offset = 0},
                {.binding = 2,
                 .buffer = NULL,
                 .size = 0,
                 .offset = 0,
                 .sampler = sampler},
                {.binding = 3,
                 .buffer = NULL,
                 .size = 0,
                 .offset = 0,
                 .textureView = wgpuTextureCreateView(
                     texture,
                     &(WGPUTextureViewDescriptor){
                         .nextInChain = NULL,
                         .label = NULL,
                         .format = WGPUTextureFormat_Undefined,
                         .dimension = WGPUTextureViewDimension_2D,
                         .baseMipLevel = 0,
                         .mipLevelCount = 1, //** mystery_setting ** - needs
                                             // this value to work
                         .baseArrayLayer = 0,
                         .arrayLayerCount = 1,
                         // this value to work
                         .aspect = WGPUTextureAspect_All,
                     })},
            },
        .entryCount = 4});

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

    glfwGetWindowSize(window, (int *)&window_width_i, (int *)&window_height_i);

    window_width_i = (window_width_i < limits.limits.maxTextureDimension2D)
                       ? window_width_i
                       : limits.limits.maxTextureDimension2D;
    window_height_i = (window_height_i < limits.limits.maxTextureDimension2D)
                        ? window_height_i
                        : limits.limits.maxTextureDimension2D;

    window_width_i = (window_width_i > 1) ? window_width_i : 1;
    window_height_i = (window_height_i > 1) ? window_height_i : 1;

    // If we don't have a depth texture OR if its size is different
    // from the canvasTexture when make a new depth texture
    if (!depth_texture ||
        wgpuTextureGetWidth(depth_texture) != window_width_i ||
        wgpuTextureGetHeight(depth_texture) != window_height_i) {
      if (depth_texture) {
        wgpuTextureDestroy(depth_texture);
      }
      depth_texture = wgpuDeviceCreateTexture(device, &(WGPUTextureDescriptor){
        .nextInChain = NULL,
        .label = NULL,
        .usage = WGPUTextureUsage_RenderAttachment,
        .dimension = WGPUTextureDimension_2D,
        .size = (WGPUExtent3D){
            .width = window_width_i,
            .height = window_height_i,
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
                                         app.vertices_size);

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
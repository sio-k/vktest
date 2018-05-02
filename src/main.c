/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

// simple Vulkan test

// convention for error codes where applicable:
//  any value above zero = things are fine
//  zero = failure
//  values below zero are unused for now to ease error checking

// fail noisy, fail fast: anything that's not implemented for a platform
// should give a compiler error when compiling on that platform, even if
// the feature is unused
// convention for severe/difficult errors:
//      a) printf error details and abort if more details than with an assert
//         can be provided by you
// or   b) assert() state that allows program to continue operation

#include "libs.h"

#include "vk_init.h"
#include "vk_platform_specific.h"
#include "device_info.h"

// TODO: group all of this mess into structs and put all of the init for those structs into their respective functions (ESPECIALLY VULKAN, FUCKING HELL THIS IS A LOT OF CODE)

extern SDL_Window*  g_window = NULL;
extern const char*  g_program_name = "Pelmeninator";
extern const char* const g_engine_name = "vodka";
extern const u32    g_program_major_version = 0;
extern const u32    g_program_minor_version = 1;
extern const u32    g_program_patch_version = 0;

extern i8 g_enable_vk_validation_layers = 1;

extern VkApplicationInfo g_vk_app_info = { 0 };
extern VkInstance   g_vk_instance = VK_NULL_HANDLE;
extern VkSurfaceKHR g_vk_surface = { 0 };

extern u32 g_vk_num_physical_devices = 0;
extern VkPhysicalDevice* g_vk_physical_devices = NULL;
extern GPU* g_gpus = NULL; // parallel array with g_vk_physical_devices

extern i32 g_graphics_family_queue_index = -1;
extern i32 g_present_family_queue_index = -1;
extern i32 g_physical_device_index = -1;
extern GPU* g_gpu_used = NULL;

extern VkDevice g_vk_device = VK_NULL_HANDLE;
extern VkQueue g_vk_graphics_queue = VK_NULL_HANDLE;
extern VkQueue g_vk_present_queue = VK_NULL_HANDLE;

extern const i32 g_vk_num_buffers = 2;
extern VkSemaphore g_vk_aquire_semaphores[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
extern VkSemaphore g_vk_render_complete_semaphores[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };

extern VkCommandPool g_vk_command_pool = VK_NULL_HANDLE;
// we have two command buffers: one front and one back buffer
extern VkCommandBuffer g_vk_command_buffers[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
extern VkFence g_vk_command_buffer_fences[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };

extern VkSurfaceFormatKHR g_vk_surface_format = { 0 };
// FIFO mode is always available
extern VkPresentModeKHR g_vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
extern VkExtent2D g_vk_surface_extent = { 0 };

extern VkSwapchainKHR g_vk_swapchain = { 0 };
extern VkFormat g_vk_swapchain_format = 0;
extern VkExtent2D g_vk_swapchain_extent = { 0 };
extern VkImage g_vk_swapchain_images[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
extern VkImageView g_vk_swapchain_image_views[2] = { VK_NULL_HANDLE, VK_NULL_HANDLE };

extern VkRenderPass g_vk_render_pass = 0;

extern VkFramebuffer g_vk_frame_buffers[2] = { 0, 0 };

int main()
{
    // generic globals setup
    {
        g_vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        g_vk_app_info.pNext = NULL;
        g_vk_app_info.pApplicationName = g_program_name;
        g_vk_app_info.applicationVersion = VK_MAKE_VERSION(
                                               g_program_major_version,
                                               g_program_minor_version,
                                               g_program_patch_version
                                           );
        g_vk_app_info.pEngineName = g_engine_name;
        g_vk_app_info.engineVersion = VK_MAKE_VERSION(
                                          g_program_major_version,
                                          g_program_minor_version,
                                          g_program_patch_version
                                      );
        g_vk_app_info.apiVersion = VK_MAKE_VERSION( 1, 0, VK_HEADER_VERSION );
    }

    // library setup
    {
        // TODO for a release build: check if any of these fails to init and abort with an appropriate message if so
        SDL_SetMainReady();
        SDL_Init( SDL_INIT_EVERYTHING );
        atexit( SDL_Quit );

        IMG_Init( IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_JPG );
        atexit( IMG_Quit );

        TTF_Init();
        atexit( TTF_Quit );
    }

    // Vulkan setup
    initVulkan();
    atexit( destroyVulkan );

    // print general GPU info
    for ( u32 i = 0; i < g_vk_num_physical_devices; i++ ) {
        puts( " " );
        gpuPrintInfo( &( g_gpus[i] ) );
    }

    // this is where the actual main loop and stuff would go but right now we ain't doin shit so we just delay six seconds and quit
    SDL_Delay( 6000 );

    exit( EXIT_SUCCESS );
}

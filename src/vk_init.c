/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

#include "vk_init.h"

extern SDL_Window*  g_window;
extern const char*  g_program_name;
extern const char* const g_engine_name;
extern const u32    g_program_major_version;
extern const u32    g_program_minor_version;
extern const u32    g_program_patch_version;

extern i8 g_enable_vk_validation_layers;

extern VkApplicationInfo g_vk_app_info;
extern VkInstance   g_vk_instance;
extern VkSurfaceKHR g_vk_surface;

extern u32 g_vk_num_physical_devices;
extern VkPhysicalDevice* g_vk_physical_devices;
extern GPU* g_gpus; // parallel array with g_vk_physical_devices

extern i32 g_graphics_family_queue_index;
extern i32 g_present_family_queue_index;
extern i32 g_physical_device_index;
extern GPU* g_gpu_used;

extern VkDevice g_vk_device;
extern VkQueue g_vk_graphics_queue;
extern VkQueue g_vk_present_queue;

extern const i32 g_vk_num_buffers;
extern VkSemaphore g_vk_aquire_semaphores[2];
extern VkSemaphore g_vk_render_complete_semaphores[2];

extern VkCommandPool g_vk_command_pool;
// we have two command buffers: one front and one back buffer
extern VkCommandBuffer g_vk_command_buffers[2];
extern VkFence g_vk_command_buffer_fences[2];

extern VkSurfaceFormatKHR g_vk_surface_format;
// FIFO mode is always available
extern VkPresentModeKHR g_vk_present_mode;
extern VkExtent2D g_vk_surface_extent;

extern VkSwapchainKHR g_vk_swapchain;
extern VkFormat g_vk_swapchain_format;
extern VkExtent2D g_vk_swapchain_extent;
extern VkImage g_vk_swapchain_images[2];
extern VkImageView g_vk_swapchain_image_views[2];

extern VkRenderPass g_vk_render_pass;

extern VkFramebuffer g_vk_frame_buffers[2];

void initVulkan()
{
    // instance creation
    u32 vk_instance_ext_count = 0;
    u32 vk_device_ext_count = 0;
    u32 vk_validation_layer_count = 0;
    const char** vk_validation_layer_names = NULL;
    #ifdef PLATFORM_UNIX
    vk_instance_ext_count = 3;
    const char* vk_instance_ext_names[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME
    };
    vk_device_ext_count = 1;
    const char* vk_device_ext_names[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    #elif defined( PLATFORM_WINDOWS )
#error "main.c: Windows does not have a defined set of required instance/device extensions"
    #endif
    VkInstanceCreateInfo vk_instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pApplicationInfo = &g_vk_app_info,
        .enabledLayerCount = vk_validation_layer_count,
        .ppEnabledLayerNames = vk_validation_layer_names,
        .enabledExtensionCount = vk_instance_ext_count,
        .ppEnabledExtensionNames = vk_instance_ext_names
    };

    VK_PRINT_AND_ABORT_ON_FAIL(
        vkCreateInstance( &vk_instance_create_info, NULL, &g_vk_instance )
    );

    // window/drawing surface setup
    {
        g_window = SDL_CreateWindow(
                       g_program_name,
                       SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED,
                       1024,
                       768,
                       SDL_WINDOW_SHOWN
                   );

        createVulkanSurface( g_window, g_vk_instance, &g_vk_surface );
    }

    // physical devices setup
    {
        vkEnumeratePhysicalDevices( g_vk_instance, &g_vk_num_physical_devices, NULL );
        assert( g_vk_num_physical_devices > 0 );
        g_vk_physical_devices = calloc( g_vk_num_physical_devices,
                                        sizeof( VkPhysicalDevice ) );
        assert( g_vk_physical_devices != NULL );
        vkEnumeratePhysicalDevices( g_vk_instance, &g_vk_num_physical_devices,
                                    g_vk_physical_devices );
        assert( g_vk_num_physical_devices > 0 );

        // fill in GPU info structs
        g_gpus = calloc( g_vk_num_physical_devices, sizeof( GPU ) );
        for ( u32 i = 0; i < g_vk_num_physical_devices; i++ ) {
            gpu( &( g_gpus[i] ), g_vk_physical_devices[i] );
        }
    }

    // choose physical device to use
    {
        for ( i32 i = 0; i < g_vk_num_physical_devices; i++ ) {
            GPU* current = &( g_gpus[i] );
            i32 graphics_queue_index = -1;
            i32 present_queue_index = -1;
            // check if device is actually capable of drawing
            if ( current -> num_surface_formats < 1 ) {
                continue;
            }
            if ( current -> num_surface_present_modes < 1 ) {
                continue;
            }
            for ( i32 j = 0; j < current -> num_queue_properties; j++ ) {
                VkQueueFamilyProperties* qfp = &( current -> queue_properties[j] );
                if ( qfp -> queueCount < 1 ) {
                    continue;
                }
                if ( qfp -> queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
                    // found a graphics queue
                    graphics_queue_index = j;
                    break;
                }
            }
            for ( i32 j = 0; j < current -> num_queue_properties; j++ ) {
                VkQueueFamilyProperties* qfp = &( current -> queue_properties[j] );
                if ( qfp -> queueCount < 1 ) {
                    continue;
                }
                VkBool32 supports_present = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    current -> device,
                    j, g_vk_surface,
                    &supports_present
                );
                if ( supports_present == VK_TRUE ) {
                    // found a present queue
                    present_queue_index = j;
                    break;
                }
            }
            if ( graphics_queue_index >= 0 && present_queue_index >= 0 ) {
                g_graphics_family_queue_index = graphics_queue_index;
                g_present_family_queue_index = present_queue_index;
                g_physical_device_index = i;
                g_gpu_used = current;
                break;
            }
        }
    }

    // create logical device and queues
    {
        const float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo graphics_queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .queueFamilyIndex = g_graphics_family_queue_index,
            .queueCount = 1,
            .pQueuePriorities = &( queue_priority )
        };
        VkDeviceQueueCreateInfo present_queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .queueFamilyIndex = g_present_family_queue_index,
            .queueCount = 1,
            .pQueuePriorities = &( queue_priority )
        };

        VkDeviceQueueCreateInfo queue_create_info[2];
        queue_create_info[0] = graphics_queue_create_info;
        queue_create_info[1] = present_queue_create_info;

        VkPhysicalDeviceFeatures device_features = { 0 };
        device_features.depthClamp = VK_TRUE;
        device_features.depthBounds = VK_TRUE;

        VkDeviceCreateInfo device_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .queueCreateInfoCount = 2,
            .pQueueCreateInfos = queue_create_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = NULL,
            .enabledExtensionCount = vk_device_ext_count,
            .ppEnabledExtensionNames = vk_device_ext_names,
            .pEnabledFeatures = &device_features
        };

        if ( g_enable_vk_validation_layers ) {
            device_info.enabledLayerCount = vk_validation_layer_count;
            device_info.ppEnabledLayerNames = vk_validation_layer_names;
        }

        VK_PRINT_AND_ABORT_ON_FAIL(
            vkCreateDevice(
                g_vk_physical_devices[g_physical_device_index],
                &device_info,
                NULL,
                &g_vk_device
            )
        );

        vkGetDeviceQueue(
            g_vk_device,
            g_graphics_family_queue_index,
            0,
            &g_vk_graphics_queue
        );
        vkGetDeviceQueue(
            g_vk_device,
            g_present_family_queue_index,
            0,
            &g_vk_present_queue
        );
    }

    // create aquire and render completion semaphores
    {
        VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0
        };
        for ( i32 i = 0; i < 2; i++ ) {
            VK_PRINT_AND_ABORT_ON_FAIL(
                vkCreateSemaphore(
                    g_vk_device,
                    &semaphore_create_info,
                    NULL,
                    &( g_vk_aquire_semaphores[i] )
                )
            );
            VK_PRINT_AND_ABORT_ON_FAIL(
                vkCreateSemaphore(
                    g_vk_device,
                    &semaphore_create_info,
                    NULL,
                    &( g_vk_render_complete_semaphores[i] )
                )
            );
        }
    }

    // create command pool
    {
        VkCommandPoolCreateInfo command_pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = NULL,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = g_graphics_family_queue_index
        };
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkCreateCommandPool(
                g_vk_device,
                &command_pool_create_info,
                NULL,
                &g_vk_command_pool
            )
        );
    }

    // create command buffer and fences
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = g_vk_command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = g_vk_num_buffers
        };
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkAllocateCommandBuffers(
                g_vk_device,
                &command_buffer_allocate_info,
                g_vk_command_buffers
            )
        );

        VkFenceCreateInfo fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0
        };
        for ( i32 i = 0; i < g_vk_num_buffers; i++ ) {
            VK_PRINT_AND_ABORT_ON_FAIL(
                vkCreateFence(
                    g_vk_device,
                    &fence_create_info,
                    NULL,
                    &( g_vk_command_buffer_fences[i] )
                )
            );
        }
    }

    // choose a surface format to use
    {
        if ( g_gpu_used -> num_surface_formats <= 0  ) {
            printf( "GPU does not support any surfaces!\n" );
            abort();
        }

        if (
            g_gpu_used -> num_surface_formats == 1
            && g_gpu_used -> surface_formats[0].format == VK_FORMAT_UNDEFINED
        ) {
            // unknown format, force our preferred format
            g_vk_surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
            g_vk_surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        } else {
            for ( u32 i = 0; i < g_gpu_used -> num_surface_formats; i++ ) {
                VkSurfaceFormatKHR* fmt = &( g_gpu_used -> surface_formats[i] );
                if (
                    fmt -> format == VK_FORMAT_B8G8R8A8_UNORM
                    && fmt -> colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                ) {
                    g_vk_surface_format = *fmt;
                    break;
                }
            }
            if ( g_vk_surface_format.format == 0 ) {
                // no format set yet
                // get R8G8B8A8 format of some sort
                for ( u32 i = 0; i < g_gpu_used -> num_surface_formats; i++ ) {
                    VkSurfaceFormatKHR* fmt = &( g_gpu_used -> surface_formats[i] );
                    if ( fmt -> format == VK_FORMAT_B8G8R8A8_UNORM ) {
                        g_vk_surface_format = *fmt;
                        break;
                    }
                }
            }
            if ( g_vk_surface_format.format == 0 ) {
                // no format set yet, just choose any sodding format at this point
                g_vk_surface_format = g_gpu_used -> surface_formats[0];
            }
        }
    }

    // check if mailbox mode for presenting is available,
    // and if available set it
    {
        // FIXME: fails because VK_PRESENT_MAILBOX_KHR is not defined. Where the fuck is it defined?
        #if 0
        for ( i32 i = 0; i < g_gpu_used -> num_surface_present_modes; i++ ) {
            if (
                g_gpu_used -> surface_present_modes[i]
                == VK_PRESENT_MODE_MAILBOX_KHR
            ) {
                g_vk_present_mode = VK_PRESENT_MAILBOX_KHR;
            }
        }
        #endif
    }

    // set extent (size) of surface (to size of window)
    {
        i32 w = 0;
        i32 h = 0;
        SDL_GetWindowSize(
            g_window,
            &w,
            &h
        );
        g_vk_surface_extent.width = w;
        g_vk_surface_extent.height = h;
    }

    // create swapchain
    {
        VkSwapchainCreateInfoKHR swapchain_create_info = { 0 };
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.pNext = NULL;
        swapchain_create_info.flags = 0;
        swapchain_create_info.surface = g_vk_surface;
        swapchain_create_info.minImageCount = g_vk_num_buffers;
        swapchain_create_info.imageFormat = g_vk_surface_format.format;
        swapchain_create_info.imageColorSpace = g_vk_surface_format.colorSpace;
        swapchain_create_info.imageExtent = g_vk_surface_extent;
        swapchain_create_info.imageArrayLayers = 1,
        swapchain_create_info.imageUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        // if graphics and present queue family don't match, we need different info
        if ( g_graphics_family_queue_index != g_present_family_queue_index ) {
            u32 indexes[] = {
                ( u32 ) g_graphics_family_queue_index,
                ( u32 ) g_present_family_queue_index
            };
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = 2;
            swapchain_create_info.pQueueFamilyIndices = indexes;
            // relies on everything in this block being in stack frame
        } else {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = g_vk_present_mode;

        // allow vk to discard ops outside renderable space
        swapchain_create_info.clipped = VK_TRUE;

        VK_PRINT_AND_ABORT_ON_FAIL(
            vkCreateSwapchainKHR(
                g_vk_device,
                &swapchain_create_info,
                NULL,
                &g_vk_swapchain
            )
        );

        g_vk_swapchain_format = g_vk_surface_format.format;
        g_vk_swapchain_extent = g_vk_surface_extent;

        u32 num_swapchain_images = 0;
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkGetSwapchainImagesKHR(
                g_vk_device,
                g_vk_swapchain,
                &num_swapchain_images,
                NULL
            )
        );
        assert( num_swapchain_images == 2 );
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkGetSwapchainImagesKHR(
                g_vk_device,
                g_vk_swapchain,
                &num_swapchain_images,
                g_vk_swapchain_images
            )
        );

        for ( u32 i = 0; i < g_vk_num_buffers; i++ ) {
            VkImageViewCreateInfo image_view_create_info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
                .image = g_vk_swapchain_images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = g_vk_swapchain_format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_R,
                    .g = VK_COMPONENT_SWIZZLE_G,
                    .b = VK_COMPONENT_SWIZZLE_B,
                    .a = VK_COMPONENT_SWIZZLE_A
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            VK_PRINT_AND_ABORT_ON_FAIL(
                vkCreateImageView(
                    g_vk_device,
                    &image_view_create_info,
                    NULL,
                    &( g_vk_swapchain_image_views[i] )
                )
            );
        }
    }

    // create render pass and thus depth buffer (one render pass right now)
    {
        VkAttachmentDescription render_pass_attachments[1] = {
            {
                // color attachment
                .flags = 0,
                .format = g_vk_swapchain_format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                // don't care what is done with img memory when loaded for use
                .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            }
        };

        VkAttachmentReference color_attachment_ref = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        // this is a graphics pass, we'll mention that using this
        VkSubpassDescription subpass = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = NULL,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_ref,
            .pResolveAttachments = NULL,
            .pDepthStencilAttachment = NULL,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = NULL
        };

        VkRenderPassCreateInfo render_pass_create_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .attachmentCount = 1,
            .pAttachments = render_pass_attachments,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 0,
            .pDependencies = NULL
        };

        VK_PRINT_AND_ABORT_ON_FAIL(
            vkCreateRenderPass(
                g_vk_device,
                &render_pass_create_info,
                NULL,
                &g_vk_render_pass
            )
        );
    }

    // create framebuffers (front and back)
    {
        VkImageView attachments[1];
        VkFramebufferCreateInfo frame_buffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .renderPass = g_vk_render_pass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = g_vk_surface_extent.width,
            .height = g_vk_surface_extent.height,
            .layers = 1
        };
        for ( i32 i = 0; i < g_vk_num_buffers; i++ ) {
            attachments[0] = g_vk_swapchain_image_views[i];
            VK_PRINT_AND_ABORT_ON_FAIL(
                vkCreateFramebuffer(
                    g_vk_device,
                    &frame_buffer_create_info,
                    NULL,
                    &( g_vk_frame_buffers[i] )
                )
            );
        }
    }
}

void destroyVulkan()
{
    // free/destroy heap-allocated globals
    for ( u32 i = 0; i < g_vk_num_physical_devices; i++ ) {
        gpuDestroy( &( g_gpus[i] ) );
    }
    free( g_gpus );
    free( g_vk_physical_devices );

    vkDestroyInstance( g_vk_instance, NULL );
    g_vk_instance = VK_NULL_HANDLE;

    SDL_DestroyWindow( g_window );
}

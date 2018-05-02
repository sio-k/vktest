#pragma once
/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

#include "libs.h"

i8 createVulkanSurface( SDL_Window* win, VkInstance instance,
                        VkSurfaceKHR* surface );

#if defined( PLATFORM_WINDOWS )
    typedef VkWin32SurfaceCreateInfoKHR WindowSurfaceCreateInfo;
#elif defined( PLATFORM_UNIX )
    // NOTE: we're assuming SDL2 decides to use XCB.
    // TODO: incorporate code to automatically detect the library used to deal with the windowing system and choose the right one depending on whether XCB or XLib was used (refer to GLFW's platform_x11.h for useful info)
    #define VK_USE_PLATFORM_XCB_KHR
    typedef VkXcbSurfaceCreateInfoKHR WindowSurfaceCreateInfo;
#else
    #error "OS has no known defined window surface and thus no defined surface init routine, please add one in vk_platform_specific.c/.h if Vulkan is supported"
#endif

/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

#include "vk_platform_specific.h"

i8 createVulkanSurface( SDL_Window* win, VkInstance instance,
                        VkSurfaceKHR* surface )
{
    WindowSurfaceCreateInfo create_info;
    memset( &create_info, 0, sizeof( create_info ) );
    SDL_SysWMinfo wm_info;
    SDL_VERSION( &wm_info.version );
    if ( !SDL_GetWindowWMInfo( win, &wm_info ) ) {
        printf( "Cannot get window manager information from SDL: %s\n",
                SDL_GetError() );
        abort();
    }
    #if defined( PLATFORM_WINDOWS )
#error "createVulkanSurface not implemented for Windows"
    #elif defined( PLATFORM_UNIX )
    create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.connection = XGetXCBConnection( wm_info.info.x11.display );
    create_info.window = wm_info.info.x11.window;
    VK_PRINT_AND_ABORT_ON_FAIL(
        vkCreateXcbSurfaceKHR( instance, &create_info, NULL, surface )
    );

    return 1;
    #endif
}

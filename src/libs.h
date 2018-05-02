#pragma once
/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

#if defined( _WIN64 ) || defined( _WIN32 )
    #define PLATFORM_WINDOWS
#elif defined( __linux__ ) || defined( BSD )
    #define PLATFORM_UNIX
#endif

#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#ifdef PLATFORM_UNIX
    #define VK_USE_PLATFORM_XCB_KHR
    #include <X11/Xlib-xcb.h>
#elif defined( PLATFORM_WINDOWS )
    #error "no Windows native window manager include file set in libs.h"
#else
    #error "OS unknown, please set Window manager include file in libs.h"
#endif
#include <vulkan/vulkan.h>
#include <SDL2/SDL_syswm.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u8 byte;
typedef size_t usize;

typedef float f32;
typedef double f64;

#define VK_PRINT_AND_ABORT_ON_FAIL(fn_call) \
    if ( VK_SUCCESS != ( fn_call ) ) { \
        fprintf( stderr, "Vulkan fail: %s, line %u", __FILE__, __LINE__ ); \
        abort(); \
    }

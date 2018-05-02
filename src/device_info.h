#pragma once
/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

#include "libs.h"

// device information containers

typedef struct GPU
{
    VkPhysicalDevice device;
    u32 num_queue_properties;
    u32 num_extensions;
    u32 num_surface_formats;
    u32 num_surface_present_modes;
    VkQueueFamilyProperties* queue_properties;
    VkExtensionProperties*   extension_properties;
    VkSurfaceFormatKHR*      surface_formats;
    VkPresentModeKHR*        surface_present_modes;
    VkSurfaceCapabilitiesKHR         surface_capabilities;
    VkPhysicalDeviceMemoryProperties memory_properties;
    VkPhysicalDeviceProperties       device_properties;
} GPU;

void gpu( GPU* g, VkPhysicalDevice dev );
void gpuPrintInfo( GPU* g );
void gpuDestroy( GPU* g );

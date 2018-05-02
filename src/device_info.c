/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

#include "device_info.h"

extern VkSurfaceKHR g_vk_surface;

void gpu( GPU* g, VkPhysicalDevice dev )
{
    assert( g != NULL );
    assert( dev != NULL );

    memset( g, 0, sizeof( GPU ) );

    g -> device = dev;

    // get queue properties
    {
        vkGetPhysicalDeviceQueueFamilyProperties(
            g -> device,
            &( g -> num_queue_properties ),
            NULL
        );
        assert( g -> num_queue_properties > 0 );
        g -> queue_properties = malloc(
                                    ( g -> num_queue_properties ) * sizeof( VkQueueFamilyProperties )
                                );
        assert( g -> queue_properties != NULL );
        vkGetPhysicalDeviceQueueFamilyProperties(
            g -> device,
            &( g -> num_queue_properties ),
            g -> queue_properties
        );
        assert( g -> num_queue_properties > 0 );
    }

    // get extension properties
    {
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkEnumerateDeviceExtensionProperties(
                g -> device,
                NULL,
                &( g -> num_extensions ),
                NULL
            )
        );
        g -> extension_properties = malloc(
                                        ( g -> num_extensions ) * sizeof( VkExtensionProperties ) + 1
                                    ); // alloc at least 1B
        assert( g -> extension_properties != NULL );
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkEnumerateDeviceExtensionProperties(
                g -> device,
                NULL,
                &( g -> num_extensions ),
                g -> extension_properties
            )
        );
    }

    // get surface capabilities
    {
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                g -> device,
                g_vk_surface,
                &( g -> surface_capabilities )
            )
        );
    }

    // get surface formats
    {
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                g -> device,
                g_vk_surface,
                &( g -> num_surface_formats ),
                NULL
            )
        );
        g -> surface_formats = malloc(
                                   ( g -> num_surface_formats ) * sizeof( VkSurfaceFormatKHR ) + 1
                               ); // alloc at least 1B
        assert( g -> surface_formats != NULL );
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                g -> device,
                g_vk_surface,
                &( g -> num_surface_formats ),
                g -> surface_formats
            )
        );
    }

    // get surface present modes
    {
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                g -> device,
                g_vk_surface,
                &( g -> num_surface_present_modes ),
                NULL
            )
        );
        g -> surface_present_modes = malloc(
                                         ( g -> num_surface_present_modes ) * sizeof( VkPresentModeKHR ) + 1
                                     ); // alloc at least 1B
        assert( g -> surface_present_modes != NULL );
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                g -> device,
                g_vk_surface,
                &( g -> num_surface_present_modes ),
                g -> surface_present_modes
            )
        );
    }

    vkGetPhysicalDeviceMemoryProperties(
        g -> device,
        &( g -> memory_properties )
    );

    vkGetPhysicalDeviceProperties(
        g -> device,
        &( g -> device_properties )
    );
}

void gpuPrintInfo( GPU* g )
{
    assert( g != NULL );
    printf( "Physical device: %s\n", g -> device_properties.deviceName );
    printf( "Queue families: %u\n", g -> num_queue_properties );
    printf( "Extension count: %u\n", g -> num_extensions );
    for ( u32 i = 0; i < g -> num_extensions; i++ ) {
        printf(
            "Spec:%u.%u.%u:Extension:%s\n",
            VK_VERSION_MAJOR( g -> extension_properties[i].specVersion ),
            VK_VERSION_MINOR( g -> extension_properties[i].specVersion ),
            VK_VERSION_PATCH( g -> extension_properties[i].specVersion ),
            g -> extension_properties[i].extensionName
        );
    }
    printf( "Memory types: %u\n", g -> memory_properties.memoryTypeCount );
    for ( u32 i = 0; i < g -> memory_properties.memoryTypeCount; i++ ) {
        printf(
            "Index:%u:Heap:%uFlags:",
            i,
            g -> memory_properties.memoryTypes[i].heapIndex
        );
        if (
            g -> memory_properties.memoryTypes[i].propertyFlags
            & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        ) {
            printf( "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT:" );
        } else {
            printf( ":" );
        }
        if (
            g -> memory_properties.memoryTypes[i].propertyFlags
            & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        ) {
            printf( "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT:" );
        } else {
            printf( ":" );
        }
        if (
            g -> memory_properties.memoryTypes[i].propertyFlags
            & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        ) {
            printf( "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:" );
        } else {
            printf( ":" );
        }
        if (
            g -> memory_properties.memoryTypes[i].propertyFlags
            & VK_MEMORY_PROPERTY_HOST_CACHED_BIT
        ) {
            printf( "VK_MEMORY_PROPERTY_HOST_CACHED_BIT:" );
        } else {
            printf( ":" );
        }
        if (
            g -> memory_properties.memoryTypes[i].propertyFlags
            & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
        ) {
            printf( "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT:" );
        } else {
            printf( ":" );
        }
        printf( "\n" );
    }
    printf( "Heaps: %u\n", g -> memory_properties.memoryHeapCount );
    for ( u32 i = 0; i < g -> memory_properties.memoryHeapCount; i++ ) {
        printf( "Heap %u:Size:%lu:Flags:", i,
                g -> memory_properties.memoryHeaps[i].size );
        if ( g -> memory_properties.memoryHeaps[i].flags&
                VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ) {
            printf( "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT:" );
        } else {
            printf( ":" );
        }
        if ( g -> memory_properties.memoryHeaps[i].flags&
                VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR ) {
            printf( "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR:" );
        } else {
            printf( ":" );
        }
        printf( "\n" );
    }
    printf(
        "Maximum number of allocations: %u\n",
        g -> device_properties.limits.maxMemoryAllocationCount
    );
}

void gpuDestroy( GPU* g )
{
    assert( g != NULL );
    if ( g -> queue_properties ) {
        free( g -> queue_properties );
    }
    if ( g -> extension_properties ) {
        free( g -> extension_properties );
    }
    if ( g -> surface_formats ) {
        free( g -> surface_formats );
    }
    if ( g -> surface_present_modes ) {
        free( g -> surface_present_modes );
    }

    memset( g, 0, sizeof( GPU ) );
}

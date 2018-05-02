/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

#include "vk_alloc.h"

extern const VkAllocation VK_ALLOCATION_NULL = {
    .mem = VK_NULL_HANDLE,
    .size = 0,
    .host_visible = NULL
};

extern GPU* g_gpu_used;
extern VkDevice g_vk_device;

// alloc method:
// go through device's memory types and find the one we want
// if don't care, only see if VK_ALLOCATION_HOST_VISIBLE flag is set and get host visible local memory
// request allocation

u32 findMemoryTypeIndex( u32 memory_type_bits, i8 is_host_visible )
{
    if ( is_host_visible ) {
        memory_type_bits |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    for ( u32 i = 0; i < g_gpu_used -> memory_properties.memoryTypeCount; i++ ) {
        VkMemoryType* memtype = &( g_gpu_used -> memory_properties.memoryTypes[i] );
        if ( memtype -> propertyFlags & memory_type_bits ) {
            // FOUND IT
            return i;
        }
    }
    return UINT32_MAX;
}

VkAllocation vkAlloc(
    const VkDeviceSize size,
    const u32 memory_type_bits,
    const i32 flags
)
{
    VkAllocation alloc = VK_ALLOCATION_NULL;
    const u32 memory_type_index =
        findMemoryTypeIndex( memory_type_bits, flags & VK_ALLOCATION_HOST_VISIBLE );
    if ( memory_type_index == UINT32_MAX ) {
        return alloc;
    }

    VkMemoryAllocateInfo memory_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = size,
        .memoryTypeIndex = memory_type_index
    };

    VK_PRINT_AND_ABORT_ON_FAIL(
        vkAllocateMemory(
            g_vk_device,
            &memory_alloc_info,
            NULL,
            &alloc.mem
        )
    );

    if ( alloc.mem == VK_NULL_HANDLE ) {
        return alloc;
    }

    alloc.size = size;

    if ( flags & VK_ALLOCATION_HOST_VISIBLE ) {
        VK_PRINT_AND_ABORT_ON_FAIL(
            vkMapMemory(
                g_vk_device,
                alloc.mem,
                0,
                alloc.size,
                0,
                ( void** ) &alloc.host_visible
            )
        );
    }

    return alloc;
}

void vkFree( VkAllocation allocated )
{
    if ( allocated.host_visible ) {
        vkUnmapMemory( g_vk_device, allocated.host_visible );
    }
    vkFreeMemory( g_vk_device, allocated.mem, NULL );
}

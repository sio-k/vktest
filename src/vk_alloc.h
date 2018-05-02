#pragma once
/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

#include "libs.h"
#include "device_info.h"

// VK Allocation/Deallocation infrastructure.

typedef struct VkAllocation
{
    VkDeviceMemory mem;
    VkDeviceSize size;
    byte* host_visible; // NULL for not host-visible, otherwise valid pointer to memory
} VkAllocation;

extern const VkAllocation VK_ALLOCATION_NULL; // defined in vk_alloc.h

enum VkAllocationFlags
{
    VK_ALLOCATION_HOST_VISIBLE = 1
};

VkAllocation vkAlloc(
    const VkDeviceSize size,
    const u32 memory_type_bits, // 0 for don't care
    const i32 flags // or'd together from VkAllocationFlags values
);
// FIXME: no tools for memory alignment (currently missing)
void vkFree( VkAllocation allocated );

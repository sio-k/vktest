#pragma once
/*
 * Copyright (C) 2017 Sio Kreuzer. All Rights Reserved.
 */

// Vulkan initialization/destruction routines. Deal with all VK related globals.

#include "libs.h"
#include "vk_platform_specific.h"
#include "device_info.h"

void initVulkan();
void destroyVulkan();

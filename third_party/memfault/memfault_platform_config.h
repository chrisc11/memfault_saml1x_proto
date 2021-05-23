#pragma once

//! @file
//!
//! Copyright (c) Memfault, Inc.
//! See License.txt for details
//!
//! Platform overrides for the default configuration settings in the memfault-firmware-sdk.
//! Default configuration settings can be found in "memfault/config.h"

#ifdef __cplusplus
extern "C" {
#endif

// For example, decide if you want to use the Gnu Build ID.
#define MEMFAULT_USE_GNU_BUILD_ID 1

#define MEMFAULT_PLATFORM_HAS_LOG_CONFIG 1
#define MEMFAULT_PLATFORM_COREDUMP_STORAGE_RAM_SIZE 800

// Cortex-M23 uses instruction set similar to Cortex-M0
#define MEMFAULT_USE_ARMV6M_FAULT_HANDLER 1

// Code space optimizations
#define MEMFAULT_SDK_LOG_SAVE_DISABLE 1
#define MEMFAULT_DATA_SOURCE_RLE_ENABLED 0
#define MEMFAULT_LOG_DATA_SOURCE_ENABLED 0
#define MEMFAULT_EVENT_STORAGE_READ_BATCHING_ENABLED 0
#define MEMFAULT_TRACE_EVENT_WITH_LOG_FROM_ISR_ENABLED 0
#define MEMFAULT_EVENT_STORAGE_NV_SUPPORT_ENABLED 0
#define MEMFAULT_COLLECT_INTERRUPT_STATE 0
#define MEMFAULT_COLLECT_FAULT_REGS 0
#define MEMFAULT_CRC16_LOOKUP_TABLE_ENABLE 0

#ifdef __cplusplus
}
#endif

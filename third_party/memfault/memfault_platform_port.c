//! @file
//!
//! Copyright (c) Memfault, Inc.
//! See License.txt for details
//!
//! Glue layer between the Memfault SDK and the underlying platform

#include "memfault/components.h"
#include "memfault/ports/reboot_reason.h"

#include "sam.h"

#include <stdbool.h>

void memfault_platform_get_device_info(sMemfaultDeviceInfo *info) {
  // IMPORTANT: All strings returned in info must be constant
  // or static as they will be used _after_ the function returns

  // See https://mflt.io/version-nomenclature for more context
  *info = (sMemfaultDeviceInfo) {
    .device_serial = "",
     // A name to represent the firmware running on the MCU.
    // (i.e "ble-fw", "main-fw", or a codename for your project)
    .software_type = "app-fw",
    // The version of the "software_type" currently running.
    // "software_type" + "software_version" must uniquely represent
    // a single binary
    .software_version = "1.0.0",// memfault_create_unique_version_string("1.0.0"),
    // The revision of hardware for the device. This value must remain
    // the same for a unique device.
    // (i.e evt, dvt, pvt, or rev1, rev2, etc)
    // Regular expression defining valid hardware versions: ^[-a-zA-Z0-9_\.\+]+$
    .hardware_version = "rev1",
  };
}

MEMFAULT_PUT_IN_SECTION(".noinit.mflt_reboot_tracking")
static uint8_t s_reboot_tracking[MEMFAULT_REBOOT_TRACKING_REGION_SIZE];

void memfault_reboot_reason_get(sResetBootupInfo *info) {
  const uint32_t reset_cause = (uint32_t)REG_RSTC_RCAUSE;
  eMemfaultRebootReason reset_reason = kMfltRebootReason_Unknown;

  if (reset_cause & RSTC_RCAUSE_POR_Msk) {
    reset_reason = kMfltRebootReason_PowerOnReset;
  } else if (reset_cause & RSTC_RCAUSE_BODCORE_Msk) {
    reset_reason = kMfltRebootReason_BrownOutReset;
  } else if (reset_cause & RSTC_RCAUSE_BODVDD_Msk) {
    reset_reason = kMfltRebootReason_BrownOutReset;
  } else if (reset_cause & RSTC_RCAUSE_EXT_Msk) {
    reset_reason = kMfltRebootReason_PinReset;
  } else if (reset_cause & RSTC_RCAUSE_WDT_Msk) {
    reset_reason = kMfltRebootReason_HardwareWatchdog;
  } else if (reset_cause & RSTC_RCAUSE_SYST_Msk) {
    reset_reason = kMfltRebootReason_SoftwareReset;
  }

  *info = (sResetBootupInfo) {
    .reset_reason_reg = reset_cause,
    .reset_reason = reset_reason,
  };
}

void memfault_platform_reboot_tracking_boot(void) {
  sResetBootupInfo reset_info = { 0 };
  memfault_reboot_reason_get(&reset_info);
  memfault_reboot_tracking_boot(s_reboot_tracking, &reset_info);
}

//! Last function called after a coredump is saved. Should perform
//! any final cleanup and then reset the device
void memfault_platform_reboot(void) {
  // !FIXME: Perform any final system cleanup here

  NVIC_SystemReset();
  MEMFAULT_UNREACHABLE;
}

bool memfault_platform_time_get_current(sMemfaultCurrentTime *time) {
  return false;
}

size_t memfault_platform_sanitize_address_range(void *start_addr, size_t desired_size) {
  const uint32_t lower_addr = HSRAM_ADDR;
  const uint32_t upper_addr = lower_addr + HSRAM_SIZE;
  if ((uint32_t)start_addr >= lower_addr && ((uint32_t)start_addr < upper_addr)) {
    return MEMFAULT_MIN(desired_size, upper_addr - (uint32_t)start_addr);
  }

  return 0;
}

//! Note: This function _must_ be called by your main() routine prior
//! to starting an RTOS or baremetal loop.
int memfault_platform_boot(void) {
  memfault_platform_reboot_tracking_boot();

  static uint8_t s_event_storage[128];
  const sMemfaultEventStorageImpl *evt_storage =
      memfault_events_storage_boot(s_event_storage, sizeof(s_event_storage));
  memfault_trace_event_boot(evt_storage);

  memfault_reboot_tracking_collect_reset_info(evt_storage);


  MEMFAULT_LOG_INFO("Memfault Initialized!");
  return 0;
}

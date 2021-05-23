/**
 * \file
 *
 * \brief Application implement
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "atmel_start.h"
#include <hal_gpio.h>
#include <hal_delay.h>

#include "memfault/components.h"

void test_assert(void) {
  MEMFAULT_ASSERT(0);
}

void test_fault(void) {
  void (*bad_func)(void) = (void *)0xEEEEDEAD;
  bad_func();
}

void test_reboot(void) {
  memfault_reboot_tracking_mark_reset_imminent(kMfltRebootReason_UserReset, NULL);
  memfault_platform_reboot();
}

// A mechanism for forcing different test paths without having to pull in a full CLI!
// From (gdb), i.e
//  (gdb) set g_test_cmd=1
int g_test_cmd = 0;

static void prv_run_test_command(void) {
  switch (g_test_cmd) {
    case 1:
      test_assert();
      break;
    case 2:
      test_fault();
      break;
    case 3:
      test_reboot();
      break;
  }
}

//
// Example draining data.
// Data can be sent with GDB:
//   https://mflt.io/posting-chunks-with-gdb
//

MEMFAULT_NO_OPT
MEMFAULT_WEAK
void user_transport_send_chunk_data(MEMFAULT_UNUSED void *chunk_data,
                                    MEMFAULT_UNUSED size_t chunk_data_len) {
}

static bool prv_try_send_memfault_data(void) {
  uint8_t buf[128];
  size_t buf_len = sizeof(buf);
  bool data_available = memfault_packetizer_get_chunk(buf, &buf_len);
  if (!data_available ) {
    return false; // no more data to send
  }
  user_transport_send_chunk_data(buf, buf_len);
  return true;
}

int main(void) {
  atmel_start_init();
  memfault_platform_boot();

  while (true) {
    delay_ms(100);
    gpio_toggle_pin_level(LED0);

    // drain any memfault data that has been collected
    while (prv_try_send_memfault_data()) {
    }

    prv_run_test_command();
  }
}

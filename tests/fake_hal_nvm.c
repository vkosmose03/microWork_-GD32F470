/**
 * @file  fake_hal_nvm.c
 * @brief RAM-backed implementation of the hal_nvm interface for host tests.
 *
 * Mimics a small flash sector: erase sets all bytes to 0xFF, writes copy bytes
 * verbatim. This lets the configuration module's load/save/CRC logic be tested
 * without any hardware.
 */
#include "hal/hal_nvm.h"

#include <string.h>

#define FAKE_NVM_SIZE 256u

static unsigned char g_nvm[FAKE_NVM_SIZE];

size_t hal_nvm_size(void) {
    return FAKE_NVM_SIZE;
}

int hal_nvm_read(uint32_t offset, void *dst, size_t len) {
    if (dst == NULL || offset + len > FAKE_NVM_SIZE) {
        return -1;
    }
    memcpy(dst, &g_nvm[offset], len);
    return 0;
}

int hal_nvm_write(uint32_t offset, const void *src, size_t len) {
    if (src == NULL || offset + len > FAKE_NVM_SIZE) {
        return -1;
    }
    memcpy(&g_nvm[offset], src, len);
    return 0;
}

int hal_nvm_erase(void) {
    memset(g_nvm, 0xFF, sizeof(g_nvm));
    return 0;
}

/* Test-only helper to corrupt storage; declared in the test file. */
void fake_nvm_corrupt(uint32_t offset);
void fake_nvm_corrupt(uint32_t offset) {
    if (offset < FAKE_NVM_SIZE) {
        g_nvm[offset] ^= 0xFFu;
    }
}

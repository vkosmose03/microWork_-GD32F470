/**
 * @file  hal_nvm_gd32.c
 * @brief hal_nvm implementation as a flash-emulated EEPROM.
 *
 * The GD32F470 has no dedicated EEPROM, so configuration is stored in the last
 * 128 KB flash sector (sector 11, base 0x080E0000). Reads are direct memory
 * accesses; writes program 32-bit words after a sector erase. This is enough
 * for the small, infrequently-changed configuration record.
 */
#include "hal/hal_nvm.h"

#include "gd32f4xx.h"

#define NVM_BASE   0x080E0000u
#define NVM_SECTOR CTL_SECTOR_NUMBER_11
#define NVM_SIZE   0x1000u /* expose 4 KB; the sector is larger */

size_t hal_nvm_size(void) {
    return NVM_SIZE;
}

int hal_nvm_read(uint32_t offset, void *dst, size_t len) {
    if (dst == NULL || (offset + len) > NVM_SIZE) {
        return -1;
    }
    const volatile uint8_t *src = (const volatile uint8_t *) (NVM_BASE + offset);
    uint8_t *d = (uint8_t *) dst;
    for (size_t i = 0u; i < len; ++i) {
        d[i] = src[i];
    }
    return 0;
}

int hal_nvm_erase(void) {
    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR |
                   FMC_FLAG_PGSERR);
    const fmc_state_enum st = fmc_sector_erase(NVM_SECTOR);
    fmc_lock();
    return (st == FMC_READY) ? 0 : -1;
}

int hal_nvm_write(uint32_t offset, const void *src, size_t len) {
    if (src == NULL || (offset + len) > NVM_SIZE || (offset & 0x3u) != 0u) {
        return -1;
    }

    const uint8_t *s = (const uint8_t *) src;
    int rc = 0;
    size_t i = 0u;

    fmc_unlock();
    for (; (i + 4u) <= len; i += 4u) {
        const uint32_t w = (uint32_t) s[i] | ((uint32_t) s[i + 1u] << 8) |
                           ((uint32_t) s[i + 2u] << 16) | ((uint32_t) s[i + 3u] << 24);
        if (fmc_word_program(NVM_BASE + offset + i, w) != FMC_READY) {
            rc = -1;
            break;
        }
    }
    if (rc == 0 && i < len) {
        /* Program the trailing 1..3 bytes, padded with erased (0xFF) bits. */
        uint32_t w = 0xFFFFFFFFu;
        for (size_t b = 0u; (i + b) < len; ++b) {
            w &= ~((uint32_t) 0xFFu << (8u * b));
            w |= (uint32_t) s[i + b] << (8u * b);
        }
        if (fmc_word_program(NVM_BASE + offset + i, w) != FMC_READY) {
            rc = -1;
        }
    }
    fmc_lock();
    return rc;
}

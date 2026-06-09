/**
 * @file  hal_nvm.h
 * @brief Abstract non-volatile storage interface.
 *
 * Backs the configuration module (req* #3: configurable parameters kept in
 * non-volatile memory). On the GD32F470 this is implemented as a flash-emulated
 * EEPROM in a dedicated sector; on the host it is backed by a RAM stub so the
 * config module can be unit tested.
 */
#ifndef HAL_NVM_H
#define HAL_NVM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @return Usable size of the NVM region in bytes. */
size_t hal_nvm_size(void);

/** Read @p len bytes from @p offset into @p dst. @return 0 on success. */
int hal_nvm_read(uint32_t offset, void *dst, size_t len);

/** Program @p len bytes from @p src at @p offset. @return 0 on success. */
int hal_nvm_write(uint32_t offset, const void *src, size_t len);

/** Erase the whole NVM region (back to 0xFF). @return 0 on success. */
int hal_nvm_erase(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_NVM_H */

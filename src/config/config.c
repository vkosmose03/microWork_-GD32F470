#include "config/config.h"

#include "hal/hal_nvm.h"

#include <string.h>

#define CONFIG_MAGIC   0x47464331u /* "1CFG" little-endian */
#define CONFIG_VERSION 1u

/* On-NVM layout. Kept POD and packed-by-design (only 32/16-bit fields and
 * floats) so it serialises identically on host and target. */
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    config_params_t params;
    uint32_t crc32;
} config_record_t;

/* ---- CRC32 (IEEE 802.3, reflected, poly 0xEDB88320) ---------------------- */
static uint32_t crc32_update(uint32_t crc, const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *) data;
    crc = ~crc;
    for (size_t i = 0u; i < len; ++i) {
        crc ^= p[i];
        for (unsigned b = 0u; b < 8u; ++b) {
            const uint32_t mask = (uint32_t) (-(int32_t) (crc & 1u));
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

static uint32_t config_record_crc(const config_record_t *rec) {
    /* CRC covers everything except the trailing crc32 field itself. */
    const size_t covered = sizeof(*rec) - sizeof(rec->crc32);
    return crc32_update(0u, rec, covered);
}

void config_set_defaults(config_params_t *p) {
    if (p == NULL) {
        return;
    }
    p->sample_rate_hz = CONFIG_MAX_SAMPLE_RATE_HZ; /* 19200 Hz */
    p->freq_periods = CONFIG_MAX_FREQ_PERIODS;     /* 3 */
    p->dfdt_periods = CONFIG_MAX_DFDT_PERIODS;     /* 6 */
    p->uart_baud = 115200u;
    /* Default calibration: 12-bit ADC over a 3.3 V reference, centred at mid-scale.
     * adc_scale = 3.3 / 4095; adc_offset = mid-scale voltage (1.65 V). */
    p->adc_scale = 3.3f / 4095.0f;
    p->adc_offset = 1.65f;
}

int config_validate(config_params_t *p) {
    if (p == NULL) {
        return -1;
    }
    int changed = 0;

    if (p->sample_rate_hz == 0u || p->sample_rate_hz > CONFIG_MAX_SAMPLE_RATE_HZ) {
        p->sample_rate_hz = CONFIG_MAX_SAMPLE_RATE_HZ;
        changed = 1;
    }
    if (p->freq_periods < 1u || p->freq_periods > CONFIG_MAX_FREQ_PERIODS) {
        p->freq_periods = CONFIG_MAX_FREQ_PERIODS;
        changed = 1;
    }
    if (p->dfdt_periods < CONFIG_MIN_DFDT_PERIODS || p->dfdt_periods > CONFIG_MAX_DFDT_PERIODS) {
        p->dfdt_periods = CONFIG_MAX_DFDT_PERIODS;
        changed = 1;
    }
    if (p->uart_baud == 0u) {
        p->uart_baud = 115200u;
        changed = 1;
    }
    if (!(p->adc_scale > 0.0f)) { /* also catches NaN */
        p->adc_scale = 3.3f / 4095.0f;
        changed = 1;
    }
    return changed;
}

int config_load(config_params_t *p) {
    if (p == NULL) {
        return -1;
    }

    config_record_t rec;
    if (hal_nvm_read(0u, &rec, sizeof(rec)) != 0) {
        config_set_defaults(p);
        return -1;
    }

    if (rec.magic != CONFIG_MAGIC || rec.version != CONFIG_VERSION ||
        rec.size != (uint16_t) sizeof(config_params_t) || rec.crc32 != config_record_crc(&rec)) {
        config_set_defaults(p);
        return -1;
    }

    *p = rec.params;
    /* Defensive: clamp anything that slipped through a version mismatch. */
    if (config_validate(p) != 0) {
        return -1;
    }
    return 0;
}

int config_save(const config_params_t *p) {
    if (p == NULL) {
        return -1;
    }

    config_record_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.magic = CONFIG_MAGIC;
    rec.version = CONFIG_VERSION;
    rec.size = (uint16_t) sizeof(config_params_t);
    rec.params = *p;
    (void) config_validate(&rec.params);
    rec.crc32 = config_record_crc(&rec);

    if (hal_nvm_erase() != 0) {
        return -1;
    }
    if (hal_nvm_write(0u, &rec, sizeof(rec)) != 0) {
        return -1;
    }
    return 0;
}

#ifndef ARCADE_A2_H_
#define ARCADE_A2_H_

/**
 * AUTOGENERATED BY gen_unions.py
*/
#include <stdint.h>

#define ARCADE_A2_ID 0x0035






typedef union {
    uint8_t bytes[8];
    uint64_t raw;

    // Sets frontal event 5
    void set_CRASH_C(bool value){ raw = (raw & 0xfbffffffffffffff) | ((uint64_t)value & 0x1) << 58; }
    // Gets frontal event 5
    bool get_CRASH_C() { return raw >> 58 & 0x1; }

    // Sets frontal event 2
    void set_CRASH_F(bool value){ raw = (raw & 0xdfffffffffffffff) | ((uint64_t)value & 0x1) << 61; }
    // Gets frontal event 2
    bool get_CRASH_F() { return raw >> 61 & 0x1; }

    // Sets Confirm bit for all crash events, toggling
    void set_CONF_CRASH(bool value){ raw = (raw & 0x7fffffffffffffff) | ((uint64_t)value & 0x1) << 63; }
    // Gets Confirm bit for all crash events, toggling
    bool get_CONF_CRASH() { return raw >> 63 & 0x1; }

    /** Imports the frame data from a source */
    void import_frame(uint32_t cid, uint8_t* data, uint8_t len) {
        if (cid == ARCADE_A2_ID) {
            for (int i = 0; i < len; i++) {
                bytes[7-i] = data[i];
            }
        }
    }

    /** Exports the frame data to a destination */
    void export_frame(uint32_t* cid, uint8_t* data, uint8_t* len) {
        *cid = ARCADE_A2_ID;
        *len = 8;
        for (int i = 0; i < *len; i++) {
            data[i] = bytes[7-i];
        }
    }
} ARCADE_A2;

#endif
const UINT8 great_burst_lvl1[] = {
    0x33, 0x22, 0x11, 0x22, 0x10 |
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x70 |
    0x00, 0x11, 0x10, 0x01, 0x00,
    0x01, 0x10, 0x01, 0x00, 0x00 |
    0x00, 0x11, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00 |
    0x03, 0x33, 0x33, 0x33, 0x33,
    0x22, 0x22, 0x22, 0x22, 0x20 |
    0x01, 0x11, 0x11, 0x11, 0x11
};

const UINT8 great_burst_lvl2[] = {
    0x71, 0x41, 0x33, 0x41, 0x70 |
    0x01, 0x14, 0x31, 0x14, 0x11,
    0x11, 0x43, 0x33, 0x41, 0x10 |
    0x01, 0x34, 0x35, 0x34, 0x11,
    0x31, 0x31, 0x33, 0x43, 0x10 |
    0x03, 0x53, 0x11, 0x13, 0x53,
    0x31, 0x31, 0x11, 0x33, 0x30 |
    0x01, 0x31, 0x44, 0x43, 0x13,
    0x00, 0x00, 0x00, 0x00, 0x00 |
    0x00, 0x00, 0x00, 0x00, 0x00
};

const UINT8 *great_burst_level[2] = {&great_burst_lvl1, &great_burst_lvl2};
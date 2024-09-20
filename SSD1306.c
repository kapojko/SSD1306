#include <string.h>
#include "SSD1306.h"

// Command/data control byte
#define SSD1306_CMD  0x80
#define SSD1306_DATA 0x40

// Commands
#define SSD1306_CMD_CONTRAST_SET 0x81
#define SSD1306_CMD_ENTIRE_DISPLAY_ON 0xA4 // 0xA4/0xA5
#define SSD1306_CMD_NORMAL_INVERSE_DISPLAY 0xA6 // 0xA6/0xA7
#define SSD1306_CMD_DISPLAY_ON 0xAE // 0xAE/0xAF

// TODO: scroll commands

#define SSD1306_CMD_SET_LOWER_COLUMN 0x00 // 0x00-0x0F
#define SSD1306_CMD_SET_HIGHER_COLUMN 0x10 // 0x10-0x1F
#define SSD1306_CMD_SET_MEM_MODE 0x20
#define SSD1306_CMD_SET_COLUMN_ADDR 0x21
#define SSD1306_CMD_SET_PAGE_ADDR 0x22
#define SSD1306_CMD_SET_PAGE_START 0xB0 // 0xB0-0xB7

#define SSD1306_CMD_SET_DISPLAY_START_LINE 0x40 // 0x40-0x7F
#define SSD1306_CMD_SET_DISPLAY_REMAP 0xA0 // 0xA0/0xA1
#define SSD1306_CMD_SET_MUX_RATION 0xA8
#define SSD1306_CMD_SET_COM_SCAN_INVERT 0xC0 // 0xC0/0xC8
#define SSD1306_CMD_SET_DISPLAY_OFFSET 0xD3
#define SSD1306_CMD_SET_COM_PINS 0xDA

#define SSD1306_CMD_SET_CLOCK_FREQ 0xD5
#define SSD1306_CMD_SET_PRECHARGE_PERIOD 0xD9
#define SSD1306_CMD_SET_VCOMH_LEVEL 0xDB
#define SSD1306_CMD_NOP 0xE3

// TODO: advanced graphics

#define SSD1306_CMD_CHARGE_PUMP_SET 0x8D

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define CHECK_ROWS(y8x, height8x) (y8x >= 0 && height8x > 0 && y8x + height8x <= SSD1306_PAGE_COUNT * SSD1306_BITS_PER_PAGE && y8x % 8 == 0 && height8x % 8 == 0)
#define CHECK_COLUMNS(x, width) (x >= 0 && width > 0 && x + width <= SSD1306_COL_COUNT)
#define DATA_SIZE(width, height8x) (height8x / SSD1306_BITS_PER_PAGE * width)

static struct SSD1306_Platform platform;

// NOTE: if array is partly initialized, all non-mentioned elements will be set to 0
static uint8_t buffer[SSD1306_BUFF_SIZE + 1] = { SSD1306_DATA, 0 };

static void writeToLcd(uint8_t data[], int len) {
    int err;

    err = platform.i2cWrite(platform.i2cAddr, data, len, 1, 1);
    if (err) {
        platform.debugPrint("I2C write error: %d\r\n", -err);
    }
}

static void writeDataFromBuffer(int pageStart, int pageEnd, int colStart, int colEnd) {
    uint8_t *bufPtr = buffer + 1;

    // Write data in chunks not exceeding SSD1306_I2C_BUFFER_SIZE (DATA byte prefix + data portion)
    int chunkSize = SSD1306_I2C_BUFFER_SIZE - 1;

    for (int page = pageStart; page <= pageEnd; page++) {
        // Write page address
        SSD1306_PageStartAddressSet(page);

        // Write data for columns
        for (int col = colStart; col <= colEnd; ) {
            // Write column address
            SSD1306_ColumnStartAddressSet(col);

            // Write chunk of data
            int chunk = MIN(chunkSize, colEnd - col + 1);
            *(bufPtr - 1) = SSD1306_DATA;
            writeToLcd(bufPtr - 1, chunk + 1);

            // Increase column
            bufPtr += chunk;
            col += chunk;
        }
    }
}

void SSD1306_Init(struct SSD1306_Platform *platformPtr) {
    platform = *platformPtr;
}

void SSD1306_DefInit(bool xyFlip) {
    // TODO: check
    SSD1306_ChargePumpSet(SSD1306_CHARGE_PUMP_ENABLE);
    SSD1306_DisplayOn(false);
    SSD1306_MemoryModeSet(SSD1306_MEM_MODE_PAGE);
    SSD1306_DisplayStartLineSet(0);
    SSD1306_ComScanInvert(xyFlip);
    SSD1306_SegmentRemap(xyFlip);
    SSD1306_DisplayInvert(false);
    SSD1306_DisplayOn(true);
    SSD1306_EntireDisplayOn(false);

    platform.delayMs(100);

    SSD1306_ClearScreen();
}

void SSD1306_DefDeinit(void) {
    // TODO: check
    SSD1306_DisplayOn(false);
    SSD1306_ChargePumpSet(SSD1306_CHARGE_PUMP_DISABLE);

    platform.delayMs(100);
}

void SSD1306_ContrastSet(uint8_t contrast) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_CONTRAST_SET,
            contrast
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_EntireDisplayOn(bool enable) {
    uint8_t cmd[] = {
        SSD1306_CMD,
        SSD1306_CMD_ENTIRE_DISPLAY_ON | (uint8_t)enable
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_SetNormalInverse(bool normal) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_NORMAL_INVERSE_DISPLAY | ((uint8_t)!normal)
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_DisplayOn(bool enable) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_DISPLAY_ON | (uint8_t)enable
    };
    writeToLcd(cmd, sizeof(cmd));
}

// TODO: scroll commands

void SSD1306_ColumnStartAddressSet(uint8_t addr) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_SET_LOWER_COLUMN | (addr & 0xF),
    };
    writeToLcd(cmd, sizeof(cmd));

    cmd[1] = SSD1306_CMD_SET_HIGHER_COLUMN | ((addr >> 4) & 0xF);
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_MemoryModeSet(SSD1306_MemMode mode) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_SET_MEM_MODE,
            (uint8_t)mode
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_ColumnAddressSet(uint8_t begin, uint8_t end) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_SET_COLUMN_ADDR,
            begin & 0x7F,
            end & 0x7F
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_PageAddressSet(uint8_t begin, uint8_t end) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_SET_PAGE_ADDR,
            begin & 0x7,
            end & 0x7
    };
    writeToLcd(cmd, sizeof(cmd));
}
void SSD1306_PageStartAddressSet(uint8_t addr) {
    uint8_t cmd[] = {
        SSD1306_CMD,
        SSD1306_CMD_SET_PAGE_START | (addr & 0x7)
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_DisplayStartLineSet(uint8_t line) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_SET_DISPLAY_START_LINE | (line & 0x3F)
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_SegmentRemap(bool remap) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_SET_DISPLAY_REMAP | (uint8_t)remap
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_MuxRatioSet(uint8_t mux) {
    uint8_t cmd[] = {
        SSD1306_CMD,
        SSD1306_CMD_SET_MUX_RATION,
        mux
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_ComScanInvert(bool invert) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_SET_COM_SCAN_INVERT | ((uint8_t)invert << 3)
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_DisplayOffsetSet(uint8_t offset) {
    uint8_t cmd[] = {
        SSD1306_CMD,
        SSD1306_CMD_SET_DISPLAY_OFFSET,
        offset
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_ComPinsSet(bool alternative, bool left_right_remap) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_SET_COM_PINS,
            0x2 | ((uint8_t)alternative << 4) | ((uint8_t)left_right_remap << 5)
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_ClockFreqSet(uint8_t freq, uint8_t divider) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_SET_CLOCK_FREQ,
            (divider & 0xF) | ((freq & 0xF) << 4)
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_PrechargePeriodSet(uint8_t period) {
    uint8_t cmd[] = {
        SSD1306_CMD,
        SSD1306_CMD_SET_PRECHARGE_PERIOD,
        period
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_VcomhLevelSet(SSD1306_VcomLevel level) {
    uint8_t cmd[] = {
        SSD1306_CMD,
        SSD1306_CMD_SET_VCOMH_LEVEL,
        (uint8_t)level
    };
    writeToLcd(cmd, sizeof(cmd));
}

void SSD1306_Nop(void) {
    uint8_t cmd[] = {
        SSD1306_CMD,
        SSD1306_CMD_NOP,
    };
    writeToLcd(cmd, sizeof(cmd));
}

// TODO: advanced graphics

void SSD1306_ChargePumpSet(SSD1306_ChargePumpSetting setting) {
    uint8_t cmd[] = {
            SSD1306_CMD,
            SSD1306_CMD_CHARGE_PUMP_SET,
            (uint8_t)setting,
    };
    writeToLcd(cmd, sizeof(cmd));
}

int SSD1306_ClearScreen(void) {
    // write zero data to LCD
    memset(buffer + 1, 0, sizeof(buffer) - 1);
    writeDataFromBuffer(0, SSD1306_PAGE_COUNT - 1, 0, SSD1306_COL_COUNT - 1);
    return 0;
}

bool SSD1306_OutputPreparedBitmap(int x, int y8x, int width, int height8x, const uint8_t *data, int dataSize) {
    // check rows
    if (!CHECK_ROWS(y8x, height8x)) {
        platform.debugPrint("Invalid y8x or height8x: %d, %d\r\n", y8x, height8x);
        return false;
    }

    // check columns
    if (!CHECK_COLUMNS(x, width)) {
        platform.debugPrint("Invalid x or width: %d, %d\r\n", x, width);
        return false;
    }

    // check length
    int calcDataSize = DATA_SIZE(width, height8x);
    if (calcDataSize != dataSize) {
        platform.debugPrint("Invalid dataSize: %d, expected %d\r\n", dataSize, calcDataSize);
        return false;
    }

    // copy data to buffer
    if (dataSize > sizeof(buffer) - 1) {
        // Unexpected error
        platform.debugPrint("Unexpected dataSize: %d\r\n", dataSize);
        return false;
    }

    memcpy(buffer + 1, data, dataSize);

    // write data to LCD
    writeDataFromBuffer(y8x / 8, (y8x + height8x) / 8 - 1, x, x + width - 1);

    return true;
}

bool SSD1306_FillArea(int x, int y8x, int width, int height8x, int data) {
    // check rows
    if (!CHECK_ROWS(y8x, height8x)) {
        platform.debugPrint("Invalid y8x or height8x: %d, %d\r\n", y8x, height8x);
        return false;
    }

    // check columns
    if (!CHECK_COLUMNS(x, width)) {
        platform.debugPrint("Invalid x or width: %d, %d\r\n", x, width);
        return false;
    }

    // fill buffer
    int dataSize = DATA_SIZE(width, height8x);
    memset(buffer + 1, data, dataSize);

    // write data to LCD
    writeDataFromBuffer(y8x / 8, (y8x + height8x) / 8 - 1, x, x + width - 1);

    return true;
}

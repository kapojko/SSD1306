#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

// I2C address
#define SSD1306_I2C_ADDR_DEF 0x3C
#define SSD1306_I2C_ADDR_ALTERNATE 0x3D // depends on D/C# pin

#define SSD1306_I2C_BUFFER_SIZE 32

// Display size
// NOTE: actual display size may be smaller, ex. 128x32
#define SSD1306_BITS_PER_PAGE 8
#define SSD1306_PAGE_COUNT  8
#define SSD1306_COL_COUNT 128
#define SSD1306_ROW_COUNT (SSD1306_PAGE_COUNT * SSD1306_BITS_PER_PAGE)

#define SSD1306_BUFF_SIZE (SSD1306_COL_COUNT * SSD1306_PAGE_COUNT)

typedef enum {
    SSD1306_MEM_MODE_HORIZ = 0x00,
    SSD1306_MEM_MODE_VERT = 0x01,
    SSD1306_MEM_MODE_PAGE = 0x02, // default
} SSD1306_MemMode;

typedef enum {
    SSD1306_VCOM_LEVEL065 = 0x00,
    SSD1306_VCOM_LEVEL077 = 0x20, // default
    SSD1306_VCOM_LEVEL083 = 0x30,
} SSD1306_VcomLevel;

typedef enum {
    SSD1306_CHARGE_PUMP_DISABLE = 0x10, // default
    SSD1306_CHARGE_PUMP_ENABLE = 0x14,
} SSD1306_ChargePumpSetting;

struct SSD1306_Platform {
    int (*i2cWrite)(uint8_t addr_7bit, const uint8_t *data, uint8_t length, uint8_t wait, uint8_t send_stop);

    void (*delayMs)(int ms);
    void (*debugPrint)(const char *fmt, ...);
    
    uint8_t i2cAddr;
};

void SSD1306_Init(struct SSD1306_Platform *platform);

void SSD1306_DefInit(bool xyFlip);
void SSD1306_DefDeinit(void);

void SSD1306_ContrastSet(uint8_t contrast);
void SSD1306_EntireDisplayOn(bool enable);
void SSD1306_SetNormalInverse(bool normal);
void SSD1306_DisplayOn(bool enable);

// TODO: scroll commands

void SSD1306_ColumnStartAddressSet(uint8_t addr);
void SSD1306_MemoryModeSet(SSD1306_MemMode mode);
void SSD1306_ColumnAddressSet(uint8_t begin, uint8_t end);
void SSD1306_PageAddressSet(uint8_t begin, uint8_t end);
void SSD1306_PageStartAddressSet(uint8_t addr);

void SSD1306_DisplayStartLineSet(uint8_t line);
void SSD1306_SegmentRemap(bool remap);
void SSD1306_MuxRatioSet(uint8_t mux);
void SSD1306_ComScanInvert(bool invert);
void SSD1306_DisplayOffsetSet(uint8_t offset);
void SSD1306_ComPinsSet(bool alternative, bool left_right_remap);

void SSD1306_ClockFreqSet(uint8_t freq, uint8_t divider);
void SSD1306_PrechargePeriodSet(uint8_t period);
void SSD1306_VcomhLevelSet(SSD1306_VcomLevel level);
void SSD1306_Nop(void);

// TODO: advanced graphics

void SSD1306_ChargePumpSet(SSD1306_ChargePumpSetting setting);

int SSD1306_ClearScreen(void);

/** Output prepared bitmap to LCD.
 * @param x start column
 * @param y8x start row (must be multiple of 8)
 * @param width width
 * @param height8x height (must be multiple of 8)
 * @param data data, encoding order: each byte denotes 8 pixels in a column (LSB on the top, MSB on the bottom),
 *     bytes go from left to right and byte sequences go from top to bottom
 * @param dataSize data length
 * @return success status
*/
bool SSD1306_OutputPreparedBitmap(int x, int y8x, int width, int height8x, const uint8_t *data, int dataSize);

bool SSD1306_FillArea(int x, int y8x, int width, int height8x, int data);


#endif // SSD1306_H

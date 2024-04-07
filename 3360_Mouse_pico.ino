#include <SPI.h>

#include "srom_3360_0x05.h"

#include <mbed.h>

REDIRECT_STDOUT_TO(Serial);

#include "PluggableUSBHID.h"

#include "relmouse_16.h"
USBMouse16 mouse(false);

#include <Ethernet.h>

#define REG_PRODUCT_ID (0x00)
#define REG_REVISION_ID (0x01)
#define REG_MOTION (0x02)
#define REG_DELTA_X_L (0x03)
#define REG_DELTA_X_H (0x04)
#define REG_DELTA_Y_L (0x05)
#define REG_DELTA_Y_H (0x06)
#define REG_SQUAL (0x07)
#define REG_RAW_DATA_SUM (0x08)
#define REG_MAX_RAW_DATA (0x09)
#define REG_MIN_RAW_DATA (0x0A)
#define REG_SHUTTER_LOWER (0x0B)
#define REG_SHUTTER_UPPER (0x0C)
#define REG_CONTROL (0x0D)

#define REG_CONFIG1 (0x0F)
#define REG_CONFIG2 (0x10)
#define REG_ANGLE_TUNE (0x11)
#define REG_FRAME_CAPTURE (0x12)
#define REG_SROM_ENABLE (0x13)
#define REG_RUN_DOWNSHIFT (0x14)
#define REG_REST1_RATE_LOWER (0x15)
#define REG_REST1_RATE_UPPER (0x16)
#define REG_REST1_DOWNSHIFT (0x17)
#define REG_REST2_RATE_LOWER (0x18)
#define REG_REST2_RATE_UPPER (0x19)
#define REG_REST2_DOWNSHIFT (0x1A)
#define REG_REST3_RATE_LOWER (0x1B)
#define REG_REST3_RATE_UPPER (0x1C)

#define REG_OBSERVATION (0x24)
#define REG_DATA_OUT_LOWER (0x25)
#define REG_DATA_OUT_UPPER (0x26)

#define REG_RAW_DATA_DUMP (0x29)
#define REG_SROM_ID (0x2A)
#define REG_MIN_SQ_RUN (0x2B)
#define REG_RAW_DATA_THRESHOLD (0x2C)

#define REG_CONFIG5 (0x2F)

#define REG_POWER_UP_RESET (0x3A)
#define REG_SHUTDOWN (0x3B)

#define REG_INVERSE_PRODUCT_ID (0x3F)

#define REG_LIFTCUTOFF_TUNE3 (0x41)
#define REG_ANGLE_SNAP (0x42)

#define REG_LIFTCUTOFF_TUNE1 (0x4A)

#define REG_MOTION_BURST (0x50)

#define REG_LIFTCUTOFF_TUNE_TIMEOUT (0x58)

#define REG_LIFTCUTOFF_TUNE_MIN_LENGTH (0x5A)

#define REG_SROM_LOAD_BURST (0x62)
#define REG_LIFT_CONFIG (0x63)
#define REG_RAW_DATA_BURST (0x64)
#define REG_LIFTOFF_TUNE2 (0x65)

#define CONFIG2_REST_ENABLED (0x30)
#define CONFIG2_REPORT_MODE (0x04)


#define BUTTONS_ON 0
#define BUTTONS_OFF 1

#define BUTTON_M1 2
#define BUTTON_M2 3
#define BUTTON_M3 4
#define BUTTON_M4 6
#define BUTTON_M5 7
#define BUTTON_DPI 5

#define ENCODER_A 8
#define ENCODER_B 10
#define ENCODER_COM 9


#define PIN_NCS 17
#define PIN_MOSI 19
#define PIN_MISO 16

SPISettings spisettings(2000000, MSBFIRST, SPI_MODE3);
MbedSPI spi(PIN_MISO, PIN_MOSI, 18);


void setup_buttons()
{
    pinMode(BUTTONS_OFF, INPUT);
    pinMode(BUTTONS_ON, INPUT);
    pinMode(BUTTON_M1, INPUT_PULLUP);
    pinMode(BUTTON_M2, INPUT_PULLUP);
    pinMode(BUTTON_M3, INPUT_PULLUP);
    pinMode(BUTTON_M4, INPUT_PULLUP);
    pinMode(BUTTON_M5, INPUT_PULLUP);
    pinMode(BUTTON_DPI, INPUT_PULLUP);
    
    pinMode(ENCODER_A, INPUT_PULLUP);
    pinMode(ENCODER_B, INPUT_PULLUP);
    pinMode(ENCODER_COM, OUTPUT);
    
    digitalWrite(ENCODER_COM, LOW);
}

mbed::Ticker wheel_timer;

void setup()
{
    Serial1.begin(1000000);
    
    delay(3000);
    
    // SCK, MISO, MOSI, SS
    SPI.begin();
    pinMode(PIN_NCS, OUTPUT);
    
    digitalWrite(PIN_NCS, HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_NCS, LOW);
    delayMicroseconds(1);
    digitalWrite(PIN_NCS, HIGH);
    delayMicroseconds(1);
    
    pmw3360_boot();
    
    delay(10);
    
    pmw3360_config();
    
    setup_buttons();
    
    wheel_timer.attach_us(update_wheel, 250);
}

uint8_t m1_state = 0;
uint8_t m2_state = 0;
uint8_t m3_state = 0;
uint8_t m4_state = 0;
uint8_t m5_state = 0;
uint8_t dpi_state = 0;

uint8_t buttons = 0;

void update_buttons()
{
    pinMode(BUTTONS_OFF, INPUT);
    pinMode(BUTTONS_ON, OUTPUT);
    digitalWrite(BUTTONS_ON, LOW);
    
    uint8_t m1_on = digitalRead(BUTTON_M1);
    uint8_t m2_on = digitalRead(BUTTON_M2);
    uint8_t m3_on = digitalRead(BUTTON_M3);
    uint8_t m4_on = digitalRead(BUTTON_M4);
    uint8_t m5_on = digitalRead(BUTTON_M5);
    uint8_t dpi_on = digitalRead(BUTTON_DPI);
    
    pinMode(BUTTONS_ON, INPUT);
    pinMode(BUTTONS_OFF, OUTPUT);
    digitalWrite(BUTTONS_OFF, LOW);
    
    uint8_t m1_off = digitalRead(BUTTON_M1);
    uint8_t m2_off = digitalRead(BUTTON_M2);
    uint8_t m3_off = digitalRead(BUTTON_M3);
    uint8_t m4_off = digitalRead(BUTTON_M4);
    uint8_t m5_off = digitalRead(BUTTON_M5);
    uint8_t dpi_off = digitalRead(BUTTON_DPI);
    
    pinMode(BUTTONS_OFF, INPUT);
    
    if (m1_on != m1_off)
        m1_state = !m1_on;
    if (m2_on != m2_off)
        m2_state = !m2_on;
    if (m3_on != m3_off)
        m3_state = !m3_on;
    if (m4_on != m4_off)
        m4_state = !m4_on;
    if (m5_on != m5_off)
        m5_state = !m5_on;
    if (dpi_on != dpi_off)
        dpi_state = !dpi_on;
    
    buttons =
        m1_state |
        (m2_state << 1) |
        ((m3_state | dpi_state) << 2) |
        (m4_state << 3) |
        (m5_state << 4);
}

uint8_t wheel_state_a = 0;
uint8_t wheel_state_b = 0;
uint8_t wheel_state_output = 0;
int8_t wheel_progress = 0;

int8_t wheel_temp_a[10];
int8_t wheel_temp_b[10];
int wheel_temp_tm[10];
int8_t wheel_temp_i = 0;

void update_wheel()
{
    // filter by gathering 3 samples
    
    uint8_t wheel_new_a = digitalRead(ENCODER_A);
    uint8_t wheel_new_b = digitalRead(ENCODER_B);
    
    delayMicroseconds(5);
    wheel_new_a += digitalRead(ENCODER_A);
    wheel_new_b += digitalRead(ENCODER_B);
    
    delayMicroseconds(5);
    wheel_new_a += digitalRead(ENCODER_A);
    wheel_new_b += digitalRead(ENCODER_B);
    
    wheel_new_a = (wheel_new_a + 1) / 3;
    wheel_new_b = (wheel_new_b + 1) / 3;
    
    if (wheel_new_a != wheel_state_a || wheel_new_b != wheel_state_b)
    {
        if (wheel_new_a == wheel_new_b && wheel_state_output != wheel_new_a)
        {
            // when scrolling up, B changes first. when scrolling down, A changes first
            if (wheel_new_b == wheel_state_b)
                wheel_progress += 1;
            // the wheel can glitch and jump straight from 00 to 11 (or vice versa)
            // so we need to check that only one state has changed since the last test
            else if(wheel_new_a == wheel_state_a)
                wheel_progress -= 1;
            
            wheel_state_output = wheel_new_a;
        }
        
        wheel_temp_a[wheel_temp_i] = wheel_new_a;
        wheel_temp_b[wheel_temp_i] = wheel_new_b;
        wheel_temp_tm[wheel_temp_i] = (int) millis();
        
        wheel_temp_i += 1;
        if (wheel_temp_i >= 10)
            wheel_temp_i = 9;
        
        wheel_state_a = wheel_new_a;
        wheel_state_b = wheel_new_b;
    }
}

int n = 0;
int usb_hid_poll_interval = 1; 

struct MotionBurstData {
    uint8_t motion;
    uint8_t observation;
    int16_t x;
    int16_t y;
    uint8_t squal;
    uint8_t raw_sum;
    uint8_t raw_max;
    uint8_t raw_min;
    uint16_t shutter;
};

// prevent Arduino IDE's evil function declaration hoisting by explicitly declaring separately from defining
MotionBurstData spi_read_motion_burst();

void loop()
{
    update_buttons();
    
    /*
    spi_write(REG_MOTION, 1);
    byte motion = spi_read(REG_MOTION);
    
    int16_t x = 0;
    int16_t y = 0;
    if (motion & 0x80)
    {
        uint16_t x_lo = spi_read(REG_DELTA_X_L);
        uint16_t x_hi = spi_read(REG_DELTA_X_H);
        uint16_t y_lo = spi_read(REG_DELTA_Y_L);
        uint16_t y_hi = spi_read(REG_DELTA_Y_H);
        //printf("x hi, lo: %02X, %02X\n", x_hi, x_lo);
        x = (x_hi << 8) | x_lo;
        y = (y_hi << 8) | y_lo;
    }
    
    n += 1;
    if (n > 1000)
    {
        n = 0;
        uint8_t squal = spi_read(REG_SQUAL);
        printf("surface quality: %d\n", squal);
    }
    */
    int16_t x = 0;
    int16_t y = 0;
    MotionBurstData data = spi_read_motion_burst();
    if (data.motion)
    {
        x = data.x;
        y = data.y;
    }
    /*
    n += 1;
    if (n > 1000)
    {
        n = 0;
        printf("surface quality: %d\n", data.squal);
    }
    */
    
    noInterrupts();
    int8_t wheel = wheel_progress;
    wheel_progress = 0;
    
    int8_t wheel_temp_a_local[10];
    int8_t wheel_temp_b_local[10];
    int wheel_temp_tm_local[10];
    int8_t wheel_temp_i_local = wheel_temp_i;
    
    for (int i = 0; i < wheel_temp_i; i++)
    {
        wheel_temp_a_local[i] = wheel_temp_a[i];
        wheel_temp_b_local[i] = wheel_temp_b[i];
        wheel_temp_tm_local[i] = wheel_temp_tm[i];
    }
    wheel_temp_i = 0;
    
    interrupts();
    
    if (wheel_temp_i_local)
        printf("---\n");
    for (int i = 0; i < wheel_temp_i_local; i++)
        printf("%c %c %d\n", wheel_temp_a_local[i] ? '#' : '.', wheel_temp_b_local[i] ? '#' : '.', wheel_temp_tm_local[i]);
    
    uint32_t b = micros();
    
    mouse.update(x, y, buttons, wheel);
    //mouse.move(x, y);
    
    uint32_t c = micros();
    //printf("%d\n", c - b);
}

void pmw3360_boot()
{
    spi_write(REG_POWER_UP_RESET, 0x5A);
    delay(50);
    
    spi_read(0x02);
    spi_read(0x03);
    spi_read(0x04);
    spi_read(0x05);
    spi_read(0x06);
    
    srom_upload();
}

void pmw3360_config()
{
    spi_write(REG_CONFIG1, 11); // 1200 dpi
    
    // 3mm liftoff (2mm is bad for 3d printed case tolerances)
    // FIXME: make configurable
    //spi_write(REG_LIFT_CONFIG, 3);
}

void spi_begin()
{
    SPI.beginTransaction(spisettings);
}
void spi_end()
{
    SPI.endTransaction();
}

void spi_write(byte addr, byte data)
{
    spi_begin();
    
    digitalWrite(PIN_NCS, LOW);
    delayMicroseconds(1); // 120 nanoseconds; t_NCS-SCLK
    
    SPI.transfer(addr | 0x80);
    SPI.transfer(data);
    
    delayMicroseconds(35); // t_SCLK-NCS(write)
    digitalWrite(PIN_NCS, HIGH);
    
    spi_end();
    
    delayMicroseconds(180); // max(t_SWW, t_SWR)
}

byte spi_read(byte addr)
{
    spi_begin();
    
    digitalWrite(PIN_NCS, LOW);
    delayMicroseconds(1); // 120 nanoseconds; t_NCS-SCLK
    
    SPI.transfer(addr & 0x7F);
    
    delayMicroseconds(160); // t_SRAD
    
    byte ret = SPI.transfer(0);
    
    delayMicroseconds(1); // 120 nanoseconds; t_SCLK-NCS(read)
    digitalWrite(PIN_NCS, HIGH);
    
    spi_end();
    
    delayMicroseconds(20); // max(t_SRW, t_SRR)
    
    return ret;
}

MotionBurstData spi_read_motion_burst()
{
    MotionBurstData ret = {0};
    
    spi_write(REG_MOTION_BURST, 0x00);
    
    spi_begin();
    
    digitalWrite(PIN_NCS, LOW);
    delayMicroseconds(1); // 120 nanoseconds; t_NCS-SCLK
    
    SPI.transfer(REG_MOTION_BURST);
    
    delayMicroseconds(35); // t_SRAD_MOTBR
    
    ret.motion = SPI.transfer(0);
    ret.observation = SPI.transfer(0);
    ret.x = SPI.transfer(0);
    ret.x |= ((uint16_t)SPI.transfer(0)) << 8;
    ret.y = SPI.transfer(0);
    ret.y |= ((uint16_t)SPI.transfer(0)) << 8;
    ret.squal = SPI.transfer(0);
    // don't care about the rest; terminate
    /*
    ret.raw_sum = SPI.transfer(0);
    ret.raw_max = SPI.transfer(0);
    ret.raw_min = SPI.transfer(0);
    ret.shutter = SPI.transfer(0);
    ret.shutter |= ((uint16_t)SPI.transfer(0)) << 8;
    */
    
    spi_end();
    
    delayMicroseconds(1); // t_BEXIT = 500ns; wait 1000ns (1us)
    
    return ret;
}

void srom_upload()
{
    spi_write(REG_CONFIG2, 0x00);
    spi_write(REG_SROM_ENABLE, 0x1D);
    delay(10);
    
    spi_write(REG_SROM_ENABLE, 0x18);
    
    spi_begin();
    
    digitalWrite(PIN_NCS, LOW);
    delayMicroseconds(1);
    
    SPI.transfer(REG_SROM_LOAD_BURST | 0x80);
    delayMicroseconds(15);
    
    for(size_t i = 0; i < SROM_LENGTH; i += 1)
    {
        SPI.transfer(srom[i]);
        delayMicroseconds(15);
    }
    
    digitalWrite(PIN_NCS, HIGH);
    delayMicroseconds(1);
    
    spi_end();
    
    delayMicroseconds(200);
    
    byte id = spi_read(REG_SROM_ID);
    printf("\n");
    printf("srom id: ");
    printf("%d", id);
    printf("\n");
    
    spi_write(REG_CONFIG2, 0x00);
}

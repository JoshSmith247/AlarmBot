#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "hw.h"
#include "pins.h"

#define IMU_OUTX_XL 0x28
#define IMU_CTRL1_XL 0x10
#define IMU_WHO 0x0F

static uint8_t imu_addr;

static void servo_init(uint pin)
{
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint s = pwm_gpio_to_slice_num(pin);
    pwm_set_clkdiv(s, clock_get_hz(clk_sys) / 1e6f);
    pwm_set_wrap(s, 19999);
    pwm_set_enabled(s, true);
}

static void in_pin(uint pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
}

static void out_pin(uint pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

static int imu_reg(uint8_t addr, uint8_t reg, uint8_t *v)
{
    return i2c_write_blocking(i2c0, addr, &reg, 1, true) == 1 && i2c_read_blocking(i2c0, addr, v, 1, false) == 1;
}

void hw_init(void)
{
    servo_init(PIN_SERVO_L);
    servo_init(PIN_SERVO_R);
    hw_motors(0, 0);

    in_pin(PIN_EDGE_L);
    in_pin(PIN_EDGE_R);
    out_pin(PIN_TRIG0);
    out_pin(PIN_TRIG1);
    in_pin(PIN_ECHO0);
    in_pin(PIN_ECHO1);

    i2c_init(i2c0, 400000);
    gpio_set_function(PIN_IMU_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_IMU_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_IMU_SDA);
    gpio_pull_up(PIN_IMU_SCL);
    for (uint8_t a = 0x6A; a <= 0x6B && !imu_addr; a++) {
        uint8_t who;
        if (imu_reg(a, IMU_WHO, &who) && who) {
            uint8_t cfg[2] = {IMU_CTRL1_XL, 0x40};
            i2c_write_blocking(i2c0, a, cfg, 2, false);
            imu_addr = a;
        }
    }
}

static void servo_us(uint pin, int speed, int sign)
{
    pwm_set_gpio_level(pin, 1500 + sign * speed * 8);
}

void hw_motors(int8_t left, int8_t right)
{
    servo_us(PIN_SERVO_L, left, MOTOR_SIGN_L);
    servo_us(PIN_SERVO_R, right, MOTOR_SIGN_R);
}

static uint16_t sonar_cm(uint trig, uint echo)
{
    gpio_put(trig, 1);
    sleep_us(10);
    gpio_put(trig, 0);
    uint64_t t = time_us_64();
    while (!gpio_get(echo))
        if (time_us_64() - t > 2000)
            return SONAR_NONE;
    t = time_us_64();
    while (gpio_get(echo))
        if (time_us_64() - t > 12000)
            return SONAR_NONE;
    return (time_us_64() - t) / 58;
}

void hw_sense(ctl_sense_t *s)
{
    static uint8_t turn;
    s->edge_l = gpio_get(PIN_EDGE_L) == EDGE_ACTIVE;
    s->edge_r = gpio_get(PIN_EDGE_R) == EDGE_ACTIVE;

    if (turn++ % 3 == 0) {
        static uint8_t which;
        s->dist_cm[which] = which ? sonar_cm(PIN_TRIG1, PIN_ECHO1) : sonar_cm(PIN_TRIG0, PIN_ECHO0);
        which ^= 1;
    }

    uint8_t b[6], reg = IMU_OUTX_XL;
    if (imu_addr && i2c_write_blocking(i2c0, imu_addr, &reg, 1, true) == 1 && i2c_read_blocking(i2c0, imu_addr, b, 6, false) == 6) {
        for (int i = 0; i < 3; i++)
            s->acc[i] = (int16_t)(b[2 * i] | b[2 * i + 1] << 8);
    }
}

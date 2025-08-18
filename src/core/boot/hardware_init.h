#ifndef HARDWARE_INIT_H
#define HARDWARE_INIT_H

#include "hardware_config.h"
#include <Arduino.h>

/*
 * Linus Torvalds 硬件初始化系统
 * 
 * 设计原则：
 * 1. 基于配置驱动 - 所有初始化都基于hardware_config
 * 2. 错误处理 - 每个初始化步骤都有明确的错误返回
 * 3. 最小依赖 - 不依赖复杂的第三方库
 * 4. 可测试性 - 每个函数都可以独立测试
 */

// ========================================
// 初始化结果类型
// ========================================

typedef enum {
    HW_INIT_OK = 0,
    HW_INIT_INVALID_CONFIG,
    HW_INIT_GPIO_FAILED,
    HW_INIT_SPI_FAILED,
    HW_INIT_I2C_FAILED,
    HW_INIT_PWM_FAILED,
    HW_INIT_TIMEOUT
} hw_init_result_t;

// ========================================
// 基础硬件初始化函数
// ========================================

// GPIO初始化 - 基础功能
static inline hw_init_result_t init_gpio_pin(gpio_pin_t pin, uint8_t mode) {
    if (pin > 48) {  // ESP32-S3最大GPIO编号
        return HW_INIT_INVALID_CONFIG;
    }
    
    pinMode(pin, mode);
    return HW_INIT_OK;
}

// SPI初始化 - 基于配置
static inline hw_init_result_t init_spi_config(const spi_config_t* spi_cfg) {
    if (!spi_cfg) {
        return HW_INIT_INVALID_CONFIG;
    }
    
    // 初始化SPI引脚
    hw_init_result_t result;
    result = init_gpio_pin(spi_cfg->miso, INPUT);
    if (result != HW_INIT_OK) return result;
    
    result = init_gpio_pin(spi_cfg->mosi, OUTPUT);
    if (result != HW_INIT_OK) return result;
    
    result = init_gpio_pin(spi_cfg->sclk, OUTPUT);
    if (result != HW_INIT_OK) return result;
    
    result = init_gpio_pin(spi_cfg->cs, OUTPUT);
    if (result != HW_INIT_OK) return result;
    
    // 设置CS为高电平（未选中）
    digitalWrite(spi_cfg->cs, HIGH);
    
    return HW_INIT_OK;
}

// PWM初始化 - 标准化
static inline hw_init_result_t init_pwm_config(const pwm_config_t* pwm_cfg) {
    if (!pwm_cfg) {
        return HW_INIT_INVALID_CONFIG;
    }
    
    // 初始化PWM引脚
    hw_init_result_t result = init_gpio_pin(pwm_cfg->pin, OUTPUT);
    if (result != HW_INIT_OK) return result;
    
    // 配置LEDC PWM
    ledcSetup(pwm_cfg->channel, pwm_cfg->frequency, pwm_cfg->resolution_bits);
    ledcAttachPin(pwm_cfg->pin, pwm_cfg->channel);
    
    // 设置初始值为0
    ledcWrite(pwm_cfg->channel, 0);
    
    return HW_INIT_OK;
}

// ========================================
// 模块化硬件初始化函数
// ========================================

// 显示硬件初始化
static inline hw_init_result_t init_display_hardware(void) {
    const display_config_t* cfg = DISPLAY_CONFIG();
    
    // 初始化SPI
    hw_init_result_t result = init_spi_config(&cfg->spi);
    if (result != HW_INIT_OK) return result;
    
    // 初始化控制引脚
    result = init_gpio_pin(cfg->dc_pin, OUTPUT);
    if (result != HW_INIT_OK) return result;
    
    result = init_gpio_pin(cfg->rst_pin, OUTPUT);
    if (result != HW_INIT_OK) return result;
    
    // 初始化背光PWM
    result = init_pwm_config(&cfg->backlight);
    if (result != HW_INIT_OK) return result;
    
    // 复位显示屏
    digitalWrite(cfg->rst_pin, LOW);
    delay(10);
    digitalWrite(cfg->rst_pin, HIGH);
    delay(10);
    
    return HW_INIT_OK;
}

// LED硬件初始化
static inline hw_init_result_t init_led_hardware(void) {
    const led_config_t* cfg = LED_CONFIG();
    
    // 初始化数据引脚
    hw_init_result_t result = init_gpio_pin(cfg->data_pin, OUTPUT);
    if (result != HW_INIT_OK) return result;
    
    // 设置初始状态为低电平
    digitalWrite(cfg->data_pin, LOW);
    
    return HW_INIT_OK;
}

// IMU硬件初始化
static inline hw_init_result_t init_imu_hardware(void) {
    const imu_config_t* cfg = IMU_CONFIG();
    
    // 初始化I2C引脚
    hw_init_result_t result = init_gpio_pin(cfg->i2c.sda, INPUT_PULLUP);
    if (result != HW_INIT_OK) return result;
    
    result = init_gpio_pin(cfg->i2c.scl, INPUT_PULLUP);
    if (result != HW_INIT_OK) return result;
    
    // 初始化中断引脚（如果配置了）
    if (cfg->interrupt_pin != 0xFF) {  // 0xFF表示未配置
        result = init_gpio_pin(cfg->interrupt_pin, INPUT_PULLUP);
        if (result != HW_INIT_OK) return result;
    }
    
    return HW_INIT_OK;
}

// ========================================
// 完整系统初始化
// ========================================

// 初始化所有硬件
static inline hw_init_result_t init_all_hardware(void) {
    hw_init_result_t result;
    
    // 验证配置
    validate_hardware_config();
    
    // 按依赖顺序初始化
    result = init_display_hardware();
    if (result != HW_INIT_OK) {
        Serial.printf("Display hardware init failed: %d\n", result);
        return result;
    }
    
    result = init_led_hardware();
    if (result != HW_INIT_OK) {
        Serial.printf("LED hardware init failed: %d\n", result);
        return result;
    }
    
    result = init_imu_hardware();
    if (result != HW_INIT_OK) {
        Serial.printf("IMU hardware init failed: %d\n", result);
        return result;
    }
    
    Serial.println("All hardware initialized successfully");
    return HW_INIT_OK;
}

// ========================================
// 配置信息打印 - 调试用
// ========================================

static inline void print_hardware_config(void) {
    const hardware_config_t* cfg = get_hardware_config();
    
    Serial.println("\n=== Hardware Configuration ===");
    Serial.printf("Board: %s %s\n", cfg->board_name, cfg->version);
    Serial.printf("System Clock: %u MHz\n", cfg->system_clock_mhz);
    Serial.printf("Serial Baud: %u\n", cfg->serial_baud_rate);
    
    Serial.println("\nDisplay:");
    Serial.printf("  Type: %d, Size: %dx%d\n", 
        cfg->display.type, cfg->display.width, cfg->display.height);
    Serial.printf("  SPI: MISO=%d, MOSI=%d, SCLK=%d, CS=%d\n",
        cfg->display.spi.miso, cfg->display.spi.mosi, 
        cfg->display.spi.sclk, cfg->display.spi.cs);
    Serial.printf("  Control: DC=%d, RST=%d, BL=%d\n",
        cfg->display.dc_pin, cfg->display.rst_pin, cfg->display.backlight.pin);
    
    Serial.println("\nLED:");
    Serial.printf("  Type: %d, Count: %d, Pin: %d\n",
        cfg->led.type, cfg->led.count, cfg->led.data_pin);
    
    Serial.println("\nIMU:");
    Serial.printf("  Type: %d, I2C: SDA=%d, SCL=%d, Addr=0x%02X\n",
        cfg->imu.type, cfg->imu.i2c.sda, cfg->imu.i2c.scl, cfg->imu.i2c.address);
    
    Serial.println("============================\n");
}

#endif // HARDWARE_INIT_H

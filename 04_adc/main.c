/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : B10K Potentiometer Resistance Measurement
  *                  : STM32G431CBT6 + I2C LCD
  ******************************************************************************
  */
/* USER CODE END Header */


/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */


/* Private variables ---------------------------------------------------------*/

ADC_HandleTypeDef hadc2;
I2C_HandleTypeDef hi2c1;


/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_ADC2_Init(void);
static void MX_I2C1_Init(void);


/* USER CODE BEGIN PFP */

/* LCD functions */

void LCD_Init(void);
void LCD_SendCommand(uint8_t command);
void LCD_SendData(uint8_t data);
void LCD_SendString(char *str);
void LCD_SetCursor(uint8_t row, uint8_t column);
void LCD_Clear(void);
void LCD_Send4Bits(uint8_t data);
uint8_t LCD_FindAddress(void);

/* USER CODE END PFP */


/* Private user variables ----------------------------------------------------*/
/* USER CODE BEGIN PV */

#define POT_TOTAL_RESISTANCE    10000UL
#define ADC_MAX_VALUE           4095UL

uint8_t lcd_address = 0;

/* USER CODE END PV */


/* USER CODE BEGIN 0 */


/* ============================================================================
 * I2C LCD FUNCTIONS
 * ============================================================================
 *
 * PCF8574 LCD BACKPACK
 *
 * P0 = RS
 * P1 = RW
 * P2 = EN
 * P3 = BACKLIGHT
 * P4 = D4
 * P5 = D5
 * P6 = D6
 * P7 = D7
 *
 */


/* --------------------------------------------------------------------------
 * Send 4 bits to LCD
 * -------------------------------------------------------------------------- */

void LCD_Send4Bits(uint8_t data)
{
    uint8_t lcd_data;
    uint8_t lcd_enable;


    /* Backlight ON */

    lcd_data = data | 0x08;


    /* EN HIGH */

    lcd_enable = lcd_data | 0x04;


    HAL_I2C_Master_Transmit(&hi2c1,
                            lcd_address,
                            &lcd_enable,
                            1,
                            HAL_MAX_DELAY);


    HAL_Delay(1);


    /* EN LOW */

    HAL_I2C_Master_Transmit(&hi2c1,
                            lcd_address,
                            &lcd_data,
                            1,
                            HAL_MAX_DELAY);


    HAL_Delay(1);
}


/* --------------------------------------------------------------------------
 * Send command to LCD
 * -------------------------------------------------------------------------- */

void LCD_SendCommand(uint8_t command)
{
    uint8_t high_nibble;
    uint8_t low_nibble;


    high_nibble = command & 0xF0;

    low_nibble = (command << 4) & 0xF0;


    /* RS = 0 */

    LCD_Send4Bits(high_nibble);

    LCD_Send4Bits(low_nibble);


    HAL_Delay(2);
}


/* --------------------------------------------------------------------------
 * Send data / character to LCD
 * -------------------------------------------------------------------------- */

void LCD_SendData(uint8_t data)
{
    uint8_t high_nibble;
    uint8_t low_nibble;

    uint8_t lcd_data;
    uint8_t lcd_enable;


    high_nibble = data & 0xF0;

    low_nibble = (data << 4) & 0xF0;


    /* ==========================================================
     * HIGH NIBBLE
     * ========================================================== */

    /*
     * RS = 1
     * Backlight = 1
     */

    lcd_data = high_nibble | 0x01 | 0x08;


    /* EN HIGH */

    lcd_enable = lcd_data | 0x04;


    HAL_I2C_Master_Transmit(&hi2c1,
                            lcd_address,
                            &lcd_enable,
                            1,
                            HAL_MAX_DELAY);


    HAL_Delay(1);


    /* EN LOW */

    HAL_I2C_Master_Transmit(&hi2c1,
                            lcd_address,
                            &lcd_data,
                            1,
                            HAL_MAX_DELAY);


    HAL_Delay(1);


    /* ==========================================================
     * LOW NIBBLE
     * ========================================================== */

    lcd_data = low_nibble | 0x01 | 0x08;


    /* EN HIGH */

    lcd_enable = lcd_data | 0x04;


    HAL_I2C_Master_Transmit(&hi2c1,
                            lcd_address,
                            &lcd_enable,
                            1,
                            HAL_MAX_DELAY);


    HAL_Delay(1);


    /* EN LOW */

    HAL_I2C_Master_Transmit(&hi2c1,
                            lcd_address,
                            &lcd_data,
                            1,
                            HAL_MAX_DELAY);


    HAL_Delay(1);
}


/* --------------------------------------------------------------------------
 * Send string
 * -------------------------------------------------------------------------- */

void LCD_SendString(char *str)
{
    while (*str)
    {
        LCD_SendData((uint8_t)*str);

        str++;
    }
}


/* --------------------------------------------------------------------------
 * Set LCD cursor
 * -------------------------------------------------------------------------- */

void LCD_SetCursor(uint8_t row, uint8_t column)
{
    uint8_t address;


    if (row == 0)
    {
        address = 0x80 + column;
    }
    else
    {
        address = 0xC0 + column;
    }


    LCD_SendCommand(address);
}


/* --------------------------------------------------------------------------
 * Clear LCD
 * -------------------------------------------------------------------------- */

void LCD_Clear(void)
{
    LCD_SendCommand(0x01);

    HAL_Delay(2);
}


/* --------------------------------------------------------------------------
 * Find LCD I2C address
 * -------------------------------------------------------------------------- */

uint8_t LCD_FindAddress(void)
{
    uint8_t address;


    /* ==========================================================
     * PCF8574
     *
     * 0x20 - 0x27
     * ========================================================== */

    for (address = 0x20;
         address <= 0x27;
         address++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1,
                                   address << 1,
                                   2,
                                   100) == HAL_OK)
        {
            return (address << 1);
        }
    }


    /* ==========================================================
     * PCF8574A
     *
     * 0x38 - 0x3F
     * ========================================================== */

    for (address = 0x38;
         address <= 0x3F;
         address++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1,
                                   address << 1,
                                   2,
                                   100) == HAL_OK)
        {
            return (address << 1);
        }
    }


    /* LCD NOT FOUND */

    return 0;
}


/* --------------------------------------------------------------------------
 * LCD initialization
 * -------------------------------------------------------------------------- */

void LCD_Init(void)
{
    HAL_Delay(50);


    /* 8-bit initialization sequence */

    LCD_Send4Bits(0x30);

    HAL_Delay(5);


    LCD_Send4Bits(0x30);

    HAL_Delay(1);


    LCD_Send4Bits(0x30);

    HAL_Delay(1);


    /* Change to 4-bit mode */

    LCD_Send4Bits(0x20);

    HAL_Delay(1);


    /* 4-bit mode
     * 2 lines
     * 5x8 font
     */

    LCD_SendCommand(0x28);


    /* Display ON
     * Cursor OFF
     * Blink OFF
     */

    LCD_SendCommand(0x0C);


    /* Entry mode */

    LCD_SendCommand(0x06);


    /* Clear display */

    LCD_Clear();
}


/* USER CODE END 0 */


/**
  * @brief  The application entry point.
  * @retval int
  */

int main(void)
{
    /* USER CODE BEGIN 1 */


    uint32_t raw_adc;

    uint32_t resistance;

    uint32_t whole_kohms;

    uint32_t remainder_ohms;

    char lcd_line1[17];

    char lcd_line2[17];


    /* USER CODE END 1 */


    /* MCU Configuration--------------------------------------------------------*/

    HAL_Init();


    /* Configure system clock */

    SystemClock_Config();


    /* Initialize peripherals */

    MX_GPIO_Init();

    MX_ADC2_Init();

    MX_I2C1_Init();


    /* USER CODE BEGIN 2 */


    /* ==========================================================
     * ADC CALIBRATION
     * ========================================================== */

    if (HAL_ADCEx_Calibration_Start(&hadc2,
                                    ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }


    /* ==========================================================
     * FIND LCD ADDRESS
     * ========================================================== */

    lcd_address = LCD_FindAddress();


    /* ==========================================================
     * INITIALIZE LCD
     * ========================================================== */

    if (lcd_address != 0)
    {
        LCD_Init();


        LCD_SetCursor(0, 0);

        LCD_SendString("B10K Resistance");


        LCD_SetCursor(1, 0);

        LCD_SendString("Initializing");


        HAL_Delay(1500);


        LCD_Clear();
    }


    /* USER CODE END 2 */


    /* Infinite loop */

    while (1)
    {

        /* ======================================================
         * START ADC
         * ====================================================== */

        if (HAL_ADC_Start(&hadc2) != HAL_OK)
        {
            Error_Handler();
        }


        /* ======================================================
         * WAIT FOR ADC CONVERSION
         * ====================================================== */

        if (HAL_ADC_PollForConversion(&hadc2,
                                      100) == HAL_OK)
        {

            /* Read ADC value */

            raw_adc = HAL_ADC_GetValue(&hadc2);


            /* Stop ADC */

            HAL_ADC_Stop(&hadc2);


            /* ==================================================
             * RESISTANCE CALCULATION
             *
             * R = ADC / 4095 × 10000
             * ================================================== */

            resistance =
                (raw_adc * POT_TOTAL_RESISTANCE)
                / ADC_MAX_VALUE;


            /* ==================================================
             * SPLIT INTO kOHM AND OHM
             * ================================================== */

            whole_kohms = resistance / 1000;

            remainder_ohms = resistance % 1000;


            /* ==================================================
             * LCD LINE 1
             * ================================================== */

            snprintf(lcd_line1,
                     sizeof(lcd_line1),
                     "ADC:%4lu",
                     (unsigned long)raw_adc);


            /* ==================================================
             * LCD LINE 2
             * ================================================== */

            snprintf(lcd_line2,
                     sizeof(lcd_line2),
                     "R:%lu.%03lu kOhm",
                     (unsigned long)whole_kohms,
                     (unsigned long)remainder_ohms);


            /* ==================================================
             * DISPLAY DATA
             * ================================================== */

            if (lcd_address != 0)
            {

                /* LINE 1 */

                LCD_SetCursor(0, 0);

                LCD_SendString("                ");


                LCD_SetCursor(0, 0);

                LCD_SendString(lcd_line1);


                /* LINE 2 */

                LCD_SetCursor(1, 0);

                LCD_SendString("                ");


                LCD_SetCursor(1, 0);

                LCD_SendString(lcd_line2);
            }
        }

        else
        {
            /* ADC conversion error */

            HAL_ADC_Stop(&hadc2);


            if (lcd_address != 0)
            {
                LCD_SetCursor(0, 0);

                LCD_SendString("ADC ERROR       ");


                LCD_SetCursor(1, 0);

                LCD_SendString("                ");
            }
        }


        /* Update every 300 ms */

        HAL_Delay(300);
    }
}


/**
  * @brief System Clock Configuration
  * @retval None
  */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


    /* Configure regulator */

    HAL_PWREx_ControlVoltageScaling(
        PWR_REGULATOR_VOLTAGE_SCALE1
    );


    /* ==========================================================
     * HSI
     * ========================================================== */

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;


    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;


    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;


    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;


    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }


    /* ==========================================================
     * CLOCK CONFIGURATION
     * ========================================================== */

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;


    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_HSI;


    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;


    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV1;


    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;


    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct,
                            FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief ADC2 Initialization Function
  * @retval None
  */

static void MX_ADC2_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};


    /* ==========================================================
     * ADC2
     * ========================================================== */

    hadc2.Instance = ADC2;


    hadc2.Init.ClockPrescaler =
        ADC_CLOCK_SYNC_PCLK_DIV2;


    hadc2.Init.Resolution =
        ADC_RESOLUTION_12B;


    hadc2.Init.DataAlign =
        ADC_DATAALIGN_RIGHT;


    hadc2.Init.GainCompensation =
        0;


    hadc2.Init.ScanConvMode =
        ADC_SCAN_DISABLE;


    hadc2.Init.EOCSelection =
        ADC_EOC_SINGLE_CONV;


    hadc2.Init.LowPowerAutoWait =
        DISABLE;


    hadc2.Init.ContinuousConvMode =
        DISABLE;


    hadc2.Init.NbrOfConversion =
        1;


    hadc2.Init.DiscontinuousConvMode =
        DISABLE;


    hadc2.Init.ExternalTrigConv =
        ADC_SOFTWARE_START;


    hadc2.Init.ExternalTrigConvEdge =
        ADC_EXTERNALTRIGCONVEDGE_NONE;


    hadc2.Init.DMAContinuousRequests =
        DISABLE;


    hadc2.Init.Overrun =
        ADC_OVR_DATA_PRESERVED;


    hadc2.Init.OversamplingMode =
        DISABLE;


    if (HAL_ADC_Init(&hadc2) != HAL_OK)
    {
        Error_Handler();
    }


    /* ==========================================================
     * PA1 = ADC2_IN2
     * ========================================================== */

    sConfig.Channel =
        ADC_CHANNEL_2;


    sConfig.Rank =
        ADC_REGULAR_RANK_1;


    /*
     * Longer sampling time
     * Good for potentiometer
     */

    sConfig.SamplingTime =
        ADC_SAMPLETIME_47CYCLES_5;


    sConfig.SingleDiff =
        ADC_SINGLE_ENDED;


    sConfig.OffsetNumber =
        ADC_OFFSET_NONE;


    sConfig.Offset =
        0;


    if (HAL_ADC_ConfigChannel(&hadc2,
                              &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief I2C1 Initialization Function
  * @retval None
  */

static void MX_I2C1_Init(void)
{
    /*
     * ==========================================================
     * I2C1
     *
     * SDA = PB7
     * SCL = PA15
     * ==========================================================
     */


    hi2c1.Instance = I2C1;


    hi2c1.Init.Timing =
        0x00303D5B;


    hi2c1.Init.OwnAddress1 =
        0;


    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;


    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;


    hi2c1.Init.OwnAddress2 =
        0;


    hi2c1.Init.OwnAddress2Masks =
        I2C_OA2_NOMASK;


    hi2c1.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;


    hi2c1.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;


    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }


    /* Analog filter */

    if (HAL_I2CEx_ConfigAnalogFilter(
            &hi2c1,
            I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }


    /* Digital filter */

    if (HAL_I2CEx_ConfigDigitalFilter(
            &hi2c1,
            0) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief GPIO Initialization Function
  * @retval None
  */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};


    /* ==========================================================
     * ENABLE GPIO CLOCKS
     * ========================================================== */

    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();


    /* ==========================================================
     * PA1 = ADC2_IN2
     * ========================================================== */

    GPIO_InitStruct.Pin =
        GPIO_PIN_1;


    GPIO_InitStruct.Mode =
        GPIO_MODE_ANALOG;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    HAL_GPIO_Init(GPIOA,
                  &GPIO_InitStruct);


    /* ==========================================================
     * PB7 = I2C1_SDA
     * ========================================================== */

    GPIO_InitStruct.Pin =
        GPIO_PIN_7;


    GPIO_InitStruct.Mode =
        GPIO_MODE_AF_OD;


    GPIO_InitStruct.Pull =
        GPIO_PULLUP;


    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_HIGH;


    GPIO_InitStruct.Alternate =
        GPIO_AF4_I2C1;


    HAL_GPIO_Init(GPIOB,
                  &GPIO_InitStruct);


    /* ==========================================================
     * PA15 = I2C1_SCL
     * ========================================================== */

    GPIO_InitStruct.Pin =
        GPIO_PIN_15;


    GPIO_InitStruct.Mode =
        GPIO_MODE_AF_OD;


    GPIO_InitStruct.Pull =
        GPIO_PULLUP;


    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_HIGH;


    GPIO_InitStruct.Alternate =
        GPIO_AF4_I2C1;


    HAL_GPIO_Init(GPIOA,
                  &GPIO_InitStruct);
}


/**
  * @brief Error Handler
  * @retval None
  */

void Error_Handler(void)
{
    __disable_irq();


    while (1)
    {
    }
}


#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  */

void assert_failed(uint8_t *file,
                   uint32_t line)
{
}

#endif
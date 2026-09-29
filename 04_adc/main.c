/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : B10K Potentiometer Resistance Measurement
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
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


/* USER CODE BEGIN PV */

/*
 * B10K potentiometer
 *
 * Nominal total resistance = 10,000 ohms
 */
#define POT_TOTAL_RESISTANCE 10000UL

/*
 * 12-bit ADC
 */
#define ADC_MAX_VALUE 4095UL

uint8_t lcd_address = 0;

/* USER CODE END PV */


/* USER CODE BEGIN 0 */


/*
 * ============================================================================
 * I2C LCD FUNCTIONS
 * ============================================================================
 *
 * PCF8574 LCD backpack mapping:
 *
 * P0 = RS
 * P1 = RW
 * P2 = EN
 * P3 = Backlight
 * P4 = D4
 * P5 = D5
 * P6 = D6
 * P7 = D7
 *
 */


/* Send 4 bits to LCD */
void LCD_Send4Bits(uint8_t data)
{
    uint8_t lcd_data;
    uint8_t lcd_enable;

    /*
     * Backlight ON
     */
    lcd_data = data | 0x08;

    /*
     * EN = HIGH
     */
    lcd_enable = lcd_data | 0x04;

    HAL_I2C_Master_Transmit(&hi2c1,
                           lcd_address,
                           &lcd_enable,
                           1,
                           HAL_MAX_DELAY);

    HAL_Delay(1);

    /*
     * EN = LOW
     */
    HAL_I2C_Master_Transmit(&hi2c1,
                           lcd_address,
                           &lcd_data,
                           1,
                           HAL_MAX_DELAY);

    HAL_Delay(1);
}


/* Send command to LCD */
void LCD_SendCommand(uint8_t command)
{
    uint8_t high_nibble;
    uint8_t low_nibble;

    high_nibble = command & 0xF0;

    low_nibble = (command << 4) & 0xF0;

    /*
     * RS = 0
     */
    LCD_Send4Bits(high_nibble);

    LCD_Send4Bits(low_nibble);

    HAL_Delay(2);
}


/* Send character to LCD */
void LCD_SendData(uint8_t data)
{
    uint8_t high_nibble;
    uint8_t low_nibble;

    uint8_t lcd_data;
    uint8_t lcd_enable;


    high_nibble = data & 0xF0;

    low_nibble = (data << 4) & 0xF0;


    /*
     * HIGH NIBBLE
     *
     * RS = 1
     * Backlight = 1
     */
    lcd_data = high_nibble | 0x01 | 0x08;

    /*
     * EN = HIGH
     */
    lcd_enable = lcd_data | 0x04;

    HAL_I2C_Master_Transmit(&hi2c1,
                           lcd_address,
                           &lcd_enable,
                           1,
                           HAL_MAX_DELAY);

    HAL_Delay(1);

    /*
     * EN = LOW
     */
    HAL_I2C_Master_Transmit(&hi2c1,
                           lcd_address,
                           &lcd_data,
                           1,
                           HAL_MAX_DELAY);

    HAL_Delay(1);


    /*
     * LOW NIBBLE
     */
    lcd_data = low_nibble | 0x01 | 0x08;

    /*
     * EN = HIGH
     */
    lcd_enable = lcd_data | 0x04;

    HAL_I2C_Master_Transmit(&hi2c1,
                           lcd_address,
                           &lcd_enable,
                           1,
                           HAL_MAX_DELAY);

    HAL_Delay(1);

    /*
     * EN = LOW
     */
    HAL_I2C_Master_Transmit(&hi2c1,
                           lcd_address,
                           &lcd_data,
                           1,
                           HAL_MAX_DELAY);

    HAL_Delay(1);
}


/* Send string */
void LCD_SendString(char *str)
{
    while (*str)
    {
        LCD_SendData((uint8_t)*str);

        str++;
    }
}


/* Set cursor position */
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


/* Clear LCD */
void LCD_Clear(void)
{
    LCD_SendCommand(0x01);

    HAL_Delay(2);
}


/*
 * Find I2C LCD address
 *
 * PCF8574:
 * 0x20 - 0x27
 *
 * PCF8574A:
 * 0x38 - 0x3F
 */
uint8_t LCD_FindAddress(void)
{
    uint8_t address;


    /*
     * PCF8574
     */
    for (address = 0x20; address <= 0x27; address++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1,
                                  address << 1,
                                  2,
                                  100) == HAL_OK)
        {
            return (address << 1);
        }
    }


    /*
     * PCF8574A
     */
    for (address = 0x38; address <= 0x3F; address++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1,
                                  address << 1,
                                  2,
                                  100) == HAL_OK)
        {
            return (address << 1);
        }
    }


    /*
     * LCD not found
     */
    return 0;
}


/* Initialize LCD */
void LCD_Init(void)
{
    HAL_Delay(50);


    /*
     * LCD initialization
     */

    LCD_Send4Bits(0x30);

    HAL_Delay(5);


    LCD_Send4Bits(0x30);

    HAL_Delay(1);


    LCD_Send4Bits(0x30);

    HAL_Delay(1);


    LCD_Send4Bits(0x20);

    HAL_Delay(1);


    /*
     * 4-bit mode
     * 2 lines
     * 5x8 font
     */
    LCD_SendCommand(0x28);


    /*
     * Display ON
     * Cursor OFF
     * Blink OFF
     */
    LCD_SendCommand(0x0C);


    /*
     * Entry mode
     */
    LCD_SendCommand(0x06);


    /*
     * Clear display
     */
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

    /*
     * Resistance in ohms
     */
    uint32_t resistance;

    /*
     * Resistance split into kΩ and Ω
     *
     * Example:
     * 9853 Ω
     *
     * whole_kohms = 9
     * remainder   = 853
     */
    uint32_t whole_kohms;
    uint32_t remainder_ohms;

    char lcd_line1[17];
    char lcd_line2[17];

    /* USER CODE END 1 */


    /* MCU Configuration--------------------------------------------------------*/

    HAL_Init();


    /* Configure system clock */
    SystemClock_Config();


    /* Initialize all configured peripherals */
    MX_GPIO_Init();

    MX_ADC2_Init();

    MX_I2C1_Init();


    /* USER CODE BEGIN 2 */


    /*
     * Find LCD address
     */
    lcd_address = LCD_FindAddress();


    /*
     * Initialize LCD if detected
     */
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
        /*
         * ============================================================
         * START ADC
         * ============================================================
         */

        HAL_ADC_Start(&hadc2);


        /*
         * Wait for ADC conversion
         */
        if (HAL_ADC_PollForConversion(&hadc2, 10) == HAL_OK)
        {
            /*
             * Read ADC value
             *
             * Range:
             *
             * 0
             * to
             * 4095
             */
            raw_adc = HAL_ADC_GetValue(&hadc2);


            /*
             * ========================================================
             * CALCULATE RESISTANCE
             * ========================================================
             *
             * B10K = 10,000 ohms
             *
             * Resistance =
             *
             * ADC value
             * ---------------- × 10000
             *     4095
             *
             *
             * Example:
             *
             * ADC = 2048
             *
             * Resistance =
             *
             * 2048 × 10000
             * -------------
             *     4095
             *
             * ≈ 5001 ohms
             */

            resistance =
                (raw_adc * POT_TOTAL_RESISTANCE)
                / ADC_MAX_VALUE;


            /*
             * Separate kΩ and Ω
             *
             * Example:
             *
             * 9853 Ω
             *
             * = 9 kΩ + 853 Ω
             */
            whole_kohms = resistance / 1000;

            remainder_ohms = resistance % 1000;


            /*
             * ========================================================
             * LCD LINE 1
             * ========================================================
             *
             * Example:
             *
             * ADC: 4035
             */

            snprintf(lcd_line1,
                     sizeof(lcd_line1),
                     "ADC: %lu",
                     raw_adc);


            /*
             * ========================================================
             * LCD LINE 2
             * ========================================================
             *
             * Example:
             *
             * R: 9.853 kOhm
             */

            snprintf(lcd_line2,
                     sizeof(lcd_line2),
                     "R: %lu.%03lu kOhm",
                     whole_kohms,
                     remainder_ohms);


            /*
             * ========================================================
             * DISPLAY ON LCD
             * ========================================================
             */

            if (lcd_address != 0)
            {
                /*
                 * First line
                 */
                LCD_SetCursor(0, 0);

                LCD_SendString("                ");

                LCD_SetCursor(0, 0);

                LCD_SendString(lcd_line1);


                /*
                 * Second line
                 */
                LCD_SetCursor(1, 0);

                LCD_SendString("                ");

                LCD_SetCursor(1, 0);

                LCD_SendString(lcd_line2);
            }
        }


        /*
         * Update every 500 ms
         */
        HAL_Delay(500);
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


    /*
     * Configure regulator
     */
    HAL_PWREx_ControlVoltageScaling(
        PWR_REGULATOR_VOLTAGE_SCALE1
    );


    /*
     * HSI oscillator
     */
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


    /*
     * Clock configuration
     */
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


    /*
     * ADC2
     */
    hadc2.Instance = ADC2;


    /*
     * ADC clock
     */
    hadc2.Init.ClockPrescaler =
        ADC_CLOCK_SYNC_PCLK_DIV2;


    /*
     * 12-bit ADC
     */
    hadc2.Init.Resolution =
        ADC_RESOLUTION_12B;


    /*
     * Right aligned
     */
    hadc2.Init.DataAlign =
        ADC_DATAALIGN_RIGHT;


    hadc2.Init.GainCompensation = 0;


    /*
     * Single channel
     */
    hadc2.Init.ScanConvMode =
        ADC_SCAN_DISABLE;


    /*
     * End of conversion
     */
    hadc2.Init.EOCSelection =
        ADC_EOC_SINGLE_CONV;


    hadc2.Init.LowPowerAutoWait =
        DISABLE;


    /*
     * One conversion at a time
     */
    hadc2.Init.ContinuousConvMode =
        DISABLE;


    hadc2.Init.NbrOfConversion = 1;


    hadc2.Init.DiscontinuousConvMode =
        DISABLE;


    /*
     * Software start
     */
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


    /*
     * PA1 = ADC2_IN2
     */
    sConfig.Channel =
        ADC_CHANNEL_2;


    sConfig.Rank =
        ADC_REGULAR_RANK_1;


    sConfig.SamplingTime =
        ADC_SAMPLETIME_2CYCLES_5;


    sConfig.SingleDiff =
        ADC_SINGLE_ENDED;


    sConfig.OffsetNumber =
        ADC_OFFSET_NONE;


    sConfig.Offset = 0;


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
     * I2C1
     *
     * PB6 = SCL
     * PB7 = SDA
     */

    hi2c1.Instance = I2C1;


    hi2c1.Init.Timing =
        0x00303D5B;


    hi2c1.Init.OwnAddress1 = 0;


    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;


    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;


    hi2c1.Init.OwnAddress2 = 0;


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


    /*
     * Analog filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(
            &hi2c1,
            I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }


    /*
     * Digital filter
     */
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
    /*
     * Enable GPIOA clock
     */
    __HAL_RCC_GPIOA_CLK_ENABLE();


    /*
     * Enable GPIOB clock
     */
    __HAL_RCC_GPIOB_CLK_ENABLE();
}


/**
  * @brief Error Handler
  */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}


#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
}

#endif

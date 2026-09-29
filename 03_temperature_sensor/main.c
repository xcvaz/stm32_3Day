/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : LM75 Temperature Sensor with I2C LCD
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* I2C1 = LCD */
#define LCD_ADDR        (0x27 << 1)

/* I2C2 = LM75 */
#define LM75_ADDR       (0x48 << 1)

/* LCD PCF8574 control bits */
#define LCD_RS          0x01
#define LCD_RW          0x02
#define LCD_EN          0x04
#define LCD_BACKLIGHT   0x08

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);

/* USER CODE BEGIN PFP */

/* LCD functions */
void LCD_Write4Bits(uint8_t data);
void LCD_SendCommand(uint8_t command);
void LCD_SendData(uint8_t data);
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t column);
void LCD_Print(char *str);

/* LM75 function */
int16_t LM75_ReadTemperature(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* -------------------------------------------------------------------------- */
/* LCD LOW LEVEL FUNCTION                                                    */
/* -------------------------------------------------------------------------- */

void LCD_Write4Bits(uint8_t data)
{
    uint8_t buffer[2];

    /*
     * Send data with EN = 1
     */
    buffer[0] = data | LCD_EN | LCD_BACKLIGHT;

    HAL_I2C_Master_Transmit(&hi2c1,
                            LCD_ADDR,
                            buffer,
                            1,
                            100);

    /*
     * Send data with EN = 0
     */
    buffer[0] = data | LCD_BACKLIGHT;

    HAL_I2C_Master_Transmit(&hi2c1,
                            LCD_ADDR,
                            buffer,
                            1,
                            100);
}

/* -------------------------------------------------------------------------- */
/* LCD COMMAND                                                               */
/* -------------------------------------------------------------------------- */

void LCD_SendCommand(uint8_t command)
{
    uint8_t upper;
    uint8_t lower;

    upper = command & 0xF0;
    lower = (command << 4) & 0xF0;

    LCD_Write4Bits(upper);
    LCD_Write4Bits(lower);

    HAL_Delay(1);
}

/* -------------------------------------------------------------------------- */
/* LCD DATA                                                                  */
/* -------------------------------------------------------------------------- */

void LCD_SendData(uint8_t data)
{
    uint8_t upper;
    uint8_t lower;
    uint8_t buffer[1];

    upper = data & 0xF0;
    lower = (data << 4) & 0xF0;

    /*
     * Upper nibble
     */

    buffer[0] = upper | LCD_RS | LCD_EN | LCD_BACKLIGHT;

    HAL_I2C_Master_Transmit(&hi2c1,
                            LCD_ADDR,
                            buffer,
                            1,
                            100);

    buffer[0] = upper | LCD_RS | LCD_BACKLIGHT;

    HAL_I2C_Master_Transmit(&hi2c1,
                            LCD_ADDR,
                            buffer,
                            1,
                            100);

    /*
     * Lower nibble
     */

    buffer[0] = lower | LCD_RS | LCD_EN | LCD_BACKLIGHT;

    HAL_I2C_Master_Transmit(&hi2c1,
                            LCD_ADDR,
                            buffer,
                            1,
                            100);

    buffer[0] = lower | LCD_RS | LCD_BACKLIGHT;

    HAL_I2C_Master_Transmit(&hi2c1,
                            LCD_ADDR,
                            buffer,
                            1,
                            100);
}

/* -------------------------------------------------------------------------- */
/* LCD INITIALIZATION                                                         */
/* -------------------------------------------------------------------------- */

void LCD_Init(void)
{
    HAL_Delay(50);

    /*
     * LCD startup sequence.
     * The LCD must first be placed into 4-bit mode.
     */

    LCD_Write4Bits(0x30);
    HAL_Delay(5);

    LCD_Write4Bits(0x30);
    HAL_Delay(1);

    LCD_Write4Bits(0x30);
    HAL_Delay(1);

    LCD_Write4Bits(0x20);
    HAL_Delay(10);

    /*
     * Function set:
     *
     * 4-bit mode
     * 2 lines
     * 5x8 font
     */
    LCD_SendCommand(0x28);

    /*
     * Display OFF
     */
    LCD_SendCommand(0x08);

    /*
     * Clear display
     */
    LCD_SendCommand(0x01);

    HAL_Delay(2);

    /*
     * Entry mode:
     * Cursor moves right
     */
    LCD_SendCommand(0x06);

    /*
     * Display ON
     * Cursor OFF
     * Blink OFF
     */
    LCD_SendCommand(0x0C);
}

/* -------------------------------------------------------------------------- */
/* LCD CLEAR                                                                 */
/* -------------------------------------------------------------------------- */

void LCD_Clear(void)
{
    LCD_SendCommand(0x01);

    HAL_Delay(2);
}

/* -------------------------------------------------------------------------- */
/* LCD SET CURSOR                                                             */
/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */
/* LCD PRINT                                                                  */
/* -------------------------------------------------------------------------- */

void LCD_Print(char *str)
{
    while (*str != '\0')
    {
        LCD_SendData((uint8_t)*str);

        str++;
    }
}

/* -------------------------------------------------------------------------- */
/* LM75 TEMPERATURE READING                                                   */
/* -------------------------------------------------------------------------- */

int16_t LM75_ReadTemperature(void)
{
    uint8_t data[2];

    int16_t raw_temperature;

    /*
     * Read LM75 temperature register.
     *
     * Register address = 0x00
     */

    if (HAL_I2C_Mem_Read(&hi2c2,
                         LM75_ADDR,
                         0x00,
                         I2C_MEMADD_SIZE_8BIT,
                         data,
                         2,
                         100) != HAL_OK)
    {
        /*
         * Error value
         */
        return -999;
    }

    /*
     * Combine MSB and LSB.
     */
    raw_temperature =
            (int16_t)((data[0] << 8) | data[1]);

    /*
     * LM75 standard temperature format:
     *
     * D15-D7 = temperature
     * D6-D0  = unused
     *
     * Each unit after shifting represents 0.5 °C.
     */

    raw_temperature = raw_temperature >> 7;

    /*
     * Example:
     *
     * 25.0 °C -> 50
     * 25.5 °C -> 51
     * 26.0 °C -> 52
     *
     * Therefore this function returns
     * temperature in units of 0.5 °C.
     */

    return raw_temperature;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */

    int16_t temperature;
    int16_t integer_part;
    uint8_t decimal_part;

    /* USER CODE END 1 */

    /* MCU Configuration------------------------------------------------------*/

    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize GPIO */
    MX_GPIO_Init();

    /* Initialize I2C1 */
    MX_I2C1_Init();

    /* Initialize I2C2 */
    MX_I2C2_Init();

    /* USER CODE BEGIN 2 */

    /*
     * Initialize LCD on I2C1
     */
    LCD_Init();

    /*
     * Startup message
     */
    LCD_Clear();

    LCD_SetCursor(0, 0);
    LCD_Print("LM75 Temperature");

    LCD_SetCursor(1, 0);
    LCD_Print("Initializing...");

    HAL_Delay(1500);

    LCD_Clear();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    while (1)
    {
        /*
         * Read LM75.
         *
         * Example returned values:
         *
         * 50 = 25.0 C
         * 51 = 25.5 C
         * 52 = 26.0 C
         */
        temperature = LM75_ReadTemperature();

        /*
         * Check for communication error
         */
        if (temperature == -999)
        {
            LCD_Clear();

            LCD_SetCursor(0, 0);
            LCD_Print("LM75 ERROR");

            LCD_SetCursor(1, 0);
            LCD_Print("Check I2C2");

            HAL_Delay(1000);

            continue;
        }

        /*
         * Clear LCD
         */
        LCD_Clear();

        /*
         * First line
         */
        LCD_SetCursor(0, 0);

        LCD_Print("Temperature:");

        /*
         * Move to second line
         */
        LCD_SetCursor(1, 0);

        /*
         * Handle negative temperature
         */
        if (temperature < 0)
        {
            LCD_SendData('-');

            temperature = -temperature;
        }

        /*
         *
         * Convert from 0.5 degree units
         *
         * Example:
         *
         * 51 / 2 = 25
         * 51 % 2 = 1
         *
         * Therefore:
         *
         * 25.5 C
         */

        integer_part = temperature / 2;

        if ((temperature % 2) == 0)
        {
            decimal_part = 0;
        }
        else
        {
            decimal_part = 5;
        }

        /*
         * Display integer part
         */

        if (integer_part >= 100)
        {
            LCD_SendData((integer_part / 100) + '0');

            LCD_SendData(((integer_part / 10) % 10) + '0');

            LCD_SendData((integer_part % 10) + '0');
        }
        else if (integer_part >= 10)
        {
            LCD_SendData((integer_part / 10) + '0');

            LCD_SendData((integer_part % 10) + '0');
        }
        else
        {
            LCD_SendData(integer_part + '0');
        }

        /*
         * Decimal point
         */
        LCD_SendData('.');

        /*
         * Decimal digit
         */
        LCD_SendData(decimal_part + '0');

        /*
         * Degree symbol cannot be reliably printed
         * without creating a custom LCD character,
         * so use C.
         */
        LCD_Print(" C");

        /*
         * Update every 1 second
         */
        HAL_Delay(1000);
    }

    /* USER CODE END WHILE */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ControlVoltageScaling(
        PWR_REGULATOR_VOLTAGE_SCALE1);

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

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_0) != HAL_OK)
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
    hi2c1.Instance = I2C1;

    hi2c1.Init.Timing = 0x00303D5B;

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

    if (HAL_I2CEx_ConfigAnalogFilter(
            &hi2c1,
            I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_I2CEx_ConfigDigitalFilter(
            &hi2c1,
            0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief I2C2 Initialization Function
  * @retval None
  */
static void MX_I2C2_Init(void)
{
    hi2c2.Instance = I2C2;

    hi2c2.Init.Timing = 0x00303D5B;

    hi2c2.Init.OwnAddress1 = 0;

    hi2c2.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;

    hi2c2.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;

    hi2c2.Init.OwnAddress2 = 0;

    hi2c2.Init.OwnAddress2Masks =
        I2C_OA2_NOMASK;

    hi2c2.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;

    hi2c2.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c2) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_I2CEx_ConfigAnalogFilter(
            &hi2c2,
            I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_I2CEx_ConfigDigitalFilter(
            &hi2c2,
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
    /* GPIO Ports Clock Enable */

    __HAL_RCC_GPIOF_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
}

/**
  * @brief  This function is executed in case of error occurrence.
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
  * @brief  Reports the name of the source file and line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */

    /* USER CODE END 6 */
}

#endif /* USE_FULL_ASSERT */

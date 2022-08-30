#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "TaskMngr.h"
#include "logging.h"
#include "PlatformSpecific.h"
#include "String.h"

#include "st7789.h"

SPI_HandleTypeDef hspi1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

void ledOFF() {
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}
void ledON() {
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}
void blinkLED() {
	ledON();
	SetTimerTask((TaskMng)ledOFF, 0, NULL, TICK_PER_SECOND);
}

static u16 currentBackground = BLACK;
static u16 currentColor[2] = {WHITE, RED};

void updateDisplay() {
	static Date_t prev = {0};
	static u16 prevBackGround = BLACK;
	static u16 prevFontColor = WHITE;
	Date_t current = getDateFromSeconds(getAllSeconds(), TRUE);
	char dateStr[19] = {0};
	dateToString(dateStr, &current);
	strSplit(' ', dateStr);
	if(prevBackGround != currentBackground) {
		ST7789_Fill_Color(currentBackground);
	}
	if(current.year != prev.year || current.mon != prev.mon || current.day != prev.day || prevFontColor != currentColor[0]) {
		prev.year = current.year;
		prev.mon = current.mon;
		prev.day = current.day;
		ST7789_WriteString(20,20, dateStr, Font_16x26, currentColor[0], currentBackground);
	}
	if(current.hour != prev.hour || current.min != prev.min || prevFontColor != currentColor[0]) {
		prev.hour = current.hour;
		prev.min = current.min;
		char *timeStr = dateStr + strSize(dateStr) + 1;
		timeStr[6] = END_STRING;
		ST7789_WriteString(35,60, timeStr, Font_16x26, currentColor[0], currentBackground);
	}
	ST7789_WriteString(132,60, dateStr+15, Font_16x26, currentColor[1], currentBackground);
	prevBackGround = currentBackground;
	prevFontColor = currentColor[0];
}


void setBackground(BaseSize_t n, string_t arguments) {
	if(n != 1) {
		writeLogStr("ERROR! Required exact one argument hex code of color for background");
		return;
	}
	currentBackground = (u16)toInt32(arguments);
	writeLogWithStr("Set background color to", currentBackground);
	execCallBack(setBackground);
}

void setFontColor(BaseSize_t n, string_t arguments) {
	if(n > 2 || n < 1) {
		writeLogStr("ERROR! The maximum arguments must be 2 parameter. For main font color and for the seconds");
		return;
	}
	currentColor[0] = (u16)toInt32(arguments);
	if(n == 2) {
		currentColor[1] = (u16)toInt32(arguments+strSize(arguments)+1);
	}
	writeLogWithStr("Set main font color to", currentColor[0]);
	writeLogWithStr("Set second font color to", currentColor[1]);
	execCallBack(setFontColor);
}

void standWithUkraine(BaseSize_t arg_n, BaseParam_t arg_p) {
	ST7789_WriteString(0, 180, "Stand with UKRAINE", Font_11x18, currentColor[0], currentBackground);
	ST7789_DrawFilledRectangle(180, 200, 60, 20, BLUE);
	ST7789_DrawFilledRectangle(180, 220, 60, 20, YELLOW);
}

void disableDisplay(BaseSize_t arg_n, BaseParam_t arg_p) {
	delCycleTask(arg_n, updateDisplay);
	HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_RESET);
	execCallBack(disableDisplay);
}

void enableDisplay(BaseSize_t n, BaseParam_t arguments) {
	Time_t seconds = 10;
	if(n>0) {
		seconds = toIntDec(arguments);
	}
	HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_SET);
	SetCycleTask(TICK_PER_SECOND, updateDisplay, TRUE);
	SetTimerTask(disableDisplay,0, NULL, seconds*TICK_PER_SECOND);
	execCallBack(enableDisplay);
}

void showColors() {
	u16 colors[] = {
		WHITE,
		BLACK,
		BLUE,
		RED,
		MAGENTA,
		GREEN,
		CYAN,
		YELLOW,
		GRAY,
		BRED,
		GRED,
		GBLUE,
		BROWN,
		BRRED,
		DARKBLUE,
		LIGHTBLUE,
		GRAYBLUE,
		LIGHTGREEN,
		LGRAY,
		LGRAYBLUE,
		LBBLUE,
	};
	const string_t colorsStr[] = {
		"WHITE",
		"BLACK",
		"BLUE",
		"RED",
		"MAGENTA",
		"GREEN",
		"CYAN",
		"YELLOW",
		"GRAY",
		"BRED",
		"GRED",
		"GBLUE",
		"BROWN",
		"BRRED",
		"DARKBLUE",
		"LIGHTBLUE",
		"GRAYBLUE",
		"LIGHTGREEN",
		"LGRAY",
		"LGRAYBLUE",
		"LBBLUE",
	};
	writeLogStr("COLORS TABLE");
	writeLogStr("================================\n");
	for(u08 i=0; i<sizeof(colors)/sizeof(colors[0]); i++) {
		writeLogWithStr(colorsStr[i], colors[i]);
	}
}

int main(void){
	HAL_Init();

	SystemClock_Config();
	MX_GPIO_Init();
	MX_SPI1_Init();
	ST7789_Init();
	MX_USB_DEVICE_Init();
	initFemtOS();
	enableLogging();
	ST7789_Fill_Color(currentBackground);
	initStandardConsoleCommands();
	initWatchDog();
	SetIdleTask(idle);
	SetCycleTask(TICK_PER_SECOND>>1, resetWatchDog, TRUE);
	SetTask(standWithUkraine, 0, NULL);
	addTaskCommand((TaskMng)setBackground, "back", "Set background color from 0000 to FFFF");
	addTaskCommand((TaskMng)setFontColor, "font", "Set font color from 0000 to FFFF and for seconds also 0000 to FFFF");
	addTaskCommand((TaskMng)enableDisplay, "showDisplay", "Enable display to get information by n seconds");
	addTaskCommand((TaskMng)showColors, "showCollors", "Display most popular color list");
	runFemtOS();
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 1;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
//  __HAL_RCC_GPIOC_CLK_ENABLE();
//  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DATA_DIRECT_GPIO_Port, DATA_DIRECT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_RESET_Pin | LCD_BLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DATA_DIRECT_Pin */
  GPIO_InitStruct.Pin = DATA_DIRECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DATA_DIRECT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RESET_Pin LCD_BLK_Pin */
  GPIO_InitStruct.Pin = LCD_RESET_Pin|LCD_BLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void){
	MaximizeErrorHandler("Error");
}

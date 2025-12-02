/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
#include "powerToAngle.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct{
	//rising and falling times in seconds
	int risingTime;
	int fallingTime;
}ramps_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PULSE_WIDTH 1000	//pulse width in TIM1's clock pulses (ARR = 58100)
#define TRIGGER_ARR 58100
#define RX_BUFFER_SIZE 4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim10;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
ramps_t ramps;
uint32_t time_ms = 0;		//stopwatch

uint32_t triggerCCR;

uint8_t rx;				//byte received from UART
uint8_t rxBuffer[RX_BUFFER_SIZE];	//stores data received from UART
uint8_t rxFlag = 0;		//flips to 1 when DMA request is complete (a message has been received)
uint8_t inputSize;	//the size of the next expected input
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM10_Init(void);
/* USER CODE BEGIN PFP */
void menu();
void setPr();
void startRamp();
void rampSettings();
void setRampTime(int *rampTime);

void setTriggerCCR(uint32_t newTrigger);

void clearRxBuffer();
void appendRxBuffer(uint8_t data);
void backspaceRxBuffer();
char isInputComplete();
int extractNumber();	//convert the byte sequence buffer (number written in ASCII) to integer
void receiveInput(int size);

int len(uint8_t *s);	//returns the length of a string
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */
  triggerCCR = TIM1->CCR2;	//initialize triggerCCR variable
  setTriggerCCR(TRIGGER_ARR);

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);	//start trigger PWM

  //clear buffer and start listening to UART
  clearRxBuffer();
  HAL_UART_Receive_DMA(&huart2, &rx, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  menu();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = PSC_7MHz;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = ARR_8300us;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim1, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM2;
  sConfigOC.Pulse = 58100;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = (84-1);
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = (1000-1);
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : alertSlow_Pin alertStop_Pin */
  GPIO_InitStruct.Pin = alertSlow_Pin|alertStop_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void menu(){
	uint8_t message[] = {"\r\n\nCommands:\n\r\t'P': set power ratio\n\r\t'S': start ramp\n\r\t'R': ramp settings\n\rType the command: "};

	HAL_UART_Transmit(&huart2, message, len(message), HAL_MAX_DELAY);	//print menu and prompt
	receiveInput(1);	//1 byte long input expected

	switch(rxBuffer[0]){
	case 'p':
		setPr();
		break;
	case 's':
		startRamp();
		break;
	case 'r':
		rampSettings();
		break;
	}
}

void setPr(){
	uint8_t message[] = {"\r\n\nInsert desired Power Ratio (in percentage): "};
	float Pr, Sr, triggerAngle;
	uint16_t triggerCCR;

	HAL_UART_Transmit(&huart2, message, len(message), HAL_MAX_DELAY);	//print prompt
	receiveInput(3);	//3 bytes long input expected

	Sr = (float)extractNumber()/100;
	Pr = speedToPower(Sr);
	triggerAngle = getTrigger(Pr);
	triggerCCR = (uint16_t)angleToCCR(triggerAngle, TRIGGER_ARR);
	setTriggerCCR(triggerCCR);
}

void startRamp(){
	uint8_t risingMsg[] = {"\r\n\nRising ramp started"};
	uint8_t fullPowerMsg[] = {"\r\nFull power\r\nType 'enter' to start falling ramp"};
	uint8_t fallingMsg[] = {"\r\nFalling ramp started"};
	uint8_t offMsg[] = {"\r\nMotor is off"};
	float Pr, Sr, triggerAngle, time_s;
	uint16_t triggerCCR;

	HAL_UART_Transmit(&huart2, risingMsg, len(risingMsg), HAL_MAX_DELAY);	//print that rising has begun

	//rising ramp
	HAL_TIM_Base_Start_IT(&htim10);	//start stopwatch
	do{
		//constantly update trigger angle according to the stopwatch
		time_s = ((float)(time_ms))/1000;
		Sr = ((float)time_s)/ramps.risingTime;	//percentage of time elapsed (100% is the rising time) is the SpeedRatio
		Pr = speedToPower(Sr);
		triggerAngle = getTrigger(Pr);
		triggerCCR = (uint16_t)angleToCCR(triggerAngle, TRIGGER_ARR);
		setTriggerCCR(triggerCCR);
	}while(time_s<ramps.risingTime);	//stop when rising time is complete
	HAL_TIM_Base_Stop_IT(&htim10);
	time_ms = 0;

	HAL_UART_Transmit(&huart2, fullPowerMsg, len(fullPowerMsg), HAL_MAX_DELAY);	//print full power indicator
	receiveInput(1);	//wait for any user input

	HAL_UART_Transmit(&huart2, fallingMsg, len(fallingMsg), HAL_MAX_DELAY);	//print that falling has begun

	//falling ramp
	HAL_TIM_Base_Start_IT(&htim10);	//start stopwatch
	do{
		//constantly update trigger angle according to the stopwatch
		time_s = ((float)(time_ms))/1000;
		Sr = ((float)(ramps.fallingTime-time_s))/ramps.fallingTime;	//percentage of time remaining to falling time is the SpeedRatio
		Pr = speedToPower(Sr);
		triggerAngle = getTrigger(Pr);
		triggerCCR = (uint16_t)angleToCCR(triggerAngle, TRIGGER_ARR);
		setTriggerCCR(triggerCCR);
	}while(time_s<ramps.fallingTime);
	HAL_TIM_Base_Stop_IT(&htim10);
	time_ms = 0;

	HAL_UART_Transmit(&huart2, offMsg, len(offMsg), HAL_MAX_DELAY);	//print that motor is off
}

void rampSettings(){
	uint8_t message[] = {"\r\n\nRising ramp ('R')\n\rFalling ramp ('F')\n\rSelect an option: "};

	HAL_UART_Transmit(&huart2, message, len(message), HAL_MAX_DELAY);
	receiveInput(1);	//1 byte long input expected

	switch(rxBuffer[0]){
	case 'r':
		setRampTime(&(ramps.risingTime));
		break;
	case 'f':
		setRampTime(&(ramps.fallingTime));
		break;
	}
}

void setRampTime(int *rampTime){
	uint8_t message[] = {"\r\nInsert ramp time in seconds (max 3 digits): "};

	HAL_UART_Transmit(&huart2, message, len(message), HAL_MAX_DELAY);
	receiveInput(3);	//3 bytes long input expected

	*rampTime = extractNumber();
}

void setTriggerCCR(uint32_t newTrigger){
	TIM1->CCR2 = newTrigger;
	triggerCCR = newTrigger;
}

void clearRxBuffer(){
	for(int i=0; i<RX_BUFFER_SIZE; i++){
		rxBuffer[i] = '\0';
	}
}

void appendRxBuffer(uint8_t data){
	int i=0;

	while(rxBuffer[i]!='\0'){
		i++;
		if(i==RX_BUFFER_SIZE)	return;	//do nothing if buffer is full
	}

	rxBuffer[i] = rx;
}

void backspaceRxBuffer(){
	uint8_t skipLine[] = {"\r\n"};

	int i;

	for(i=RX_BUFFER_SIZE; rxBuffer[i]=='\0'; i--);	//run from end to front in the buffer up to the last character

	rxBuffer[i] = '\0';	//delete the last character in buffer

	//print rxBuffer in next line
	HAL_UART_Transmit(&huart2, skipLine, len(skipLine), HAL_MAX_DELAY);
	i = 0;
	while(rxBuffer[i]!='\0'){
		HAL_UART_Transmit(&huart2, &rxBuffer[i], 1, HAL_MAX_DELAY);
		i++;
	}
}

char isInputComplete(){
	int i=0;

	while(rxBuffer[i]!='\0'){
		i++;
		if(i==inputSize)	return 'c';	//return that input is complete (true)
	}

	return '\0';	//if there are bytes remaining, return false
}

int extractNumber(){
	int num = 0, exponent = 0, i=RX_BUFFER_SIZE-1;

	//run from end to start in the buffer, until data is found
	while(rxBuffer[i]=='\0'){
		i--;
	}

	for(i=i; i>=0; i--){
		num += (rxBuffer[i]-'0')*pow(10, exponent);
		exponent++;
	}

	return num;
}

void receiveInput(int size){
	clearRxBuffer();
	inputSize = size;	//'size' byte long input expected

	//wait for an input
	while(!rxFlag);
	rxFlag = 0;
}

int len(uint8_t *s){
	int size = 0;

	while(*s != '\0'){
		size++;
		s++;
	}

	return size;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(rx!='\177'){
		HAL_UART_Transmit(huart, &rx, 1, HAL_MAX_DELAY);	//echo if key typed was not backspace/del
	}
	//allow user to delete last character, if they typed backspace/del
	else{
		backspaceRxBuffer();
		return;
	}

	//flip flag if user typed 'enter'
	if(rx=='\r'){
		rxFlag++;
		return;
	}

	appendRxBuffer(rx);

	//flip flag when input has been completely received
	if(isInputComplete())	rxFlag++;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM10){
		time_ms++;	//increment stopwatch by 1 milisecond
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	uint8_t stopMsg[] = {"\r\n>1A"};
	uint8_t slowMsg[] = {"\r\n>750mA"};

	if(GPIO_Pin == alertStop_Pin){
		HAL_UART_Transmit(&huart2, stopMsg, len(stopMsg), HAL_MAX_DELAY);
	}
	else if(GPIO_Pin == alertSlow_Pin){
		HAL_UART_Transmit(&huart2, slowMsg, len(slowMsg), HAL_MAX_DELAY);
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

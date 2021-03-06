/**
* \file
*
* \brief Example of usage of the maXTouch component with USART
*
* This example shows how to receive touch data from a maXTouch device
* using the maXTouch component, and display them in a terminal window by using
* the USART driver.
*
* Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
*
* \asf_license_start
*
* \page License
*
* Subject to your compliance with these terms, you may use Microchip
* software and any derivatives exclusively with Microchip products.
* It is your responsibility to comply with third party license terms applicable
* to your use of third party software (including open source software) that
* may accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
* INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
* AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
* LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
* LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
* SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
* POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
* ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
* RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*
* \asf_license_stop
*
*/

/**
* \mainpage
*
* \section intro Introduction
* This simple example reads data from the maXTouch device and sends it over
* USART as ASCII formatted text.
*
* \section files Main files:
* - example_usart.c: maXTouch component USART example file
* - conf_mxt.h: configuration of the maXTouch component
* - conf_board.h: configuration of board
* - conf_clock.h: configuration of system clock
* - conf_example.h: configuration of example
* - conf_sleepmgr.h: configuration of sleep manager
* - conf_twim.h: configuration of TWI driver
* - conf_usart_serial.h: configuration of USART driver
*
* \section apiinfo maXTouch low level component API
* The maXTouch component API can be found \ref mxt_group "here".
*
* \section deviceinfo Device Info
* All UC3 and Xmega devices with a TWI module can be used with this component
*
* \section exampledescription Description of the example
* This example will read data from the connected maXTouch explained board
* over TWI. This data is then processed and sent over a USART data line
* to the board controller. The board controller will create a USB CDC class
* object on the host computer and repeat the incoming USART data from the
* main controller to the host. On the host this object should appear as a
* serial port object (COMx on windows, /dev/ttyxxx on your chosen Linux flavour).
*
* Connect a terminal application to the serial port object with the settings
* Baud: 57600
* Data bits: 8-bit
* Stop bits: 1 bit
* Parity: None
*
* \section compinfo Compilation Info
* This software was written for the GNU GCC and IAR for AVR.
* Other compilers may or may not work.
*
* \section contactinfo Contact Information
* For further information, visit
* <A href="http://www.atmel.com/">Atmel</A>.\n
*/
/*
* Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
*/

/* includes */

#include <asf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "conf_board.h"
#include "conf_example.h"
#include "conf_uart_serial.h"

#include "tfont.h"
#include "sourcecodepro_28.h"
#include "calibri_36.h"
#include "ioport.h"

#include "icons/Play.h"
#include "icons/Pause.h"
#include "icons/next.h"
#include "icons/prev.h"
#include "icons/cent.h"
#include "icons/day.h"
#include "icons/strong.h"
#include "icons/enx.h"
#include "icons/fast.h"
#include "icons/porta.h"
#include "icons/lock.h"
#include "icons/unlock.h"
#include "icons/ON.h"
#include "maquina1.h"

/* DEFINES */

#define MAX_ENTRIES        3
#define STRING_LENGTH     70
#define USART_TX_MAX_LENGTH     0xff

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

#define PORTA_PIO      PIOA
#define PORTA_PIO_ID   ID_PIOA
#define PORTA_IDX      19
#define PORTA_IDX_MASK (1 << PORTA_IDX)

#define SAFE_PIO      PIOA
#define SAFE_PIO_ID   ID_PIOA
#define SAFE_IDX      0
#define SAFE_IDX_MASK (1 << SAFE_IDX)

#define YEAR        2018
#define MOUNT       3
#define DAY         19
#define WEEK        12
#define HOUR        0
#define MINUTE      0
#define SECOND      0


#define STRING_EOL    "\r\n"
#define STRING_HEADER "-- SAME70 LCD DEMO --"STRING_EOL	\
"-- "BOARD_NAME " --"STRING_EOL	\
"-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL


struct ili9488_opt_t g_ili9488_display_opt;

const uint32_t LOCK_W = 80;
const uint32_t LOCK_H = 80;
const uint32_t LOCK_BORDER = 2;
const uint32_t LOCK_X = 280;
const uint32_t LOCK_Y = 240;

const uint32_t NEXT_W = 80;
const uint32_t NEXT_H = 80;
const uint32_t NEXT_BORDER = 2;
const uint32_t NEXT_X = 280;
const uint32_t NEXT_Y = 440;

const uint32_t PREV_W = 80;
const uint32_t PREV_H = 80;
const uint32_t PREV_BORDER = 2;
const uint32_t PREV_X = 40;
const uint32_t PREV_Y = 440;

const uint32_t PLAY_W = 80;
const uint32_t PLAY_H = 80;
const uint32_t PLAY_BORDER = 2;
const uint32_t PLAY_X = 160;
const uint32_t PLAY_Y = 440;

uint32_t convert_axis_system_x(uint32_t touch_y);
uint32_t convert_axis_system_y(uint32_t touch_x);
void update_screen(uint32_t tx, uint32_t ty, uint32_t status);
static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses);
void update_timer();
void led_update();
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
void RTC_init(void);
void draw_play_pause(Bool is_on);
void font_draw_text(tFont *font, const char *text, int x, int y, int spacing);

/************************************************************************/
/* variaveis globais                                                  */
/************************************************************************/
volatile Bool porta_aberta=0;
volatile t_ciclo *ciclo; /* Inicia os ciclos */
volatile Bool is_on = 0;
volatile Bool is_locked =0;
volatile Bool safety = 0;
volatile Bool flag = 0;
volatile Bool f_rtt_alarme = false;
volatile int seg=0;
volatile int minu=0;
volatile int tempo=0;
/************************************************************************/
/* handler / callbacks                                                  */
/************************************************************************/

void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);

	/*Verifica por qual motivo entro na interrupcao, se foi por segundo ou Alarm*/
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
			rtc_clear_status(RTC, RTC_SCCR_ALRCLR);

			
	}
	
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
	
}
	
void TC1_Handler(void){
	volatile uint32_t ul_dummy;

	ul_dummy = tc_get_status(TC0, 1);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);

	led_update();
	
	if (is_on){
		seg++;
	
	if (seg==60)
	{
		seg=0;
		minu++;
	}
	update_timer();

	if (minu==tempo)
	{
		Bool temp=is_locked;
		is_locked =0;
		update_screen(PLAY_X,PLAY_Y,0x20);
		is_locked =temp;
		font_draw_text(&calibri_36, "LAVAGEM ", 60, 10, 2);
		font_draw_text(&calibri_36, "CONCLUIDA!", 60, 45, 2);
	}
	}
}

void mxt_handler(struct mxt_device *device)
{
	/* USART tx buffer initialized to 0 */
	char tx_buf[STRING_LENGTH * MAX_ENTRIES] = {0};
	uint8_t i = 0; /* Iterator */

	/* Temporary touch event data struct */
	struct mxt_touch_event touch_event;

	/* Collect touch events and put the data in a string,
	* maximum 2 events at the time */
	do {
		/* Temporary buffer for each new touch event line */
		char buf[STRING_LENGTH];
		
		/* Read next next touch event in the queue, discard if read fails */
		if (mxt_read_touch_event(device, &touch_event) != STATUS_OK) {
			continue;
		}
		
		// eixos trocados (quando na vertical LCD)
		uint32_t conv_x = convert_axis_system_x(touch_event.y);
		uint32_t conv_y = convert_axis_system_y(touch_event.x);
		
		/* Format a new entry in the data string that will be sent over USART */
		sprintf(buf, "Nr: %1d, X:%4d, Y:%4d, Status:0x%2x conv X:%3d Y:%3d\n\r",
		touch_event.id, touch_event.x, touch_event.y,
		touch_event.status, conv_x, conv_y);
		update_screen(conv_x, conv_y,touch_event.status);

		/* Add the new string to the string buffer */
		strcat(tx_buf, buf);
		i++;

		/* Check if there is still messages in the queue and
		* if we have reached the maximum numbers of events */
	} while ((mxt_is_message_pending(device)) & (i < MAX_ENTRIES));

	/* If there is any entries in the buffer, send them over USART */
	if (i > 0) {
		usart_serial_write_packet(USART_SERIAL_EXAMPLE, (uint8_t *)tx_buf, strlen(tx_buf));
	}
}

void RTT_Handler(void)
{
	uint32_t ul_status;

	/* Get RTT status */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {  }

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		
		
	}
}

void PORTA_callback(void){
	if (!is_on){
		porta_aberta = !porta_aberta;
		if (!flag){
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
			ili9488_draw_filled_rectangle(0, 0, 64,64);
			//flag=!flag;
		}
		
	}
}

void safe_callback(void){
	safety = !safety;
}
/************************************************************************/
/* inits / configs                                                */
/************************************************************************/

t_ciclo *initMenuOrder(){
	c_rapido.previous = &c_enxague;
	c_rapido.next = &c_diario;

	c_diario.previous = &c_rapido;
	c_diario.next = &c_pesado;

	c_pesado.previous = &c_diario;
	c_pesado.next = &c_enxague;

	c_enxague.previous = &c_pesado;
	c_enxague.next = &c_centrifuga;

	c_centrifuga.previous = &c_enxague;
	c_centrifuga.next = &c_rapido;

	return(&c_diario);
}

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	uint32_t channel = 1;

	/* Configura o PMC */
	/* O TimerCounter é meio confuso
	o uC possui 3 TCs, cada TC possui 3 canais
	TC0 : ID_TC0, ID_TC1, ID_TC2
	TC1 : ID_TC3, ID_TC4, ID_TC5
	TC2 : ID_TC6, ID_TC7, ID_TC8
	*/
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  4Mhz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura e ativa interrupçcão no TC canal 0 */
	/* Interrupção no C */
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);

	/* Inicializa o canal 0 do TC */
	tc_start(TC, TC_CHANNEL);
}

void RTC_init(){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(RTC, YEAR, MOUNT, DAY, WEEK);
	rtc_set_time(RTC, HOUR, MINUTE, SECOND);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 0);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(RTC,  RTC_IER_ALREN);

}

static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate =		USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength =	USART_SERIAL_CHAR_LENGTH,
		.paritytype =	USART_SERIAL_PARITY,
		.stopbits =		USART_SERIAL_PARITY,
	};

	/* Configure UART console. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init( USART_SERIAL_EXAMPLE  , &uart_serial_options);
}

static void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
	
}

static void mxt_init(struct mxt_device *device)
{
	enum status_code status;

	/* T8 configuration object data */
	uint8_t t8_object[] = {
		0x0d, 0x00, 0x05, 0x0a, 0x4b, 0x00, 0x00,
		0x00, 0x32, 0x19
	};

	/* T9 configuration object data */
	uint8_t t9_object[] = {
		0x8B, 0x00, 0x00, 0x0E, 0x08, 0x00, 0x80,
		0x32, 0x05, 0x02, 0x0A, 0x03, 0x03, 0x20,
		0x02, 0x0F, 0x0F, 0x0A, 0x00, 0x00, 0x00,
		0x00, 0x18, 0x18, 0x20, 0x20, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x02,
		0x02
	};

	/* T46 configuration object data */
	uint8_t t46_object[] = {
		0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x03,
		0x00, 0x00
	};
	
	/* T56 configuration object data */
	uint8_t t56_object[] = {
		0x02, 0x00, 0x01, 0x18, 0x1E, 0x1E, 0x1E,
		0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
		0x1E, 0x1E, 0x1E, 0x1E, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00
	};

	/* TWI configuration */
	twihs_master_options_t twi_opt = {
		.speed = MXT_TWI_SPEED,
		.chip  = MAXTOUCH_TWI_ADDRESS,
	};

	status = (enum status_code)twihs_master_setup(MAXTOUCH_TWI_INTERFACE, &twi_opt);
	Assert(status == STATUS_OK);

	/* Initialize the maXTouch device */
	status = mxt_init_device(device, MAXTOUCH_TWI_INTERFACE,
	MAXTOUCH_TWI_ADDRESS, MAXTOUCH_XPRO_CHG_PIO);
	Assert(status == STATUS_OK);

	/* Issue soft reset of maXTouch device by writing a non-zero value to
	* the reset register */
	mxt_write_config_reg(device, mxt_get_object_address(device,
	MXT_GEN_COMMANDPROCESSOR_T6, 0)
	+ MXT_GEN_COMMANDPROCESSOR_RESET, 0x01);

	/* Wait for the reset of the device to complete */
	delay_ms(MXT_RESET_TIME);

	/* Write data to configuration registers in T7 configuration object */
	mxt_write_config_reg(device, mxt_get_object_address(device,
	MXT_GEN_POWERCONFIG_T7, 0) + 0, 0x20);
	mxt_write_config_reg(device, mxt_get_object_address(device,
	MXT_GEN_POWERCONFIG_T7, 0) + 1, 0x10);
	mxt_write_config_reg(device, mxt_get_object_address(device,
	MXT_GEN_POWERCONFIG_T7, 0) + 2, 0x4b);
	mxt_write_config_reg(device, mxt_get_object_address(device,
	MXT_GEN_POWERCONFIG_T7, 0) + 3, 0x84);

	/* Write predefined configuration data to configuration objects */
	mxt_write_config_object(device, mxt_get_object_address(device,
	MXT_GEN_ACQUISITIONCONFIG_T8, 0), &t8_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
	MXT_TOUCH_MULTITOUCHSCREEN_T9, 0), &t9_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
	MXT_SPT_CTE_CONFIGURATION_T46, 0), &t46_object);
	mxt_write_config_object(device, mxt_get_object_address(device,
	MXT_PROCI_SHIELDLESS_T56, 0), &t56_object);

	/* Issue recalibration command to maXTouch device by writing a non-zero
	* value to the calibrate register */
	mxt_write_config_reg(device, mxt_get_object_address(device,
	MXT_GEN_COMMANDPROCESSOR_T6, 0)
	+ MXT_GEN_COMMANDPROCESSOR_CALIBRATE, 0x01);
}

void io_init(void)
{
	
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	
	pmc_enable_periph_clk(PORTA_PIO_ID);
	pmc_enable_periph_clk(SAFE_PIO_ID);
	
	pio_configure(PORTA_PIO, PIO_INPUT, PORTA_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(SAFE_PIO, PIO_INPUT, SAFE_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	
	pio_set_debounce_filter(PORTA_PIO, PORTA_IDX_MASK, 10);
	
	pio_enable_interrupt(PORTA_PIO, PORTA_IDX_MASK);
	pio_enable_interrupt(SAFE_PIO, SAFE_IDX_MASK);

	NVIC_EnableIRQ(PORTA_PIO_ID);
	NVIC_EnableIRQ(SAFE_PIO_ID);
	
	NVIC_SetPriority(PORTA_PIO_ID, 4); // Prioridade 4
	NVIC_SetPriority(SAFE_PIO_ID, 4);
	
	pio_handler_set(PORTA_PIO,
	PORTA_PIO_ID,
	PORTA_IDX_MASK,
	PIO_IT_RISE_EDGE,
	PORTA_callback);
	
	pio_handler_set(SAFE_PIO,
	SAFE_PIO_ID,
	SAFE_IDX_MASK,
	PIO_IT_RISE_EDGE,
	safe_callback);

}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	
	while (ul_previous_time > rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 0);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
}
/************************************************************************/
/* funções                                                              */
/************************************************************************/

static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
}

void font_draw_text(tFont *font, const char *text, int x, int y, int spacing) {
	char *p = text;
	while(*p != NULL) {
		char letter = *p;
		int letter_offset = letter - font->start_char;
		if(letter <= font->end_char) {
			tChar *current_char = font->chars + letter_offset;
			ili9488_draw_pixmap(x, y, current_char->image->width, current_char->image->height, current_char->image->data);
			x += current_char->image->width + spacing;
		}
		p++;
	}
}

void draw_screen(void) {
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
	
}

void draw_lock(uint32_t is_locked) {

	//-LOCK_W/2+LOCK_BORDER
	//ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	//ili9488_draw_filled_rectangle(LOCK_X-LOCK_W/2, LOCK_Y-LOCK_H/2, LOCK_X+LOCK_W/2, LOCK_Y+LOCK_H/2);
	if(is_locked) {
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
		ili9488_draw_filled_rectangle(LOCK_X-LOCK_W/2, LOCK_Y-LOCK_H/2, LOCK_X+LOCK_W/2, LOCK_Y+LOCK_H/2);
		ili9488_draw_pixmap(LOCK_X-32, LOCK_Y-32, unlock.width, unlock.height, unlock.data);
		
		} else {
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
		ili9488_draw_filled_rectangle(LOCK_X-LOCK_W/2, LOCK_Y-LOCK_H/2, LOCK_X+LOCK_W/2, LOCK_Y+LOCK_H/2);
		ili9488_draw_pixmap(LOCK_X-32, LOCK_Y-32, lock.width, lock.height, lock.data);
	}
	
}

void draw_next(uint32_t clicked) {
	//static uint32_t last_state = 255; // undefined
	//if(clicked == last_state) return;
	
	ciclo = ciclo->next;
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(NEXT_X-NEXT_W/2, NEXT_Y-NEXT_H/2, NEXT_X+NEXT_W/2, NEXT_Y+NEXT_H/2);
	ili9488_draw_pixmap(245, 410, next.width, next.height, next.data);
	//if(clicked) {
	//ili9488_set_foreground_color(COLOR_CONVERT(COLOR_GREEN));
	//ili9488_draw_filled_rectangle(NEXT_X-NEXT_W/2, NEXT_Y-NEXT_H/2, NEXT_X+NEXT_W/2, NEXT_Y+NEXT_H/2);
	//} else {
	//ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	//ili9488_draw_filled_rectangle(NEXT_X-NEXT_W/2, NEXT_Y-NEXT_H/2, NEXT_X+NEXT_W/2, NEXT_Y+NEXT_H/2);
	//}
	////last_state = clicked;
}

void draw_prev(uint32_t clicked) {
	//static uint32_t last_state = 255; // undefined
	//if(clicked == last_state) return;
	
	ciclo = ciclo->previous;
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(PREV_X-PREV_W/2, PREV_Y-PREV_H/2, PREV_X+PREV_W/2, PREV_Y+PREV_H/2);
	ili9488_draw_pixmap(13, 410, prev.width, prev.height, prev.data);
	//if(clicked) {
	//ili9488_set_foreground_color(COLOR_CONVERT(COLOR_GREEN));
	//ili9488_draw_filled_rectangle(PREV_X-PREV_W/2, PREV_Y-PREV_H/2, PREV_X+PREV_W/2, PREV_Y+PREV_H/2);
	//} else {
	//ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	//ili9488_draw_filled_rectangle(PREV_X-PREV_W/2, PREV_Y-PREV_H/2, PREV_X+PREV_W/2, PREV_Y+PREV_H/2);
	//}
	//last_state = clicked;
}

void draw_mode(uint32_t clicked){
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(50, 330, 30+170, 330+40);
	font_draw_text(&calibri_36, ciclo->nome , 30, 330, 1);
	
	
	ili9488_draw_pixmap(50, 160, ciclo->icone->width,ciclo->icone->height, ciclo->icone->data);
}

void draw_play_pause(Bool is_on){
	
	
	if (!is_on){
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
		ili9488_draw_filled_rectangle(100, 0, 320, 92);
		ili9488_draw_pixmap(128, 410, Play.width, Play.height, Play.data);
		
	}
	else {
		
		ili9488_draw_pixmap(128, 410, pause.width, pause.height, pause.data);
		tempo=ciclo->enxagueTempo+ciclo->centrifugacaoTempo;
		
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
		ili9488_draw_filled_rectangle(100, 60, 320, 52);
		char buffer[32];
		sprintf(buffer, "%d",tempo);
		font_draw_text(&calibri_36, "Total:", 100, 60, 2);
		font_draw_text(&calibri_36, buffer, 215, 60, 2);
		font_draw_text(&calibri_36, "min", 260, 60, 2);
		
	}
	
	
	
	
}

uint32_t convert_axis_system_x(uint32_t touch_y) {
	// entrada: 4096 - 0 (sistema de coordenadas atual)
	// saida: 0 - 320
	return ILI9488_LCD_WIDTH - ILI9488_LCD_WIDTH*touch_y/4096;
}

uint32_t convert_axis_system_y(uint32_t touch_x) {
	// entrada: 0 - 4096 (sistema de coordenadas atual)
	// saida: 0 - 320
	return ILI9488_LCD_HEIGHT*touch_x/4096;
}

void update_timer(){
	char bufferSeg[32];
	char bufferMin[32];
	
	sprintf(bufferSeg, "%d",seg);
	sprintf(bufferMin, "%d",minu);
	
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(175, 20, 300, 52);
	
	font_draw_text(&calibri_36, bufferMin, 175, 20, 2);
	font_draw_text(&calibri_36, ":", 220, 20, 2);
	font_draw_text(&calibri_36, bufferSeg, 245, 20, 2);
	}

void update_screen(uint32_t tx, uint32_t ty, uint32_t status) {
	if(status==0x20){
		if(tx >= LOCK_X-LOCK_W/2 && tx <= LOCK_X + LOCK_W/2){
			if(ty >= LOCK_Y-LOCK_H/2 &&  ty <= LOCK_Y + LOCK_H/2){
				if( safety )
				{
					draw_lock(is_locked);
					is_locked = !is_locked;
				}
			}
		}
		if (!is_locked){
			/*PRINT DO MODO DE LAVAGEM*/
			
			
			if(tx >= NEXT_X-NEXT_W/2 && tx <= NEXT_X + NEXT_W/2){
				if(ty >= NEXT_Y-NEXT_H/2 &&  ty <= NEXT_Y + NEXT_H/2){
					if (!is_on){
						draw_next(1);
						draw_mode(1);
					}
				}
			}
			
			if(tx >= PREV_X-PREV_W/2 && tx <= PREV_X + PREV_W/2){
				if(ty >= PREV_Y-PREV_H/2 &&  ty <= PREV_Y + PREV_H/2){
					if (!is_on){
						draw_prev(1);
						draw_mode(1);
					}
				}
			}

			if((tx >= PLAY_X-PLAY_W/2 && tx <=PLAY_X + PLAY_W/2) && (ty >= PLAY_Y-PLAY_H/2 &&  ty <= PLAY_Y + PLAY_H/2)){
				if (!is_on){
					if (porta_aberta){
						ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
						ili9488_draw_filled_rectangle(0, 0, 64,64);
						ili9488_draw_pixmap(0, 0, porta.width, porta.height, porta.data);
					}
					else{
						seg=0;
						minu=0;
						ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
						ili9488_draw_filled_rectangle(0, 0,320, 90);
						
						ili9488_draw_pixmap(0, 0, ON.width, ON.height, ON.data);
						flag=!flag;
						is_on =!is_on;
						draw_play_pause(is_on);
					}
					
				}
				else {
					/*PAUSE*/
					flag=!flag;
					is_on =!is_on;
					ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
					ili9488_draw_filled_rectangle(0, 0,64, 64);
					draw_play_pause(is_on);

				}
			}
		}
	}
}

void led_update(){
	if (porta_aberta){
		pio_clear(LED_PIO, LED_IDX_MASK);
	}
	else {
		pio_set(LED_PIO, LED_IDX_MASK);
	}
	
};
/*MAIN*/

int main(void)
{
	struct mxt_device device;
	
	/*INICIA O DETECTOR DA PORTA */
	porta_aberta=1;
	
	f_rtt_alarme = true;
	
	/* Initialize the USART configuration struct */
	const usart_serial_options_t usart_serial_options = {
		.baudrate     = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength   = USART_SERIAL_CHAR_LENGTH,
		.paritytype   = USART_SERIAL_PARITY,
		.stopbits     = USART_SERIAL_STOP_BIT
	};
	uint8_t stingLCD[256];
	
	sysclk_init(); /* Initialize system clocks */
	board_init();  /* Initialize board */
	io_init();
	ioport_init();
	
	ciclo = initMenuOrder();
	
	configure_lcd();
	configure_console();
	
	draw_screen();
	draw_lock(1);
	draw_next(0);
	draw_prev(0);
	draw_play_pause(0);
	draw_mode(0);
	
	/* Initialize the mXT touch device */
	mxt_init(&device);
	
	/* Initialize stdio on USART */
	stdio_serial_init(USART_SERIAL_EXAMPLE, &usart_serial_options);

	rtc_set_date_alarm(RTC, 1, MOUNT, 1, DAY);
	rtc_set_time_alarm(RTC, 1, HOUR, 1, MINUTE, 1, SECOND+2);
	
	TC_init(TC0, ID_TC1, 1, 1);
	
	while (true) {
		/* Check for any pending messages and run message handler if any
		* message is found in the queue */
		//
		//uint16_t pllPreScale = (int) (((float) 32768) / 2.0);
		//uint32_t irqRTTvalue  = 2;
		//RTT_init(pllPreScale, irqRTTvalue);
		//

		
		//led_update(porta_aberta);
		
		if (mxt_is_message_pending(&device)) {
			mxt_handler(&device);
		}
		
		//pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}

	return 0;
}

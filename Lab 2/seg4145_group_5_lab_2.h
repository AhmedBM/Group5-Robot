#ifndef _LAB_TEMPLATE_H_
#define _LAB_TEMPLATE_H_
#endif //_LAB_TEMPLATE_H_

/* Includes */

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "alt_types.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_uart_regs.h"

/* Debugging Constants */

#undef DEBUG_CHAR_DISPATCH
#define DEBUG_MESSAGE_DISPATCH 1

/* Only enable this if you are sure you won't lose any characters
 * (i.e. the Altera UART is buffered or its baud rate is low enough)
 */
#undef DEBUG_UART_RECV_ISR

#undef DEBUG_UART_XMIT

/* LCD Related Prototype */

static void displayMsgLCD(alt_u8*);

/* LED Related Prototype */

static void displayLED(alt_u8);

/* Seven-Segment Display Related Prototypes */

static void sevenseg_set_hex(alt_u8);

/* Push Button Related Prototypes */

static void handle_button_interrupts(void*, alt_u32);
static void init_button_pio();
static void handle_button_press();

/* UART Related Constants and Prototypes*/

#define UART1 0x01

static void handle_uart1_interrupt(void*, alt_u32);
static void init_uart(alt_u8);
static void send_message_to_uart(alt_u8, alt_u8*);

/* 
 * LIN Related Constants 
 */

/* Message Fields */

#define MSG_F_TYPE       0x00
#define MSG_F_LENGTH     0x01
#define MSG_F_MOD_TYPE   0x02
#define MSG_F_MOD_SERIAL 0x03
#define MSG_F_REG_NO     0x04
#define MSG_F_DLEN       0x05
#define MSG_F_DATA       0x05

/* Message Types */

#define MSG_O_CONNECT    0x01
#define MSG_O_DISCONNECT 0x02
#define MSG_O_READ_MOD   0x03
#define MSG_O_WRITE_MOD  0x03
#define MSG_O_SUBSCRIBE  0x05

#define MSG_I_ACK         0x01
#define MSG_I_MOD_REPLY   0x02
#define MSG_I_ADD_MODULE  0x03
#define MSG_I_DATA_STREAM 0x04

/* Emergency Use Only:
 * 0x08 0x04 0x08 0x02 
 */

#define MSG_O_EXIT_BOOTLDR_MODE 0x08

/* Module Types */

#define MOD_LED    0x01
#define MOD_PWM    0x02
#define MOD_CMPS03 0x03
#define MOD_DIG_IN 0x06
#define MOD_GP2D02 0x0A
#define MOD_UART   0x0B
#define MOD_PWR    0x0F

/* Read/Write Register Masks */

#define REG_MASK_READ  0xC0
#define REG_MASK_WRITE 0x40

/* Register Addresses By Module */

#define REG_R_ALL_VERSION 0x01

#define REG_RW_LED_OUTPUT 0x02

#define REG_RW_PWM_PERIOD_H  0x02
#define REG_RW_PWM_PERIOD_L  0x03
#define REG_RW_PWM_M1_DUTY_H 0x04
#define REG_RW_PWM_M1_DUTY_L 0x05
#define REG_RW_PWM_M2_DUTY_H 0x06
#define REG_RW_PWM_M2_DUTY_L 0x07
#define REG_RW_PWM_M3_DUTY_H 0x08
#define REG_RW_PWM_M3_DUTY_L 0x09
#define REG_RW_PWM_M4_DUTY_H 0x0A
#define REG_RW_PWM_M4_DUTY_L 0x0B

#define REG_R_CMPS03_HEADING   0x02
#define REG_RW_CMPS03_SAMPLE_H 0x03
#define REG_RW_CMPS03_SAMPLE_L 0x04

#define REG_R_DIG_IN_INPUT     0x02
#define REG_RW_DIG_IN_SAMPLE_H 0x03
#define REG_RW_DIG_IN_SAMPLE_L 0x04

#define REG_SUB_DIG_IN_RAW_VALUE 0x01
#define REG_SUB_DIG_IN_COUNTER   0x02
 
#define REG_R_GP2D02_IR1_DISTANCE  0x02
#define REG_R_GP2D02_IR2_DISTANCE  0x03
#define REG_R_GP2D02_IR3_DISTANCE  0x04
#define REG_RW_GP2D02_IR1_SAMPLE_H 0x05
#define REG_RW_GP2D02_IR1_SAMPLE_L 0x06
#define REG_RW_GP2D02_IR2_SAMPLE_H 0x07
#define REG_RW_GP2D02_IR2_SAMPLE_L 0x08
#define REG_RW_GP2D02_IR3_SAMPLE_H 0x09
#define REG_RW_GP2D02_IR3_SAMPLE_L 0x0A

#define REG_R_PWR_CURRENT_H         0x02
#define REG_R_PWR_CURRENT_L         0x03
#define REG_R_PWR_VOLTAGE_INT       0x04
#define REG_R_PWR_VOLTAGE_FRAC      0x05
#define REG_RW_PWR_CURRENT_SAMPLE_H 0x06
#define REG_RW_PWR_CURRENT_SAMPLE_L 0x07
#define REG_RW_PWR_VOLTAGE_SAMPLE_H 0x09
#define REG_RW_PWR_VOLTAGE_SAMPLE_L 0x0A

/* 
 * Other Constants 
 */

/* LED Values */

#define VAL_LED_ALL_ON  0xFF
#define VAL_LED_ALL_OFF 0x00
#define VAL_LED_GREEN   0x01
#define VAL_LED_YELLOW  0x08
#define VAL_LED_RED     0x80

/* PWM Period, High and Low */

#define VAL_PWM_PERIOD_H 0x02
#define VAL_PWM_PERIOD_L 0xF6

/* PWM Duty Cycle for a Full Stop, High and Low */

#define VAL_PWM_DUTY_STOP_H 0x03
#define VAL_PWM_DUTY_STOP_L 0x00

/* PWM Duty Cycle for Forward Movement, High and Low */

#define VAL_PWM_DUTY_LEFT_FORWARD_H 0x00
#define VAL_PWM_DUTY_LEFT_FORWARD_L 0x37
#define VAL_PWM_DUTY_RIGHT_FORWARD_H 0x00
#define VAL_PWM_DUTY_RIGHT_FORWARD_L 0x14

/* PWM Duty Cycle for Backward Movement, High and Low */

#define VAL_PWM_DUTY_LEFT_BACKWARD_H 0x00
#define VAL_PWM_DUTY_LEFT_BACKWARD_L 0x14
#define VAL_PWM_DUTY_RIGHT_BACKWARD_H 0x00
#define VAL_PWM_DUTY_RIGHT_BACKWARD_L 0x37

/* Digital Inputs Subscription Configuration Options */

#define VAL_DIG_IN_SUB_CONFIG_TIMESTAMPS  0x01
#define VAL_DIG_IN_SUB_CONFIG_DIFF_FILTER 0x02

/* Command Constants by Module */

#define CMD_DIG_IN_RESET_COUNTERS 0x01
#define CMD_DIG_IN_ENABLE_HSI     0x02

/* LIN Related Functions */

static void lin_connect();
static void lin_disconnect();

static void lin_exit_bootldr_mode();

static void lin_led_set(alt_u8);

static void lin_pwm_init_period();
static void lin_pwm_full_stop();
static void lin_pwm_move_forward();
static void lin_pwm_move_backward();
static void lin_pwm_rotate_cw();
static void lin_pwm_rotate_ccw();

static void lin_dig_in_reset_counters();
static void lin_dig_in_enable_hsi();
static void lin_dig_in_subscribe();

/* Helper Data Types for LIN Message Dispatch */

#define LIN_MESSAGE_MAX_SIZE 255
#define LIN_CHAR_QUEUE_SIZE 1024

typedef struct lin_message {
  alt_u8 byte_counter;
  alt_u8 message_data[LIN_MESSAGE_MAX_SIZE];
} LIN_MESSAGE;

/* Helper Functions for LIN Message Dispatch */

static void lin_message_init(LIN_MESSAGE*);
static void lin_char_dispatch(alt_u8*);
static void lin_message_dispatch(LIN_MESSAGE*);

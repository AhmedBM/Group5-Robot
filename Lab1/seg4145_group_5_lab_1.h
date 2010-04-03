/******************************************************************************
 *
 * Names:	Ahmed Ben Messaoud	4291509
 *		Elvis-Philip Niyonkuru	3441001
 *
 * Course Code:		SEG4145
 * Lab Number:		1
 * File name:		seg4145_group_5_lab_1.c
 * Date:		      Febuary 3rd, 2010
 *
 *
 * Description
 * *************
 * This is the implementation of the first lab.
 *
 *
 ******************************************************************************
 */

#ifndef _LAB_1_H_
#define _LAB_1_H_

#endif //_LAB_1_H_

/* Defines */
#define STUDENT1 4291509
#define STUDENT2 3441001
#define NUMCHARACTERS 16
#define NUMLINES 2
#define BASE10 10
#define BASE16 16
#define MAXNUM 99

/* Includes */
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "alt_types.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_uart_regs.h"

/******************************************************************************
*
* Name
* ************
* reverse_bit_pattern
*
*
* Description
* *************
* 8-Bit Bit-Wise Reverse
*
*
* Parameters
* *************
* Name Type In/Out Description
* ----------- ---------- --------------- ---------------
* inByte alt_u8 in an unsigned charater of 8 bits
*
* Returns
* *************
* Type Description
* ---------- ---------------
* alt_u8 the bit-wise invwerse of the input parameter
*
*
******************************************************************************
*/
static alt_u8 reverse_bit_pattern(alt_u8 inByte);

/******************************************************************************
*
* Name
* ************
* alt_u8 hex_to_dec
*
*
* Description
* *************
* Produce a haxadecimal which is written like the decimal input.
* Ex: 23 >> 0x23
*
* Parameters
* *************
* Name Type In/Out Description
* ----------- ---------- --------------- ---------------
* inByte alt_u8 in unsigned charater of 8 bits which is the decimal value.
*
* Returns
* *************
* Type Description
* ---------- ---------------
* alt_u8 nsigned charater of 8 bits which is the hexadecimal value.
*
*
******************************************************************************
*/
static alt_u8 hex_to_dec(alt_u8 hex);

/******************************************************************************
*
* Name
* ************
* alt_u8 *center_string
*
*
* Description
* *************
* Centers the input string. The input string takes in a string with up to
* 2 lines of which it centers both into 2x16 character lines.
* 
* Note: The function returns a heap allocated pointer which must be free'd!
* 
* Ex: 
* Input = "Testing\n123"
* Output = "    Testing\n      123"
*
* Parameters
* *************
* Name Type In/Out Description
* ----------- ---------- --------------- ---------------
* msg alt_u8* In The 8-bit pointer to the string to center.
*
* Returns
* *************
* Type Description
* ---------- ---------------
* alt_u8* 8-bit pointer to the centered string
*
*
******************************************************************************
*/
static alt_u8 *center_string(alt_u8 * msg);

/* LCD Related Prototype */
static void displayMsgLCD( alt_u8* msg );

/* LED Related Prototype */
static void displayLED( alt_u8 pattern );

/* Seven segment display related prototypes */
static void sevenseg_set_hex(alt_u8 hex);

/* Push button related prototypes */
static void handle_button_interrupts(void* context, alt_u32 id);
static void init_button_pio();
static void handle_button_press();

/* Helper Funtions */

/******************************************************************************
*
* Name
* ************
* static void center_str
*
*
* Description
* *************
* A helper function for center_string. Its purpose is to center ONE line of
* text. The center_string function calls this function twice in order to center
* both lines. This function only centers up to the newline character.
* 
* 
* Ex: 
* Input = "Testing\n123"
* Output = "    Testing\n123"
*
* Parameters
* *************
* Name Type In/Out Description
* ----------- ---------- --------------- ---------------
* msg alt_u8* In The 8-bit pointer to the string of which to center
*
* Returns
* *************
* Type Description
* ---------- ---------------
* N/A
*
*
******************************************************************************
*/
static void center_str(alt_u8 *input, alt_u8 *output);
static void cls(FILE *fp);







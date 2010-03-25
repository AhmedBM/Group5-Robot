/******************************************************************************
*
* Names:   Ahmed Ben Messaoud  4291509
*      Elvis-Philip Niyonkuru  3441001
*
* Course Code:     SEG4145
* Lab Number:      2
* File name:       seg4145_group_5_lab_2.c
* Date:        	March 19th, 2010
*
*
* Description
* *************
* This is the implementation of the first lab.
*
*
******************************************************************************
*/

#include "seg4145_group_5_lab_3.h"

/* Declare one global variable to capture the output of the buttons (SW0-SW3),
* when they are pressed.
*/

volatile int edge_capture;

/* Declare a global variable to control the button presses
* Used in handle_button_press to control which buttons are allowed to execute
*/
volatile int mode;

alt_u8 global_lin_char_queue[LIN_CHAR_QUEUE_SIZE];
LIN_MESSAGE global_lin_message;

volatile alt_u16 global_lin_char_current = 0;
volatile alt_u16 global_lin_char_next = 0;

/* Counters used to capture the current location */
volatile int counter_left = 0;
volatile int counter_right = 0;
volatile int reply_left = 0;
volatile int reply_right = 0;

/* Variables used to calculate the side/angle length */
double sideLength = 0.0, radious = 0.0;
int wheelTurns[] = {0,0};// nimber of turns per wheel, L-R,

int main()
{
	int count = 0;

	/* Initialize the push button pio and enable interrupts for 
	* the push buttons */
	init_button_pio();

	/* Initialize the UART interrupt handler(s) */
	init_uart(UART1);

	/* Initialize edge_capture to avoid any "false" triggers from
	* a previous run.
	*/
	edge_capture = 0;

	/* Initialize the global LIN character queue */
	for (count = 0; count < LIN_CHAR_QUEUE_SIZE; count++)
		global_lin_char_queue[count] = '\0';

	/* Initialize the global LIN message structure */
	lin_message_init(&global_lin_message);

	/* Connect to the LIN */
	lin_connect();

	// INSERT YOUR CODE BETWEEN THE TWO ROWS OF STARS
	/*************************************************************/

	// Display pattern on LEDs
	displayLED(0);

	// Display number onm the 7-segement
	sevenseg_set_hex(0);

	// msg to send
	alt_u8 msg[] = "3441001\n4291509";
	displayMsgLCD(msg);

	while (1) {
		if (global_lin_char_current != global_lin_char_next) {
#ifdef DEBUG_CHAR_DISPATCH
			printf("[DEBUG-CHAR-DISPATCH] Dispatch condition hit: %d != %d\n",
				global_lin_char_current, global_lin_char_next);
#endif

			/* Dispatch the outstanding character */
			lin_char_dispatch(&global_lin_char_queue[global_lin_char_current]);
		}
		handle_button_press();
	}

	/*************************************************************/

	return 0;
}

static alt_u8 reverse_bit_pattern(alt_u8 inByte)
{
	alt_u8 output = 0x00;
	alt_u8 temp = 0x00;
	alt_u8 bit = 0x01; // 0000 0001

	int i = 0;
	for(; i < 8; i++)
	{
		output = output << 1; // shift to left
		// get the bit at ith position
		// and shit it to the far left
		temp = (inByte & bit) >> i;
		output = output | temp;
		bit = bit << 1;
	}
	return output;
}

static alt_u8 hex_to_dec(alt_u8 hex)
{
	// check the hex against requirements (must be less then 100)
	if (hex > MAXNUM) return 0;
	alt_u8 lestSign = hex % BASE10;
	alt_u8 mostSign = hex / BASE10;
	mostSign = mostSign * BASE16;
	return lestSign + mostSign;
}

static alt_u8 *center_string(alt_u8 * msg)
{
	// 16 characters on two lines + 1 char for null or newline on every line
	// NUMCHARACTERS is the number of characters per line
	// NUMLINE the number of lines on teh LCD
	// we add NUMLINES because every line has a control character, either
	// being a newline or null char          
	char *newStr = (char *)calloc(NUMCHARACTERS * NUMLINES + NUMLINES, sizeof(char));

	//was the memory allocated
	if (newStr == NULL)
		return NULL;

	int i = 0;

	alt_u8 *pLoc = strstr(msg, "\n");

	// Is the string longer than 14 chars?
	// we do not need to center the string if it is only 1 char away from a full line
	if (strlen(msg) >= NUMCHARACTERS-1)
	{
		// if there a newline in the string?
		if (pLoc != NULL)
		{
			// There is a newline - is it further than 16 chars away?
			if ((pLoc - msg) > NUMCHARACTERS)
			{
				// trim chars after 16 chars and add a newline, proceed with the rest of the string
				// copy the chars
				strncpy(newStr, msg, NUMCHARACTERS);
				// add the newline
				newStr[NUMCHARACTERS] = '\n';

				// center the rest of the string
				center_str(pLoc+1, &newStr[NUMCHARACTERS+1]);
			}
			else
			{
				// center text before the newline
				center_str(msg, newStr);

				// find the null char in the old string and the newline in the new string
				char locNullChar = (char)strlen(newStr);    //using a char instead of int to save a byte
				char *pLocNewLine = strstr(msg, "\n");

				// add newline to newStr
				newStr[locNullChar] = '\n';

				//center the rest of the string
				center_str(pLocNewLine+1, &newStr[locNullChar+1]);
			}
		}
		else
		{
			// There is a no newline - trim characters away
			// COPY chars into new one but trim after 16 chars
			strncpy(newStr, msg, NUMCHARACTERS);
		}
	}
	else
	{
		//is there a newline
		if (pLoc != NULL)
		{
			// there is a newline, center the string - before the newline
			center_str(msg, newStr);

			// add the newline to the new string - replace the null char with a newline
			char locNullChar = strlen(newStr);  //using char instead of int to save a byte
			newStr[(int)locNullChar] = '\n';

			//center the string - after the newline
			center_str(pLoc+1, &newStr[locNullChar+1]);
		}
		else
		{
			// no newline, just center the string
			center_str(msg, newStr);
		}
	}

	return newStr;
}

static void center_str(alt_u8 *input, alt_u8 *output)
{
	// How many spaces are needed? Cannot use string size as we must not include newline char
	int size = 0;
	while (input[size] != '\0')
	{
		if (input[size] == '\n')
			break;
		size++;
	}
	int spacesNeeded = (NUMCHARACTERS-size)/2;

	// Add spaces to output
	int i=0;
	for (i=0; i<spacesNeeded; i++, output++)
	{
		*output = ' ';
	}

	// Add input text
	for (i=0; i<size; i++, output++)
	{
		*output = input[i];
	}
}

static float calculateAngles(int numberOfSides)
{
	return 360 / (float)numberOfSides;
} // end calculateAngles

static float calculateSideLength(int numberOfSides, int radius)
{
	int innerAngle = calculateAngles(numberOfSides);
	return (float)(radius*sin(innerAngle*(PI/180))) / (float)sin((180-innerAngle)*(PI/2));
} // end calculateSideLength

/* Used to handle the button presses based on the value
* of edge_capture that had been previously set by the
* button_pio interrupt handler.
*/

static void handle_button_press()
{
	switch (edge_capture)
	{
	case 0x1:
		// INSERT THE CODE TO PROCESS SW0
		// BETWEEN THE TWO ROWS OF STARS
		/************************************/
		if (mode)
		{
			// Mode 2   
		}
		else
		{
			// Mode 1
			lin_pwm_move_forward(1);
		}
		/***********************************/
		edge_capture = 0;
		break;

	case 0x2:
		// INSERT THE CODE TO PROCESS SW1
		// BETWEEN THE TWO ROWS OF STARS
		/************************************/
		if (mode)
		{
			// Mode 2
			lin_pwm_rotate_ccw(90);
		}
		else
		{
			// Mode 1
		}
		/************************************/
		edge_capture = 0;
		break;

	case 0x4:
		// INSERT THE CODE TO PROCESS SW2
		// BETWEEN THE TWO ROWS OF STARS
		/************************************/
		if (mode)
		{
			// Mode 2   
		}
		else
		{
			// Mode 1
			lin_pwm_rotate_cw(90);
		}
		/************************************/
		edge_capture = 0;
		break;

	case 0x8:
		// INSERT THE CODE TO PROCESS SW3
		// BETWEEN THE TWO ROWS OF STARS
		/************************************/
		mode = !mode;
		if (mode)
		{
			// Mode 2
			sevenseg_set_hex(2);
		}
		else
		{
			// Mode 1
			sevenseg_set_hex(1);
		}
		/************************************/
		edge_capture = 0;
		break;
	}
}

/* Displays a string on the LCD display. */

static void displayMsgLCD(alt_u8* msg)
{
	// center the tex on two lines
	alt_u8 *pNewMsg = center_string(msg);
	FILE *lcd;
	lcd = fopen("/dev/lcd_display", "w");
	// Clear the screen
	fprintf(lcd, "\n\n");
	// Display msg
	fprintf(lcd, pNewMsg);
	fflush(lcd);
	fclose(lcd);
	//pNewMsg must be free'd
	free(pNewMsg);
}

/* Displays an 8-bit pattern on the eight LEDs. */

static void displayLED(alt_u8 pattern)
{
	// reverse the LED pattern
	pattern = reverse_bit_pattern(pattern);
	IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, pattern);
}

/* Displays an 8-bit number on the seven-segment display. */

static void sevenseg_set_hex(alt_u8 hex)
{
	// change the hex to dec display
	hex = hex_to_dec(hex);
	static alt_u8 segments[16] = {
		0x81, 0xCF, 0x92, 0x86, 0xCC, 0xA4, 0xA0, 0x8F, 0x80, 0x84, /* 0-9 */
		0x88, 0xE0, 0xF2, 0xC2, 0xB0, 0xB8 };                       /* a-f */

		alt_u32 data = segments[hex & 15] | (segments[(hex >> 4) & 15] << 8);

		IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_PIO_BASE, data);
}

/* Interrupt handler for button_pio */

static void handle_button_interrupts(void* context, alt_u32 id)
{
	/* Cast context to edge_capture's type.
	* It is important to keep this volatile,
	* to avoid compiler optimization issues.
	*/
	volatile int* edge_capture_ptr = (volatile int*) context;

	/* Store the value in the Button's edge capture register in *context. */
	*edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE);

	/* Reset the Button's edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0);
}

/* Initializes the button_pio interrupt handler. */

static void init_button_pio()
{
	/* Recast the edge_capture pointer to match the alt_irq_register() function
	* prototype. */
	void* edge_capture_ptr = (void*) &edge_capture;

	/* Enable all 4 button interrupts. */
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTON_PIO_BASE, 0xf);

	/* Reset the edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0x0);

	/* Register the interrupt handler. */
	alt_irq_register(BUTTON_PIO_IRQ, edge_capture_ptr, handle_button_interrupts ); 
}

/* Interrupt handler for UART1 */

static void handle_uart1_interrupt(void* context, alt_u32 id)
{
	/* Just read the data as it is received such that
	* the UART buffer doesn't overflow
	*/

	alt_u16 uart_status;
	alt_u8 uart_chr;

	/* Read UART status */
	uart_status  = IORD_FE_UART_STATUS(FE_BUFFERED_UART_BASE);

	/* Read and process char if any data is available */
	if (uart_status & ASB_STATUS_SLOTRDY_MSK) {
		// Read character from UART
		uart_chr = IORD_FE_UART_RXDATA(FE_BUFFERED_UART_BASE);

		global_lin_char_queue[global_lin_char_next++] = uart_chr;   

		/* Wrap around if we have hit the maximum size
		* of the character circular buffer
		*/
		if (global_lin_char_next == LIN_CHAR_QUEUE_SIZE)
			global_lin_char_next = 0;

#ifdef DEBUG_UART_RECV_ISR
		printf("[DEBUG-UART-RECV-ISR] UART1 received byte (%d)\n", uart_chr);
#endif    
	}
}

/* Initializes the UART interrupt handler(s). */

static void init_uart(alt_u8 mask)
{
	if (mask & UART1) {
		/* Set the baud rate */
		IOWR_FE_UART_BAUDRATE(FE_BUFFERED_UART_BASE, BAUD_57600);

		/* Reset the UART */
		IOWR_FE_UART_CONTROL(FE_BUFFERED_UART_BASE, FE_UART_RESET);

		/* Settup the ISR for the uart */    
		alt_irq_register(FE_BUFFERED_UART_IRQ, NULL, handle_uart1_interrupt);

		/* enable the receiver and transmitter on the uart */
		IOWR_FE_UART_CONTROL(FE_BUFFERED_UART_BASE, FE_UART_EN_RX | FE_UART_EN_TX);
	}
}

/* Used to send a message (null-terminated string) to
* the specified UART port. 
*/

static void send_message_to_uart(alt_u8 uart_no, alt_u8* message)
{
	int uart_base = 0;
	int i = 0;

	/* See which UART port we want to send the message to,
	* and set the uart_base accordingly.
	*/

	if (uart_no == UART1)
		uart_base = FE_BUFFERED_UART_BASE;

	if (uart_base) {
#ifdef DEBUG_UART_XMIT
		printf("[DEBUG-UART-XMIT] Sending message of length %d to UART\%d: ", message[MSG_F_LENGTH], uart_no);
#endif

		for (i = 0; i < message[MSG_F_LENGTH]; i++) {
#ifdef DEBUG_UART_XMIT
			printf("%d ", message[i]);
#endif

			IOWR_FE_UART_TXDATA(uart_base, message[i]);
			usleep(5000);
		}
#ifdef DEBUG_UART_XMIT
		printf("\n");
#endif
	}
#ifdef DEBUG_UART_XMIT
	else 
		printf("[DEBUG-UART-XMIT] Invalid UART number provided (uart_base=%d)!\n", uart_base);
#endif      
}

/* Initializes the LIN message structure received as
* a parameter with the proper values.
*/

static void lin_message_init(LIN_MESSAGE *msg)
{
	msg->byte_counter = 0;
	memset(msg->message_data, '\0', LIN_MESSAGE_MAX_SIZE);
}

/* Dispatches the LIN message received as a parameter. */

static void lin_message_dispatch(LIN_MESSAGE *msg)
{
	int count = 0;

#ifdef DEBUG_MESSAGE_DISPATCH
	printf("[DEBUG-MESSAGE-DISPATCH] Message received and is ready to be dispatched:\n");
	printf("[DEBUG-MESSAGE-DISPATCH]     Type: %d (", msg->message_data[MSG_F_TYPE]);
	switch (msg->message_data[MSG_F_TYPE]) {
	   case MSG_I_ACK:
		   printf("ACK)\n");
		   break;
	   case MSG_I_MOD_REPLY:
		   printf("MOD_REPLY)\n");
		   break;
	   case MSG_I_ADD_MODULE:
		   printf("ADD_MODULE)\n");
		   break;
	   case MSG_I_DATA_STREAM:
		   printf("DATA_STREAM)\n");
		   break;
	   default:
		   printf("UNKNOWN)\n");
		   break; 
	}
	printf("[DEBUG-MESSAGE-DISPATCH]     Data: ");
	for (count = 0; count < msg->message_data[MSG_F_LENGTH]; count++)
		printf("%d ", msg->message_data[count]);
	printf("\n");
	if ((msg->message_data[MSG_F_TYPE] == MSG_I_DATA_STREAM) &&
		(msg->message_data[MSG_F_MOD_TYPE] == MOD_UART)) {
			printf("[DEBUG-MESSAGE-DISPATCH]     Data (data bytes only, as chars): ");
			for (count = MSG_F_DATA; count < msg->message_data[MSG_F_LENGTH]; count++)
				printf("%c", msg->message_data[count]);
			printf("\n");
	}
#endif

	char buffer[16];
	switch (msg->message_data[MSG_F_TYPE]) {
	 case MSG_I_ACK:
		 break;
	 case MSG_I_MOD_REPLY:

		 /* Depending on the desired functionality,
		 * you may need to insert code here
		 */

		 /* Left Wheel Value */
		 if (msg->message_data[MSG_F_REG_NO] == NAME_LEFT_WHEEL)
		 {
			 counter_left = msg->message_data[MSG_F_DATA];
			 reply_left = 1;
		 }
		 else if (msg->message_data[MSG_F_REG_NO] == NAME_RIGHT_WHEEL)
		 {
			 counter_right = msg->message_data[MSG_F_DATA];
			 reply_right = 1;
		 }

		 break;
	 case MSG_I_ADD_MODULE:
		 /* Once the last module (Safe Power) has been added,
		 * perform some initialization steps for all modules
		 */
		 if (msg->message_data[MSG_F_MOD_TYPE] == MOD_PWR) {
			 /* Reset LED status - turn all LEDs off */
			 lin_led_set(VAL_LED_ALL_OFF);

			 /* Reset motor status - initialize periods and
			 * make sure that the motors are stopped after this
			 */
			 lin_pwm_init_period();
			 lin_pwm_full_stop();

			 /* Initialize the Digital Inputs module:
			 * - reset the counter values;
			 * - enable the high-speed interrupts;
			 * - subscribe to the counters.
			 * 
			 * The last step is optional, you can comment it out
			 * if you do not use subscription.
			 */
			 lin_dig_in_reset_counters();
			 lin_dig_in_enable_hsi();
			 //lin_dig_in_subscribe();

			 /* Connect to the UART module.
			 * This must be done in order to ensure proper
			 * communication to the UART.
			 */
			 lin_uart_open();

			 /* You can add other initialization steps here,
			 * as needed
			 */  
		 }
		 break;
	 case MSG_I_DATA_STREAM:

		 /* Depending on the desired functionality,
		 * you may need to insert code here
		 */

		 sprintf (buffer, "loc=%d\nwheel=%d", msg->message_data[MSG_F_DATA+1], msg->message_data[MSG_F_DATA]);
		 displayMsgLCD(buffer);

		 break;
	 default:
		 break;
	}

	/* At the end, re-initialize the message structure
	* such that it can be used for the next message.
	*/    
	lin_message_init(msg);
}

/* Dispatches the character received as a parameter. */

static void lin_char_dispatch(alt_u8 *chr)
{
	int count = 0;

	/* Insert the character in its place into the
	* global LIN message structure
	*/
	global_lin_message.message_data[global_lin_message.byte_counter] = *chr;

#ifdef DEBUG_CHAR_DISPATCH
	printf("[DEBUG-CHAR-DISPATCH] Current contents of global_lin_message:\n");
	printf("[DEBUG-CHAR-DISPATCH]     byte_counter = %d\n", global_lin_message.byte_counter);
	printf("[DEBUG-CHAR-DISPATCH]     message_data = ");
	for (count = 0; count <= global_lin_message.byte_counter; count++)
		printf("%d ", global_lin_message.message_data[count]);
	printf("\n"); 
#endif

	/* If we have already received all the required
	* characters, then dispatch the message; otherwise,
	* increment the byte counter and advance the current
	* char pointer
	*/
	if ((global_lin_message.byte_counter >= MSG_F_LENGTH) &&
		(global_lin_message.byte_counter + 1 == 
		global_lin_message.message_data[MSG_F_LENGTH]))
		lin_message_dispatch(&global_lin_message);
	else
		global_lin_message.byte_counter++;

	/* Advance the current char pointer; wrap around if
	* the queue size was hit
	*/
	global_lin_char_current++;
	if (global_lin_char_current == LIN_CHAR_QUEUE_SIZE)
		global_lin_char_current = 0;
}

/* Sends a CONNECT message to the LIN. */

static void lin_connect()
{
	char message[3];

	memset(message, '\0', 3);

	message[MSG_F_TYPE]   = MSG_O_CONNECT;
	message[MSG_F_LENGTH] = 2;

	send_message_to_uart(1, message); 
}

/* Sends a DISCONNECT message to the LIN. */

static void lin_disconnect()
{
	char message[3];

	memset(message, '\0', 3);

	message[MSG_F_TYPE]   = MSG_O_DISCONNECT;
	message[MSG_F_LENGTH] = 2;

	send_message_to_uart(1, message); 
}

/* Sends a EXIT_BOOTLDR_MODE message to the LIN - 
* this fixes the bug that results in no ADD_MODULE
* messages being sent back upon receiving a CONNECT.
* Note that using this function is not normally
* required.
*/

static void lin_exit_bootldr_mode()
{
	char message[5];

	memset(message, '\0', 5);

	message[MSG_F_TYPE]       = MSG_O_EXIT_BOOTLDR_MODE;
	message[MSG_F_LENGTH]     = 4;
	message[MSG_F_MOD_TYPE]   = 0x08;
	message[MSG_F_MOD_SERIAL] = 0x02;

	send_message_to_uart(1, message); 
}

/* Sets the OUTPUT register of the LED module to a
* specified value.
*/

static void lin_led_set(alt_u8 data)
{
	char message[7];

	memset(message, '\0', 7);

	message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
	message[MSG_F_LENGTH]     = 6;
	message[MSG_F_MOD_TYPE]   = MOD_LED;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_LED_OUTPUT;
	message[MSG_F_DATA]       = data;

	send_message_to_uart(1, message);   
}

/* Initializes the PWM module of the lin with the
* appropriate values in the period registers.
*/

static void lin_pwm_init_period()
{
	char message[7];

	memset(message, '\0', 7);

	message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
	message[MSG_F_LENGTH]     = 6;
	message[MSG_F_MOD_TYPE]   = MOD_PWM;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_PERIOD_H;
	message[MSG_F_DATA]       = VAL_PWM_PERIOD_H;

	send_message_to_uart(UART1, message);   

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_PERIOD_L;
	message[MSG_F_DATA]       = VAL_PWM_PERIOD_L;

	send_message_to_uart(UART1, message);   
}

/* Sends messages to the PWM module of the LIN such
* that the robot comes to a full stop.
*/

static void lin_pwm_full_stop()
{
	char message[7];

	memset(message, '\0', 7);

	message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
	message[MSG_F_LENGTH]     = 6;
	message[MSG_F_MOD_TYPE]   = MOD_PWM;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_H;

	send_message_to_uart(UART1, message);   

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_L;

	send_message_to_uart(UART1, message);   

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_H;

	send_message_to_uart(UART1, message);   

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_L;

	send_message_to_uart(UART1, message);   
}

/* Sends messages to the LIN such that the robot starts
* moving forward.
*/

static void lin_pwm_move_forward(int numTiles)
{
	char message[7];

	memset(message, '\0', 7);
    lin_dig_in_reset_counters();

	message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
	message[MSG_F_LENGTH]     = 6;
	message[MSG_F_MOD_TYPE]   = MOD_PWM;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_H;

	send_message_to_uart(UART1, message);

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_FORWARD_H;
	send_message_to_uart(UART1, message);

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_L;
	send_message_to_uart(UART1, message);

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_FORWARD_L;
	send_message_to_uart(UART1, message);

    // Self Correct wheel
    lin_dig_in_read_counters();
    self_correct_wheel();

	char buffer[16];
    int loc = 0;
    do
    {
        loc += counter_left;
        lin_dig_in_reset_counters();
        lin_dig_in_read_counters();
        sprintf(buffer, "loc=%d", loc);
        displayMsgLCD(buffer);
        usleep(10000);
    }while(loc < (numTiles * SLICES_PER_TILE));

    lin_pwm_full_stop();
}

static void self_correct_wheel()
{
    char message[7];

    memset(message, '\0', 7);

    message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
    message[MSG_F_LENGTH]     = 6;
    message[MSG_F_MOD_TYPE]   = MOD_PWM;
    message[MSG_F_MOD_SERIAL] = 0x00;
    
    printf("counter_left=%d, counter_right=%d\n", counter_left, counter_right);
    
    // Check what wheel is off
    if (counter_left > counter_right)
    {
        // correct right wheel - stop left
        message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
        message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_H;
    
        send_message_to_uart(UART1, message);   
    
        message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
        message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_L;
    
        send_message_to_uart(UART1, message);
        
        do{
           lin_dig_in_read_counters();
        }while(counter_right < counter_left);
        
        // Resume left wheel
        message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
        message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_H;

        send_message_to_uart(UART1, message);
    
        message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
        message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_L;
        send_message_to_uart(UART1, message);
    }
    else if(counter_right > counter_left)
    {
        // correct left wheel - stop right wheel
        message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
        message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_H;
    
        send_message_to_uart(UART1, message);   
    
        message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
        message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_L;
    
        send_message_to_uart(UART1, message);
        
        do{
           lin_dig_in_read_counters();
        }while(counter_left < counter_right);
        
        // Resume right wheel
        message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
        message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_FORWARD_H;

        send_message_to_uart(UART1, message);
    
        message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
        message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_FORWARD_L;
        send_message_to_uart(UART1, message);        
    }
    
    printf("counter_left=%d, counter_right=%d\n", counter_left, counter_right);
}

/* Sends messages to the LIN such that the robot starts
* moving backwards.
*/

static void lin_pwm_move_backward()
{
	char message[7];

	memset(message, '\0', 7);

	message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
	message[MSG_F_LENGTH]     = 6;
	message[MSG_F_MOD_TYPE]   = MOD_PWM;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_BACKWARD_H;

	send_message_to_uart(UART1, message);

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_BACKWARD_H;
	send_message_to_uart(UART1, message);

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_BACKWARD_L;
	send_message_to_uart(UART1, message);

	message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
	message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_BACKWARD_L;
	send_message_to_uart(UART1, message);

	usleep(1800000);

}

/* Sends messages to the LIN such that the robot starts
* rotating clockwise.
*/

static void lin_pwm_rotate_cw(int degrees)
{
	lin_pwm_rotate(degrees);
}

static void lin_pwm_rotate(int degrees)
{
    char message[7];

    memset(message, '\0', 7);

    message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
    message[MSG_F_LENGTH]     = 6;
    message[MSG_F_MOD_TYPE]   = MOD_PWM;
    message[MSG_F_MOD_SERIAL] = 0x00;
    message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
    if (degrees < 0)
        message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_BACKWARD_H;
    else
        message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_H;

    send_message_to_uart(UART1, message);

    message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
    if (degrees < 0)
        message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_FORWARD_H;
    else
        message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_BACKWARD_H;
    
    send_message_to_uart(UART1, message);

    message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
    if (degrees < 0)
        message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_BACKWARD_L;
    else
        message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_L;

    send_message_to_uart(UART1, message);

    message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
    if (degrees < 0)
        message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_FORWARD_L;
    else
        message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_BACKWARD_L;

    send_message_to_uart(UART1, message);

    // Reset counters
    lin_dig_in_reset_counters();

    char buffer[16];
    int loc = 0;
    do
    {
        lin_dig_in_read_counters();
        sprintf(buffer, "counter_left=%d", counter_left);
        displayMsgLCD(buffer);
        usleep(20000);
    }while(counter_left < (degrees / PI_SLICES));

    lin_pwm_full_stop();   
}

/* Sends messages to the LIN such that the robot starts
* rotating counterclockwise.
*/

static void lin_pwm_rotate_ccw(int degrees)
{
	lin_pwm_rotate(degrees * -1);
}

/* Reads the current counter value */

static void lin_dig_in_read_counters()
{   
	char message[6];

	memset(message, '\0', 6);

	message[MSG_F_TYPE]       = MSG_O_READ_MOD;
	message[MSG_F_LENGTH]     = 5;
	message[MSG_F_MOD_TYPE]   = MOD_DIG_IN;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = REG_MASK_READ | VAL_RIGHT_WHEEL;;

	send_message_to_uart(UART1, message);

	message[MSG_F_REG_NO]     = REG_MASK_READ | VAL_LEFT_WHEEL;;
	send_message_to_uart(UART1, message);

    // Check reply
	while(!reply_left && !reply_right)
	{
		if (global_lin_char_current != global_lin_char_next) {
#ifdef DEBUG_CHAR_DISPATCH
			printf("[DEBUG-CHAR-DISPATCH] Dispatch condition hit: %d != %d\n",
				global_lin_char_current, global_lin_char_next);
#endif

			/* Dispatch the outstanding character */
			lin_char_dispatch(&global_lin_char_queue[global_lin_char_current]);
		}
	}

	reply_left = 0;
	reply_right = 0;
}

/* Resets the counters of the Digital Inputs module. */

static void lin_dig_in_reset_counters()
{
	char message[6];

	memset(message, '\0', 6);

	message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
	message[MSG_F_LENGTH]     = 5;
	message[MSG_F_MOD_TYPE]   = MOD_DIG_IN;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = CMD_DIG_IN_RESET_COUNTERS;

	send_message_to_uart(UART1, message);

	counter_left = 0;
	counter_right = 0;
}

/* Enables the high-speed inputs of the 
* Digital Inputs module.
*/

static void lin_dig_in_enable_hsi()
{
	char message[6];

	memset(message, '\0', 6);

	message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
	message[MSG_F_LENGTH]     = 5;
	message[MSG_F_MOD_TYPE]   = MOD_DIG_IN;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = CMD_DIG_IN_ENABLE_HSI;

	send_message_to_uart(UART1, message);   
}

/* Subscribes to the counters of the Digital Inputs 
* module; sampling will be every 100 ms, and a message
* will be generated only when values in the counters
* change.
*/

static void lin_dig_in_subscribe()
{
	char message[9];

	memset(message, '\0', 9);

	message[MSG_F_TYPE]       = MSG_O_SUBSCRIBE;
	message[MSG_F_LENGTH]     = 8;
	message[MSG_F_MOD_TYPE]   = MOD_DIG_IN;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = REG_SUB_DIG_IN_COUNTER;
	message[MSG_F_DATA]       = 0x00;
	message[MSG_F_DATA + 1]   = 0x0A;
	message[MSG_F_DATA + 2]   = VAL_DIG_IN_SUB_CONFIG_DIFF_FILTER;

	send_message_to_uart(UART1, message);   
}

/* Initializes ("opens") the COM port of the LIN UART module                                                            
* in order to receive packets.                                                                                         
*/                                                                                                                     

static void lin_uart_open()                                                                                             
{                                                                                                                       
	char message[6];                                                                                                      

	memset(message, '\0', 6);

	message[MSG_F_TYPE]       = MSG_O_OPEN_COM;
	message[MSG_F_LENGTH]     = 5;
	message[MSG_F_MOD_TYPE]   = MOD_UART;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = VAL_UART_PORT_NO;

	send_message_to_uart(UART1, message);
}

/* Sends a null-terminated string to the device attached to the
* UART module.
*/

static void lin_uart_send_message(alt_u8 *msg)
{
	int i = 0;
	char message[255];

	memset(message, '\0', 255);

	message[MSG_F_TYPE]       = MSG_O_DATA_STREAM;
	message[MSG_F_LENGTH]     = 5 + strlen(msg);
	message[MSG_F_MOD_TYPE]   = MOD_UART;
	message[MSG_F_MOD_SERIAL] = 0x00;
	message[MSG_F_REG_NO]     = VAL_UART_PORT_NO;

	for (i = 0; i < strlen(msg); i++)

		send_message_to_uart(UART1, message);
}

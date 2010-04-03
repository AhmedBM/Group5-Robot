/******************************************************************************
 *
 * Names:	Ahmed Ben Messaoud	4291509
 *		Elvis-Philip Niyonkuru	3441001
 *
 * Course Code:		SEG4145
 * Lab Number:		1
 * File name:		seg4145_group_5_lab_1.c
 * Date:		Febuary 3rd, 2010
 *
 *
 * Description
 * *************
 * This is the implementation of the first lab.
 *
 *
 ******************************************************************************
 */

#include "seg4145_group_5_lab_1.h"

/* Declare one global variable to capture the output of the buttons (SW0-SW3),
 * when they are pressed.
 */
volatile int edge_capture;

/* Declare a global variable to control the button presses
 * Used in handle_button_press to control which buttons are allowed to execute
 */
volatile int ignore_action;

#include <stdio.h>

int main()
{
  /* Initialize the push button pio and enable interrupts for 
   * the push buttons */
  init_button_pio();

  /* Initialize edge_capture to avoid any "false" triggers from
   * a previous run.
   */
  edge_capture = 0;
  
  /* Initialize ignore_action to disallow action from SW1,2 and 3
   * Only SW0 allowed, until SW0 is pressed, then SW0 is disallowed and the
   * other are allowed.
   */
  ignore_action = 0;

  // INSERT YOUR CODE BETWEEN THE TWO ROWS OF STARS
  /*************************************************************/   
    // Display pattern on LEDs
    displayLED(0);
    
    // Display number onm the 7-segement
    sevenseg_set_hex(0);
    
    // msg to send
    alt_u8 msg[] = "3441001\n4291509";
    displayMsgLCD(msg);
    
    while(1)
    {
        // Loop endlessly
        
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

static void cls(FILE *fp)
/* Clear the screen on a given device */
{
    fprintf(fp,"%c%c",(char)0x1b,(char)0x0c);
}

static void displayMsgLCD( alt_u8* msg )
{
  // center the tex on two lines
  alt_u8 *pNewMsg = center_string(msg);
  FILE *lcd;
  lcd = fopen("/dev/lcd_display", "w");
  cls(lcd);
  // Clear the screen
  fprintf(lcd, "\n\n");
  // Display msg
  fprintf(lcd, pNewMsg);
  fflush(lcd);
  fclose(lcd);
  //pNewMsg must be free'd
  free(pNewMsg);
}

static void displayLED( alt_u8 pattern )
{
   // reverse the LED pattern
   pattern = reverse_bit_pattern(pattern);
  IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, pattern);
}

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

/* Initialize the button_pio. */

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

static void handle_button_press()
{
    int count = 0;
    alt_u8 buffer[12];
  switch (edge_capture)
    {
      case 0x1:
        // INSERT THE CODE TO PROCESS SW0
        // BETWEEN THE TWO ROWS OF STARS
        /************************************/
        //Clear screen
        if (!ignore_action)
        {
            displayMsgLCD("");
            ignore_action = !ignore_action;
        }
        //
        /***********************************/
        edge_capture = 0;
        break;

      case 0x2:
        // INSERT THE CODE TO PROCESS SW1
        // BETWEEN THE TWO ROWS OF STARS
        /************************************/
        // Count form 0 to 99 and display on LED array and
        // on the seven segment display
        if (ignore_action)
        {
            for( ; count <= MAXNUM; count ++)
            {
                sevenseg_set_hex(count);
                displayLED(count);
                // Build the string
                if(count < 10)
                    ((void)sprintf(buffer, "SW1 pressed\nWe are at 0%d", count));
                else
                    ((void)sprintf(buffer, "SW1 pressed\nWe are at %d", count));
                // Display current count on LCD screen
                displayMsgLCD(buffer);
                
                // pause for one second
                usleep(1000000);
            }
            // clear the displays
            sevenseg_set_hex(0);
            displayLED(0);
            //Clear screen
            displayMsgLCD("");
        }
        /************************************/
        edge_capture = 0;
        break;

      case 0x4:
        // INSERT THE CODE TO PROCESS SW2
        // BETWEEN THE TWO ROWS OF STARS
        /************************************/
        // Display 02 on the seven segment display
        if (ignore_action)
        {
            sevenseg_set_hex(2);
            // Dislay "No action /n for SW2" on the LCD dispaly
            displayMsgLCD("No action\nfor SW2");
            // wait (or pause) the process for 5 seconds
            usleep(5000000);
    
            sevenseg_set_hex(0);
            //Clear screen
            displayMsgLCD("");
        }
        /************************************/
        edge_capture = 0;
        break;
        
      case 0x8:
        // INSERT THE CODE TO PROCESS SW3
        // BETWEEN THE TWO ROWS OF STARS
        /************************************/
        // Display 03 on the seven segment display
        if (ignore_action)
        {
            sevenseg_set_hex(3);
            // Dislay "No action /n for SW3" on the LCD dispaly
            displayMsgLCD("No action\nfor SW3");
            // wait (or pause) the process for 5 seconds
            usleep(5000000);
            sevenseg_set_hex(0);
            //Clear screen
            displayMsgLCD("");
        }
        /************************************/
        edge_capture = 0;
        break;
    }
}

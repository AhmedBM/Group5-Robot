0001 /******************************************************************************
0002  *
0003  * Names:   Ahmed Ben Messaoud  4291509
0004  *      Elvis-Philip Niyonkuru  3441001
0005  *
0006  * Course Code:     SEG4145
0007  * Lab Number:      2
0008  * File name:       seg4145_group_5_lab_2.c
0009  * Date:        	March 19th, 2010
0010  *
0011  *
0012  * Description
0013  * *************
0014  * This is the implementation of the first lab.
0015  *
0016  *
0017  ******************************************************************************
0018  */
0019 
0020 #include "seg4145_group_5_lab_2.h"
0021 
0022 /* Declare one global variable to capture the output of the buttons (SW0-SW3),
0023  * when they are pressed.
0024  */
0025 
0026 volatile int edge_capture;
0027 
0028 /* Declare a global variable to control the button presses
0029  * Used in handle_button_press to control which buttons are allowed to execute
0030  */
0031 volatile int mode;
0032 
0033 alt_u8 global_lin_char_queue[LIN_CHAR_QUEUE_SIZE];
0034 LIN_MESSAGE global_lin_message;
0035 
0036 volatile alt_u16 global_lin_char_current = 0;
0037 volatile alt_u16 global_lin_char_next = 0;
0038 
0039 /* Counters used to capture the current location */
0040 volatile int counter_left = 0;
0041 volatile int counter_right = 0;
0042 volatile int reply_left = 0;
0043 volatile int reply_right = 0;
0044 
0045 /* Variables used to calculate the side/angle length */
0046 double sideLength = 0.0, radious = 0.0;
0047 int wheelTurns[] = {0,0};// nimber of turns per wheel, L-R,
0048 
0049 int main()
0050 {
0051   int count = 0;
0052   
0053   /* Initialize the push button pio and enable interrupts for 
0054    * the push buttons */
0055   init_button_pio();
0056 
0057   /* Initialize the UART interrupt handler(s) */
0058   init_uart(UART1);
0059   
0060   /* Initialize edge_capture to avoid any "false" triggers from
0061    * a previous run.
0062    */
0063   edge_capture = 0;
0064 
0065   /* Initialize the global LIN character queue */
0066   for (count = 0; count < LIN_CHAR_QUEUE_SIZE; count++)
0067     global_lin_char_queue[count] = '\0';
0068     
0069   /* Initialize the global LIN message structure */
0070   lin_message_init(&global_lin_message);
0071   
0072   /* Connect to the LIN */
0073   lin_connect();
0074   
0075   // INSERT YOUR CODE BETWEEN THE TWO ROWS OF STARS
0076   /*************************************************************/
0077 
0078     // Display pattern on LEDs
0079     displayLED(0);
0080     
0081     // Display number onm the 7-segement
0082     sevenseg_set_hex(0);
0083     
0084     // msg to send
0085     alt_u8 msg[] = "3441001\n4291509";
0086     displayMsgLCD(msg);
0087 
0088   while (1) {
0089     if (global_lin_char_current != global_lin_char_next) {
0090       #ifdef DEBUG_CHAR_DISPATCH
0091         printf("[DEBUG-CHAR-DISPATCH] Dispatch condition hit: %d != %d\n",
0092           global_lin_char_current, global_lin_char_next);
0093       #endif
0094 
0095       /* Dispatch the outstanding character */
0096       lin_char_dispatch(&global_lin_char_queue[global_lin_char_current]);
0097     }
0098     handle_button_press();
0099   }
0100   
0101   /*************************************************************/
0102  
0103   return 0;
0104 }
0105 
0106 static alt_u8 reverse_bit_pattern(alt_u8 inByte)
0107 {
0108     alt_u8 output = 0x00;
0109     alt_u8 temp = 0x00;
0110     alt_u8 bit = 0x01; // 0000 0001
0111     
0112     int i = 0;
0113     for(; i < 8; i++)
0114     {
0115         output = output << 1; // shift to left
0116         // get the bit at ith position
0117         // and shit it to the far left
0118         temp = (inByte & bit) >> i;
0119         output = output | temp;
0120         bit = bit << 1;
0121     }
0122     return output;
0123 }
0124 
0125 static alt_u8 hex_to_dec(alt_u8 hex)
0126 {
0127     // check the hex against requirements (must be less then 100)
0128     if (hex > MAXNUM) return 0;
0129     alt_u8 lestSign = hex % BASE10;
0130     alt_u8 mostSign = hex / BASE10;
0131     mostSign = mostSign * BASE16;
0132     return lestSign + mostSign;
0133 }
0134 
0135 static alt_u8 *center_string(alt_u8 * msg)
0136 {
0137     // 16 characters on two lines + 1 char for null or newline on every line
0138     // NUMCHARACTERS is the number of characters per line
0139     // NUMLINE the number of lines on teh LCD
0140     // we add NUMLINES because every line has a control character, either
0141     // being a newline or null char          
0142     char *newStr = (char *)calloc(NUMCHARACTERS * NUMLINES + NUMLINES, sizeof(char));
0143 
0144     //was the memory allocated
0145     if (newStr == NULL)
0146         return NULL;
0147 
0148     int i = 0;
0149 
0150     alt_u8 *pLoc = strstr(msg, "\n");
0151 
0152     // Is the string longer than 14 chars?
0153     // we do not need to center the string if it is only 1 char away from a full line
0154     if (strlen(msg) >= NUMCHARACTERS-1)
0155     {
0156         // if there a newline in the string?
0157         if (pLoc != NULL)
0158         {
0159             // There is a newline - is it further than 16 chars away?
0160             if ((pLoc - msg) > NUMCHARACTERS)
0161             {
0162                 // trim chars after 16 chars and add a newline, proceed with the rest of the string
0163                 // copy the chars
0164                 strncpy(newStr, msg, NUMCHARACTERS);
0165                 // add the newline
0166                 newStr[NUMCHARACTERS] = '\n';
0167 
0168                 // center the rest of the string
0169                 center_str(pLoc+1, &newStr[NUMCHARACTERS+1]);
0170             }
0171             else
0172             {
0173                 // center text before the newline
0174                 center_str(msg, newStr);
0175 
0176                 // find the null char in the old string and the newline in the new string
0177                 char locNullChar = (char)strlen(newStr);    //using a char instead of int to save a byte
0178                 char *pLocNewLine = strstr(msg, "\n");
0179 
0180                 // add newline to newStr
0181                 newStr[locNullChar] = '\n';
0182 
0183                 //center the rest of the string
0184                 center_str(pLocNewLine+1, &newStr[locNullChar+1]);
0185             }
0186         }
0187         else
0188         {
0189             // There is a no newline - trim characters away
0190             // COPY chars into new one but trim after 16 chars
0191             strncpy(newStr, msg, NUMCHARACTERS);
0192         }
0193     }
0194     else
0195     {
0196         //is there a newline
0197         if (pLoc != NULL)
0198         {
0199             // there is a newline, center the string - before the newline
0200             center_str(msg, newStr);
0201 
0202             // add the newline to the new string - replace the null char with a newline
0203             char locNullChar = strlen(newStr);  //using char instead of int to save a byte
0204             newStr[(int)locNullChar] = '\n';
0205 
0206             //center the string - after the newline
0207             center_str(pLoc+1, &newStr[locNullChar+1]);
0208         }
0209         else
0210         {
0211             // no newline, just center the string
0212             center_str(msg, newStr);
0213         }
0214     }
0215 
0216     return newStr;
0217 }
0218 
0219 static void center_str(alt_u8 *input, alt_u8 *output)
0220 {
0221     // How many spaces are needed? Cannot use string size as we must not include newline char
0222     int size = 0;
0223     while (input[size] != '\0')
0224     {
0225         if (input[size] == '\n')
0226             break;
0227         size++;
0228     }
0229     int spacesNeeded = (NUMCHARACTERS-size)/2;
0230 
0231     // Add spaces to output
0232     int i=0;
0233     for (i=0; i<spacesNeeded; i++, output++)
0234     {
0235         *output = ' ';
0236     }
0237 
0238     // Add input text
0239     for (i=0; i<size; i++, output++)
0240     {
0241         *output = input[i];
0242     }
0243 }
0244 
0245 static float calculateAngles(int numberOfSides)
0246 {
0247     return 360 / (float)numberOfSides;
0248 } // end calculateAngles
0249 
0250 static float calculateSideLength(int numberOfSides, int radius)
0251 {
0252 	int innerAngle = calculateAngles(numberOfSides);
0253     return (float)(radius*sin(innerAngle*(PI/180))) / (float)sin((180-innerAngle)*(PI/2));
0254 } // end calculateSideLength
0255 
0256 /* Used to handle the button presses based on the value
0257  * of edge_capture that had been previously set by the
0258  * button_pio interrupt handler.
0259  */
0260  
0261 static void handle_button_press()
0262 {
0263   switch (edge_capture)
0264     {
0265       case 0x1:
0266         // INSERT THE CODE TO PROCESS SW0
0267         // BETWEEN THE TWO ROWS OF STARS
0268         /************************************/
0269         if (mode)
0270         {
0271             // Mode 2   
0272         }
0273         else
0274         {
0275             // Mode 1
0276             lin_pwm_move_forward();
0277         }
0278         /***********************************/
0279         edge_capture = 0;
0280         break;
0281 
0282       case 0x2:
0283         // INSERT THE CODE TO PROCESS SW1
0284         // BETWEEN THE TWO ROWS OF STARS
0285         /************************************/
0286         if (mode)
0287         {
0288             // Mode 2
0289             lin_pwm_rotate_ccw();   
0290         }
0291         else
0292         {
0293             // Mode 1
0294         }
0295         /************************************/
0296         edge_capture = 0;
0297         break;
0298 
0299       case 0x4:
0300         // INSERT THE CODE TO PROCESS SW2
0301         // BETWEEN THE TWO ROWS OF STARS
0302         /************************************/
0303         if (mode)
0304         {
0305             // Mode 2   
0306         }
0307         else
0308         {
0309             // Mode 1
0310             lin_pwm_rotate_cw();
0311         }
0312         /************************************/
0313         edge_capture = 0;
0314         break;
0315         
0316       case 0x8:
0317         // INSERT THE CODE TO PROCESS SW3
0318         // BETWEEN THE TWO ROWS OF STARS
0319         /************************************/
0320         mode = !mode;
0321         if (mode)
0322         {
0323             // Mode 2
0324             sevenseg_set_hex(2);
0325         }
0326         else
0327         {
0328             // Mode 1
0329             sevenseg_set_hex(1);
0330         }
0331         /************************************/
0332         edge_capture = 0;
0333         break;
0334     }
0335 }
0336 
0337 /* Displays a string on the LCD display. */
0338 
0339 static void displayMsgLCD(alt_u8* msg)
0340 {
0341   // center the tex on two lines
0342   alt_u8 *pNewMsg = center_string(msg);
0343   FILE *lcd;
0344   lcd = fopen("/dev/lcd_display", "w");
0345   // Clear the screen
0346   fprintf(lcd, "\n\n");
0347   // Display msg
0348   fprintf(lcd, pNewMsg);
0349   fflush(lcd);
0350   fclose(lcd);
0351   //pNewMsg must be free'd
0352   free(pNewMsg);
0353 }
0354 
0355 /* Displays an 8-bit pattern on the eight LEDs. */
0356 
0357 static void displayLED(alt_u8 pattern)
0358 {
0359   // reverse the LED pattern
0360   pattern = reverse_bit_pattern(pattern);
0361   IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, pattern);
0362 }
0363 
0364 /* Displays an 8-bit number on the seven-segment display. */
0365 
0366 static void sevenseg_set_hex(alt_u8 hex)
0367 {
0368     // change the hex to dec display
0369     hex = hex_to_dec(hex);
0370   static alt_u8 segments[16] = {
0371     0x81, 0xCF, 0x92, 0x86, 0xCC, 0xA4, 0xA0, 0x8F, 0x80, 0x84, /* 0-9 */
0372     0x88, 0xE0, 0xF2, 0xC2, 0xB0, 0xB8 };                       /* a-f */
0373 
0374   alt_u32 data = segments[hex & 15] | (segments[(hex >> 4) & 15] << 8);
0375 
0376   IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_PIO_BASE, data);
0377 }
0378 
0379 /* Interrupt handler for button_pio */
0380 
0381 static void handle_button_interrupts(void* context, alt_u32 id)
0382 {
0383   /* Cast context to edge_capture's type.
0384    * It is important to keep this volatile,
0385    * to avoid compiler optimization issues.
0386    */
0387   volatile int* edge_capture_ptr = (volatile int*) context;
0388 
0389   /* Store the value in the Button's edge capture register in *context. */
0390   *edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE);
0391 
0392   /* Reset the Button's edge capture register. */
0393   IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0);
0394 }
0395 
0396 /* Initializes the button_pio interrupt handler. */
0397 
0398 static void init_button_pio()
0399 {
0400   /* Recast the edge_capture pointer to match the alt_irq_register() function
0401   * prototype. */
0402   void* edge_capture_ptr = (void*) &edge_capture;
0403 
0404   /* Enable all 4 button interrupts. */
0405   IOWR_ALTERA_AVALON_PIO_IRQ_MASK(BUTTON_PIO_BASE, 0xf);
0406 
0407   /* Reset the edge capture register. */
0408   IOWR_ALTERA_AVALON_PIO_EDGE_CAP(BUTTON_PIO_BASE, 0x0);
0409 
0410   /* Register the interrupt handler. */
0411   alt_irq_register(BUTTON_PIO_IRQ, edge_capture_ptr, handle_button_interrupts ); 
0412 }
0413 
0414 /* Interrupt handler for UART1 */
0415 
0416 static void handle_uart1_interrupt(void* context, alt_u32 id)
0417 {
0418   /* Just read the data as it is received such that
0419    * the UART buffer doesn't overflow
0420    */
0421 
0422   alt_u16 uart_status;
0423   alt_u8 uart_chr;
0424 
0425   /* Read UART status */
0426   uart_status  = IORD_FE_UART_STATUS(FE_BUFFERED_UART_BASE);
0427 
0428   /* Read and process char if any data is available */
0429   if (uart_status & ASB_STATUS_SLOTRDY_MSK) {
0430     // Read character from UART
0431     uart_chr = IORD_FE_UART_RXDATA(FE_BUFFERED_UART_BASE);
0432     
0433     global_lin_char_queue[global_lin_char_next++] = uart_chr;   
0434 
0435     /* Wrap around if we have hit the maximum size
0436      * of the character circular buffer
0437      */
0438     if (global_lin_char_next == LIN_CHAR_QUEUE_SIZE)
0439       global_lin_char_next = 0;
0440 
0441     #ifdef DEBUG_UART_RECV_ISR
0442       printf("[DEBUG-UART-RECV-ISR] UART1 received byte (%d)\n", uart_chr);
0443     #endif    
0444   }
0445 }
0446 
0447 /* Initializes the UART interrupt handler(s). */
0448 
0449 static void init_uart(alt_u8 mask)
0450 {
0451   if (mask & UART1) {
0452     /* Set the baud rate */
0453     IOWR_FE_UART_BAUDRATE(FE_BUFFERED_UART_BASE, BAUD_57600);
0454     
0455     /* Reset the UART */
0456     IOWR_FE_UART_CONTROL(FE_BUFFERED_UART_BASE, FE_UART_RESET);
0457 
0458     /* Settup the ISR for the uart */    
0459     alt_irq_register(FE_BUFFERED_UART_IRQ, NULL, handle_uart1_interrupt);
0460     
0461     /* enable the receiver and transmitter on the uart */
0462     IOWR_FE_UART_CONTROL(FE_BUFFERED_UART_BASE, FE_UART_EN_RX | FE_UART_EN_TX);
0463   }
0464 }
0465 
0466 /* Used to send a message (null-terminated string) to
0467  * the specified UART port. 
0468  */
0469 
0470 static void send_message_to_uart(alt_u8 uart_no, alt_u8* message)
0471 {
0472   int uart_base = 0;
0473   int i = 0;
0474     
0475   /* See which UART port we want to send the message to,
0476    * and set the uart_base accordingly.
0477    */
0478    
0479   if (uart_no == UART1)
0480     uart_base = FE_BUFFERED_UART_BASE;
0481   
0482   if (uart_base) {
0483     #ifdef DEBUG_UART_XMIT
0484       printf("[DEBUG-UART-XMIT] Sending message of length %d to UART\%d: ", message[MSG_F_LENGTH], uart_no);
0485     #endif
0486     
0487     for (i = 0; i < message[MSG_F_LENGTH]; i++) {
0488       #ifdef DEBUG_UART_XMIT
0489         printf("%d ", message[i]);
0490       #endif
0491       
0492       IOWR_FE_UART_TXDATA(uart_base, message[i]);
0493       usleep(5000);
0494     }
0495     #ifdef DEBUG_UART_XMIT
0496       printf("\n");
0497     #endif
0498   }
0499   #ifdef DEBUG_UART_XMIT
0500     else 
0501       printf("[DEBUG-UART-XMIT] Invalid UART number provided (uart_base=%d)!\n", uart_base);
0502   #endif      
0503 }
0504 
0505 /* Initializes the LIN message structure received as
0506  * a parameter with the proper values.
0507  */
0508 
0509 static void lin_message_init(LIN_MESSAGE *msg)
0510 {
0511   msg->byte_counter = 0;
0512   memset(msg->message_data, '\0', LIN_MESSAGE_MAX_SIZE);
0513 }
0514 
0515 /* Dispatches the LIN message received as a parameter. */
0516 
0517 static void lin_message_dispatch(LIN_MESSAGE *msg)
0518 {
0519   int count = 0;
0520   
0521   #ifdef DEBUG_MESSAGE_DISPATCH
0522     printf("[DEBUG-MESSAGE-DISPATCH] Message received and is ready to be dispatched:\n");
0523     printf("[DEBUG-MESSAGE-DISPATCH]     Type: %d (", msg->message_data[MSG_F_TYPE]);
0524     switch (msg->message_data[MSG_F_TYPE]) {
0525       case MSG_I_ACK:
0526         printf("ACK)\n");
0527         break;
0528       case MSG_I_MOD_REPLY:
0529         printf("MOD_REPLY)\n");
0530         break;
0531       case MSG_I_ADD_MODULE:
0532         printf("ADD_MODULE)\n");
0533         break;
0534       case MSG_I_DATA_STREAM:
0535         printf("DATA_STREAM)\n");
0536         break;
0537       default:
0538         printf("UNKNOWN)\n");
0539         break; 
0540     }
0541     printf("[DEBUG-MESSAGE-DISPATCH]     Data: ");
0542     for (count = 0; count < msg->message_data[MSG_F_LENGTH]; count++)
0543       printf("%d ", msg->message_data[count]);
0544     printf("\n");
0545     if ((msg->message_data[MSG_F_TYPE] == MSG_I_DATA_STREAM) &&
0546         (msg->message_data[MSG_F_MOD_TYPE] == MOD_UART)) {
0547       printf("[DEBUG-MESSAGE-DISPATCH]     Data (data bytes only, as chars): ");
0548       for (count = MSG_F_DATA; count < msg->message_data[MSG_F_LENGTH]; count++)
0549         printf("%c", msg->message_data[count]);
0550       printf("\n");
0551     }
0552   #endif
0553   
0554   char buffer[16];
0555   switch (msg->message_data[MSG_F_TYPE]) {
0556     case MSG_I_ACK:
0557       break;
0558     case MSG_I_MOD_REPLY:
0559 
0560       /* Depending on the desired functionality,
0561        * you may need to insert code here
0562        */
0563        
0564        /* Left Wheel Value */
0565        if (msg->message_data[MSG_F_DATA] == LEFT_WHEEL)
0566        {
0567         counter_left = msg->message_data[MSG_F_DATA+1];
0568         reply_left = 1;
0569        }
0570        else if (msg->message_data[MSG_F_DATA] == RIGHT_WHEEL)
0571        {
0572         counter_right = msg->message_data[MSG_F_DATA+1];
0573         reply_right = 1;
0574        }
0575 
0576       break;
0577     case MSG_I_ADD_MODULE:
0578       /* Once the last module (Safe Power) has been added,
0579        * perform some initialization steps for all modules
0580        */
0581       if (msg->message_data[MSG_F_MOD_TYPE] == MOD_PWR) {
0582         /* Reset LED status - turn all LEDs off */
0583         lin_led_set(VAL_LED_ALL_OFF);
0584 
0585         /* Reset motor status - initialize periods and
0586          * make sure that the motors are stopped after this
0587          */
0588         lin_pwm_init_period();
0589         lin_pwm_full_stop();
0590 
0591         /* Initialize the Digital Inputs module:
0592          * - reset the counter values;
0593          * - enable the high-speed interrupts;
0594          * - subscribe to the counters.
0595          * 
0596          * The last step is optional, you can comment it out
0597          * if you do not use subscription.
0598          */
0599         lin_dig_in_reset_counters();
0600         lin_dig_in_enable_hsi();
0601         //lin_dig_in_subscribe();
0602 
0603         /* Connect to the UART module.
0604          * This must be done in order to ensure proper
0605          * communication to the UART.
0606          */
0607         lin_uart_open();
0608         
0609         /* You can add other initialization steps here,
0610          * as needed
0611          */  
0612       }
0613       break;
0614     case MSG_I_DATA_STREAM:
0615 
0616       /* Depending on the desired functionality,
0617        * you may need to insert code here
0618        */
0619        
0620        sprintf (buffer, "loc=%d\nwheel=%d", msg->message_data[MSG_F_DATA+1], msg->message_data[MSG_F_DATA]);
0621        displayMsgLCD(buffer);
0622 
0623       break;
0624     default:
0625       break;
0626   }
0627    
0628   /* At the end, re-initialize the message structure
0629    * such that it can be used for the next message.
0630    */    
0631   lin_message_init(msg);
0632   }
0633 
0634 /* Dispatches the character received as a parameter. */
0635 
0636 static void lin_char_dispatch(alt_u8 *chr)
0637 {
0638   int count = 0;
0639   
0640   /* Insert the character in its place into the
0641    * global LIN message structure
0642    */
0643   global_lin_message.message_data[global_lin_message.byte_counter] = *chr;
0644 
0645   #ifdef DEBUG_CHAR_DISPATCH
0646     printf("[DEBUG-CHAR-DISPATCH] Current contents of global_lin_message:\n");
0647     printf("[DEBUG-CHAR-DISPATCH]     byte_counter = %d\n", global_lin_message.byte_counter);
0648     printf("[DEBUG-CHAR-DISPATCH]     message_data = ");
0649     for (count = 0; count <= global_lin_message.byte_counter; count++)
0650       printf("%d ", global_lin_message.message_data[count]);
0651     printf("\n"); 
0652   #endif
0653       
0654   /* If we have already received all the required
0655    * characters, then dispatch the message; otherwise,
0656    * increment the byte counter and advance the current
0657    * char pointer
0658    */
0659   if ((global_lin_message.byte_counter >= MSG_F_LENGTH) &&
0660       (global_lin_message.byte_counter + 1 == 
0661        global_lin_message.message_data[MSG_F_LENGTH]))
0662     lin_message_dispatch(&global_lin_message);
0663   else
0664     global_lin_message.byte_counter++;
0665     
0666   /* Advance the current char pointer; wrap around if
0667    * the queue size was hit
0668    */
0669   global_lin_char_current++;
0670   if (global_lin_char_current == LIN_CHAR_QUEUE_SIZE)
0671     global_lin_char_current = 0;
0672 }
0673 
0674 /* Sends a CONNECT message to the LIN. */
0675 
0676 static void lin_connect()
0677 {
0678   char message[3];
0679     
0680   memset(message, '\0', 3);
0681     
0682   message[MSG_F_TYPE]   = MSG_O_CONNECT;
0683   message[MSG_F_LENGTH] = 2;
0684     
0685   send_message_to_uart(1, message); 
0686 }
0687 
0688 /* Sends a DISCONNECT message to the LIN. */
0689 
0690 static void lin_disconnect()
0691 {
0692   char message[3];
0693     
0694   memset(message, '\0', 3);
0695     
0696   message[MSG_F_TYPE]   = MSG_O_DISCONNECT;
0697   message[MSG_F_LENGTH] = 2;
0698     
0699   send_message_to_uart(1, message); 
0700 }
0701 
0702 /* Sends a EXIT_BOOTLDR_MODE message to the LIN - 
0703  * this fixes the bug that results in no ADD_MODULE
0704  * messages being sent back upon receiving a CONNECT.
0705  * Note that using this function is not normally
0706  * required.
0707  */
0708  
0709 static void lin_exit_bootldr_mode()
0710 {
0711   char message[5];
0712     
0713   memset(message, '\0', 5);
0714     
0715   message[MSG_F_TYPE]       = MSG_O_EXIT_BOOTLDR_MODE;
0716   message[MSG_F_LENGTH]     = 4;
0717   message[MSG_F_MOD_TYPE]   = 0x08;
0718   message[MSG_F_MOD_SERIAL] = 0x02;
0719 
0720   send_message_to_uart(1, message); 
0721 }
0722 
0723 /* Sets the OUTPUT register of the LED module to a
0724  * specified value.
0725  */
0726 
0727 static void lin_led_set(alt_u8 data)
0728 {
0729   char message[7];
0730     
0731   memset(message, '\0', 7);
0732     
0733   message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
0734   message[MSG_F_LENGTH]     = 6;
0735   message[MSG_F_MOD_TYPE]   = MOD_LED;
0736   message[MSG_F_MOD_SERIAL] = 0x00;
0737   message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_LED_OUTPUT;
0738   message[MSG_F_DATA]       = data;
0739     
0740   send_message_to_uart(1, message);   
0741 }
0742 
0743 /* Initializes the PWM module of the lin with the
0744  * appropriate values in the period registers.
0745  */
0746  
0747 static void lin_pwm_init_period()
0748 {
0749   char message[7];
0750     
0751   memset(message, '\0', 7);
0752     
0753   message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
0754   message[MSG_F_LENGTH]     = 6;
0755   message[MSG_F_MOD_TYPE]   = MOD_PWM;
0756   message[MSG_F_MOD_SERIAL] = 0x00;
0757   message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_PERIOD_H;
0758   message[MSG_F_DATA]       = VAL_PWM_PERIOD_H;
0759     
0760   send_message_to_uart(UART1, message);   
0761 
0762   message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_PERIOD_L;
0763   message[MSG_F_DATA]       = VAL_PWM_PERIOD_L;
0764     
0765   send_message_to_uart(UART1, message);   
0766 }
0767 
0768 /* Sends messages to the PWM module of the LIN such
0769  * that the robot comes to a full stop.
0770  */
0771 
0772 static void lin_pwm_full_stop()
0773 {
0774   char message[7];
0775     
0776   memset(message, '\0', 7);
0777     
0778   message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
0779   message[MSG_F_LENGTH]     = 6;
0780   message[MSG_F_MOD_TYPE]   = MOD_PWM;
0781   message[MSG_F_MOD_SERIAL] = 0x00;
0782   message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
0783   message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_H;
0784     
0785   send_message_to_uart(UART1, message);   
0786 
0787   message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
0788   message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_L;
0789     
0790   send_message_to_uart(UART1, message);   
0791 
0792   message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
0793   message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_H;
0794     
0795   send_message_to_uart(UART1, message);   
0796 
0797   message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
0798   message[MSG_F_DATA]       = VAL_PWM_DUTY_STOP_L;
0799     
0800   send_message_to_uart(UART1, message);   
0801 }
0802 
0803 /* Sends messages to the LIN such that the robot starts
0804  * moving forward.
0805  */
0806 
0807 static void lin_pwm_move_forward()
0808 {
0809     char message[7];
0810     
0811     memset(message, '\0', 7);
0812         
0813     message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
0814     message[MSG_F_LENGTH]     = 6;
0815     message[MSG_F_MOD_TYPE]   = MOD_PWM;
0816     message[MSG_F_MOD_SERIAL] = 0x00;
0817     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
0818     message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_H;
0819     
0820     send_message_to_uart(UART1, message);
0821     
0822     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
0823     message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_FORWARD_H;
0824     send_message_to_uart(UART1, message);
0825     
0826     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
0827     message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_L;
0828     send_message_to_uart(UART1, message);
0829     
0830     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
0831     message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_FORWARD_L;
0832     send_message_to_uart(UART1, message);
0833     
0834     usleep(1800000);
0835 }
0836 
0837 /* Sends messages to the LIN such that the robot starts
0838  * moving backwards.
0839  */
0840 
0841 static void lin_pwm_move_backward()
0842 {
0843 
0844     char message[7];
0845     
0846     memset(message, '\0', 7);
0847         
0848     message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
0849     message[MSG_F_LENGTH]     = 6;
0850     message[MSG_F_MOD_TYPE]   = MOD_PWM;
0851     message[MSG_F_MOD_SERIAL] = 0x00;
0852     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
0853     message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_BACKWARD_H;
0854     
0855     send_message_to_uart(UART1, message);
0856     
0857     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
0858     message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_BACKWARD_H;
0859     send_message_to_uart(UART1, message);
0860     
0861     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
0862     message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_BACKWARD_L;
0863     send_message_to_uart(UART1, message);
0864     
0865     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
0866     message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_BACKWARD_L;
0867     send_message_to_uart(UART1, message);
0868     
0869     usleep(1800000);
0870 
0871 }
0872 
0873 /* Sends messages to the LIN such that the robot starts
0874  * rotating clockwise.
0875  */
0876 
0877 static void lin_pwm_rotate_cw()
0878 {
0879     char message[7];
0880     
0881     memset(message, '\0', 7);
0882         
0883     message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
0884     message[MSG_F_LENGTH]     = 6;
0885     message[MSG_F_MOD_TYPE]   = MOD_PWM;
0886     message[MSG_F_MOD_SERIAL] = 0x00;
0887     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_H;
0888     message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_H;
0889     
0890     send_message_to_uart(UART1, message);
0891     
0892     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_H;
0893     message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_BACKWARD_H;
0894     send_message_to_uart(UART1, message);
0895     
0896     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M2_DUTY_L;
0897     message[MSG_F_DATA]       = VAL_PWM_DUTY_LEFT_FORWARD_L;
0898     send_message_to_uart(UART1, message);
0899     
0900     message[MSG_F_REG_NO]     = REG_MASK_WRITE | REG_RW_PWM_M1_DUTY_L;
0901     message[MSG_F_DATA]       = VAL_PWM_DUTY_RIGHT_BACKWARD_L;
0902     send_message_to_uart(UART1, message);
0903 
0904     // Reset counters
0905     lin_dig_in_reset_counters();
0906     
0907     char buffer[16];
0908     int loc = 0;
0909     do
0910     {
0911         lin_dig_in_read_counters();
0912         sprintf(buffer, "counter_left=%d", counter_left);
0913         displayMsgLCD(buffer);
0914         usleep(40000);
0915     }while(counter_left < 58);
0916     
0917     lin_pwm_full_stop();
0918 }
0919 
0920 /* Sends messages to the LIN such that the robot starts
0921  * rotating counterclockwise.
0922  */
0923 
0924 static void lin_pwm_rotate_ccw()
0925 {
0926 
0927 /* Replace this comment with your code. */
0928 
0929 }
0930 
0931 /* Reads the current counter value */
0932 
0933 static void lin_dig_in_read_counters()
0934 {   
0935   char message[6];
0936     
0937   memset(message, '\0', 6);
0938     
0939   message[MSG_F_TYPE]       = MSG_O_READ_MOD;
0940   message[MSG_F_LENGTH]     = 5;
0941   message[MSG_F_MOD_TYPE]   = MOD_DIG_IN;
0942   message[MSG_F_MOD_SERIAL] = 0x00;
0943   message[MSG_F_REG_NO]     = REG_MASK_READ | VAL_RIGHT_WHEEL;;
0944 
0945   send_message_to_uart(UART1, message);
0946   
0947    message[MSG_F_REG_NO]     = REG_MASK_READ | VAL_LEFT_WHEEL;;
0948    send_message_to_uart(UART1, message);
0949   
0950   // Check reply now
0951   int reply_left = 0;
0952   int reply_right = 0;
0953   
0954   while(!reply_left && !reply_right)
0955   {
0956       if (global_lin_char_current != global_lin_char_next) {
0957           #ifdef DEBUG_CHAR_DISPATCH
0958             printf("[DEBUG-CHAR-DISPATCH] Dispatch condition hit: %d != %d\n",
0959               global_lin_char_current, global_lin_char_next);
0960           #endif
0961     
0962           /* Dispatch the outstanding character */
0963           lin_char_dispatch(&global_lin_char_queue[global_lin_char_current]);
0964       }
0965   }
0966   
0967   reply_left = 0;
0968   reply_right = 0;
0969 }
0970 
0971 /* Resets the counters of the Digital Inputs module. */
0972 
0973 static void lin_dig_in_reset_counters()
0974 {
0975   char message[6];
0976     
0977   memset(message, '\0', 6);
0978     
0979   message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
0980   message[MSG_F_LENGTH]     = 5;
0981   message[MSG_F_MOD_TYPE]   = MOD_DIG_IN;
0982   message[MSG_F_MOD_SERIAL] = 0x00;
0983   message[MSG_F_REG_NO]     = CMD_DIG_IN_RESET_COUNTERS;
0984     
0985   send_message_to_uart(UART1, message);
0986   
0987   counter_left = 0;
0988   counter_right = 0;   
0989 }
0990 
0991 /* Enables the high-speed inputs of the 
0992  * Digital Inputs module.
0993  */
0994 
0995 static void lin_dig_in_enable_hsi()
0996 {
0997   char message[6];
0998     
0999   memset(message, '\0', 6);
1000     
1001   message[MSG_F_TYPE]       = MSG_O_WRITE_MOD;
1002   message[MSG_F_LENGTH]     = 5;
1003   message[MSG_F_MOD_TYPE]   = MOD_DIG_IN;
1004   message[MSG_F_MOD_SERIAL] = 0x00;
1005   message[MSG_F_REG_NO]     = CMD_DIG_IN_ENABLE_HSI;
1006     
1007   send_message_to_uart(UART1, message);   
1008 }
1009 
1010 /* Subscribes to the counters of the Digital Inputs 
1011  * module; sampling will be every 100 ms, and a message
1012  * will be generated only when values in the counters
1013  * change.
1014  */
1015 
1016 static void lin_dig_in_subscribe()
1017 {
1018   char message[9];
1019     
1020   memset(message, '\0', 9);
1021     
1022   message[MSG_F_TYPE]       = MSG_O_SUBSCRIBE;
1023   message[MSG_F_LENGTH]     = 8;
1024   message[MSG_F_MOD_TYPE]   = MOD_DIG_IN;
1025   message[MSG_F_MOD_SERIAL] = 0x00;
1026   message[MSG_F_REG_NO]     = REG_SUB_DIG_IN_COUNTER;
1027   message[MSG_F_DATA]       = 0x00;
1028   message[MSG_F_DATA + 1]   = 0x0A;
1029   message[MSG_F_DATA + 2]   = VAL_DIG_IN_SUB_CONFIG_DIFF_FILTER;
1030     
1031   send_message_to_uart(UART1, message);   
1032 }
1033 
1034 /* Initializes ("opens") the COM port of the LIN UART module                                                            
1035  * in order to receive packets.                                                                                         
1036  */                                                                                                                     
1037                                                                                                                         
1038 static void lin_uart_open()                                                                                             
1039 {                                                                                                                       
1040   char message[6];                                                                                                      
1041                                                                                                                         
1042   memset(message, '\0', 6);
1043 
1044   message[MSG_F_TYPE]       = MSG_O_OPEN_COM;
1045   message[MSG_F_LENGTH]     = 5;
1046   message[MSG_F_MOD_TYPE]   = MOD_UART;
1047   message[MSG_F_MOD_SERIAL] = 0x00;
1048   message[MSG_F_REG_NO]     = VAL_UART_PORT_NO;
1049 
1050   send_message_to_uart(UART1, message);
1051 }
1052 
1053 /* Sends a null-terminated string to the device attached to the
1054  * UART module.
1055  */
1056 
1057 static void lin_uart_send_message(alt_u8 *msg)
1058 {
1059   int i = 0;
1060   char message[255];
1061 
1062   memset(message, '\0', 255);
1063 
1064   message[MSG_F_TYPE]       = MSG_O_DATA_STREAM;
1065   message[MSG_F_LENGTH]     = 5 + strlen(msg);
1066   message[MSG_F_MOD_TYPE]   = MOD_UART;
1067   message[MSG_F_MOD_SERIAL] = 0x00;
1068   message[MSG_F_REG_NO]     = VAL_UART_PORT_NO;
1069 
1070   for (i = 0; i < strlen(msg); i++)
1071     message[MSG_F_DATA + i] = msg[i];
1072 
1073   send_message_to_uart(UART1, message);
1074 }

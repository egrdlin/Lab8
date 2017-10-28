#include "driverlib.h"
#include <stdio.h>
#include <stdint.h>
#include "RTC.h"

const eUSCI_I2C_MasterConfig i2cConfig = {
EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        3000000,                                // SMCLK = 3MHz
        EUSCI_B_I2C_SET_DATA_RATE_100KBPS,      // Desired I2C Clock of 400khz
        0,                                      // No byte counter threshold
        EUSCI_B_I2C_NO_AUTO_STOP                // No Autostop
        };

//Tera term configuration
const eUSCI_UART_Config uartConfig =
 {
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
         26,                                      // BRDIV = 26
         0,                                       // UCxBRF = 0
         0,                                       // UCxBRS = 0
         EUSCI_A_UART_NO_PARITY,                  // No Parity
         EUSCI_A_UART_LSB_FIRST,                  // MSB First
         EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
         EUSCI_A_UART_MODE,                       // UART mode
         EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION  // Low Frequency Mode
 };

unsigned char calendar[13];


int month_flag=0;
int day_flag=0;
int year_flag=0;
int DOW_flag=0;
int hour_flag=0;
int min_flag=0;
int sec_flag=0;
short temp=0;

void uart_init()
{
    /* Selecting P1.2 and P1.3 in UART mode. */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
        GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);
}

void I2C_init() //EGR 326 lecture slide
{
    // For example, select Port 6 for I2C
    // Set Pin 4, 5 to input Primary Module Function,
    // P6.4 is UCB1SDA, P6.5 is UCB1SCL
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
    GPIO_PIN4 + GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);
    // Initializing I2C Master (see description in Driver Lib for
    // proper configuration options)
    MAP_I2C_initMaster(EUSCI_B1_BASE, &i2cConfig);
    // Specify slave address
    MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, RTC_ADDRESS);
    // Set Master in transmit mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    // Enable I2C Module to start operations
    MAP_I2C_enableModule(EUSCI_B1_BASE);
}

void Port4_Initb() // Initiate port pin 1.0 as input with internal pull up resistor
{
    int i;
    for (i=0;i<4;i++)
    {
        P4SEL0 &= ~BIT(i);
        P4SEL1 &= ~BIT(i); // 1) configure P4.0 4.1 4.2 4.3 as GPIO
        P4DIR &= ~BIT(i); // 2) make P4.0 4.1 4.2 4.3 inputs
        P4REN |= BIT(i); // 3) enable pull resistors on P4.0 4.1 4.2 4.3
        P4OUT |= BIT(i); // P4.0 4.1 4.2 4.3 are pull-up
    }

}

void user_prompt(){
    printf("\n\rPlease set up your time for the Real Time Clock (RTC) as instructed");
    printf("\n\rFor Month - Enter values from 01-12 representing January to December respectfully ");
    printf("\n\rFor Day - Enter values from 01-31 representing the date of the month");
    printf("\n\rFor Year - Enter values from 00-99 representing the last two digits of the year");
    printf("\n\rFor Day of the week - Enter values from 1-7 representing Sunday to Saturday respectfully");
    printf("\n\rFor Hour - Enter values from 00-24 representing the hour");
    printf("\n\rFor Minute - Enter values from 00-60 representing the minute");
    printf("\n\rFor Second- Enter values from 00-60 representing the second");
    printf("\n\r************************************************************************************************");
}

void date_set() //asking user to using keypad to set the time
{
    /*
     * After user inputs, unsigned char calendar[13] has form:
     * __________________________________
     * |M M D D Y Y DOW H H M M   S   S |
     * |0 1 2 3 4 5  6  7 8 9 10  11  12|
     * |________________________________|
     *
     * */
    printf("\n\n\rEnter the current time on the keypad:");

//****************************************************************************************************************************************

    while(month_flag == 0){
        printf("\n\rMonth: ");
        calendar[0] = keypad_getkey();              //get first digit of month
        printf("%c",calendar[0]);                   //print it
        calendar[1] = keypad_getkey();              //get second digit of month
        printf("%c",calendar[1]);                   // print it
        ck_valid();
    }

    printf("\tYou entered: ");
    //calendar will be ASCII char - 48 to gain its decimal form
    // *16 is for converting to its hex form so the slave device can read
    date[5] = (calendar[0]-48)*16+(calendar[1]-48);         //date will be transmitted to slave device
    printMonth(date[5]);                        //print entered info.
    printf(" Press # to confirm");             //prompt user to press # to confirm
    while(keypad_getkey()!='#');                             //stay here until confirmed

//****************************************************************************************************************************************
//Gather info for day
    reset_flag();
    while(day_flag ==0){
        printf("\n\rDay: ");                        //get first digit for the day
        calendar[2] = keypad_getkey();
        printf("%c",calendar[2]);                   //get second digit for the day
        calendar[3] = keypad_getkey();
        printf("%c",calendar[3]);
        ck_valid();
    }

    printf(" \tYou entered: ");
    date[4] = (calendar[2]-48)*16+(calendar[3]-48);         //see month section from the lines above
    printf("%x", date[4]);
    printf(" Press # to confirm");             //prompt user to press # to confirm
    while(keypad_getkey()!='#');                             //stay here until confirmed

//****************************************************************************************************************************************
//Gather info for year
    reset_flag();
    while(year_flag==0)
    {
    printf("\n\rYear: ");
    calendar[4] = keypad_getkey();
    printf("%c",calendar[4]);
    calendar[5] = keypad_getkey();
    printf("%c",calendar[5]);
    ck_valid();
    }

    date[6] = (calendar[4]-48)*16+(calendar[5]-48);
    printf("\tYou entered: 20");
    printf("%x", date[6]);
    printf(" Press # to confirm");             //prompt user to press # to confirm
    while(keypad_getkey()!='#');                             //stay here until confirmed

//****************************************************************************************************************************************
//Gather info for day of the week
    reset_flag();
    while(DOW_flag ==0)
    {
    printf("\n\rDay of the Week: ");
    calendar[6] = keypad_getkey();
    printf("%c",calendar[6]);
    ck_valid();
    }

    date[3] = (calendar[6]-48);
    printf("\tYou entered: ");
    printDay(date[3]);
    printf(" Press # to confirm");             //prompt user to press # to confirm
    while(keypad_getkey()!='#');                             //stay here until confirmed

//****************************************************************************************************************************************
//Gather info for hour
    reset_flag();
    while(hour_flag == 0)
    {
    printf("\n\rHours: ");
    calendar[7] = keypad_getkey();
    printf("%c",calendar[7]);
    calendar[8] = keypad_getkey();
    printf("%c",calendar[8]);
    ck_valid();
    }

    date[2] = (calendar[7]-48)*16+(calendar[8]-48);
    printf("\tYou entered: ");
    printf("%x", date[2]);
    printf(" Press # to confirm");             //prompt user to press # to confirm
    while(keypad_getkey()!='#');                             //stay here until confirmed

//****************************************************************************************************************************************
//Gather info for minute
    reset_flag();
    while(min_flag ==0)
    {
    printf("\n\rMinutes: ");
    calendar[9] = keypad_getkey();
    printf("%c",calendar[9]);
    calendar[10] = keypad_getkey();
    printf("%c",calendar[10]);
    ck_valid();
    }

    date[1] = (calendar[9]-48)*16+(calendar[10]-48);
    printf("\tYou entered: ");
    printf("%x", date[1]);
    printf(" Press # to confirm");             //prompt user to press # to confirm
    while(keypad_getkey()!='#');                             //stay here until confirmed

//****************************************************************************************************************************************
//Gather info for second
    reset_flag();
    while(sec_flag==0)
    {
    printf("\n\rSeconds: ");
    calendar[11] = keypad_getkey();
    printf("%c",calendar[11]);
    calendar[12] = keypad_getkey();
    printf("%c",calendar[12]);
    ck_valid();
    }

    date[0] = (calendar[11]-48)*16+(calendar[12]-48);
    printf("\tYou entered: ");
    printf("%x", date[0]);
    printf(" Press # to confirm\n\r");             //prompt user to press # to confirm
    while(keypad_getkey()!='#');                             //stay here until confirmed

}

char keypad_getkey() //EGR 326 Lecture
{
    do{
                     // assumes port 4 bits 0-3 are connected to rows
       int row, col; // bits 4,5,6 are connected to columns

           const char column_select[] = {0x10, 0x20, 0x40}; // one column is active
           // Activates one column at a time, read the input to see which column

           for (col = 0; col < 3; col++) {
           P4->DIR &= ~0xF0; // disable all columns, aka set them as input
           P4->DIR |= column_select[col]; // enable one column at a time
           P4->OUT &= ~column_select[col]; // drive the active output column to low
           __delay_cycles(10); // wait for signal to settle
           row = P4->IN & 0x0F; // read all rows

           if(row != 0x0F){ // if one input is pressed, go into debounce sequence
               while(!debounce(row)); // loop until read 4 new consecutive inputs (0)
               P4->OUT |= column_select[col]; // drive active column high
               break;
           }
           P4->OUT |= column_select[col]; // drve the active column high
           }

           P4->OUT |= 0xF0; // drive all columns high before disable
           P4->DIR &= ~0xF0; // disable all columns

           if (row == 0x0E) return  (48+col + 1); // key in row 0  +48 for converting int to ACII char
           if (row == 0x0D) return (48+3 + col + 1); // key in row 1
           if (row == 0x0B) return (48+6 + col + 1); // key in row 2
           if (row == 0x07)
           {
               if(col==0)                return '*';

               if(col==1)                return '0';

               if(col==2)                return '#';
           }
       }while(1);
}

int debounce(int row)
{
    static uint16_t State = 0;  // Current debounce status
    int select = 4; // Initialize as unused value for debugging

    if (row == 0x0E) select = 0;
    if (row == 0x0D) select = 1;
    if (row == 0x0B) select = 2;
    if (row == 0x07) select = 3;

    State = (State << 1) | (P4IN & BIT(select))>>select | 0xff00;    // Read switch
    return (State == 0xffff); // Return true if button released for 8 consecutive calls
}

void ck_valid()
{ //Maximum calendar: 1 2 3 1 9 9  7  2 3 5   9    5    9
  //                  M M D D Y Y DOW H H min min  sec  sec
    if (((calendar[0] - 48) * 10 + (calendar[1] - 48)) <= 12)
    {
        month_flag = 1;
    }
    if (((calendar[2] - 48) * 10 + (calendar[3] - 48)) <= 31)
    {
        day_flag = 1;
    }

    if (((calendar[4] - 48) * 10 + (calendar[5] - 48)) <= 99)
    {
        year_flag = 1;
    }
    if (((calendar[6] - 48)) <= 7 && ((calendar[6]-48))>= 1)
    {
        DOW_flag = 1;
    }

    if (((calendar[7] - 48) * 10 + (calendar[8] - 48)) < 24)
    {
        hour_flag = 1;
    }
    if (((calendar[9] - 48) * 10 + (calendar[10] - 48)) < 60)
    {
        min_flag = 1;
    }

    if (((calendar[11] - 48) * 10 + (calendar[12] - 48)) < 60)
    {
        sec_flag = 1;
    }

}

void printDay(char day){

    switch(day){
    case 01:
        printf("Sunday");
        break;
    case 02:
        printf("Monday");
        break;
    case 03:
        printf("Tuesday");
        break;
    case 04:
        printf("Wednesday");
        break;
    case 05:
        printf("Thursday");
        break;
    case 06:
        printf("Friday");
        break;
    case 07:
        printf("Saturday");
        break;
    }
}

void printMonth(char month){
    switch(month){
        case 0x01:
            printf("January");
            break;
        case 0x02:
            printf("February");
            break;
        case 0x03:
            printf("March");
            break;
        case 0x04:
            printf("April");
            break;
        case 0x05:
            printf("May");
            break;
        case 0x06:
            printf("June");
            break;
        case 0x07:
            printf("July");
            break;
        case 0x08:
            printf("August");
            break;
        case 0x09:
            printf("September");
            break;
        case 0x10:
            printf("October");
            break;
        case 0x11:
            printf("November");
            break;
        case 0x12:
            printf("December");
            break;
    }
}

void reset_flag() //reset input checking flags
{
     month_flag=0;
     day_flag=0;
     year_flag=0;
     DOW_flag=0;
     hour_flag=0;
     min_flag=0;
     sec_flag=0;
}

void RTC_write() //EGR 326 lecture slide
{
    // Setting to Sunday, November 11, 2011 at 11:11:11 so I don't have to enter it every time for testing
    unsigned char date[15]={0x11, 0x11, 0x11, 0x01, 0x11, 0x11, 0x11, 0};
    /* Set Master in transmit mode */
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    // Wait for bus release, ready to write
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
    // set pointer to beginning of RTC registers
    MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE,0);
    // and write to seconds register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE,date[0]);
    // write to minutes register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, date[1]);
    // write to hours register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, date[2]);
    // write to day register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, date[3]);
    // write to date register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, date[4]);
    // write to months register
    MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, date[5]);
    // write to year register and send stop
    MAP_I2C_masterSendMultiByteFinish(EUSCI_B1_BASE, date[6]);
}


float RTC_read_temperature()
{
    uint8_t tUBYTE = 0;
    uint8_t tLBYTE = 0;

    // Set Master in transmit mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    // Wait for bus release, ready to write
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
    // set pointer to beginning of RTC registers
    MAP_I2C_masterSendSingleByte(EUSCI_B1_BASE,RTC_TEMPERATURE_ADDRESS);
    // Wait for bus release
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
    // Set Master in receive mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
    // Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));

    // read from RTC registers (pointer auto increments after each read)
    tUBYTE = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
    tLBYTE = MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);

    return convertTemperature(tUBYTE,tLBYTE);
}


//https://github.com/adafruit/RTClib/issues/72
float convertTemperature(uint8_t tUBYTE, uint8_t tLBYTE){
    float fTemperatureCelsius;
    float fTemperatureFarenheit;

    if(tUBYTE & 0b10000000) //check if negative temperature
    {
       // perform conversion for 2s compliment
       tUBYTE  ^= 0b11111111;
       tUBYTE  += 0x1;
       fTemperatureCelsius = tUBYTE + ((tLBYTE >> 6) * 0.25);
       fTemperatureCelsius = fTemperatureCelsius * -1;
    }
    else
    {
        fTemperatureCelsius = tUBYTE + ((tLBYTE >> 6) * 0.25);
    }

    fTemperatureFarenheit = fTemperatureCelsius*1.8 + 32;

    return (fTemperatureFarenheit);
}

void RTC_read() //EGR 326 lecture slide
{
    // Set Master in transmit mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    // Wait for bus release, ready to write
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
    // set pointer to beginning of RTC registers
    MAP_I2C_masterSendSingleByte(EUSCI_B1_BASE,0);
    // Wait for bus release
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
    // Set Master in receive mode
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
    // Wait for bus release, ready to receive
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
    // read from RTC registers (pointer auto increments after each read)
    RTC_registers[0]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
    RTC_registers[1]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
    RTC_registers[2]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
    RTC_registers[3]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
    RTC_registers[4]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
    RTC_registers[5]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
    RTC_registers[6]=MAP_I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
}

void printDate(){
    printDay(RTC_registers[3]);
    printf(", ");
    printMonth(RTC_registers[5]);
    printf(" %x, 20%x", RTC_registers[4], RTC_registers[6]);
}

void printTime(){
    printf("%x:%x:%x", RTC_registers[2], RTC_registers[1], RTC_registers[0]);
}

void printDateTimeStored(int num_dates_stored){
    int i;
    for(i=0; i<num_dates_stored; i++){
        printDay(read_back_data[3+(i*7)]);
        printf(", ");
        printMonth(read_back_data[5+(i*7)]);
        printf(" %x, 20%x at ", read_back_data[4+(i*7)], read_back_data[6+(i*7)]);
        printf("%x:%x:%x", read_back_data[2+(i*7)], read_back_data[1+(i*7)], read_back_data[0+(i*7)]);
        printf("\r\n ");
    }
}

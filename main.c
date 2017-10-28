/* DriverLib Includes */
#include "driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include "LCD.h"
#include <stdio.h>
#include "ST7735.h"
#include "RTC.h"
//#include "msp.h"
//#include <string.h>

#define CALIBRATION_START 0x000200000

void Init48MHz();
void read_date();
//char RTC_registers[7]; //return read values from slave device

char dates_to_store[36];
//uint8_t read_back_data[16]; // array to hold values read back from flash
//char RTC_registers[7]; //return read values from slave device
uint8_t* addr_pointer; // pointer to address in flash for reading back values
void store_date();
void update_dates_to_store();
void clear_log();
//void display_logs();

int main(void)
 {
    /*
    // Reset dates to store
    int num_dates_stored=0;
    update_dates_to_store(num_dates_stored);
    store_date(); //write to slave device RTC, to set the date and time info
    */
    read_date();
    ST7735_InitR(INITR_REDTAB);
                             //check power off                          //clear_log();
                                                                        //update_dates_to_store();
                                                                        //store_date();
                                                                        //read_date();
    // halting the watch dog is done in the system_msp432p401r.c startup
    Init48MHz();// Setting MCLK to 48MHz for faster programming
    //uart_init();
    I2C_init(); //initiate I2C protocol
    Port4_Initb(); //initiate the port 4 pins for keypad input
    //user_prompt();
    /*
    date_set(); //prompt user to enter date info, it will then be dressed as the address passing to register

    RTC_write(); //write to slave device RTC, to set the date and time info

    RTC_read();
    RTC_read();
    */

    RTC_write(); //write to slave device RTC, to set the date and time info
    int i;
    // Initialize to 0s
        //for testing                                           //    for(i=0;i<35;i++){
                                                                //        read_back_data[i] = 0;
                                                                //    }

    RTC_read();
    RTC_read();
    ST7735_FillScreen(0);                 // set screen to black
    //ST7735_DrawString(0,0,"You set the date as",ST7735_GREEN);
    printDateLCD();
    printTimeLCD();

    /*
    printf(" and time as ");
    printTime();

    printf("\n\rPress '*' to log the current time and '#' to view the last 5 logs on the LCD");
*/
    while (1)
    {
       char key1;
       // printf("\n\rEntry: ");

        key1 = keypad_getkey();
        //printf("%c",key1);

        while (key1 != '*' && key1 !='#' ){
            //printf(" Invalid input, try again.");
           // printf("\n\rEntry: ");
            key1 = keypad_getkey();
           // printf("%c",key1);
        }

        if(key1 == '*'){

            RTC_read();
            // log date here
            update_dates_to_store();
            store_date();
            read_date();

            if(num_dates_stored < 5){
                num_dates_stored++;
                update_dates_to_store();
                store_date(); //write to slave device RTC, to set the date and time info
                read_date();
            }


            // compare read_back_data with RTC_registers
            //printf("\n\rYou logged \n\r ");
            printDateLCD();
            printTimeLCD();


        }

        if(key1 == '#'){
            //printf("\n\rDisplay logs");
           // display_logs(num_dates_stored);// Display logs on LCD
            printDateTimeStored_LCD();
        }
    }

}

// Read logs and number of logs from MSP 432 memory
void read_date(){
    addr_pointer = CALIBRATION_START+4; // point to address in flash for saved data
    //for(i=0; i<7; i++) {// read values in flash after programming
    //for(i=0; i<(7+(num_dates_stored*7)); i++) {// read values in flash after programming
    int i;
    for(i=0; i<36; i++) {// read values in flash after programming
        read_back_data[i] = *addr_pointer++;
        dates_to_store[i] = read_back_data[i];
    }
    num_dates_stored = read_back_data[35]; // TODO: need to convert to int?
}

// Write logs and number of logs to MSP 432 memory
void store_date(){
    uint8_t i; // index
    addr_pointer = CALIBRATION_START+4; // point to address in flash for saving data
    //for(i=0; i<16; i++) {// read values in flash before programming
    for(i=0; i<(7+(num_dates_stored*7)); i++) {// read values in flash before programming
        read_back_data[i] = *addr_pointer++;
    }
    /* Unprotecting Info Bank 0, Sector 0 */
    MAP_FlashCtl_unprotectSector(FLASH_INFO_MEMORY_SPACE_BANK0,FLASH_SECTOR0);
    /* Erase the flash sector starting CALIBRATION_START. */
    while(!MAP_FlashCtl_eraseSector(CALIBRATION_START));
    /* Program the flash with the new data. */
    //while (!MAP_FlashCtl_programMemory(simulatedCalibrationData, (void*) CALIBRATION_START+4, 16 )); // leave first 4 bytes unprogrammed
    while (!MAP_FlashCtl_programMemory(dates_to_store, (void*) CALIBRATION_START+4, 36 )); // leave first 4 bytes unprogrammed
    /* Setting the sector back to protected */
    MAP_FlashCtl_protectSector(FLASH_INFO_MEMORY_SPACE_BANK0,FLASH_SECTOR0);
    addr_pointer = CALIBRATION_START+4; // point to address in flash for saved data
    //for(i=0; i<7; i++) {// read values in flash after programming
    for(i=0; i<36; i++) {// read values in flash after programming
        read_back_data[i] = *addr_pointer++;
    }

}

void Init48MHz() { // sets the clock module to use the external 48 MHz crystal
    /* Configuring pins for peripheral/crystal usage */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
    CS_setExternalClockSourceFrequency(32000,48000000); // enables getMCLK, getSMCLK to know externally set frequencies
    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     *
    * we have to change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false); // false means that there are no timeouts set,will return when stable
    /* Initializing MCLK to HFXT (effectively 48MHz) */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

void update_dates_to_store(){
    int i;
    if(num_dates_stored<5){
        for(i=0;i<7;i++){
            dates_to_store[i+(num_dates_stored*7)] = RTC_registers[i]; // load dates_to_store with read date
        }
    }else{ // need to shift dates and put new one in first 7 spots

        for(i=0;i<(35-7);i++){ // Shift info up 7 spots
            //dates_to_store[i+7] = dates_to_store[i];
            //dates_to_store[i+7] = dates_to_store[i];
            dates_to_store[i] = dates_to_store[i+7];
        }

        for(i=0;i<7;i++){ // Add new date
            dates_to_store[i+28] = RTC_registers[i];
        }

    }
    dates_to_store[35]= num_dates_stored;
}

void display_logs(){
        /*
        int i;
        for(i=0; i<num_dates_stored; i++){
            printDay(read_back_data[3+(i*7)]);
            printf(", ");
            printMonth(read_back_data[5+(i*7)]);
            printf(" %x, 20%x at ", read_back_data[4+(i*7)], read_back_data[6+(i*7)]);
            printf("%x:%x:%x", read_back_data[2+(i*7)], read_back_data[1+(i*7)], read_back_data[0+(i*7)]);
            printf("\r\n ");
        }*/
    ST7735_FillScreen(0);                 // set screen to black
    int i;
    int x=5;
    int y=20;
    for(i=0; i<(num_dates_stored+1);i++){
        for(i=0; i<6; i++){
            ST7735_DrawChar(x+(i*20), y, read_back_data[i], ST7735_Color565(180, 240, 250), 0, 1);
        }
        y+=30;
    }

}

void clear_log()
{
    num_dates_stored = 0;
}

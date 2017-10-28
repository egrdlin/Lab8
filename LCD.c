#include "driverlib.h"
#include <stdio.h>
#include <stdint.h>
#include "LCD.h"
#include "RTC.h"
#include "ST7735.h"
#include <string.h>
void printDayLCD(char day) //read from RTC[3]
{
    switch(day){
    case 01:
        strcpy(return_char_day, "Sun" );
        break;
    case 02:
        strcpy(return_char_day, "Mon" );
        break;
    case 03:
        strcpy(return_char_day, "Tue" );
        break;
    case 04:
        strcpy(return_char_day, "Wed" );
        break;
    case 05:
        strcpy(return_char_day, "Thr" );
        break;
    case 06:
        strcpy(return_char_day, "Fri" );
        break;
    case 07:
        strcpy(return_char_day, "Sat" );
        break;
    }

}

void printMonthLCD(char month)//read from RTC[5]
{

    switch(month){
        case 0x01:
            strcpy(return_char_month, "Jan" );
            break;
        case 0x02:
            strcpy(return_char_month, "Feb" );
            break;
        case 0x03:
            strcpy(return_char_month, "Mar" );
            break;
        case 0x04:
            strcpy(return_char_month, "Apr" );
            break;
        case 0x05:
            strcpy(return_char_month, "May" );
            break;
        case 0x06:
            strcpy(return_char_month, "Jun" );
            break;
        case 0x07:
            strcpy(return_char_month, "Jul" );
            break;
        case 0x08:
            strcpy(return_char_month, "Aug" );
            break;
        case 0x09:
            strcpy(return_char_month, "Sep" );
            break;
        case 0x10:
            strcpy(return_char_month, "Oct" );
            break;
        case 0x11:
            strcpy(return_char_month, "Nov" );
            break;
        case 0x12:
            strcpy(return_char_month, "Dec" );
            break;
    }
}

void printDateLCD(){

    printDayLCD(RTC_registers[3]); //get day format Sun/Mon..
    printMonthLCD(RTC_registers[5]); //get month format Jan

    char str_date[20];
    ST7735_FillScreen(0);
    sprintf(str_date, "%s. %s - %x, 20%x", return_char_day, return_char_month, RTC_registers[4] , RTC_registers[6]); //format string: day-month-year
    ST7735_DrawString(0, 0, "Date: ", ST7735_GREEN);
    ST7735_DrawString(0, 1, str_date, ST7735_GREEN);
}

void printTimeLCD(){

        char str_time[20];

       sprintf(str_time,"%x:%x:%x",RTC_registers[2],RTC_registers[1],RTC_registers[0]); //format string hour:min:sec

       ST7735_DrawString(0, 2, "Time: ", ST7735_GREEN);
       ST7735_DrawString(0, 3, str_time, ST7735_GREEN);
}
///To fix
void printDateTimeStored_LCD(){
    int i;
    char str_stored_date[100];
    char str_stored_time[100];
    ST7735_FillScreen(0);
    for(i=0; i<num_dates_stored; i++){
        printDayLCD(read_back_data[3+(i*7)]);
        printMonthLCD(read_back_data[5+(i*7)]);


        sprintf(str_stored_date,"%s - %x, 20%x at",return_char_month,read_back_data[4+(i*7)],read_back_data[6+(i*7)]);
        sprintf(str_stored_time,"%x:%x:%x", read_back_data[2+(i*7)], read_back_data[1+(i*7)], read_back_data[0+(i*7)]);

        ST7735_DrawString(0, i*3, "Stored time: ", ST7735_GREEN);
        ST7735_DrawString(0, i*3+1, str_stored_date, ST7735_GREEN);
        ST7735_DrawString(0, i*3+2, str_stored_time, ST7735_GREEN);
    }

}


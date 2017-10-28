#ifndef _LCDH_
#define _LCDH_


void printDateLCD();
void printMonthLCD(char month);
void printDayLCD(char day);
//void printTimeLCD();
//void printDateTimeStored(int num_dates_stored);
char return_char_day[4];
char return_char_month[4];
void printTimeLCD();
void printDateTimeStored_LCD();
int num_dates_stored;
#endif

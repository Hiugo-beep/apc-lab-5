#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
int attr = 0x6E;
void play();
int melody[] = {
698,659,698,784,880,988,784,880,988,1047,
880,988,1047,1175,988,1047,
82,125,165,247,330,415,494,415,330,247,330,247,208,165,125,
82,0
};

int clock[] = {
75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,75,
150,75,75,75,75,75,75,75,75,75,75,75,75,75,75,
600,0
};
void setFrequency(unsigned int freq){
    //Порт 42 – канал 2, связан с динамиком
if(freq == 1) delay(200);
else {
	long base = 1193180, kd;
	// Устанавливаем режим 3 для второго канала таймера, 10 11 011 0 – канал, старший+младший байты, режим, формат (двоичный)
	outp(0x43, 0xB6); //Включение динамика 10 11 011 0 – канал, старший+младший байты, режим, формат (двоичный)
	kd = base / freq; //Коэффициент деления
	outp(0x42, kd % 256); //младший бит
	kd /= 256; 
	outp(0x42, kd);  //старшимй бит
	}
}
void play()
{
	for(int i=0;melody[i]!=0 || clock[i]!=0;i++){
		setFrequency(melody[i]);
		// Включаем громкоговоритель. Сигнал от
		// канала 2 таймера теперь будет проходить
		// на вход громкоговорителя
		outp(0x61, inp(0x61)|3);//turn ON
		//Пауза
		delay(clock[i]);
		// Выключаем громкоговоритель.
		outp(0x61, inp(0x61) & 0xfc);//turn OFF
	}
}



int msCounter = 0;
void interrupt (*intOld70)(...); //IRQ8 от часов реального времени
void interrupt intNew70(...)  { 
	msCounter++; 
	intOld70(); 
}

void interrupt (*intOld4A)(...); //для будильника
void interrupt intNew4A(...)  { 
system("cls");
printf("GET UP!!!");
	play(); 
	intOld4A(); 
}
int countTime = 0;
int secT, minT, hourT,  dayT, monthT, yearT;
struct VIDEO
{
	unsigned char symb;
	unsigned char attr;
};
void hexadecimal_notation(long x, int offset){
	char temp;
	int i;
	VIDEO far* screen = (VIDEO far *)MK_FP(0xB800,offset);
	char number[2];
	for(i=1;i>-1;i--) //поделить по битам
	{
		temp = x% 16;
		x /= 16;
		if(temp>=0 && temp <=9) {
			number[i]=temp + '0';
		}
		else {
		
			number[i]=temp + 55;//заполнить экран
		}
	}
	for(i=0;i<2;i++) {
		screen->symb=number[i];//заполнить экран
		screen->attr=attr;
		screen++;
	}
	if(countTime!=1) {
		screen->symb = '.';
		screen++;
	}
}
void saveTime() {
	outp(0x70, 0x00);
	secT = inp(0x71);
	outp(0x70, 0x02);
	minT = inp(0x71);
	outp(0x70, 0x04);
	hourT = inp(0x71);
	outp(0x70, 0x07);
	dayT = inp(0x71);
	outp(0x70, 0x08);
	monthT = inp(0x71);
	outp(0x70, 0x09);
	yearT = inp(0x71);
}
void loadTime() {
	int value;
	outp(0x70, 0xA);
	for(long i = 0;i<65535;i++) {
		value = inp(0x71);
		value /= 64;
		if (value == 0) break;
		if(i==65534) {
			printf("Can't access to RTC.\nPlease, try later");
			return;
		}		
	}
	outp(0x70,0xB);//
	outp(0x71, inp(0x71) | 0x80); //отключить внутренний цикл обновлении=я часов
	outp(0x70, 0x00);
	outp(0x71, secT);

	outp(0x70, 0x02);
	outp(0x71, minT);

	outp(0x70, 0x04);
	outp(0x71, hourT);

	outp(0x70, 0x07);
	outp(0x71, dayT);

	outp(0x70, 0x08);
	outp(0x71, monthT);

	outp(0x70, 0x09);
	outp(0x71, yearT);
	outp(0x70, 0xB);
	outp(0x71, inp(0x71) & 0x7f);//разрешить цикл обновлени часов
}

void readTime() {
	int sec, min, hour, dayWeek, day, month, year;
	outp(0x70, 0x00);
	sec = inp(0x71);
	outp(0x70, 0x02);
	min = inp(0x71);
	outp(0x70, 0x04);
	hour = inp(0x71);

	outp(0x70, 0x06);
	dayWeek = inp(0x71);
	outp(0x70, 0x07);
	day = inp(0x71);
	outp(0x70, 0x08);
	month = inp(0x71);
	outp(0x70, 0x09);
	year = inp(0x71);
	switch(dayWeek){
		case 1:	
			printf("\n\nSunday");
			break;
		case 2:
			printf("\n\nMonday");
			break;
		case 3:
			printf("\n\nTuesday");
			break;
		case 4:
			printf("\n\nWednesday");
			break;
		case 5:
			printf("\n\nThursday");
			break;
		case 6:
			printf("\n\nFriday");
			break;
		case 7:
			printf("\n\nSaturday");
			break;
	}
	countTime = 0;
	hexadecimal_notation(day,160);
	hexadecimal_notation(month,160+6);
	countTime++;
	hexadecimal_notation(year,160+12);
	countTime = 0;	
	hexadecimal_notation(hour, 0);
	hexadecimal_notation(min,6);
	countTime++;
	hexadecimal_notation(sec, 12);	
	
}
void setTime()
{
	int value;
do {
	outp(0x70, 0x0A);
} while( inp(0x71) & 0x80 );

outp(0x71, inp(0x71) | 0x80);
 //отключить внутренний цикл обновлении=я часов

outp(0x70, 0x00);
	outp(0x71, 5);

	outp(0x70, 0x02);
	outp(0x71, 5);

	outp(0x70, 0x04);
	outp(0x71, 5);


	outp(0x70, 0x07);
	outp(0x71, 5);


	outp(0x70, 0x08);
	outp(0x71, 5);


	outp(0x70, 0x09);
	outp(0x71, 5);

	outp(0x70, 0x0B);
	outp(0x71, inp(0x71) & 0x7f);// //разрешить цикл обновлени часов
}

void delay() {
	long delayP;
	int value;
	_disable();//запретить прерывания (cli)// перед записью дабы ничего не испортить
	intOld70 = getvect(0x70);
	setvect(0x70 , intNew70);
	_enable();//разрешить прерывания
	
	printf("Enter delay time in mlsec:");
	scanf("%ld", &delayP);
	printf("Delaying...");

	value = inp(0xA1);
	outp(0xA1,value & 0xFE);// 0xFE = 11111110, бит 0 в 0,
	outp(0x70, 0x0B); // Выбираем регистр B

	value = inp(0x0B); // Читаем содержимое регистра B

	outp(0x70, 0x0B); // Выбираем регистр B

	outp(0x71, value|0x40); // 0x40 = 01000000,
	msCounter = 0;
	while(msCounter != delayP); // ‡ ¤Ґа¦Є  ­  § ¤ ­­®Ґ Є®«-ў® ¬Ё««ЁбҐЄг­¤
	printf("\nEnd delay of %d ms\n", msCounter);
	_disable();//запретить прерывания (cli)// перед записью дабы ничего не испортить
	setvect(0x70 , intOld70);
	_enable();//разрешить прерывания


	outp(0x70, 0x0B); // Выбираем регистр B

	value = inp(0x0B); // Читаем содержимое регистра B

	outp(0x70, 0x0B); // Выбираем регистр B
	
	outp(0x71, value&0x40); // 0x40 = 10111111,
	value = inp(0xA1);
	outp(0xA1,value | 0xFE);// 0xFE = 00000001, бит 0 в 0,

}
void alarm(){

	printf("Enter hour alarm:\n");
	int hour, min, sec;

outp(0x70, 0x00);
	sec = inp(0x71);
	outp(0x70, 0x02);
	min = inp(0x71);
	outp(0x70, 0x04);
	hour = inp(0x71);
        min++;

	outp(0x70, 0x01);
	outp(0x71,  sec);

	outp(0x70, 0x03);
	outp(0x71,  min);

	outp(0x70, 0x04);
	outp(0x71,  hour);
	

	
	int value = inp(0xA1);
	outp(0xA1,value & 0xFE);// 0xFE = 11111110, бит 0 в 0,
	_disable();//запретить прерывания (cli)// перед записью дабы ничего не испортить
	intOld4A = getvect(0x4A);
	setvect(0x4A , intNew4A);
	_enable();//разрешить прерывания
	value = inp(0xA1);
	outp(0xA1,value & 0xFE);// 0xFE = 11111110, бит 0 в 0,
}

int main()
{
	char selection;
	saveTime();
        while (1) {
		printf("1: Read the time in RTC\n");
		printf("2: Set the time in RTC\n");
		printf("3: Millisecond precision delay\n");
		printf("4: Programmable alarm\n");
		printf("5: Exit(e)\n\n");
		selection = getch();
		switch (selection) {
			case '1':
				system("cls");
				readTime();
				getch();
				system("cls");
				break;  
			case '2':
				system("cls");
				setTime();
				getch();
				printf("OK!\n");
				getch();
				system("cls");
				break; 
			case '3':
				system("cls");
				delay();
				getch();
				system("cls");
				break;
			case '4':
				system("cls");
				alarm();
				getch();
                                system("cls");
				break;
			case '5':
				printf("Exiting...");
				loadTime();
				setvect(0x4A , intOld4A);
				return 0;
                         case 'e':
				printf("Exiting...");
				loadTime();
				setvect(0x4A , intOld4A);
				return 0;
			default:
				printf("Error key! Please try again\n");
				break;
		}
	}
}

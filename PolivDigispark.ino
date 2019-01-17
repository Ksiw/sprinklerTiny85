/*
  Name:    sprinkler.ino
  Created: 06.11.2018 22:41:02
  Author:  Ksiw
*/
//значени¤ и сна указываютс¤ кратно 4м секундам

#define LED 1          //нога светодиода
#define WORK 8      //сек качать воду 
#define SLEEP 3600     //1 ед - 8 сек // 8ч
#define WORKLOAD 3    //пин насоса
#define BUTTON 0

#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
//#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

//#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
uint32_t  timerSLEEP = 0;
byte adcsra_save;

// Вектор (функция) прерывания PCINT, у ATtiny85 один порт (B), следовательно только один вектор PCINT

// Вектор (функция) прерывания PCINT, у ATtiny85 один порт (B), следовательно только один вектор PCINT
ISR(PCINT0_vect)
{
	delay(50);
	hand_Work();
}





void setup()
{
	adc_disable();
	pinMode(BUTTON, INPUT); // кнопка с подт¤гивающим резистором
	digitalWrite(BUTTON, LOW);

	pinMode(WORKLOAD, OUTPUT);
	pinMode(LED, OUTPUT);
}

void loop()
{
	digitalWrite(LED, HIGH);
	delay(2);
	//myWatchdogEnable(WDTO_15MS);  // засыпаем
	digitalWrite(LED, LOW);
	if (digitalRead(BUTTON)) 
		hand_Work();

	if (SLEEP <= timerSLEEP)
		func_Work();

	  /*wdt_enable(WDTO_8S);
		Возможные значения для константы
		WDTO_15MS    WDTO_30MS    WDTO_60MS    WDTO_120MS    WDTO_250MS    WDTO_500MS    WDTO_1S    WDTO_2S    WDTO_4S    WDTO_8S
	  */
	myWatchdogEnable(WDTO_8S);  // засыпаем на 8 секунд
	timerSLEEP++;
}
//--------------------------------------------------------------------------------------
// прерывание сторожевого таймера
ISR(WDT_vect)
{
	WDTCR |= _BV(WDIE);
}
//----------------------------------------------------------------------------------------
void myWatchdogEnable(const byte interval)
{
	GIMSK = _BV(PCIE); // Включить Pin Change прерывания
	PCMSK |= _BV(BUTTON); // PCINT3; включить если нет проблем с освещением или иначе нет шансов проснуться
	ADCSRA &= ~_BV(ADEN); // отключить ADC; уменьшает энергопотребление
	wdt_enable(interval); // установить таймер
	WDTCR |= _BV(WDIE); // включить прерывания от таймера; фикс для ATtiny85

	// Установить режим сна Power-down; MCUSR &= ~_BV(SM0); MCUSR |= _BV(SM1);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable(); // разрешить режим сна; MCUSR |= _BV(SE);
	sei(); // включить прерывания

	sleep_cpu(); // заснуть

	cli(); // отключить прерывания; для безопасного отключения PCINT3
	PCMSK &= ~_BV(BUTTON); // PCINT3; отключить
	sleep_disable(); // запретить режим сна; MCUSR &= ~_BV(SE);
	//ADCSRA |= _BV(ADEN); // включить ADC
	sei(); // включить прерывания; иначе таймеры не будут работать
	
	////noInterrupts();
	//wdt_reset();
	//wdt_enable(interval);
	//WDTCR |= _BV(WDIE);
	//adcsra_save = ADCSRA;
	//ADCSRA = 0;  // запрещаем работу ј÷ѕ
	//power_all_disable();   // выключаем все модули
	//set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // устанавливаем режим сна
	//sleep_enable();
	////attachInterrupt(0, hand_Work, LOW);   // позвол¤ем заземлить pin 2 дл¤ выхода из сна
	//interrupts();
	//sleep_cpu();            // переходим в сон и ожидаем прерывание
	////detachInterrupt(0);     // останавливаем прерывание LOW
	//ADCSRA = adcsra_save;  // останавливаем понижение питани¤
	//power_all_enable();   // включаем все модули
}
//---------------------------------------------------------------------------------------------
void func_Work()
{
	digitalWrite(WORKLOAD, HIGH);
	digitalWrite(LED, HIGH);
	delay(WORK * 1000);

	digitalWrite(WORKLOAD, LOW);
	digitalWrite(LED, LOW);
	timerSLEEP = 0;
}

void hand_Work()
{
	wdt_disable();  // отключаем сторожевой таймер

	while (digitalRead(BUTTON))
	{
		digitalWrite(WORKLOAD, HIGH);
		digitalWrite(LED, HIGH);
		delay(100);
	}

	digitalWrite(WORKLOAD, LOW);
	digitalWrite(LED, LOW);
	timerSLEEP = 0;
	//myWatchdogEnable(WDTO_1S);
}

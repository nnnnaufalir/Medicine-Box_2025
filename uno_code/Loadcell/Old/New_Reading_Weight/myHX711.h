#ifndef myHX711_h
#define myHX711_h

#include <HX711.h>

class myHX711
{
private:
	long myOFFSET = 0;		 // used for tare weight
	float mySCALE = 1;		 // used to return weight in grams, kg, ounces, whatever
	float myREAD = 0.0f;	 // used for reading values
	HX711 _hardware_scale; // instance of the myHX711 class to handle hardware operations

public:
	myHX711();
	void begin(byte dout, byte pd_sck, byte gain = 128);
	bool is_ready();
	void set_gain(byte gain = 128);
	long read();
	void wait_ready(unsigned long delay_ms = 0);
	bool wait_ready_retry(int retries = 3, unsigned long delay_ms = 0);
	bool wait_ready_timeout(unsigned long timeout = 1000, unsigned long delay_ms = 0);
	void power_down();
	void power_up();
	float filter_read(float raw_reading = 0.0f);
	long read_average(byte times = 10);
	double get_value(byte times = 1);
	float get_units(byte times = 1);
	void tare(byte times = 10);
	void set_scale(float scale = 1.f);
	float get_scale();
	void set_offset(long offset = 0);
	long get_offset();
};

#endif

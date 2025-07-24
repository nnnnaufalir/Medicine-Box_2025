#include <Arduino.h>
#include "myHX711.h"

myHX711::myHX711() {}

void myHX711::begin(byte dout, byte pd_sck, byte gain)
{
	_hardware_scale.begin(dout, pd_sck, gain);
}

bool myHX711::is_ready()
{
	return _hardware_scale.is_ready();
}

void myHX711::set_gain(byte gain)
{
	_hardware_scale.set_gain(gain);
}

long myHX711::read()
{
	return _hardware_scale.read();
}

void myHX711::wait_ready(unsigned long delay_ms)
{
	_hardware_scale.wait_ready(delay_ms);
}

bool myHX711::wait_ready_retry(int retries, unsigned long delay_ms)
{
	return _hardware_scale.wait_ready_retry(retries, delay_ms);
}

bool myHX711::wait_ready_timeout(unsigned long timeout, unsigned long delay_ms)
{
	return _hardware_scale.wait_ready_timeout(timeout, delay_ms);
}

void myHX711::power_down()
{
	_hardware_scale.power_down();
}

void myHX711::power_up()
{
	_hardware_scale.power_up();
}

float myHX711::filter_read(float raw_reading)
{
	myREAD = raw_reading;
	return myREAD;
}

long myHX711::read_average(byte times)
{
	long sum = 0;
	for (byte i = 0; i < times; i++)
	{
		sum += myREAD;
		delay(1);
	}
	return sum / times;
}

double myHX711::get_value(byte times)
{
	return read_average(times) - myOFFSET;
}

float myHX711::get_units(byte times)
{
	return get_value(times) / mySCALE;
}

void myHX711::tare(byte times)
{
	double sum = read_average(times);
	set_offset(sum);
}

void myHX711::set_scale(float scale)
{
	mySCALE = scale;
}

float myHX711::get_scale()
{
	return mySCALE;
}

void myHX711::set_offset(long offset)
{
	myOFFSET = offset;
}

long myHX711::get_offset()
{
	return myOFFSET;
}

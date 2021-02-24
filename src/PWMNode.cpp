/*
 * PWMNode.cpp
 *
 * HomieNode to control a single channel PWM output.
 *
 * Loosely based on PWM example from Roger Marvin.
 *
 * 'sequence' is advertised and when a new JSON dataset is posted
 * to <Homie base topic>/<deviceID>/<NodeID>/sequence/set it is parsed
 * for a sequence of t-v (time-value) pairs.
 *
 * Each 't' represents the time in milliseconds since the previous event.
 * The corresponding 'v' represents an ouput level from 0 to 100 that is
 * reached at the end of this time segment.
 *
 * Several t-v pairs can be run in sequence, which continuously repeats
 * from the start.
 *
 * sync() is optionally called from a Broadcast Handler.  This is used to
 * apply a time offset to align the local system clock to a reported time
 * being broadcast.  A periodic broadcast (e.g. 10 seconds) will keep all
 * controllers synchronised, so distributed light fades and sequences can be
 * achieved.
 */

#include "PWMNode.h"
#include "Homie.hpp"
#include "LoggerNode.h"
#include "Arduino.h"
#include <string>
#include <iostream>

using namespace std;

// Gamma correction, for details see https://learn.adafruit.com/led-tricks-gamma-correction/
// This table maps [0%-100%] to [0-1023] (PWMRANGE of ESP8266's arduino.h)
// See tools directory for C++ program to create table for different ranges

const uint16_t /*PROGMEM*/ PWMNode::gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  2,  2,  3,  3,  4,  5,
    6,  7,  8, 10, 11, 13, 15, 17, 19, 21, 24, 26, 29, 32, 35, 39,
   42, 46, 50, 54, 59, 63, 68, 73, 79, 84, 90, 96,103,109,116,124,
  131,139,147,155,164,173,182,192,202,212,223,234,245,257,269,281,
  293,307,320,334,348,362,377,392,408,424,441,458,475,493,511,529,
  548,568,587,608,628,650,671,693,716,739,762,786,811,836,861,887,
  913,940,968,996,1023 };

PWMNode::PWMNode(const char* name, uint8_t pin) :
		HomieNode(name, "LED PWM Output", "pwm"),
		output_pin(pin),
		initialized(false)
{
	if (output_pin != NOPIN) {
		advertise("sequence").settable().setRetained().setDatatype("json");
	}

	setRunLoopDisconnected(true);
}

GAMMATYPE PWMNode::gammaConvert(VALUETYPE value) {
	if (value > 100) {
		value = 100;
	}

	return 1023 - gamma8[value];
}

bool PWMNode::handleInput(const HomieRange& range, const String& property, const String& value) {
	if (property.compareTo("sequence") == 0) {
		sequence.clear();

		size_t ix = 0;
		KEYTYPE time = 0;

		while (value[ix]) {
			if (value[ix++] == '[') break;
		}

		while (value[ix]) {
			// move to 't'
			while (value[ix]) {
				if (value[ix++] == 't') break;
			}

			if (!value[ix]) break;

			// get first parameter
			while (value[ix]) {
				if (value[ix++] == ':') break;
			}

			// locate opening quote
			while (value[ix]) {
				if (value[ix++] == '\"') break;
			}

			size_t cur = ix;

			// locate closing quote
			while (value[ix]) {
				if (value[ix++] == '\"') break;
			}

			String s1 = value.substring(cur, ix-1);
			KEYTYPE t = atoi(s1.c_str());

			// get first parameter
			while (value[ix]) {
				if (value[ix++] == ':') break;
			}

			// locate opening quote
			while (value[ix]) {
				if (value[ix++] == '\"') break;
			}

			cur = ix;

			// locate closing quote
			while (value[ix]) {
				if (value[ix++] == '\"') break;
			}

			String s2 = value.substring(cur, ix-1);
			VALUETYPE v = atoi(s2.c_str());

			sequence[time] = v;	// gammaConvert(v);

			time += t;
		}

		sequence[time] = sequence[0];
		sequence_length = time;
	} else {
		return false;
	}

	setProperty(property).send(value);
	return true;
}

void PWMNode::sync(uint64_t delta_time) {
	_delta_time = delta_time;
}

void PWMNode::setup() {
}

void PWMNode::loop() {
	uint64_t now = millis() + _delta_time;

	if (sequence_length > 0) {
		KEYTYPE phase = (KEYTYPE)(now % (unsigned long)sequence_length);

		if (sequence.count(phase)) {
			// we are exactly on point
			analogWrite(output_pin, gammaConvert(sequence[phase]));
		} else {
			//time to interpolate
			std::map<KEYTYPE,VALUETYPE>::iterator it;

			it = sequence.lower_bound(phase);
			it--;

			KEYTYPE prev_t = it->first;
			uint32_t prev_v = (uint32_t)it->second;

			it++;
			KEYTYPE next_t = it->first;
			uint32_t next_v = (uint32_t)it->second;

			uint32_t interp = (phase - prev_t) * next_v + (next_t - phase) * prev_v + (next_t - prev_t) / 2;

			analogWrite(output_pin, gammaConvert((VALUETYPE)(interp / (next_t - prev_t))));
		}
	}
}

void PWMNode::onReadyToOperate() {
	initialized = true;
}

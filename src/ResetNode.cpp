/*
 * ResetNode.cpp
 *
 * Monitors the 'reset' property, and when 'true' is published via
 * <Homie base topic>/<deviceID>/RESET/reset the do_reset flag is set.
 *
 * isDoReset() is called from the main application and an appropriate
 * action is taken.
 *
 * In the case of the HomiePWM application the SPIFFS config file is deleted
 * forcing a reconfigure on next boot.
 */

#include "ResetNode.h"
#include "Homie.hpp"
#include "LoggerNode.h"
#include "Arduino.h"
#include <string>
#include <iostream>

using namespace std;

ResetNode::ResetNode(const char* name) :
		HomieNode(name, "Reset Node", "rst"),
		initialized(false)
{
    advertise("reset").settable().setDatatype("boolean");
}

bool ResetNode::handleInput(const HomieRange& range, const String& property, const String& value) {
	if (property == "reset") {
  		if (value == "true") {
			do_reset = true;
		}
	} else {
		return false;
	}

	setProperty(property).send(value);
	return true;
}

void ResetNode::setup() {
	do_reset = false;
}

void ResetNode::loop() {
}

void ResetNode::onReadyToOperate() {
}

bool ResetNode::isDoReset() {
	return do_reset;
}
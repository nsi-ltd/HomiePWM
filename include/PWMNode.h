/*
 * PWMNode.h
 */

#ifndef SRC_PWMNODE_H_
#define SRC_PWMNODE_H_

#include <HomieNode.hpp>
#include <map>

typedef uint16_t KEYTYPE;
typedef uint8_t VALUETYPE;
typedef uint16_t GAMMATYPE;

class PWMNode: public HomieNode {
public:
	/* Define output pins:
	 *   In general, the ESP8266 supports only 12mA on its GPIO (source or sink), so please connect
	 *   some current amplifier, e.g. a darlington array like ULN2003 or ULN2803.
	 *   With these using GPIO 0, 2, and 15 is possible, when they are pulled to GND or HIGH with
	 *   e.g. 3k3 Ohm resistors.
	 *   Note that the LED connected to PINs that are pulled to VCC (0 and 2) will be switched on
	 *   during reset (until RGWNode::setup() is called from within Homie.setup()).
	 */
	
    static const uint8_t NOPIN = 0xFF;

	PWMNode(const char* name, uint8_t pin = PWMNode::NOPIN);

	void setup();
    void onReadyToOperate();

	void sync(uint64_t time);

private:
	static const uint16_t /*PROGMEM*/ gamma8[];
    uint8_t output_pin = PWMNode::NOPIN;
	uint64_t _delta_time;

	// linked list of duration, value pairs.  Duration in milliseconds
	std::map<KEYTYPE,VALUETYPE> sequence;

	// sequence length in milliseconds, 1 - 65,535
	KEYTYPE sequence_length = 0;

    // HomieNode
	virtual bool handleInput(const HomieRange& range, const String  &property, const String &value) override;

	virtual void loop() override;

    GAMMATYPE gammaConvert(VALUETYPE value);
    bool initialized;
};

#endif /* SRC_PWMNODE_H_ */

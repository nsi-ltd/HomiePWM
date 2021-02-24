/*
 * ResetNode.h
 */

#ifndef SRC_RESETNODE_H_
#define SRC_RESETNODE_H_

#include <HomieNode.hpp>

class ResetNode: public HomieNode {
public:

	ResetNode(const char* name);

	void setup();
    void onReadyToOperate();
	bool isDoReset();

private:
    bool initialized;
	bool do_reset;

    // HomieNode
	virtual bool handleInput(const HomieRange& range, const String  &property, const String &value) override;

	virtual void loop() override;
};

#endif /* SRC_RESETNODE_H_ */

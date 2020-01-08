#pragma once
#include "quantum.h"

class MyQASMIntepreter {
private:
	QRegistry* registry;

public:
	MyQASMIntepreter(int registry_size) {
		registry = new QRegistry(registry_size);
	}

	~MyQASMIntepreter() { delete registry; }
};
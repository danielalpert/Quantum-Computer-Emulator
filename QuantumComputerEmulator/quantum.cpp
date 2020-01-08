#include "quantum.h"
#include <complex>
#include <cmath>
#include <iostream>

using namespace std;

bool bit(int n, int b) {
	if (b == 0) return (n % 2 == 1);
	return bit(n / 2, b - 1);
}

void GateInstruction::operator()(QRegistry& registry) const {
	if (this->size() > registry.size()) throw runtime_error("registry not large enough");
	
	int pw = (int) pow(2, registry.size());
	
	complex<double>* rstate = registry.registry;
	complex<double>* new_rstate = new complex<double>[pw];

	for (int i = 0; i < pw; i++) new_rstate[i] = 0;

	int q = target_;
	Gate gate = gate_;


	for (int i = 0; i < pw; i++) {
		if (bit(i, q)) {
			new_rstate[i] += rstate[i] * gate.state1()->state1();
			new_rstate[i - (int)pow(2, q)] += rstate[i] * gate.state1()->state0();
		} else {
			new_rstate[i + (int)pow(2, q)] += rstate[i] * gate.state0()->state1();
			new_rstate[i] += rstate[i] * gate.state0()->state0();
		}
	}

	delete[] rstate;
	registry.registry = new_rstate;
}

void CGateInstruction::operator()(QRegistry& registry) const {
	if (this->size() > registry.size()) throw runtime_error("registry not large enough");
	
	int pw = (int)pow(2, registry.size());

	complex<double>* rstate = registry.registry;
	complex<double>* new_rstate = new complex<double>[pw];

	for (int i = 0; i < pw; i++) new_rstate[i] = 0;

	int c = control_;
	int q = target_;
	CGate gate = gate_;

	if (c == q) throw runtime_error("error: control qubit must be different from target qubit");

	for (int i = 0; i < pw; i++) {
		if (!bit(i, c)) {
			new_rstate[i] += rstate[i];
			continue;
		} else {
			if (bit(i, q)) {
				new_rstate[i] += rstate[i] * gate.transform().state1()->state1();
				new_rstate[i - (int)pow(2, q)] += rstate[i] * gate.transform().state1()->state0();
			}
			else {
				new_rstate[i + (int)pow(2, q)] += rstate[i] * gate.transform().state0()->state1();
				new_rstate[i] += rstate[i] * gate.transform().state0()->state0();
			}
		}
	}

	delete[] rstate;
	registry.registry = new_rstate;
}

void Routine::operator()(QRegistry& registry) {
	if (registry.size() < size_) throw size_exception(registry.size());

	for (std::list<Instruction*>::iterator i = instructions.begin(); i != instructions.end(); i++) {
		(**i)(registry);
	}
}

int QRegistry::measure_all() {
	double random = ((double) rand()) / ((double) RAND_MAX);
	//cout << random << endl;
	double p = 0;

	int val = 0;

	for (int i = 0; i < pow(2, this->size()); i++) {
		//cout << registry[i] << endl;
		
		p += norm(registry[i]);
		if (p >= random) {
			val = i;
			break;
		}
	}

	for (int i = 0; i < pow(2, this->size()); i++) {
		if (i == val) registry[i] = 1;
		else registry[i] = 0;
	}

	return val;
}

#pragma once
#include "myqasm_interpreter.h"
#include "quantum.h"
#include <vector>
#include <cmath>
#include <complex>
#include <list>
#include <memory>
#include <utility>

//using namespace std::complex_literals;

extern QRegistry* registry;

namespace consts {
	extern std::complex<double> i;

	extern double pi;
}

class : public qasm::gate {
public:
	unsigned int paramc() const override { return 1; }

	unsigned int argc() const override { return 1; }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override {
		double th = params[0]; //angle of rotation, in radians
		const Qubit q1(cos(th / 2), -(consts::i) * sin(th / 2));
		const Qubit q2(-consts::i * sin(th / 2), cos(th / 2));
		const Gate gate(q1, q2);

		unsigned int q = args[0]; //target qubit

		GateInstruction i(gate, q);
		i(*registry);
	}
} Rx;

class : public qasm::gate {
public:
	unsigned int paramc() const override { return 1; }

	unsigned int argc() const override { return 1; }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override {
		double th = params[0]; //angle of rotation, in radians
		const Qubit q1(cos(th / 2), sin(th / 2));
		const Qubit q2(-sin(th / 2), cos(th / 2));
		const Gate gate(q1, q2);

		unsigned int q = args[0]; //target qubit

		GateInstruction i(gate, q);
		i(*registry);
	}
} Ry;

class : public qasm::gate {
public:
	unsigned int paramc() const override { return 1; }

	unsigned int argc() const override { return 1; }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override {
		double th = params[0]; //angle of rotation, in radians
		const Qubit q1(cos(th / 2) - consts::i * sin(th / 2), 0);
		const Qubit q2(0, cos(th / 2) + consts::i * sin(th / 2));
		const Gate gate(q1, q2);

		unsigned int q = args[0]; //target qubit

		GateInstruction i(gate, q);
		i(*registry);
	}
} Rz;

class : public qasm::gate {
public:
	unsigned int paramc() const override { return 1; }

	unsigned int argc() const override { return 1; }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override {
		double th = params[0];
		
		const Qubit q1(1, 0);
		const Qubit q2(0, cos(th) + consts::i * sin(th));
		const Gate gate(q1, q2);

		unsigned int q = args[0]; //target qubit

		GateInstruction i(gate, q);
		i(*registry);
	}
} Ph; //multiplies qubit by phase e^(i*theta) for state |1>, does nothing for phase |0>

class : public qasm::gate {
public:
	unsigned int paramc() const override { return 0; }

	unsigned int argc() const override { return 1; }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override {
		const Qubit q1(1, 0);
		const Qubit q2(0, cos(consts::pi / 4) + consts::i * sin(consts::pi / 4));
		const Gate gate(q1, q2);

		unsigned int q = args[0]; //target qubit

		GateInstruction i(gate, q);
		i(*registry);
	}
} T;

class : public qasm::gate {
public:
	unsigned int paramc() const override { return 0; }

	unsigned int argc() const override { return 1; }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override {
		const Qubit q1(1, 0);
		const Qubit q2(0, cos(consts::pi / 4) - consts::i * sin(consts::pi / 4));
		const Gate gate(q1, q2);

		unsigned int q = args[0]; //target qubit

		GateInstruction i(gate, q);
		i(*registry);
	}
} Tdag;

class : public qasm::gate {
public:
	unsigned int paramc() const override { return 0; }

	unsigned int argc() const override { return 1; }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override {
		const Qubit q1(1, 1);
		const Qubit q2(1, -1);
		const Gate gate(q1, q2);

		unsigned int q = args[0]; //target qubit

		GateInstruction i(gate, q);
		i(*registry);
	}
} H;

class : public qasm::gate {
public:
	unsigned int paramc() const override { return 0; }

	unsigned int argc() const override { return 2; }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override {
		const Qubit q1(0, 1);
		const Qubit q2(1, 0);
		const Gate nt(q1, q2); //not gate
		const CGate cnot(nt);

		unsigned int control = args[0]; //control qubit
		unsigned int target = args[1]; //target qubit

		CGateInstruction i(cnot, control, target);
		i(*registry);
	}
} CNot;

class : public qasm::gate {
public:
	unsigned int paramc() const override { return 0; }

	unsigned int argc() const override { return 2; }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override {
		const Qubit q1(1, 1);
		const Qubit q2(1, -1);
		const Gate nt(q1, q2); //not gate
		const CGate cnot(nt);

		unsigned int control = args[0]; //control qubit
		unsigned int target = args[1]; //target qubit

		CGateInstruction i(cnot, control, target);
		i(*registry);
	}
} CH;

class qasm::custom_gate : public qasm::gate {
public:
	class instruction;

private:
	std::list<std::unique_ptr<qasm::custom_gate::instruction>>* li_;

	std::vector<std::shared_ptr<double>>* params_;

	std::vector<std::shared_ptr<unsigned int>>* args_;

public:
	//construct new empty custom_gate with paramc parameters and argc arguments
	custom_gate(unsigned int paramc, unsigned int argc) {
		li_ = new std::list<std::unique_ptr<instruction>>;

		params_ = new std::vector<std::shared_ptr<double>>;
		for (unsigned int i = 0; i < paramc; i++) params_->push_back(std::shared_ptr<double>(new double(0)));

		args_ = new std::vector<std::shared_ptr<unsigned int>>;
		for (unsigned int i = 0; i < argc; i++) args_->push_back(std::shared_ptr<unsigned int>(new unsigned int(0)));
	}
	
	~custom_gate() override {
		delete li_;

		delete params_;

		delete args_;
	}

	//add an instruction to end of custom_gate, defined by a gate, a vector of parameters (or if nullptr then
	//index of parameter in custom_gate), and a vector of indexes of arguments in custom_gate.
	void add_instruction(qasm::gate*, const std::vector<std::pair<double*, unsigned int>>&,
		const std::vector<unsigned int>&);
	
	unsigned int paramc() const override { return params_->size(); }

	unsigned int argc() const override { return args_->size(); }

	void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const override;

	class instruction {
	private:
		const qasm::gate* gate_;
		const std::vector<std::shared_ptr<double>> params_;
		const std::vector<std::shared_ptr<unsigned int>> args_;

	public:
		instruction(const qasm::gate* gate, const std::vector<std::shared_ptr<double>>& params,
			const std::vector<std::shared_ptr<unsigned int>>& args) : gate_(gate), params_(params), args_(args) {}

		~instruction() { delete gate_; }

		void apply();
	};
};
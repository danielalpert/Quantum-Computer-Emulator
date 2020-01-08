#pragma once
#include <complex>
#include <cmath>
#include <utility>
#include <list>
#include <string>
#include <iostream>

class Qubit;

class Gate;

class CGate;

class Instruction;

class GateInstruction;

class Routine;

class QRegistry;





class Qubit {
private:
	std::complex<double> state0_;

	std::complex<double> state1_;

public:
	Qubit(std::complex<double> state0, std::complex<double> state1) {
		double abs = std::sqrt(std::norm(state0) + std::norm(state1));
		state0_ = state0 / abs;
		state1_ = state1 / abs;
	}

	friend Qubit&& operator+ (const Qubit& q1, const Qubit& q2) {
		std::complex<double> state0 = q1.state0() + q2.state0();
		std::complex<double> state1 = q1.state1() + q2.state1();
		return Qubit(state0, state1);
	}

	static std::complex<double> inner_product(const Qubit& q1, const Qubit& q2) {
		return (q1.state0() * q2.state0()) + (q1.state1() * q2.state1());
	}

	std::complex<double> state0() const { return state0_; }

	std::complex<double> state1() const { return state1_; }
};



//represents a 1-qubit gate applying a unitary operation to a target qubit:
//takes a qubit in state 0 to state state0, qubit in state 1 to state state1
class Gate {
private:
	const Qubit* state0_;

	const Qubit* state1_;

public:
	class unitary_exception : public std::exception {
	public:
		const char* what() const override { return "Gate must represent a unitary operation"; }
	};


	Gate(const Qubit& state0, const Qubit& state1) : state0_(&state0), state1_(&state1) {
		//if (Qubit::inner_product(*state0_, *state1_) != 0.0) throw unitary_exception();
	}
	
	const Qubit* state0() const { return state0_; }
	const Qubit* state1() const { return state1_; }
	
};

//represents a 2-qubit gate applying a certain unitary transformation to a target qubit,
//conditioned by a control qubit being in a certain state ctrl (thus entangling both qubits)
class CGate {
private:
	Gate transform_;

public:
	CGate(const Gate& transform) : transform_(transform) {}

	const Gate& transform() const {
		return transform_;
	}
};

//an instruction for a quantum registry
class Instruction {
private:
	//size of registry
	//int size;

	//class GateInstruction;

	//class CGateInstruction;

public:
	virtual ~Instruction() = default;

	virtual void operator()(QRegistry& registry) const = 0;

	virtual unsigned int size() const = 0;
};

class GateInstruction : public Instruction {
private:
	const Gate gate_;
	unsigned int target_;

public:
	GateInstruction(const Gate& gate, unsigned int target) : gate_(gate), target_(target) {}

	void operator()(QRegistry& registry) const override;

	unsigned int size() const override { return target_; }
};

class CGateInstruction : public Instruction {
private:
	const CGate gate_;
	unsigned int control_;
	unsigned int target_;

public:
	CGateInstruction(const CGate& gate, unsigned int control, unsigned int target) : gate_(gate), control_(control), target_(target) {}

	void operator()(QRegistry& registry) const override;

	unsigned int size() const override {
		if (target_ > control_) return target_;
		return control_;
	}
};


//a sequence of instructions for a quantum registry of a given size
class Routine {
private:
	//size of registry
	unsigned int size_;

	std::list<Instruction*> instructions;

public:
	Routine(int size) : size_(size) { instructions = std::list<Instruction*>(); }

	class size_exception : public std::exception {
	private:
		unsigned int size_;

	public:
		size_exception(unsigned int size) : size_(size) {}

		virtual const char* what() const override {
			std::string str("registry size exception: size of registry ");
			str += std::to_string(size_);
			str += " qubits";
			return str.c_str();
		}
	};
	
	//adds an instruction to the end of routine.
	//throws size_exception if instruction requires too large size.
	//throws bad_alloc if failed to allocate memory for instruction.
	void append(Instruction* it) {
		if (it->size() > size_) throw size_exception(size_);
		instructions.push_back(it);
	}

	void operator()(QRegistry& registry);
};

class QRegistry {
private:
	unsigned int size_;

	std::complex<double>* registry;

	void display() const { for (int i = 0; i < (int)std::pow(2, size_); i++) std::cout << registry[i] << std::endl; }

public:
	QRegistry(unsigned int size) {
		size_ = size;
		registry = new std::complex<double>[(int)std::pow(2, size)];
		registry[0] = 1;
		for (int i = 1; i < std::pow(2, size); i++) registry[i] = 0;
	}

	QRegistry(QRegistry&& registry) noexcept {
		this->registry = registry.registry;
		size_ = registry.size();
	}

	~QRegistry() { delete[] registry; }

	unsigned int size() const { return size_;  }

	//measures value of particular qubit (true for 1, false for 0)
	bool measure(int i);

	//measures value of entire registry (qubits are binary representation of number)
	int measure_all();

	friend void GateInstruction::operator()(QRegistry&) const;

	friend void CGateInstruction::operator()(QRegistry&) const;
};
#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <iostream>

namespace qasm {
	class gate;
	class custom_gate;
}

void interpret(std::string line, std::istream& in);

int interpret_file(const std::string& filename);

//apply instruction represented by given vector of words in line, given instruction is a gate
void apply_gate_instruction(const std::vector<std::string>& words);

void define_gate(const std::vector<std::string>& words, std::istream& in);

//a header may only include gate definitions and include statements
void include_header(const std::string& filename);

std::vector<std::string>* get_words(std::string line);

class qasm::gate {
public:
	virtual ~gate() = default;
	
	virtual unsigned int paramc() const = 0;
	
	virtual unsigned int argc() const = 0;

	virtual void apply(const std::vector<double>& params, const std::vector<unsigned int>& args) const = 0;
};
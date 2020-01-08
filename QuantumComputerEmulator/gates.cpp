#include "gates.h"
#include <list>

using namespace std;

void qasm::custom_gate::instruction::apply() {
	vector<double> params;
	for (auto i : params_) params.push_back(*i);

	vector<unsigned int> args;
	for (auto i : args_) args.push_back(*i);
	
	gate_->apply(params, args);
}

void qasm::custom_gate::add_instruction(qasm::gate* gate, const vector<pair<double*, unsigned int>>& params,
	const vector<unsigned int>& args) {
	
	vector<shared_ptr<double>> params_vector;
	for (auto i : params) {
		if (i.first != nullptr) params_vector.push_back(shared_ptr<double>(new double(*i.first)));
		else params_vector.push_back(shared_ptr<double>((*params_)[i.second]));
	}

	vector<shared_ptr<unsigned int>> args_vector;
	for (auto i : args) args_vector.push_back(shared_ptr<unsigned int>((*args_)[i]));

	li_->push_back(unique_ptr<instruction>(new instruction(gate, params_vector, args_vector)));
}

void qasm::custom_gate::apply(const vector<double>& params, const vector<unsigned int>& args) const {
	for (int i = 0; i < paramc(); i++) *((*params_)[i]) = params[i];

	for (int i = 0; i < argc(); i++) *((*args_)[i]) = args[i];

	for (list<unique_ptr<instruction>>::iterator it = li_->begin(); it != li_->end(); it++) (*it)->apply();
}
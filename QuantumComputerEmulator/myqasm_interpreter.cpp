#include "quantum.h"
#include "myqasm_interpreter.h"
#include "gates.h"
#include <iostream>
#include <ctime>
#include <string>
#include <unordered_map>
#include <set>
#include <fstream>

using namespace std;

unordered_map<string, qasm::gate*> gates;

extern QRegistry* registry = nullptr;

extern complex<double> consts::i(0, 1);

extern double consts::pi = 3.14159265;

int main(int argc, char* argv[]) {
	//generate random seed
	srand(time(0));
	rand();

	//define names of built-in gates
	gates.emplace("Rx", &Rx);
	gates.emplace("Ry", &Ry);
	gates.emplace("Rz", &Rz);
	gates.emplace("H", &H);
	gates.emplace("CNOT", &CNot);
	gates.emplace("CH", &CH);
	gates.emplace("Ph", &Ph);
	gates.emplace("T", &T);
	gates.emplace("Tdag", &Tdag);

	//check command line arguments: 
	//valid arguments should be one integral value representing size of quantum registry (between 2 & 8)
	if (argc != 2) {
		cout << "Usage: myqasm <size> | myqasm <filename> " << endl;
		return 0;
	}

	try {
		int size = stoi(argv[1]);
		if (size < 2 || size > 8) {
			cout << "Size of registry must be between 2 and 8 qubits" << endl;
			return 0;
		}
		registry = new QRegistry(size);
		cout << "Ready..." << endl;
	} catch (invalid_argument) {
		cout << interpret_file(argv[1]) << endl;
		return 0;
	} catch (out_of_range) {
		cout << "Size of registry must be between 2 and 8 qubits" << endl;
		return 0;
	}
	
	string line = "";

	while (true) {
		getline(cin, line);
		interpret(line, cin);
		line = "";
	}

	delete registry;
}

int interpret_file(const string& filename) {
	ifstream in(filename);
	if (in.fail()) throw runtime_error("error: failed to load file " + filename);

	string line = "";
	getline(in, line);
	vector<string>* words = get_words(line);
	while ((*words).size() == 0) {
		delete words;
		line = "";
		getline(in, line);
		words = get_words(line);
	}

	if ((*words)[0].compare("qubits") != 0) throw runtime_error("error: file must begin with instruction qubits <size>");
	if (words->size() != 2) throw runtime_error("error: file must begin with instruction qubits <size>");
	
	try {
		int size = stoi((*words)[1]);
		if (size < 2 || size > 8) {
			cout << "error: size of registry must be between 2 and 8 qubits" << endl;
			return 0;
		}
		registry = new QRegistry(size);
		cout << "Ready..." << endl;
	}
	catch (invalid_argument) {
		cout << "error: size must be of integral type" << endl;
		return 0;
	}
	catch (out_of_range) {
		cout << "error: size of registry must be between 2 and 8 qubits" << endl;
		return 0;
	}
	
	//delete words;

	while (words->size() == 0 || (*words)[0].compare("measure") != 0) {
		delete words;
		line = "";
		getline(in, line);
		words = get_words(line);
		if (words->size() == 0) continue;
		interpret(line, in);
	}

	delete words;
	return registry->measure_all();
}

void interpret(string line, istream& in) {
	const vector<string>* words = get_words(line);


	if ((*words)[0].compare("gate") == 0) {
		try {
			define_gate(*words, in);
		}
		catch (runtime_error e) {
			cout << e.what() << endl;
			abort();
		}
		catch (exception e) {
			cout << e.what() << " [unexpected]" << endl;
			abort();
		}
		return;
	}

	if ((*words)[0].compare("measure") == 0) {
		cout << registry->measure_all() << endl;
		return;
	}

	if ((*words)[0].compare("include") == 0) {
		if (words->size() != 2) throw runtime_error("syntax error");
		try {
			include_header((*words)[1]);
		}
		catch (runtime_error e) {
			cout << e.what() << endl;
			abort();
		}
		catch (exception e) {
			cout << e.what() << " [unexpected]" << endl;
			abort();
		}
		return;
	}
	
	try {
		apply_gate_instruction(*words);
	} catch (runtime_error e) {
		cout << e.what() << endl;
		abort();
	}
	catch (exception e) {
		cout << e.what() << " [unexpected]" << endl;
		abort();
	}

	delete words;
}

vector<string>* get_words(string line) {
	int wcount = 0;
	bool whitespace = true; //last character read was whitespace
	for (char c : line) {
		if (whitespace) {
			if (isspace(c)) continue;
			whitespace = false;
			wcount++;
		}
		else if (isspace(c)) whitespace = true;
	}

	//now value of wcount is number of words (substrings of non-space characters seperated by whitespace) in line

	vector<string>* words = new vector<string>; //vector of words in line
	whitespace = true;
	
	for (char c : line) {
		if (whitespace) {
			if (isspace(c)) continue;
			whitespace = false;
			words->push_back(string(""));
			*(words->end() - 1) += c;
		}
		else if (isspace(c)) whitespace = true;
		else *(words->end() - 1) += c;
	}

	return words;
}

void apply_gate_instruction(const vector<string>& words) {
	//parse the name of the gate and its parameters.
	//for gate U and parameters (p1, p2, ..., pi) correct syntax is U(p1,p2,...,pi) - no whitspaces allowed.
	//a gate with no parameters must omit parantheses.
	string gate_name = "";
	vector<double> params;
	string param = ""; //current parameter to read
	bool open_paranth = false; //found open paranthesis - start parsing gate parameters
	bool closed_paranth = false; //found closed paranthesis - finish parsing gate parameters
	for (char c : words[0]) {
		if (closed_paranth) throw runtime_error("syntax error");
		if (!open_paranth) {
			if (c == ')') throw runtime_error("syntax error");
			if (c == '(') {
				open_paranth = true;
				continue;
			}
			gate_name += c;
		}
		else {
			if (c == '(') throw runtime_error("syntax error");
			if (c == ')') {
				if (param.compare("") == 0) throw runtime_error("syntax error");
				else {
					closed_paranth = true;
					c = ',';
				}
			}
			if (c != ',') param += c;
			else {
				try {
					params.push_back(stod(param));
					param = "";
					continue;
				}
				catch (invalid_argument) {
					throw runtime_error("syntax error");
				}
			}
		}
	}

	//check if gate exists and if number of parameters is correct
	if (gates.count(gate_name) == 0) throw runtime_error("error: gate " + gate_name + " not found");
	qasm::gate* g = gates.at(gate_name);
	if (g->paramc() != params.size())
		throw runtime_error("error: number of parameters for gate " + gate_name + " is " + to_string(g->paramc()));

	//now parse arguments into vector
	vector<unsigned int> args;

	for (int i = 1; i < words.size(); i++) {
		try {
			args.push_back(abs(stoi(words[i])));
		}
		catch (invalid_argument) {
			throw runtime_error("syntax error");
		}
	}

	if (g->argc() != args.size())
		throw runtime_error("error: number of arguments for gate " + gate_name + " is " + to_string(g->argc()));

	g->apply(params, args);
}

void define_gate(const vector<string>& words, istream& in) {
	unordered_map<string, unsigned int> param_ids; //parameter identifiers
	unordered_map<string, unsigned int> arg_ids; //argument identifiers

	if (words.size() < 4) throw runtime_error("syntax error");

	//parse the name of the gate and its parameters.
	//for gate U and parameters (p1, p2, ..., pi) correct syntax is U(p1,p2,...,pi) - no whitspaces allowed.
	//a gate with no parameters must omit parantheses.
	string gate_name = "";
	vector<double> params;
	string param = ""; //current parameter to read
	unsigned int paramc = 0; //index of current parameter, at end of loop number of parameters
	bool open_paranth = false; //found open paranthesis - start parsing gate parameters
	bool closed_paranth = false; //found closed paranthesis - finish parsing gate parameters
	for (char c : words[1]) {
		if (closed_paranth) throw runtime_error("syntax error");
		if (!open_paranth) {
			if (c == ')') throw runtime_error("syntax error");
			if (c == '(') {
				open_paranth = true;
				continue;
			}
			gate_name += c;
		}
		else {
			if (c == '(') throw runtime_error("syntax error");
			if (c == ')') {
				if (param.compare("") == 0) throw runtime_error("syntax error");
				else {
					closed_paranth = true;
					c = ',';
				}
			}
			if (c != ',') param += c;
			else {
				try {
					if (param_ids.count(param) != 0) throw runtime_error("error: parameter " + param + " already defined");
					param_ids.emplace(param, paramc);
					param = "";
					paramc++;
					continue;
				}
				catch (invalid_argument) {
					throw runtime_error("syntax error");
				}
			}
		}
	}

	if (gates.count(gate_name) != 0) throw runtime_error("error: gate " + gate_name + " already exists");

	unsigned int argc = 0;
	for (int i = 2; i < words.size() - 1; i++) {
		if (words[i].find('{') != string::npos) throw runtime_error("syntax error");
		if (arg_ids.count(words[i]) != 0) throw runtime_error("error: argument " + words[i] + " already defined");
		arg_ids.emplace(words[i], argc);
		argc++;
	}

	if (argc == 0) throw runtime_error("error: gate must take at least 1 argument");
	if (words[words.size() - 1].compare("{") != 0) throw runtime_error("syntax error");

	qasm::custom_gate* gate = new qasm::custom_gate(paramc, argc);
	
	string line = "";
	getline(in, line);
	vector<string>* words_in = get_words(line);
	while (line.find("}") == string::npos) {
		if (words_in->size() == 0) continue;
		
		//parse the name of the gate and its parameters.
		//for gate U and parameters (p1, p2, ..., pi) correct syntax is U(p1,p2,...,pi) - no whitspaces allowed.
		//a gate with no parameters must omit parantheses.
		string gate_name_in = "";
		vector<pair<double*, unsigned int>> params_in;
		string param = ""; //current parameter to read
		bool open_paranth = false; //found open paranthesis - start parsing gate parameters
		bool closed_paranth = false; //found closed paranthesis - finish parsing gate parameters
		for (char c : (*words_in)[0]) {
			if (closed_paranth) throw runtime_error("syntax error");
			if (!open_paranth) {
				if (c == ')') throw runtime_error("syntax error");
				if (c == '(') {
					open_paranth = true;
					continue;
				}
				gate_name_in += c;
			}
			else {
				if (c == '(') throw runtime_error("syntax error");
				if (c == ')') {
					if (param.compare("") == 0) throw runtime_error("syntax error");
					else {
						closed_paranth = true;
						c = ',';
					}
				}
				if (c != ',') param += c;
				else {
					try {
						params_in.push_back(pair<double*, unsigned int>(new double(stod(param)), 0));
						param = "";
						continue;
					}
					catch (invalid_argument) {
						if (param_ids.count(param) == 0) throw runtime_error("error: undefined identifier " + param);
						params_in.push_back(pair<double*, unsigned int>(nullptr, param_ids.at(param)));
					}
				}
			}
		}

		//check if gate exists and if number of parameters is correct
		if (gates.count(gate_name_in) == 0) throw runtime_error("error: gate " + gate_name_in + " not found");
		qasm::gate* g = gates.at(gate_name_in);
		if (g->paramc() != params_in.size())
			throw runtime_error("error: number of parameters for gate " + gate_name_in + " is " + to_string(g->paramc()));

		//parse arguments
		vector<unsigned int> args_in;
		for (int i = 1; i < words_in->size(); i++) {
			if (arg_ids.count((*words_in)[i]) == 0) throw runtime_error("error: undefined identifier " + (*words_in)[i]);
			args_in.push_back(arg_ids.at((*words_in)[i]));
		}

		//add instruction
		gate->add_instruction(gates.at(gate_name_in), params_in, args_in);

		for (auto i : params_in) if (i.first != nullptr) delete i.first;

		line = "";
		getline(in, line);
		words_in = get_words(line);
	}

	if (line.compare("}") != 0) throw runtime_error("syntax error");

	gates.insert(pair<string, qasm::gate*>(gate_name, gate));

	cout << gate_name << gates.count(gate_name) << endl;
}

void include_header(const string& filename) {
	ifstream in(filename);
	if (in.fail()) throw runtime_error("error: header file " + filename + " not found");

	string line;
	vector<string>* words;
	while (!in.eof()) {
		line = "";
		getline(in, line);
		words = get_words(line);

		if (words->size() == 0) {
			delete words;
			continue;
		}

		if ((*words)[0].compare("include") == 0) {
			if (words->size() != 2) throw runtime_error("syntax error");
			include_header((*words)[1]);
			delete words;
			continue;
		}

		if ((*words)[0].compare("gate") == 0) {
			define_gate(*words, in);
			delete words;
			continue;
		}



		delete words;
		throw runtime_error("error: header file may only contain gate definitions and include statements");
	}
}
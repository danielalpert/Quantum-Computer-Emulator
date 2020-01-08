#include "quantum.h"
#include <iostream>
#include <ctime>

using namespace std;

void mainy() {
	QRegistry registry(4);

	Gate Haddamard(Qubit(1, 1), Qubit(1, -1));
	Gate Not(Qubit(0, 1), Qubit(1, 0));
	CGate CNot(Not);

	GateInstruction i1(Haddamard, 0);
	CGateInstruction i2(CNot, 0, 1);

	i1(registry);

	//registry.display();
	cout << "entangling" << endl;

	i2(registry);

	//registry.display();
	//cout << registry.measure_all() << endl;
}

int maini() {
	srand(time(0));

	for (int i = 0; i < 1; i++) mainy();

	return 0;
}
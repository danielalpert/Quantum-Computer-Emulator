# Quantum-Computer-Emulator
I created an emulator for a quantum computer in c++, which can process instructions written in a pseudo-assembly language I defined, on an emulated quantum registry.

Features:
1. A command line interpreter of instructions
2. An interpreter of text files including instructions on a quantum registry, and a single measurement
3. Basic 1-qubit and 2-qubit gates on an emulated quantum registry of up to 8 qubits
4. User defined gates, in command line or in seperate text file, consisting of the basic gates (or other user defined gates)
5. An example for usage: a text file implementing the [Deutsch-Josza algorithm](https://en.wikipedia.org/wiki/Deutsch%E2%80%93Jozsa_algorithm) for a 3-qubit function (using a 5-qubit registry), with oracle in a seperate text file that may be altered

Quantum Gates included:
1. Rotation of a single qubit on the [Bloch Sphere](https://en.wikipedia.org/wiki/Bloch_sphere) (Rx, Ry, Rx), by an angle given by parameters
2. [Haddamard transform](https://en.wikipedia.org/wiki/Quantum_logic_gate#Hadamard_(H)_gate) of a single qubit(H)
3. Phase shift gates, by an angle given by parameters (Ph), or by a fixed angle of &#177;&pi;/4 (T, Tdag)
4. [Controlled NOT gate](https://en.wikipedia.org/wiki/Controlled_NOT_gate) (CNOT), similarly Controlled Hadamard gate (CH)

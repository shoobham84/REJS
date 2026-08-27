#pragma once

#include "netlistParser.hpp"
#include "z3++.h"

namespace Unroll
{

class CircuitUnroller {

public:
	CircuitUnroller(z3::context& ctx, const PN::NetlistAST& ast);

	void unroll(int num_cycles);

	void setInput(int time, const std::string& portName, const z3::expr& value);
	void setInputBool(int time, const std::string& portName, bool value);

	z3::expr getNet(int time, const std::string& netName);
	z3::expr getPort(int time, const std::string& portName);

	z3::expr getFlipFLopOutput(int time, const std::string& instanceName);

	void evalueCombinational(int time);

	void transitionFlipFlops(int time);

	z3::expr evaluateGate(int time, const PN::CellInstance& inst);

	int getNumCycles() const {
		return m_numCycles;
	}

	const PN::NetlistAST& getAST() const {
		return m_AST;
	}

	
private:
	z3::context& m_Ctx;
	PN::NetlistAST m_AST;
	int m_numCycles {0};

	std::vector<std::unordered_map<std::string, std::shared_ptr<z3::expr>>> m_netMap;

    std::vector<std::unordered_map<std::string, std::shared_ptr<z3::expr>>> m_flipFlopStates;

    std::unordered_map<std::string, std::string> m_padBindings;

    std::vector<PN::CellInstance> m_SortedGates;

    void setupPadBindings();
    void sortCombinationalGates();
};



}

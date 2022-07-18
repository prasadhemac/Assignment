#include <stdexcept>
#include "Circuit.h"


/* 
Adds a truth table that can be later attached to gate types.
Args:
	name: A unique string used to identify the truth table.
	output_list: A list of outputs for the truth table.
*/
void Circuit::AddTruthTable(std::string type, std::vector<int> outputs)
{
	if (m_truthTables.find(type) != m_truthTables.end())
		throw std::runtime_error("Truth table name already used");
	m_truthTables.insert({ type, TruthTable(type, outputs) });
}

/* 
 Adds a gate type that can be later attached to gates.

Args:
	name: A unique string used to identify the gate type.
	truth_table_name: The name of the gate's truth table.
	delay: The gate's delay from an input transition to an output
		transition.
*/
void Circuit::AddGateType(std::string name, std::string truthTableName, int delay)
{
	if (m_gateTypes.find(name) != m_gateTypes.end())
		throw std::runtime_error("Gate type name already used");
	if (delay < 0)
		throw std::runtime_error("Invalid delay");
	auto& truthTable = GetTruthTable(truthTableName);

	m_gateTypes.insert({ name, GateType(name, truthTable, delay) });
}

/* 
 Adds a gate and connects it to other gates.

Args:
	name: A unique string used to identify the gate.
	type_name: The name of the gate's type.
	input_names: List of the names of gates whose outputs are connected
		to this gate's inputs.

*/
void Circuit::AddGate(std::string name, const std::string& typeName, const std::vector<std::string>& inputNames)
{
	if (m_gates.find(name) != m_gates.end())
		throw std::runtime_error("Gate name already used");
	GateType& type = GetType(typeName);
	auto [pair, _] = m_gates.insert({ name, Gate(name, type) });
	auto& gate = pair->second;
	for (unsigned i = 0; i < inputNames.size() ; ++i)
	{
		auto& target = GetGate(inputNames[i]);
		gate.ConnectInput(i, &target);
	}
}

Gate& Circuit::GetGate(const std::string& gateName){
	auto it = m_gates.find(gateName);
	if(it == m_gates.end())
		throw std::runtime_error("Gate not found");
	return it->second;
}

TruthTable& Circuit::GetTruthTable(const std::string& name){
	auto it = m_truthTables.find(name);
	if(it == m_truthTables.end())
		throw std::runtime_error("TruthTable not found");
	return it->second;
}

GateType& Circuit::GetType(const std::string& name){
	auto it = m_gateTypes.find(name);
	if(it == m_gateTypes.end())
		throw std::runtime_error("GateType not found");
	return it->second;
}

void Circuit::AddProbe(const std::string& gateName)
{
	auto& gate = GetGate(gateName);
	gate.Probe();
}

std::vector<Gate*> Circuit::ProbeAllGates()
{
	std::vector<Gate*> probed;
	for (auto& [k, v] : m_gates)
	{
		if (v.IsProbed())
			continue;
		v.Probe();
		probed.emplace_back(&v);
	}
	return probed;
}


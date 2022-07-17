#pragma once
#include <string>
#include <map>
#include <vector>
#include "TruthTable.h"
#include "GateType.h"
#include "Gate.h"
#include "rapidjson/prettywriter.h"

using namespace rapidjson;

class Circuit
{
public:
	void AddTruthTable(std::string type, std::vector<int> outputs);
	void AddGateType(std::string name, std::string truthTableName, int delay);
	void AddGate(std::string name, std::string typeName, std::vector<std::string> inputNames);
	void AddProbe(std::string gateName);
	Gate* GetGate(std::string gateName) { return &(m_gates[gateName]); }
	std::vector<Gate*> ProbeAllGates();

	template <typename Writer>
	void Serialize(Writer& writer) const noexcept{
		writer.StartObject();

		writer.String(("gates"));
		writer.StartArray();
		for (const auto& [k, v] : m_gates)
			v.Serialize(writer);
		writer.EndArray();

		writer.EndObject();
	}
private:
	std::map<std::string, TruthTable> m_truthTables;
	std::map<std::string, GateType> m_gateTypes;
	std::map<std::string, Gate> m_gates;
};
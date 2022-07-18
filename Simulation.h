#pragma once
#include <fstream>
#include <memory>
#include <queue>

#include "Circuit.h"
#include "rapidjson/prettywriter.h"

struct Transition
{
	explicit Transition(Gate* g, int output, int t) : gate(g), newOutput(output), time(t) {}
	bool operator<(const Transition& other) const noexcept
	{
		if (time == other.time)
			return objectId < other.objectId;
		return time < other.time ;
	}
	bool IsValid() const noexcept {  return gate->GetOutput() != newOutput; }
	void Apply();
	Gate* gate{nullptr};
	int newOutput{0};
	int time{0};
	int objectId = ++GlobalId;
	static int GlobalId;
};

struct Probe
{
	bool operator<(const Probe& other) const noexcept
	{
		if (time == other.time)
			return newValue < other.newValue;
		return time < other.time;
	}

	template <typename Writer>
	void Serialize(Writer& writer) const noexcept{
		writer.StartArray();
		writer.Int(time);
	#if RAPIDJSON_HAS_STDSTRING
		writer.String(gateName);
	#else
		writer.String(gateName.c_str(), static_cast<rapidjson::SizeType>(gateName.length()));
	#endif
		writer.Int(newValue);
		writer.EndArray();
	}
	int time{0};
	std::string gateName;
	int newValue{0};
};

class Simulation
{
public:
	Simulation() : m_circuit(new Circuit()) {};
	static std::unique_ptr<Simulation> GetSimulationFromFile(std::ifstream& is);
	void LayoutFromFile(std::ifstream& is);
	void AddTransition(std::string gateName, int outputValue, int outputTime);
	Circuit* GetCircut() { return m_circuit.get(); }
	int Step();
	void Run();
	void ProbeAllGates() { m_undoProbes = m_circuit->ProbeAllGates(); }
	void UndoProbeAllGates();

	template <typename Writer>
	void Serialize(Writer& writer) const noexcept{
		writer.StartObject();

		writer.String("circuit");
		m_circuit->Serialize(writer);
		writer.String(("trace"));
		writer.StartArray();
		for(const auto& probe: m_probes){
			probe.Serialize(writer);
		}
		writer.EndArray();
		writer.String("layout");
	#if RAPIDJSON_HAS_STDSTRING
		writer.String(m_layout);
	#else
		writer.String(m_layout.c_str(), static_cast<rapidjson::SizeType>(m_layout.length()));
	#endif
		writer.EndObject();
	}
	void PrintProbes(std::ostream& os);
private:
	std::unique_ptr<Circuit> m_circuit;
	std::string m_layout;
	std::vector<Transition> m_inTransitions;
	std::priority_queue<Transition, std::vector<Transition>, auto(*)(Transition&,Transition&)->bool> m_queue{
		[]( Transition& a, Transition& b )->bool { return a.time >= b.time; }
	};
	std::vector<Probe> m_probes;
	std::vector<Gate*> m_undoProbes;
};
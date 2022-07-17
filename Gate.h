#pragma once
#include <map>
#include <vector>
#include "GateType.h"
#include "rapidjson/prettywriter.h"

using namespace rapidjson;

class Gate
{
public:
	Gate() {}
	explicit Gate(std::string name, const GateType* type) : m_name(name), m_type(type) {};
	void ConnectInput(int i, Gate* target);
	void AddOutput(Gate* target);
	void Probe() noexcept;
	bool IsProbed() const noexcept { return m_probed; }
	int GetOutput() const noexcept { return m_output; }
	void SetOutput(int value) noexcept { m_output = value; }
	std::string GetName() const noexcept { return m_name; }
	std::vector<Gate*> GetOutGates() { return m_outGates; }
	int GetTransitionOutput() const;
	int GetTransitionTime(int time) const;
	void UndoProbe();

	template <typename Writer>
	void Serialize(Writer& writer) const noexcept{
		writer.StartObject();

		writer.String("id");
	#if RAPIDJSON_HAS_STDSTRING
		writer.String(m_name);
	#else
		writer.String(m_name.c_str(), static_cast<SizeType>(m_name.length()));
	#endif
		writer.String("table");
	#if RAPIDJSON_HAS_STDSTRING
		writer.String(m_type->GetTruthTableName());
	#else
		writer.String(m_type->GetTruthTableName().c_str(), static_cast<SizeType>(m_type->GetTruthTableName().length()));
	#endif
		writer.String("type");
	#if RAPIDJSON_HAS_STDSTRING
		writer.String(m_type->GetType());
	#else
		writer.String(m_type->GetType().c_str(), static_cast<SizeType>(m_type->GetType().length()));
	#endif
		writer.String("probed");
		writer.Bool(m_probed);

		writer.String(("inputs"));
		writer.StartArray();
		for (const auto& [k,v] : m_inGates)
		{
	#if RAPIDJSON_HAS_STDSTRING
			writer.String(v->GetName());
	#else
			writer.String(v->GetName().c_str(), static_cast<SizeType>(v->GetName().length()));
	#endif
		}			
		writer.EndArray();

		writer.String(("outputs"));
		writer.StartArray();
		for(const auto& v: m_outGates)
		{
	#if RAPIDJSON_HAS_STDSTRING
			writer.String(v->GetName());
	#else
			writer.String(v->GetName().c_str(), static_cast<SizeType>(v->GetName().length()));
	#endif
		}
		writer.EndArray();

		writer.EndObject();
	}
private:
	const GateType* m_type{};
	std::string m_name;
	std::map<int, Gate*> m_inGates;
	std::vector<Gate*> m_outGates;
	bool m_probed{};
	int m_output{};
	int m_delay{};
};
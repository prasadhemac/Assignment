#include <stdexcept>
#include "Gate.h"
#include "ErrorHandler.h"

void Gate::ConnectInput(int i, Gate* target)
{
	if (m_inGates.find(i) != m_inGates.end())
		error_handler::throw_with_trace(std::runtime_error("input terminal already connected"));
	m_inGates.insert({ i, target });
	target->AddOutput(this);
}

void Gate::AddOutput(Gate* target)
{
	m_outGates.emplace_back(target);
}

void Gate::Probe()
{
	if (m_probed)
		error_handler::throw_with_trace(std::runtime_error("Gate already probed"));
		
	m_probed = true;
}

int Gate::GetTransitionOutput() const
{
	std::vector<int> inputs;
	for (const auto& [k, v] : m_inGates)
		inputs.emplace_back(v->GetOutput());
	return m_type.GetOutput(inputs);
}

int Gate::GetTransitionTime(int time) const
{
	return time + m_type.GetDelay();
}

void Gate::UndoProbe() noexcept
{
	m_probed = false;
}

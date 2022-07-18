#include <sstream>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <algorithm>

#include "Circuit.h"
#include "Simulation.h"
#include "PriorityQueue.h"

int Transition::GlobalId = 0;

void Transition::Apply()
{
	if (!IsValid())
		error_handler::throw_with_trace(std::runtime_error("Gate output should not transition to the same value"));
	gate->SetOutput(newOutput);
}

void Simulation::AddTransition(const std::string& gateName, int outputValue, int outputTime)
{
	Gate& pGate = m_circuit->GetGate(gateName);
	m_inTransitions.emplace_back(Transition{ &pGate, outputValue, outputTime });
}

std::unique_ptr<Simulation> Simulation::GetSimulationFromFile(std::ifstream& is)
{
	auto simulation = std::make_unique<Simulation>();
	auto* circut = simulation->GetCircut();
	for (;;)
	{
		boost::char_separator<char> sep(" ");
		std::string line;
		std::getline(is, line);
		boost::tokenizer< boost::char_separator<char>> tokens(line, sep);
		std::vector<std::string> command;
		for (const auto& v : tokens)
			command.emplace_back(v);
		if (command.empty())
			continue;
		if (command[0] == "table")
		{
			std::vector<int> outputs;
			for (size_t i = 2; i < command.size(); ++i)
				outputs.emplace_back(std::stoi(command[i]));
			circut->AddTruthTable(command[1], outputs);
		}
		else if (command[0] == "type")
		{
			if (command.size() != 4)
				error_handler::throw_with_trace(std::runtime_error("Invalid number of arguments for gate type"));
			circut->AddGateType(command[1], command[2], std::stoi(command[3]));
		}
		else if (command[0] == "gate")
		{
			std::vector<std::string> inputs;
			for (size_t i = 3; i < command.size(); ++i)
				inputs.emplace_back(command[i]);
			circut->AddGate(command[1], command[2], inputs);
		}
		else if (command[0] == "probe")
		{
			if (command.size() != 2)
				error_handler::throw_with_trace(std::runtime_error("Invalid number of arguments for probe type"));
			circut->AddProbe(command[1]);
		}
		else if (command[0] == "flip")
		{
			if (command.size() != 4)
				error_handler::throw_with_trace(std::runtime_error("Invalid number of arguments for flip type"));
			simulation->AddTransition(command[1], std::stoi(command[2]), std::stoi(command[3]));
		}
		else if (command[0] == "done")
			break;

	}
	return simulation;
}

void Simulation::LayoutFromFile(std::ifstream& is)
{
	std::string temp;
	while (std::getline(is, temp))
	{
		if (temp == "layout")
			break;
	}
	
	boost::regex xmlRegex("^<\\?xml.*\\?>\n");
	boost::regex docTypeRegex("^<!DOCTYPE.*?>\n");
	std::ostringstream sstr;
	sstr << is.rdbuf();
	std::string str = sstr.str();
	str = boost::regex_replace(str, xmlRegex, "");
	str = boost::regex_replace(str, docTypeRegex, "");
	m_layout = str;
}

int Simulation::Step()
{
	// The issue with this custom PriorityQueue is min() function always iterate through all the elements 
	// to find the min element which is not latency friendly.
	// We can rather use STL container with sorting.
	// So we have two options for m_queue
	// 		1. use STL vector and sort it at the begining of Step() function with O(n*logn) time complexity
	//		2. use STL priority_queue which gets sorted when pushing the element with O(logn) time complexity,
	//			So the time complexitity of the queue building loops becomes O(klogn) where k is no of new transitions 
	//			and n is the no of elements in the queue and k < n
	// Though time complexity of both options is O(nlogn), second option is slightly better 
	// since k (i.e. new transitions) < n (i.e. m_queue.size())
	//
	// This improves performance significantly from few 10s of seconds to few 100s of miliseconds for 5devadas13.in.
	int stepTime = m_queue.top().time;
	std::vector<Transition> transitions;
	while (m_queue.size() > 0 && m_queue.top().time == stepTime)
	{
		auto transition = m_queue.top();
		m_queue.pop();
		if (!transition.IsValid())
			continue;
		transition.Apply();
		if (transition.gate->IsProbed())
			m_probes.emplace_back(Probe{ transition.time, transition.gate->GetName(), transition.newOutput });
		transitions.emplace_back(transition);
	}

	for (const auto& transition : transitions)
	{
		for (auto* gate : transition.gate->GetOutGates())
		{
			auto output = gate->GetTransitionOutput();
			auto time = gate->GetTransitionTime(stepTime);
			m_queue.push(Transition(gate, output, time));
		}
	}
	return stepTime;
}

void Simulation::Run()
{
	std::sort(m_inTransitions.begin(), m_inTransitions.end());
	for (const auto& t : m_inTransitions)
		m_queue.push(t);
	while (m_queue.size() > 0)
		Step();
	std::sort(m_probes.begin(), m_probes.end());
}

void Simulation::UndoProbeAllGates()
{
	for (auto* gate : m_undoProbes)
	{
		gate->UndoProbe();
	}
	m_undoProbes.clear();
}

void Simulation::PrintProbes(std::ostream& os)
{
	for (const auto& probe : m_probes)
	{
		if (!m_circuit->GetGate(probe.gateName).IsProbed())
			continue;
		os << probe.time << " " << probe.gateName << " " << probe.newValue << std::endl;
	}
		
}
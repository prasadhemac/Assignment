// Simulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <fstream>
#include <iostream>

#include "rapidjson/prettywriter.h"
#include "Simulation.h"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Simulator.exe <simfile> [json]" << std::endl;
        std::cout << "simulation output is in circuit.jsonp" << std::endl;
        exit(0);
    }
    bool json = (argc >= 3 && "json" == std::string(argv[2]));
    std::ifstream input(argv[1], std::ios::in);
    auto simulation = Simulation::GetSimulationFromFile(input);
    
    if (json)
    {
        simulation->LayoutFromFile(input);
        // probe all gates should only be executed when 
        // json output is on 
        simulation->ProbeAllGates();
    }
        
    simulation->Run();

    if (json)
    {
        // boost property_tree is replaced by rapidjson
        // rapidjson is one of the widely used open source json parser/generator
        // performance of the json output generating functionality is improved from seconds to miliseconds for 5devadas13.in
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
        simulation->Serialize(writer);
        std::ofstream output("circuit.jsonp", std::ios::out);
        output << "onJsonp(";
        output << sb.GetString();
        output << ");\n";
    }

    simulation->PrintProbes(std::cout);
    if (json)
        simulation->UndoProbeAllGates();
    
}

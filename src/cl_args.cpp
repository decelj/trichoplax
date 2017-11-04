#include "cl_args.h"
#include <iostream>
#include <iomanip>

CLArgs::CLArgs()
{
}

bool CLArgs::Parse(char** args, uint32_t count, std::vector<std::string>* extraArgs)
{
    for (uint32_t i = 1; i < count; ++i)
    {
        if (std::strcmp(args[i], "-h") == 0)
        {
            PrintUsage();
            return false;
        }

        ArgSet::iterator argIt = mArgs.find(Arg(args[i]));
        if (argIt == mArgs.end())
        {
            if (extraArgs)
            {
                extraArgs->emplace_back(args[i]);
            }
            continue;
        }

        if (!argIt->IsFlag())
        {
            ++i;
            if (i >= count)
            {
                throw std::invalid_argument(args[i - 1]);
            }
        }
        
        if (!argIt->Parse(args[i]))
        {
            throw std::invalid_argument(args[i]);
        }
    }

    return true;
}

void CLArgs::PrintUsage() const
{
    for (const Arg& a : mArgs)
    {
        a.Print();
    }
}

void CLArgs::Arg::Print() const
{
    std::cout << std::left << std::setw(20) << name;
    if (!IsFlag())
    {
        std::cout << " <value>";
    }
    std::cout << " (default ";

    switch (type)
    {
        case INT: std::cout << *(int*)dest; break;
        case FLOAT: std::cout << *(float*)dest; break;
        case STRING: std::cout << *(std::string*)dest; break;
        case BOOL: std::cout << *(bool*)dest; break;
        default: break;
    }

    std::cout << ")\n";
}

bool CLArgs::Arg::Parse(const char* value) const
{
    switch (type)
    {
        case INT: *(int*)dest = std::atoi(value); break;
        case FLOAT: *(float*)dest = (float)std::atof(value); break;
        case STRING: *(std::string*)dest = value; break;
        case BOOL: *(bool*)dest = true; break;
        default: return false; break;
    }

    return true;
}

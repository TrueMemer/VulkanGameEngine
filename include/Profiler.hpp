#pragma once

#include "PCH.hpp"
#include "Time.hpp"
#include "Engine.hpp"

class Profiler
{
public:
    Profiler() {}
    ~Profiler() {}

    static void start(std::string id)
    {
        times[id].first.setMicroSeconds(Engine::clock.now());
    }

    static void end(std::string id)
    {
        times[id].second.setMicroSeconds(Engine::clock.now() - times[id].first.getMicroSeconds());
    }

    static Time getTime(std::string id)
    {
        return times[id].second;
    }

private:
    static std::unordered_map<std::string, std::pair<Time,Time>> times;
};

std::unordered_map<std::string, std::pair<Time, Time>> Profiler::times;
#pragma once

#include <string>

// I keep WeatherClient focused solely on fetching and formatting weather data.
class WeatherClient {
public:
    // I fetch current weather for a city using an API key
    // provided through the environment.
    std::string getWeather(const std::string& city);

private:
    // I isolate API key access so secrets stay out of call sites.
    std::string getApiKey() const;
};

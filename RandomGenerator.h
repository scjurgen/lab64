
#pragma once

#include <memory>
#include <random>

class RandomGenerator
{
  public:
    RandomGenerator();
    void seed(int sd);
    double GetNormalizedUniformRange();
    double GetUniformRange(double minValue, double maxValue);

private:
    //     std::mt19937       m_generator;
    std::default_random_engine       m_generator;
    std::uniform_real_distribution<> m_uniformRealDistribution;
    std::normal_distribution<> m_normalDistribution;

};

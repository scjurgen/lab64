
#include "RandomGenerator.h"

static std::unique_ptr<RandomGenerator> g;

RandomGenerator::RandomGenerator()
        : m_generator()
          , m_uniformRealDistribution(0.0, 1.0)
          , m_normalDistribution(0.0, 1.0)
{
}

void RandomGenerator::seed(int sd)
{
    m_generator.seed(sd);
}

double RandomGenerator::GetNormalizedUniformRange()
{
    return m_uniformRealDistribution(m_generator);
}

double RandomGenerator::GetUniformRange(double minValue, double maxValue)
{
    double number = m_uniformRealDistribution(m_generator);
    return minValue + (maxValue - minValue) * number;
}

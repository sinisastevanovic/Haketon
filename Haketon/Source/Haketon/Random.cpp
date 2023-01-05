#include "Random.h"

namespace Haketon
{
    thread_local std::mt19937 Random::RandomEngine_;
    thread_local std::uniform_int_distribution<std::mt19937::result_type> Random::Distribution_;
}

#pragma once

#include "common.hpp"
#include <chrono>

constexpr size_t prediction_horizon{ 25 };
constexpr size_t sp_size{ 14 + (prediction_horizon - 1) };

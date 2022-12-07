#pragma once

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "Timer.h"

#define FLOAT_AS_DOUBLE

#ifdef FLOAT_AS_DOUBLE
using Float = double;
#else
using Float = float;
#endif

// Useful Constants
static const Float Pi = 3.14159265358979323846;
static const Float InvPi = 0.31830988618379067154;
static const Float Inv2Pi = 0.15915494309189533577;
static const Float Inv4Pi = 0.07957747154594766788;
static const Float PiOver2 = 1.57079632679489661923;
static const Float PiOver4 = 0.78539816339744830961;
static const Float Sqrt2 = 1.41421356237309504880;

#ifndef NDEBUG

#define PROFILE_FUNCTION() Timer t##__LINE__(##__FUNCSIG__);
#define PROFILE_SCOPE(name) Timer t##__LINE__(#name);

#else

#define PROFILE_FUNCTION()
#define PROFILE_SCOPE()

#endif

namespace Utils
{
	// Utility Functions
	template <typename T, typename U, typename V>
	inline T Clamp(T val, U low, V high) {
		if (val < low) return low;
		else if (val > high) return high;
		else return val;
	}

	template<typename T>
	inline T Max(T one, T two)
	{
		if (one > two)
			return one;
		else
			return two;
	}

	template <typename T>
	inline T Mod(T a, T b) {
		T result = a - (a / b) * b;
		return (T)((result < 0) ? result + b : result);
	}

	//template <>
	inline Float Mod(Float a, Float b) {
		return std::fmod(a, b);
	}

	inline Float Radians(Float deg) {
		return (Pi / 180) * deg;
	}
	inline Float Degrees(Float rad) {
		return (180 / Pi) * rad;
	}

	inline Float Log2(Float x) {
		constexpr Float invLog2 = 1.442695040888963387004650940071;
		return std::log(x) * invLog2;
	}

	inline int Log2Int(uint32_t v) {
		return 31 - __lzcnt(v);
	}

	template <typename T>
	inline bool IsPowerOf2(T v) {
		return v && !(v & (v - 1));
	}

	inline int32_t RoundUpPow2(int32_t v) {
		v--;
		v |= v >> 1;    v |= v >> 2;
		v |= v >> 4;    v |= v >> 8;
		v |= v >> 16;
		return v + 1;
	}

	inline int CountTrailingZeros(uint32_t v) {
		return _tzcnt_u32(v);
	}
}

//
// Logging Includes and Defines
//

#include "Logger.h"

#define ASSERT(x, ...) if (!x) { ::Logger::Get().Error(__VA_ARGS__); __debugbreak(); }

#ifdef ENABLE_LOG

#define LOG_DEBUG(...) ::Logger::Get().Debug(__VA_ARGS__)

#define LOG_INFO(...) ::Logger::Get().Info(__VA_ARGS__)

#define LOG_WARN(...) ::Logger::Get().Warn(__VA_ARGS__)

#define LOG_ERROR(...) ::Logger::Get().Error(__VA_ARGS__)

#define LOG_CRITICAL(...) ::Logger::Get().Critical(__VA_ARGS__); __debugbreak()

#else

#define LOG_INFO(...)

#define LOG_WARN(...)

#define LOG_ERROR(...)

#define LOG_CRITICAL(...)

#endif
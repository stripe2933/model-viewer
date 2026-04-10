#pragma once

#include <cstdint>
#include <algorithm>
#include <any>
#include <concepts>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <set>
#include <shared_mutex>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_ENABLE_FREETYPE
#define IMGUI_USE_WCHAR32
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <vku.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#undef MemoryBarrier
#endif
#pragma once

#ifdef NO_PCH
#error "This header should not be included!"
#endif

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <any>
#include <charconv>
#include <concepts>
#include <expected>
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
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>

#include <boost/container_hash/hash.hpp>

#include <boost/iostreams/device/mapped_file.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp> // Will include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#if __APPLE__

#include <imgui_impl_metal.h>
#include <imgui_impl_osx.h>

#else

#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <vku.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#undef MemoryBarrier
#endif // _Win32

#endif // __APPLE__
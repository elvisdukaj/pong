module;

#include <volk.h>

module vis.graphic.vulkan.vkh:helper;

import std;

import :traits;
import :concepts;

export namespace helper {

template <std::integral T> constexpr uint32_t api_version_major(T const version) {
  return (static_cast<uint32_t>(version) >> 22U) & 0x7FU;
}

template <std::integral T> constexpr uint32_t api_version_minor(T const version) {
  return (static_cast<uint32_t>(version) >> 12U) & 0x3FFU;
}

template <std::integral T> constexpr uint32_t api_version_patch(T const version) {
  return static_cast<uint32_t>(version) & 0xFFFU;
}

template <std::integral T> constexpr uint32_t api_version_variant(T const version) {
  return static_cast<uint32_t>(version) >> 29U;
}

template <std::integral T>
constexpr uint32_t make_api_version(T const variant, T const major, T const minor, T const patch) {
  return ((((uint32_t)(variant)) << 29U) | (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) |
          ((uint32_t)(patch)));
}

template <std::integral T> constexpr uint32_t make_version(T const major, T const minor, T const patch) {
  return ((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)));
}

std::string vk_version_to_string(uint32_t version) noexcept {
  return std::format("{}.{}.{}", api_version_major(version), api_version_minor(version), api_version_patch(version));
}

std::string vk_api_version_to_string(uint32_t version) noexcept {
  return std::format("{}.{}.{}-{}", api_version_major(version), api_version_minor(version), api_version_patch(version),
                     api_version_variant(version));
}

} // namespace helper

export namespace vkh {
template <typename T, typename Func, typename... Args>
  requires DoesNotHaveSType<T>
std::vector<T> enumerate(Func func, Args... args) {
  uint32_t count{};

  // TODO: some function could return incomplete and need to be polled in a loop
  func(args..., &count, nullptr);
  std::vector<T> container(count);
  func(args..., &count, container.data());

  return container;
}

template <typename T, typename Func, typename... Args>
  requires HasSType<T>
std::vector<T> enumerate(Func func, Args... args) {
  uint32_t count{};

  // TODO: some function could return incomplete and need to be polled in a loop
  func(args..., &count, nullptr);
  T t{};
  t.sType = VulkanSTypeValue<T>;
  std::vector<T> container(count, t);
  func(args..., &count, container.data());

  return container;
}
} // namespace vkh
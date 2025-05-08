module;

#include <volk.h>

export module vis.graphic.vulkan.vkh:traits;

import std;
import vis.window;

export namespace vkh {
template <typename T> struct VulkanSType;

template <> struct VulkanSType<VkInstanceCreateInfo> {
  static constexpr VkStructureType value = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
};

template <> struct VulkanSType<VkSurfaceFormat2KHR> {
  static constexpr VkStructureType value = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
};

template <> struct VulkanSType<VkQueueFamilyProperties2> {
  static constexpr VkStructureType value = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
};

template <typename T> constexpr VkStructureType VulkanSTypeValue = VulkanSType<T>::value;

} // namespace vkh
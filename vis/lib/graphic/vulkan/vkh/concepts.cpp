module;

#include <volk.h>

export module vis.graphic.vulkan.vkh:concepts;

export namespace vkh {
template <typename T>
concept HasSType = requires(T t) {
  { t.sType };
};

template <typename T>
concept DoesNotHaveSType = !HasSType<T>;
} // namespace vkh
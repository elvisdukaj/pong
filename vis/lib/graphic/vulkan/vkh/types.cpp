module;

#include <volk.h>

export module vis.graphic.vulkan.vkh:types;

import std;

export namespace vkh {

enum class Result {
  Success = VK_SUCCESS,
  NotReady = VK_NOT_READY,
  Timeout = VK_TIMEOUT,
  EventSet = VK_EVENT_SET,
  EventReset = VK_EVENT_RESET,
  Incomplete = VK_INCOMPLETE,
  ErrorOutOfHostMemory = VK_ERROR_OUT_OF_HOST_MEMORY,
  ErrorOutOfDeviceMemory = VK_ERROR_OUT_OF_DEVICE_MEMORY,
  ErrorInitializationFailed = VK_ERROR_INITIALIZATION_FAILED,
  ErrorDeviceLost = VK_ERROR_DEVICE_LOST,
  ErrorMemoryMapFailed = VK_ERROR_MEMORY_MAP_FAILED,
  ErrorLayerNotPresent = VK_ERROR_LAYER_NOT_PRESENT,
  ErrorExtensionNotPresent = VK_ERROR_EXTENSION_NOT_PRESENT,
  ErrorFeatureNotPresent = VK_ERROR_FEATURE_NOT_PRESENT,
  ErrorIncompatibleDriver = VK_ERROR_INCOMPATIBLE_DRIVER,
  ErrorTooManyObjects = VK_ERROR_TOO_MANY_OBJECTS,
  ErrorFormatNotSupported = VK_ERROR_FORMAT_NOT_SUPPORTED,
  ErrorFragmentedPool = VK_ERROR_FRAGMENTED_POOL,
  ErrorUnknown = VK_ERROR_UNKNOWN,
  ErrorOutOfPoolMemory = VK_ERROR_OUT_OF_POOL_MEMORY,
  ErrorOutOfPoolMemoryKHR = VK_ERROR_OUT_OF_POOL_MEMORY_KHR,
  ErrorInvalidExternalHandle = VK_ERROR_INVALID_EXTERNAL_HANDLE,
  ErrorInvalidExternalHandleKHR = VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR,
  ErrorFragmentation = VK_ERROR_FRAGMENTATION,
  ErrorFragmentationEXT = VK_ERROR_FRAGMENTATION_EXT,
  ErrorInvalidOpaqueCaptureAddress = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
  ErrorInvalidDeviceAddressEXT = VK_ERROR_INVALID_DEVICE_ADDRESS_EXT,
  ErrorInvalidOpaqueCaptureAddressKHR = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR,
  PipelineCompileRequired = VK_PIPELINE_COMPILE_REQUIRED,
  ErrorPipelineCompileRequiredEXT = VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT,
  PipelineCompileRequiredEXT = VK_PIPELINE_COMPILE_REQUIRED_EXT,
  ErrorSurfaceLostKHR = VK_ERROR_SURFACE_LOST_KHR,
  ErrorNativeWindowInUseKHR = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
  SuboptimalKHR = VK_SUBOPTIMAL_KHR,
  ErrorOutOfDateKHR = VK_ERROR_OUT_OF_DATE_KHR,
  ErrorIncompatibleDisplayKHR = VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
  ErrorValidationFailedEXT = VK_ERROR_VALIDATION_FAILED_EXT,
  ErrorInvalidShaderNV = VK_ERROR_INVALID_SHADER_NV,
  ErrorImageUsageNotSupportedKHR = VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR,
  ErrorVideoPictureLayoutNotSupportedKHR = VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,
  ErrorVideoProfileOperationNotSupportedKHR = VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,
  ErrorVideoProfileFormatNotSupportedKHR = VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,
  ErrorVideoProfileCodecNotSupportedKHR = VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR,
  ErrorVideoStdVersionNotSupportedKHR = VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR,
  ErrorInvalidDrmFormatModifierPlaneLayoutEXT = VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
  ErrorNotPermittedKHR = VK_ERROR_NOT_PERMITTED_KHR,
  ErrorNotPermittedEXT = VK_ERROR_NOT_PERMITTED_EXT,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  ErrorFullScreenExclusiveModeLostEXT = VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
#endif /*VK_USE_PLATFORM_WIN32_KHR*/
  ThreadIdleKHR = VK_THREAD_IDLE_KHR,
  ThreadDoneKHR = VK_THREAD_DONE_KHR,
  OperationDeferredKHR = VK_OPERATION_DEFERRED_KHR,
  OperationNotDeferredKHR = VK_OPERATION_NOT_DEFERRED_KHR,
  ErrorInvalidVideoStdParametersKHR = VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR,
  ErrorCompressionExhaustedEXT = VK_ERROR_COMPRESSION_EXHAUSTED_EXT,
  IncompatibleShaderBinaryEXT = VK_INCOMPATIBLE_SHADER_BINARY_EXT,
  ErrorIncompatibleShaderBinaryEXT = VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT,
  PipelineBinaryMissingKHR = VK_PIPELINE_BINARY_MISSING_KHR,
  ErrorNotEnoughSpaceKHR = VK_ERROR_NOT_ENOUGH_SPACE_KHR
};

template <typename FlagBitsType> struct FlagTraits {
  static constexpr bool is_bit_mask = false;
};

template <typename BitType> class Flags {
public:
  using MaskType = typename std::underlying_type<BitType>::type;

  // constructors
  constexpr Flags() noexcept : mask(0) {}

  constexpr Flags(BitType bit) noexcept : mask(static_cast<MaskType>(bit)) {}
  constexpr Flags(Flags<BitType> const& rhs) noexcept = default;

  constexpr explicit Flags(MaskType flags) noexcept : mask(flags) {}

  constexpr Flags<BitType> operator&(Flags<BitType> const& rhs) const noexcept {
    return Flags<BitType>(mask & rhs.mask);
  }

  constexpr Flags<BitType> operator|(Flags<BitType> const& rhs) const noexcept {
    return Flags<BitType>(mask | rhs.mask);
  }

  constexpr Flags<BitType> operator^(Flags<BitType> const& rhs) const noexcept {
    return Flags<BitType>(mask ^ rhs.mask);
  }

  constexpr Flags<BitType> operator~() const noexcept {
    return Flags<BitType>(mask ^ FlagTraits<BitType>::allFlags.m_mask);
  }

  // assignment operators
  constexpr Flags<BitType>& operator=(Flags<BitType> const& rhs) noexcept = default;

  constexpr Flags<BitType>& operator|=(Flags<BitType> const& rhs) noexcept {
    mask |= rhs.mask;
    return *this;
  }

  constexpr Flags<BitType>& operator&=(Flags<BitType> const& rhs) noexcept {
    mask &= rhs.mask;
    return *this;
  }

  constexpr Flags<BitType>& operator^=(Flags<BitType> const& rhs) noexcept {
    mask ^= rhs.mask;
    return *this;
  }

  // cast operators
  explicit constexpr operator bool() const noexcept {
    return !!mask;
  }

  explicit constexpr operator MaskType() const noexcept {
    return mask;
  }

private:
  MaskType mask;
};

enum class InstanceCreateFlagBits : VkInstanceCreateFlags {
  EnumeratePortabilityKHR = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
};

using InstanceCreateFlags = Flags<InstanceCreateFlagBits>;

} // namespace vkh
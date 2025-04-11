module;

#include <SDL3/SDL.h>

#include <vulkan/vulkan.h>
#if defined(__APPLE__)
#include <vulkan/vulkan_metal.h>
#endif

#include <cassert>
#include <optional>
#include <stdexcept>

export module vis:vulkan;

import std;

import :math;
import :window;

namespace views = std::ranges::views;

namespace {

struct LayerProperties {
	VkLayerProperties properties;
	std::vector<VkExtensionProperties> extensions;
};

std::vector<VkLayerProperties> enumerate_instance_layers_properties() {
	std::vector<VkLayerProperties> layers;
	uint32_t layerCount = 0;

	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	layers.resize(layerCount);

	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
	return layers;
}

std::vector<VkExtensionProperties> enumerate_instance_layer_extensions(std::string_view layer_name) {
	std::vector<VkExtensionProperties> extensions;
	uint32_t layerCount = 0;

	vkEnumerateInstanceExtensionProperties(layer_name.data(), &layerCount, nullptr);
	extensions.resize(layerCount);

	vkEnumerateInstanceExtensionProperties(layer_name.data(), &layerCount, extensions.data());
	return extensions;
}

std::vector<LayerProperties> enumerate_instance_layers() {
	std::vector<LayerProperties> res;

	// Instance extensions
	VkLayerProperties layerProperties{.layerName = {'\0'},
																		.specVersion = VK_VERSION_1_4,
																		.implementationVersion = VK_VERSION_1_4,
																		.description = {'\0'}};
	res.emplace_back(layerProperties, enumerate_instance_layer_extensions(""));

	auto layers = enumerate_instance_layers_properties();
	std::transform(begin(layers), end(layers), std::back_inserter(res), [](VkLayerProperties prop) {
		return LayerProperties{.properties = prop, .extensions = enumerate_instance_layer_extensions(prop.layerName)};
	});

	return res;
}

std::string vk_version_to_string(uint32_t version) {
	const auto maj = VK_API_VERSION_MAJOR(version);
	const auto min = VK_API_VERSION_MINOR(version);
	const auto patch = VK_API_VERSION_PATCH(version);
	return std::format("{}.{}.{}", maj, min, patch);
}

struct PhysicalDevice {
	VkPhysicalDevice device;
	VkPhysicalDeviceProperties properties;
	std::vector<VkExtensionProperties> extensions;
	std::vector<VkQueueFamilyProperties> queue_properties;
	VkPhysicalDeviceMemoryProperties memory_properties;

	std::string_view name() const {
		return std::string_view{properties.deviceName};
	}

	std::string api_version() const {
		return vk_version_to_string(properties.apiVersion);
	}

	std::string type() const {
		switch (properties.deviceType) {
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			return "other";

		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return "integrated GPU";

		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return "discrete GPU";

		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return "virtual GPU";

		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			return "CPU";

		default:
			return "unknown";
		}
	}
};

class LogicalDevice {
public:
	static std::optional<LogicalDevice> create(const PhysicalDevice& physical_device, size_t queue_index,
																						 const std::vector<const char*>& extensions) {
		float queue_priorities[] = {0.0f};
		auto device_queue_create_infos = VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = static_cast<uint32_t>(queue_index),
				.queueCount = 1,
				.pQueuePriorities = queue_priorities,
		};

		VkDeviceCreateInfo create_info{
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext = nullptr,
				.flags = VK_QUEUE_GRAPHICS_BIT,
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &device_queue_create_infos,

				// deprecated
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,

				.enabledExtensionCount = static_cast<uint32_t>(size(extensions)),
				.ppEnabledExtensionNames = extensions.data(),
				.pEnabledFeatures = nullptr,
		};

		VkDevice device_handle;
		vkCreateDevice(physical_device.device, &create_info, nullptr, &device_handle);
		return LogicalDevice{device_handle};
	}

	LogicalDevice(LogicalDevice&) = delete;
	LogicalDevice& operator=(LogicalDevice&) = delete;

	~LogicalDevice() {
		if (device == VK_NULL_HANDLE)
			return;

		vkDestroyDevice(device, nullptr);
	}

	friend void swap(LogicalDevice& lhs, LogicalDevice& rhs) {
		std::swap(lhs.device, rhs.device);
	}

	LogicalDevice(LogicalDevice&& other) : device{VK_NULL_HANDLE} {
		swap(*this, other);
	}

	LogicalDevice& operator=(LogicalDevice&& other) {
		swap(*this, other);
		return *this;
	}

private:
	explicit LogicalDevice(VkDevice device) : device{device} {}

private:
	VkDevice device = VK_NULL_HANDLE;
};

namespace internal {
std::vector<VkPhysicalDevice> enumerate_physical_devices(VkInstance instance) {
	uint32_t devices_count = 0;
	vkEnumeratePhysicalDevices(instance, &devices_count, nullptr);
	std::vector<VkPhysicalDevice> devices(devices_count, VK_NULL_HANDLE);

	vkEnumeratePhysicalDevices(instance, &devices_count, devices.data());

	return devices;
}

std::vector<VkExtensionProperties> enumerate_device_properties(VkPhysicalDevice device,
																															 std::vector<const char*> layers) {
	uint32_t properties_count = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &properties_count, nullptr);

	std::vector<VkExtensionProperties> properties;
	properties.resize(properties_count);

	if (layers.size() == 0) {
		vkEnumerateDeviceExtensionProperties(device, nullptr, &properties_count, properties.data());
	} else {
		for (auto layer : layers) {
			vkEnumerateDeviceExtensionProperties(device, layer, &properties_count, properties.data());
		}
	}

	return properties;
}

std::vector<VkQueueFamilyProperties> enumerate_device_queue_family_properties(VkPhysicalDevice device) {
	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

	std::vector<VkQueueFamilyProperties> queues(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queues.data());

	return queues;
}

std::vector<const char*> get_required_extensions() {
	std::vector<const char*> required_extensions = {VK_KHR_SURFACE_EXTENSION_NAME};

#if defined(__APPLE__)
	required_extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
	required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

	return required_extensions;
}

std::vector<const char*> get_required_layers() {
	std::vector<const char*> required_layers;

#if not defined(NDEBUG)
	required_layers.push_back("VK_LAYER_KHRONOS_validation");

#if not defined(__linux)
	required_layers.push_back("VK_LAYER_LUNARG_api_dump");
#endif

#endif

	return required_layers;
}

VkInstanceCreateFlags get_required_instance_flags() {
	VkInstanceCreateFlags flags = 0;

#if defined(__APPLE__)
	flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
	return flags;
}

} // namespace internal

std::vector<PhysicalDevice> enumerate_devices(VkInstance instance, std::vector<const char*> required_layers) {
	auto devices = internal::enumerate_physical_devices(instance);
	std::vector<PhysicalDevice> result(devices.size());
	std::transform(begin(devices), end(devices), begin(result), [&required_layers](VkPhysicalDevice device) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

		return PhysicalDevice{
				.device = device,
				.properties = std::move(properties),
				.extensions = internal::enumerate_device_properties(device, required_layers),
				.queue_properties = internal::enumerate_device_queue_family_properties(device),
				.memory_properties = std::move(memory_properties),
		};
	});
	return result;
}

std::vector<int> score_gpus(const std::vector<PhysicalDevice>& devices) {
	std::vector<int> scores(devices.size());

	std::transform(begin(devices), end(devices), begin(scores), [](const PhysicalDevice& device) {
		int score = 0;
		score += device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1000 : 0;
		score += device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 500 : 0;

		bool no_gpu_queue = std::find_if(begin(device.queue_properties), end(device.queue_properties),
																		 [](const VkQueueFamilyProperties& prop) -> bool {
																			 return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
																		 }) == end(device.queue_properties);

		VkDeviceSize total_memory{0};
		for (auto i = 0u; i < device.memory_properties.memoryHeapCount; ++i) {
			total_memory += device.memory_properties.memoryHeaps[i].size;
		}

		score += total_memory / (1024 * 1024 * 1024) * 10;

		if (no_gpu_queue)
			score = 0;

		return score;
	});

	return scores;
}

size_t get_graphic_queue_index(PhysicalDevice& device) {
	auto it =
			std::find_if(begin(device.queue_properties), end(device.queue_properties),
									 [](const VkQueueFamilyProperties& prop) -> bool { return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT; });

	auto index = static_cast<size_t>(std::distance(begin(device.queue_properties), it));
	return index;
}

std::expected<VkInstance, std::string> vk_create_instance(std::string_view application_name,
																													uint32_t application_version,
																													std::vector<const char*> required_extensions,
																													std::vector<const char*> required_layers,
																													VkInstanceCreateFlags flags) {
	VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
														 .pNext = nullptr,
														 .pApplicationName = application_name.data(),
														 .applicationVersion = application_version,
														 .pEngineName = "vis game engine",
														 .engineVersion = VK_MAKE_VERSION(0, 0, 1),
														 .apiVersion = VK_API_VERSION_1_2};
	VkInstanceCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags,
			.pApplicationInfo = &app_info,
			.enabledLayerCount = static_cast<uint32_t>(required_layers.size()),
			.ppEnabledLayerNames = required_layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
			.ppEnabledExtensionNames = required_extensions.data(),
	};

	VkInstance instance = VK_NULL_HANDLE;
	auto res = vkCreateInstance(&create_info, nullptr, &instance);
	if (res != VK_SUCCESS) {
		return std::unexpected("Unable to create a Vulkan Instance");
	}
	return instance;
} // namespace

} // namespace

namespace std {
template <> struct std::formatter<VkLayerProperties> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const VkLayerProperties& layer, std::format_context& ctx) const {
		return std::format_to(ctx.out(),
													" - name:        {}\n"
													"     description: {}\n"
													"     spec version: {}\n"
													"     impl version: {}\n",
													std::string_view{layer.layerName}, std::string_view{layer.description},
													vk_version_to_string(layer.specVersion), layer.implementationVersion);
	}
};

template <> struct std::formatter<VkExtensionProperties> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const VkExtensionProperties& extension, std::format_context& ctx) const {
		return std::format_to(ctx.out(),
													" - name:        {}\n"
													"   spec version: {}\n",
													std::string_view{extension.extensionName}, vk_version_to_string(extension.specVersion));
	}
};

template <> struct std::formatter<std::vector<VkExtensionProperties>> : std::formatter<std::string> {
	auto format(const std::vector<VkExtensionProperties>& extensions, std::format_context& ctx) const {
		std::string temp;
		for (const auto& extension : extensions) {
			std::format_to(back_inserter(temp), "{}", extension);
		}
		return std::formatter<std::string>::format(temp, ctx);
	}
};

template <> struct std::formatter<LayerProperties> : std::formatter<std::string> {
	auto format(const LayerProperties& layer, std::format_context& ctx) const {
		std::string temp;

		if (std::string_view{layer.properties.layerName}.length() > 0) {
			std::format_to(back_inserter(temp), "Layer:\n  {}", layer.properties);
		}
		std::format_to(back_inserter(temp), "Extensions:\n{}", layer.extensions);
		return std::formatter<std::string>::format(temp, ctx);
	}
};

template <> struct std::formatter<VkQueueFamilyProperties> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const VkQueueFamilyProperties& prop, std::format_context& ctx) const {
		auto flags = prop.queueFlags;
		std::string flags_str;

		if (flags & VK_QUEUE_GRAPHICS_BIT)
			flags_str += "graphic|";

		if (flags & VK_QUEUE_COMPUTE_BIT)
			flags_str += "compute|";

		if (flags & VK_QUEUE_TRANSFER_BIT)
			flags_str += "transfer|";

		if (flags & VK_QUEUE_SPARSE_BINDING_BIT)
			flags_str += "sparse binding|";

		if (flags & VK_QUEUE_PROTECTED_BIT)
			flags_str += "protected|";

		if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
			flags_str += "video decoding|";

		if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
			flags_str += "video encoding|";

		if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
			flags_str += "optical_flow|";

		if (not empty(flags_str))
			flags_str.pop_back();

		return std::format_to(ctx.out(),
													"  - flags: {}\n"
													"  - count: {}\n"
													"  - min transfer limits: [width x height x depth] {}x{}x{}\n",
													flags_str, prop.queueCount, prop.minImageTransferGranularity.width,
													prop.minImageTransferGranularity.height, prop.minImageTransferGranularity.depth);
	}
};

template <> struct std::formatter<std::vector<VkQueueFamilyProperties>> : std::formatter<std::string> {
	auto format(const std::vector<VkQueueFamilyProperties>& props, std::format_context& ctx) const {
		std::string temp;

		for (auto& p : props)
			std::format_to(std::back_inserter(temp), "{}\n", p);

		return std::formatter<std::string>::format(temp, ctx);
	}
};

template <> struct std::formatter<VkMemoryType> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const VkMemoryType& memory_type, std::format_context& ctx) const {
		std::string type;
		if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			type += "device local|";
		if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			type += "host visible|";
		if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			type += "host coherent|";
		if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
			type += "host cached|";
		if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
			type += "lazily allocated|";
		if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
			type += "protected|";
		if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
			type += "device coherent|";
		if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
			type += "device uncached|";
		if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)
			type += "RDMA capable|";

		if (not empty(type))
			type.pop_back();

		if (empty(type))
			type = "none";

		return std::format_to(ctx.out(), "type \"{}\" at index {}", type, memory_type.heapIndex);
	}
};

template <> struct std::formatter<VkMemoryHeap> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const VkMemoryHeap& memory_heap, std::format_context& ctx) const {
		std::string type;
		if (memory_heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			type += "device local|";
		if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT)
			type += "multi instance|";

		if (not empty(type))
			type.pop_back();

		if (empty(type))
			type = "none";

		return std::format_to(ctx.out(), "size: {}  [\"{}\"]", memory_heap.size, type);
	}
};

template <> struct std::formatter<VkPhysicalDeviceMemoryProperties> : std::formatter<std::string> {

	auto format(const VkPhysicalDeviceMemoryProperties& memory_properties, std::format_context& ctx) const {
		std::string temp{"Types:\n"};
		for (uint32_t i = 0u; i < memory_properties.memoryTypeCount; i++) {
			std::format_to(back_inserter(temp), "{}\n", memory_properties.memoryTypes[i]);
		}

		temp += "Heaps:\n";
		for (uint32_t i = 0u; i < memory_properties.memoryHeapCount; i++) {
			std::format_to(back_inserter(temp), "{}: {}\n", i, memory_properties.memoryHeaps[i]);
		}

		return std::formatter<std::string>::format(temp, ctx);
	}
};

template <> struct std::formatter<PhysicalDevice> : std::formatter<std::string> {
	auto format(const PhysicalDevice& device, std::format_context& ctx) const {
		std::string temp;

		std::format_to(back_inserter(temp),
									 "name: {}\n"
									 "type: {}\n"
									 "vulkan version: {}\n"
									 "queues:\n{}"
									 "Memory:\n{}",
									 device.name(), device.type(), device.api_version(), device.queue_properties,
									 device.memory_properties);
		std::format_to(back_inserter(temp), "Extensions:\n{}", device.extensions);
		return std::formatter<std::string>::format(temp, ctx);
	}
};

} // namespace std

export namespace vis::vk {

class Renderer {
public:
	static std::expected<Renderer, std::string> create(Window* window) {
		auto title = SDL_GetWindowTitle(static_cast<SDL_Window*>(*window));

		VkInstanceCreateFlags flags = internal::get_required_instance_flags();
		std::vector<const char*> required_extensions = internal::get_required_extensions();
		std::vector<const char*> required_layers = internal::get_required_layers();

		auto instance = vk_create_instance(title, VK_MAKE_VERSION(0, 0, 1), required_extensions, required_layers, flags);
		if (not instance.has_value()) {
			return std::unexpected(instance.error());
		}

		return Renderer{required_extensions, required_layers, window, *instance};
	}

	~Renderer() {
		if (instance == VK_NULL_HANDLE)
			return;

		device = std::nullopt;
		vkDestroyInstance(instance, nullptr);
	}

	Renderer(Renderer&) = delete;
	Renderer& operator=(Renderer&) = delete;

	friend void swap(Renderer& lhs, Renderer& rhs) {
		std::swap(lhs.required_extensions, rhs.required_extensions);
		std::swap(lhs.required_layers, rhs.required_layers);
		std::swap(lhs.window, rhs.window);
		std::swap(lhs.layers, rhs.layers);
		std::swap(lhs.instance, rhs.instance);
		std::swap(lhs.devices, rhs.devices);
		std::swap(lhs.physical_device, rhs.physical_device);
		std::swap(lhs.gpu_score, rhs.gpu_score);
		std::swap(lhs.gpu_queue_index, rhs.gpu_queue_index);
		std::swap(lhs.device, rhs.device);
	}

	Renderer(Renderer&& other) : window{nullptr}, layers{}, instance{VK_NULL_HANDLE} {
		swap(*this, other);
	}

	Renderer& operator=(Renderer&& rhs) {
		swap(*this, rhs);
		return *this;
	}

	void set_clear_color([[maybe_unused]] const vec4& color) {}
	void clear() {}
	void render() const {
		// SDL_GL_SwapWindow(*context.window);
	}

	void set_viewport([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int width,
										[[maybe_unused]] int height) {}

	std::string show_info() const {
		std::string result = "Vulkan Renderer\n";

		std::format_to(std::back_inserter(result), "Layers:\n");
		for (auto& layer : layers) {
			std::format_to(back_inserter(result), "{}\n", layer);
		}

		std::format_to(std::back_inserter(result), "Founded {} devices", devices.size());
		for (const auto& dev : devices) {
			std::format_to(back_inserter(result), "\n{}", dev);
		};

		std::format_to(std::back_inserter(result), "Selected device is: {} (score: {})", physical_device.name(), gpu_score);
		return result;
	}

private:
	explicit Renderer(std::vector<const char*> required_extensions, std::vector<const char*> required_layers,
										Window* window, VkInstance instance)
			: required_extensions{std::move(required_extensions)},
				required_layers{std::move(required_layers)},
				window{window},
				layers{enumerate_instance_layers()},
				instance{instance} {
		devices = enumerate_devices(instance, required_layers);

		if (empty(devices))
			throw std::runtime_error{"no physical device found"};

		auto gpu_scores = score_gpus(devices);
		auto it = std::max_element(begin(gpu_scores), end(gpu_scores));

		if (*it == 0)
			throw std::runtime_error{"No suitable GPU scores found"};

		size_t gpu_idx = static_cast<size_t>(std::distance(begin(gpu_scores), it));
		gpu_score = *it;
		physical_device = devices.at(gpu_idx);

		gpu_queue_index = get_graphic_queue_index(physical_device);

		device = LogicalDevice::create(physical_device, gpu_queue_index, required_extensions);
	}

private:
	std::vector<const char*> required_extensions;
	std::vector<const char*> required_layers;

	Window* window;
	std::vector<LayerProperties> layers;
	VkInstance instance;

	std::vector<PhysicalDevice> devices;
	PhysicalDevice physical_device;
	int gpu_score = 0;
	size_t gpu_queue_index = 0;
	std::optional<LogicalDevice> device;
}; // namespace vis::vk

} // namespace vis::vk
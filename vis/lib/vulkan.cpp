module;

#include <SDL3/SDL.h>

#include <vulkan/vulkan.h>
#if defined(__APPLE__)
#include <vulkan/vulkan_metal.h>
#endif

#include <cassert>

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

std::vector<const char*> get_required_extensions() {
	std::vector<const char*> required_extensions = {VK_KHR_SURFACE_EXTENSION_NAME};

#if defined(__APPLE__)
	required_extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif

	return required_extensions;
}

std::vector<const char*> get_required_layers() {
	std::vector<const char*> required_layers;

#if not defined(NDEBUG) and not defined(__linux__)
	required_layers.push_back("VK_LAYER_LUNARG_api_dump");
	required_layers.push_back("VK_LAYER_KHRONOS_validation");
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
	std::vector<PhysicalDevice> result;
	std::transform(begin(devices), end(devices), std::back_inserter(result), [&required_layers](VkPhysicalDevice device) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		return PhysicalDevice{
				.device = device,
				.properties = std::move(properties),
				.extensions = internal::enumerate_device_properties(device, required_layers),
		};
	});
	return result;
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
														 .apiVersion = VK_API_VERSION_1_4};
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
													std::string_view{extension.extensionName}, extension.specVersion);
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

template <> struct std::formatter<PhysicalDevice> : std::formatter<std::string> {
	auto format(const PhysicalDevice& device, std::format_context& ctx) const {
		std::string temp;

		std::format_to(back_inserter(temp),
									 "name: {}\n"
									 "type: {}\n"
									 "vulkan version: {}\n",
									 device.name(), device.type(), device.api_version());
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
		if (instance != VK_NULL_HANDLE) {
			vkDestroyInstance(instance, nullptr);
		}
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

		std::format_to(std::back_inserter(result), "Founded devices: {}", devices.size());
		for (auto& device : devices) {
			std::format_to(back_inserter(result), "Device: \n{}", device);
		};
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
	}

private:
	std::vector<const char*> required_extensions;
	std::vector<const char*> required_layers;

	Window* window;
	std::vector<LayerProperties> layers;
	VkInstance instance;

	std::vector<PhysicalDevice> devices;
}; // namespace vis::vk

} // namespace vis::vk
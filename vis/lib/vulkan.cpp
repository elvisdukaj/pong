module;

#include <SDL3/SDL.h>

#include <vulkan/vulkan.h>

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

std::vector<VkLayerProperties> enumerateInstanceLayersProperties() {
	std::vector<VkLayerProperties> layers;
	uint32_t layerCount = 0;

	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	layers.resize(layerCount);

	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
	return layers;
}

std::vector<VkExtensionProperties> enumerateInstanceLayerExtensions(std::string_view layer_name) {
	std::vector<VkExtensionProperties> extensions;
	uint32_t layerCount = 0;

	vkEnumerateInstanceExtensionProperties(layer_name.data(), &layerCount, nullptr);
	extensions.resize(layerCount);

	vkEnumerateInstanceExtensionProperties(layer_name.data(), &layerCount, extensions.data());
	return extensions;
}

std::vector<LayerProperties> enumerateInstanceLayers() {
	std::vector<LayerProperties> res;

	// Instance extensions
	VkLayerProperties layerProperties{.layerName = {'\0'},
																		.specVersion = VK_VERSION_1_4,
																		.implementationVersion = VK_VERSION_1_4,
																		.description = {'\0'}};
	res.emplace_back(layerProperties, enumerateInstanceLayerExtensions(""));

	auto layers = enumerateInstanceLayersProperties();
	std::transform(begin(layers), end(layers), std::back_inserter(res), [](VkLayerProperties prop) {
		return LayerProperties{.properties = prop, .extensions = enumerateInstanceLayerExtensions(prop.layerName)};
	});

	return res;
}

std::string vk_version_to_string(uint32_t version) {
	const auto maj = VK_API_VERSION_MAJOR(version);
	const auto min = VK_API_VERSION_MINOR(version);
	const auto patch = VK_API_VERSION_PATCH(version);
	return std::format("{}.{}.{}", maj, min, patch);
}

std::expected<VkInstance, std::string> vk_create_instance(std::string_view application_name,
																													uint32_t application_version) {
	VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
														 .pNext = nullptr,
														 .pApplicationName = application_name.data(),
														 .applicationVersion = application_version,
														 .pEngineName = "vis game engine",
														 .engineVersion = VK_MAKE_VERSION(0, 0, 1),
														 .apiVersion = VK_API_VERSION_1_4};

	VkInstanceCreateFlags flags = 0;
	std::vector<const char*> required_extensions;

#if defined(__APPLE__)
	flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

	VkInstanceCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags,
			.pApplicationInfo = &app_info,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
			.ppEnabledExtensionNames = required_extensions.data(),
	};

	VkInstance instance = VK_NULL_HANDLE;
	auto res = vkCreateInstance(&create_info, nullptr, &instance);
	if (res != VK_SUCCESS) {
		return std::unexpected("Unable to create a Vulkan Instance");
	}
	return instance;
}

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
} // namespace std

export namespace vis::vk {

class Renderer {
public:
	static std::expected<Renderer, std::string> create(Window* window) {
		auto title = SDL_GetWindowTitle(static_cast<SDL_Window*>(*window));
		auto instance = vk_create_instance(title, VK_MAKE_VERSION(0, 0, 1));
		if (not instance.has_value()) {
			return std::unexpected(instance.error());
		}
		return Renderer{window, *instance};
	}

	~Renderer() {
		if (instance != VK_NULL_HANDLE) {
			vkDestroyInstance(instance, nullptr);
		}
	}

	Renderer(Renderer&) = delete;
	Renderer& operator=(Renderer&) = delete;

	friend void swap(Renderer& lhs, Renderer& rhs) {
		std::swap(lhs.window, rhs.window);
		std::swap(lhs.layers, rhs.layers);
		std::swap(lhs.instance, rhs.instance);
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
		return result;
	}

private:
	explicit Renderer(Window* window, VkInstance instance)
			: window{window}, layers{enumerateInstanceLayers()}, instance{instance} {}

private:
	Window* window;
	std::vector<LayerProperties> layers;
	VkInstance instance;
}; // namespace vis::vk

} // namespace vis::vk
module;

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include <SDL3/SDL.h>

#include <yaml-cpp/yaml.h>

export module vis.graphic.vulkan;

import std;
import vulkan_helper;
import vis.math;
import vis.window;

export namespace vis::vulkan {

class Renderer {
public:
	static std::expected<Renderer, std::string> create(Window* window);

	~Renderer() = default;

	Renderer(Renderer&) = delete;
	Renderer& operator=(Renderer&) = delete;

	friend void swap(Renderer& lhs, Renderer& rhs);

	Renderer(Renderer&& other) = default;
	Renderer& operator=(Renderer&& rhs);

	void set_clear_color([[maybe_unused]] const vec4& color) {}
	void clear() {}
	void render() const {
		// SDL_GL_SwapWindow(*context.window);
	}

	void set_viewport([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int width,
										[[maybe_unused]] int height) {}

	std::string show_info() const;

private:
	explicit Renderer(Window* window);

	void create_instance();
	void create_surface();
	void enumerate_gpus(vkh::PhysicalDeviceSelector& device_selector);
	void select_gpu(vkh::PhysicalDeviceSelector& device_selector);

private:
	Window* window;
	vkh::Context context;
	vkh::Instance vk_instance{nullptr};
	vkh::Surface surface{nullptr};
	std::vector<vkh::PhysicalDevice> physical_devices;
	vkh::PhysicalDevice selected_physical_device;
	YAML::Node vk_config;
}; // namespace vis::vk

} // namespace vis::vulkan
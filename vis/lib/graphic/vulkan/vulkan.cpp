export module vis.graphic.vulkan;

import std;
import vis.window;
import vis.math;

export namespace vis::vulkan {

class Renderer {
public:
	static std::expected<Renderer, std::string> create(Window* window);

	Renderer(Renderer&&);
	Renderer& operator=(Renderer&&);

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	~Renderer();

	void set_clear_color(vec4 color);
	void clear();
	void render() const;

	void set_viewport([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int width,
										[[maybe_unused]] int height);

	std::string show_info() const;

private:
	explicit Renderer(Window* window);

private:
	class Impl;
	std::unique_ptr<Impl> impl;
}; // namespace vis::vk

} // namespace vis::vulkan
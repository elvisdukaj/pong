from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class PongRecipe(ConanFile):
    name = "pong"
    version = "1"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Elvis Dukaj"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "Simple Phisical simulations"
    topics = ("pong", "game")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = (
        "CMakeLists.txt",
        "pong/*",
        "vis/*",
    )

    def build_requirements(self):
        self.tool_requires("glslang/1.3.268.0")

    def requirements(self):
        self.requires("sdl/3.2.6")
        self.requires("glm/1.0.1")
        self.requires("box2d/3.0.0")
        self.requires("glew/2.2.0")
        self.requires("sml/1.1.9")
        self.requires("vulkan-loader/1.4.309.0")
        self.requires("yaml-cpp/0.8.0")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

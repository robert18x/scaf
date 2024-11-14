from conans import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout, CMake

required_conan_version = ">=1.65.0 <2"

class ScafConan(ConanFile):
    name = "scaf"
    version = "0.1"
    description = "Smart Contracting Agents Framework"
    settings = "os", "compiler", "build_type", "arch", "cppstd"
    requires = [
        "fmt/11.0.2",
        "nlohmann_json/3.11.3",
        "bshoshany-thread-pool/4.1.0",
    ]
    generators = "CMakeDeps", "CMakeToolchain"
    exports_sources = "scaf/*", "tests/*", "CMakeLists.txt", "readme.md"
    no_copy_source = True

    def layout(self):
        cmake_layout(self)

    def package(self):
        self.copy("*.h", dst="include/scaf", src="scaf")
        self.copy("*.a", dst="lib", src="", keep_path=False)

    def build_requirements(self):
        self.test_requires("magic_enum/0.9.6")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_BUILD_TYPE"] = self.settings.build_type
        tc.variables["BUILD_TESTING"] = "ON"
        tc.generate()

        cmake = CMakeDeps(self)
        cmake.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

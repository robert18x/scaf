from conans import ConanFile, CMake, tools


required_conan_version = ">=1.60.0 <2"

class Scafonan(ConanFile):
    name = "scaf"
    version = "0.1"
    description = "Smart Contracting Agents Framework"
    settings = "os", "compiler", "build_type", "arch"
    requires = [
        "fmt/10.2.1",
        "nlohmann_json/3.11.3",
    ]
    generators = "cmake_find_package", "cmake_paths"

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_BUILD_TYPE"] = self.settings.build_type.upper()
        cmake.configure()
        return cmake

    def configure(self):
        tools.check_min_cppstd(self, "23")

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

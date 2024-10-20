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
        "magic_enum/0.9.6",
    ]
    generators = "CMakeDeps", "CMakeToolchain"
    exports_sources = "scaf/*", "tests/*", "CMakeLists.txt", "readme.md"
    options = {
        "build_tests": [True, False],
    }
    default_options = {
        "build_tests": True,
    }
    no_copy_source = True

    def layout(self):
        cmake_layout(self)

    def package(self):
        self.copy("*.h", dst="include/scaf", src="scaf")

    def build_requirements(self):
        pass
        # if self.options.build_tests:

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_BUILD_TYPE"] = self.settings.build_type
        tc.variables["BUILD_TESTING"] = self.options.build_tests
        tc.generate()

        cmake = CMakeDeps(self)
        cmake.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package_id(self):
        self.info.clear()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "scaf")
        self.cpp_info.set_property("cmake_target_name", "scaf::scaf")
        self.cpp_info.set_property("pkg_config_name", "scaf")
        self.cpp_info.includedirs = ['include']
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.requires = [
            'fmt',
            'nlohmann_json',
            'magic_enum',
        ]


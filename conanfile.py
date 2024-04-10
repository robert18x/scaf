from conans import ConanFile

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
    exports_sources = "include/*"
    no_copy_source = True

    def package(self):
        self.copy("*.h", dst="include/scaf", src="include")

    def package_id(self):
        self.info.clear()

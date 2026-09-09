set_project("sylar")

add_rules("mode.debug", "mode.release")

add_cxxflags("-rdynamic", "-Wall", "-Wno-deprecated", "-Werror", "-Wno-unused-function")

add_includedirs("src/log", "src/util", "src/config")

target("test_log")
set_kind("binary")
add_files("tests/test_log.cpp")
add_deps("sylar")

target("test_config")
set_kind("binary")
add_files("tests/test_config.cpp")
add_deps("sylar")

target("sylar")
set_kind("shared")
add_files("src/log/*.cpp", "src/util/*.cpp", "src/config/*.cpp")

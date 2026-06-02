set_project("sylar")

add_rules("mode.debug", "mode.release")

add_cxxflags("-rdynamic", "-Wall", "-Wno-deprecated", "-Werror", "-Wno-unused-function")

add_includedirs("src/")

target("test")
set_kind("binary")
add_files("tests/test.cpp")
add_deps("sylar")

target("sylar")
set_kind("shared")
add_files("src/*.cpp")

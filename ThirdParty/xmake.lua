--
-- > Notice: Amélie Heinrich @ 2024
-- > Create Time: 2024-12-03 05:46:04
--

target("spdlog")
    set_kind("static")
    add_files("spdlog/src/*.cpp")
    add_includedirs("spdlog/include")
    set_encodings("utf-8")
    add_cxxflags("-DSPDLOG_COMPILED_LIB")

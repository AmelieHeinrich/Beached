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

target("ImGui")
    set_kind("static")
    add_files("imgui/*.cpp", "imgui/backends/imgui_impl_win32.cpp", "imgui/backends/imgui_impl_dx12.cpp")
    add_includedirs("imgui/")

target("STB")
    set_kind("static")
    add_files("stb.c")

target("CGLTF")
    set_kind("static")
    add_files("cgltf.c")

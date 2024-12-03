--
-- > Notice: Amélie Heinrich @ 2024
-- > Create Time: 2024-12-03 05:17:30
--

includes("ThirdParty")

add_rules("mode.debug", "mode.release")

target("Beached")
    set_rundir(".")
    set_languages("c++20")
    set_encodings("utf-8")

    if is_plat("windows") then
        add_syslinks("user32", "gdi32", "kernel32", "d3d12", "dxgi", "ThirdParty/DXC/lib/dxcompiler.lib")
    end

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
        add_defines("BEACHED_DEBUG")
    end
    if is_mode("release") then
        set_symbols("hidden")
        set_optimize("fastest")
        set_strip("all")
    end

    -- Copy DLLs in build folder
    before_link(function (target)
        if not os.exists("$(buildir)/$(plat)/$(arch)/$(mode)/Assets/") then
            os.mkdir("$(buildir)/$(plat)/$(arch)/$(mode)/Assets/")
        end
        os.cp("Binaries/*", "$(buildir)/$(plat)/$(arch)/$(mode)/")
        os.cp("Assets/*", "$(buildir)/$(plat)/$(arch)/$(mode)/Assets/")
    end)

    add_files("Source/**.cpp")
    add_includedirs("Source", "ThirdParty/spdlog/include", "ThirdParty/DirectX/include", "ThirdParty/imgui", "ThirdParty/imgui/backends", "ThirdParty/DXC/Include")
    add_deps("spdlog", "ImGui")

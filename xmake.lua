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
        add_syslinks("user32", "gdi32", "kernel32")
    end

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    end

    if is_mode("release") then
        set_symbols("hidden")
        set_optimize("fastest")
        set_strip("all")
    end

    add_files("Source/**.cpp")
    add_includedirs("Source", "ThirdParty/spdlog/include")
    add_deps("spdlog")

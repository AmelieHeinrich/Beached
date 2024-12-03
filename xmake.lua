--
-- > Notice: Amélie Heinrich @ 2024
-- > Create Time: 2024-12-03 05:17:30
--

add_rules("mode.debug", "mode.release")

target("Beached")
    set_rundir(".")
    set_languages("c++20")
    add_files("Source/**.cpp")
    
    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    end

    if is_mode("release") then
        set_symbols("hidden")
        set_optimize("fastest")
        set_strip("all")
    end

--
-- > Notice: Amélie Heinrich @ 2024
-- > Create Time: 2024-12-03 05:17:30
--

includes("ThirdParty")

add_rules("mode.debug", "mode.release", "mode.releasedbg")

target("Beached")
    set_rundir(".")
    set_languages("c++20")
    set_encodings("utf-8")
    -- set_policy("build.sanitizer.address", true)

    if is_plat("windows") then
        add_syslinks("user32",
                     "gdi32",
                     "kernel32",
                     "d3d12",
                     "dxgi",
                     "ThirdParty/DXC/lib/dxcompiler.lib",
                     "ThirdParty/nvtt/lib64/nvtt30205.lib",
                     "ThirdParty/PIX/lib/WinPixEventRuntime.lib")
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
    if is_mode("releasedbg") then
        set_symbols("debug")
        set_optimize("fastest")
        set_strip("all")
    end

    -- Copy DLLs in build folder
    before_link(function (target)
        if not os.exists("$(buildir)/$(plat)/$(arch)/$(mode)/Assets/") then
            os.mkdir("$(buildir)/$(plat)/$(arch)/$(mode)/Assets/")
        end
        if not os.exists("$(buildir)/$(plat)/$(arch)/$(mode)/.cache/") then
            os.mkdir("$(buildir)/$(plat)/$(arch)/$(mode)/.cache/")
        end
        os.cp("Binaries/*", "$(buildir)/$(plat)/$(arch)/$(mode)/")
        os.cp("Assets/*", "$(buildir)/$(plat)/$(arch)/$(mode)/Assets/")
        os.cp(".cache/*", "$(buildir)/$(plat)/$(arch)/$(mode)/.cache/")
    end)

    add_files("Source/**.cpp")
    add_includedirs("Source",
                    "ThirdParty/",
                    "ThirdParty/spdlog/include",
                    "ThirdParty/DirectX/include",
                    "ThirdParty/imgui",
                    "ThirdParty/imgui/backends",
                    "ThirdParty/DXC/Include",
                    "ThirdParty/glm",
                    "ThirdParty/nvtt/",
                    "ThirdParty/PIX/include")
    add_deps("spdlog", "ImGui", "STB", "CGLTF")
    add_defines("GLM_ENABLE_EXPERIMENTAL", "USE_PIX", "GLM_FORCE_DEPTH_ZERO_TO_ONE")

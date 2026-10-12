-- include subprojects
includes("lib/commonlibf4")

set_project("PeekOnly")
set_version("0.1.0")
set_languages("c++23")
set_warnings("allextra")
add_rules("mode.debug", "mode.releasedbg")

target("PeekOnly")
    add_rules("commonlibf4.plugin", {
        name = "PeekOnly",
        author = "Private project",
        description = "Standalone corner peek for Fallout 4 Old-Gen"
    })
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")


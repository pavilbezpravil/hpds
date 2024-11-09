workspace "hpds"
    configurations { "Debug", "Release" }
    flags { "MultiProcessorCompile" }
    platforms { "x64" }
    systemversion "latest"
    cppdialect "C++20"

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"

    filter { "system:linux" }
        libdirs { "bin/linux", "/usr/local/lib"  }
        buildoptions "-std=c++11"
        linkoptions "-pthread"

    startproject "test"

group "deps"
    project "gtest"
        language "C++"
        kind "StaticLib"

        includedirs { "deps/gtest/googletest", "deps/gtest/googletest/include" }
        files { "deps/gtest/googletest/src/*.cc", "/deps/gtest/googletest/src/*.h" }
        
    project "benchmark"
        language "C++"
        kind "SharedLib"
        defines { "benchmark_EXPORTS" }

        includedirs { "deps/benchmark/include" }
        files { "deps/benchmark/src/*.cc", "/deps/benchmark/src/*.h" }

        filter { "system:windows" }
            links { "Shlwapi" }
group ""

project "tests"
    kind "ConsoleApp"
    language "C++"
    files { "tests/*.h", "tests/*.cpp" }

    includedirs { "deps/gtest/googletest/include", "hpds" }
    links { "gtest" }

project "benchmarks"
    kind "ConsoleApp"
    language "C++"
    files { "benchmarks/*.h", "benchmarks/*.cpp" }

    includedirs { "deps/benchmark/include", "hpds" }
    links { "benchmark" }

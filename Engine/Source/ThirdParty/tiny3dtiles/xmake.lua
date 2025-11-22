-- This is a small library to read 3dtiles files writen by myself (Steelwall2014).
target("tiny3dtiles")
    set_kind("static")
    add_deps("headeronlys")
    set_languages("clatest")
    set_languages("cxx20")
    add_files("./tiny3dtiles_impl.cpp")
    add_includedirs("./", {public = true})
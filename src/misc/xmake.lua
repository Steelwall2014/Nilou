target("test_map")
    set_kind("binary")
    set_languages("cxx20")
    add_files("test_map.cpp")

target("test_enumerate")
    set_kind("binary")
    set_languages("cxx20")
    add_files("test_enumerate.cpp")

target("test_refcount")
    set_kind("binary")
    set_languages("cxx20")
    add_files("test_refcount.cpp")
    
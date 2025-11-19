# src/misc/CMakeLists.txt

add_executable(test_map test_map.cpp)
add_executable(test_enumerate test_enumerate.cpp)
add_executable(test_refcount test_refcount.cpp)

if(MSVC)
    target_compile_options(test_map PRIVATE /EHsc)
    target_compile_options(test_enumerate PRIVATE /EHsc)
    target_compile_options(test_refcount PRIVATE /EHsc)
endif()

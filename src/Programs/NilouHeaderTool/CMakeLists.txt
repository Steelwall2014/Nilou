# src/Programs/NilouHeaderTool/CMakeLists.txt

add_executable(NilouHeaderTool
    main.cpp
    utils.cpp
)
target_include_directories(NilouHeaderTool PRIVATE 
    ${CMAKE_SOURCE_DIR}/External/include
)

target_link_directories(NilouHeaderTool PRIVATE 
    ${CMAKE_SOURCE_DIR}/External/lib
)   

target_link_libraries(NilouHeaderTool PRIVATE
    libclang
)

target_compile_definitions(NilouHeaderTool PRIVATE 
    $<$<CONFIG:Debug>:NILOU_DEBUG>
)

add_custom_command(TARGET NilouHeaderTool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/External/bin"
        "$<TARGET_FILE_DIR:NilouHeaderTool>"
)
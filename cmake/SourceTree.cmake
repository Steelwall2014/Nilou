# SourceTree.cmake
# 功能：将代码文件进行分组
# Ref:
# CMake生成VS工程显示头文件和工程目录分级：https://zhuanlan.zhihu.com/p/574885656

macro(source_group_by_dir source_files)
    if(MSVC)
        set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
        foreach(sgbd_file ${${source_files}})
            string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
            string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
            endif(sgbd_nogroup)
            source_group(${sgbd_group_name} FILES ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC)
endmacro(source_group_by_dir)

# 调用示例：source_group_by_dir(CPP_FILES)

# Automatically create source_group directives for the sources passed in.
# 自动为源文件创建 IDE 分组（使用 REGULAR_EXPRESSION，兼容路径分隔符）
function(auto_source_group rootName rootDir)
    # 确保 rootDir 是绝对路径且使用 CMake 风格（/）
    get_filename_component(rootDir "${rootDir}" ABSOLUTE)

    foreach(file IN LISTS ARGN)
        # 获取文件所在目录（绝对路径）
        get_filename_component(fileDir "${file}" DIRECTORY)

        # 计算相对于 rootDir 的相对路径（用于分组名）
        file(RELATIVE_PATH relPath "${rootDir}" "${fileDir}")

        # 跳过不在 rootDir 下的文件
        if(relPath MATCHES "^\\.\\." OR NOT IS_ABSOLUTE "${fileDir}")
            continue()
        endif()

        # === 构造分组名称（使用 \，符合 VS 习惯）===
        if(relPath STREQUAL "")
            set(group "\\${rootName}")
        else()
            string(REPLACE "/" "\\" group "\\${rootName}\\${relPath}")
        endif()

        # === 构造正则表达式（关键：兼容 / 和 \）===
        # 将 fileDir 中的 / 转为 [\\/]
        string(REGEX REPLACE "/" "[\\\\/]" escapedDir "${fileDir}")
        # 正则：匹配该目录下的 .cpp/.h/.hpp/.c 文件
        set(regex "${escapedDir}[\\\\/][^\\\\/]+\\.(cpp|h|hpp|c)$")

        # 应用分组（CMake 会自动去重）
        source_group("${group}" REGULAR_EXPRESSION "${regex}")
    endforeach()
endfunction()

function(auto_dynamic_source_group rootName rootDir)
    file(TO_CMAKE_PATH "${rootDir}" rootDir)

    foreach(file IN LISTS ARGN)
        if(NOT EXISTS "${file}")
            # 可选：跳过不存在的文件（如尚未生成的 .gen.cpp）
            continue()
        endif()

        file(TO_CMAKE_PATH "${file}" absFile)
        file(RELATIVE_PATH relPath "${rootDir}" "${absFile}")

        # 如果文件不在 rootDir 下，跳过
        if(relPath MATCHES "^\\.\\.")
            continue()
        endif()

        # 获取目录部分（不含文件名）
        get_filename_component(dirPart "${relPath}" DIRECTORY)

        if(dirPart STREQUAL "")
            set(group "\\${rootName}")
        else()
            string(REPLACE "/" "\\" dirPart "${dirPart}")
            set(group "\\${rootName}\\${dirPart}")
        endif()

        # 👇 关键：使用 FILES 而不是 REGULAR_EXPRESSION
        source_group("${group}" FILES "${absFile}")
    endforeach()
endfunction()

# 调用示例：auto_source_group(source ${CMAKE_CURRENT_SOURCE_DIR}/source ${CPP_FILES})

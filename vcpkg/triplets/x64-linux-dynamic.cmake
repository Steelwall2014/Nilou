# 文件: triplets/x64-linux-mixed.cmake
# 基于 QGIS 示例修改，以支持混合链接

# 设置目标系统为 Linux
set(VCPKG_CMAKE_SYSTEM_NAME Linux)
# 设置目标架构
set(VCPKG_TARGET_ARCHITECTURE x64)

# 设置 C 运行时 (CRT) 链接方式为动态
set(VCPKG_CRT_LINKAGE dynamic)
# 设置默认的库链接方式为动态 (这是混合链接的基础)
set(VCPKG_LIBRARY_LINKAGE dynamic)

# (可选) 只构建 Release 版本以节省空间和时间
# set(VCPKG_BUILD_TYPE release)

# 启用 RPATH 修复，这对于动态链接库在 Linux 上正确找到依赖很重要
set(VCPKG_FIXUP_ELF_RPATH ON)

# --- 混合链接的关键部分 ---
# 为那些只能或需要静态链接的特定库强制设置链接方式
# 将 'problematic-lib' 替换为你实际需要静态链接的库名
# set(VCPKG_LIBRARY_LINKAGE_xlnt static)
# 如果有其他库也需要静态链接，可以继续添加
# set(VCPKG_LIBRARY_LINKAGE_another-static-only-lib static)

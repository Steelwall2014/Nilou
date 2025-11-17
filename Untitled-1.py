#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
比较两个文件夹，找出第一个文件夹中存在但第二个文件夹中不存在的文件
"""

import os
from pathlib import Path


def find_missing_files(folder1_path, folder2_path):
    """
    遍历第一个文件夹，找出第二个文件夹中不存在的同名文件
    
    Args:
        folder1_path: 第一个文件夹路径
        folder2_path: 第二个文件夹路径
    """
    # 转换为Path对象
    folder1 = Path(folder1_path)
    folder2 = Path(folder2_path)
    
    # 检查文件夹是否存在
    if not folder1.exists():
        print(f"错误：第一个文件夹不存在: {folder1_path}")
        return
    
    if not folder2.exists():
        print(f"错误：第二个文件夹不存在: {folder2_path}")
        return
    
    if not folder1.is_dir():
        print(f"错误：第一个路径不是文件夹: {folder1_path}")
        return
    
    if not folder2.is_dir():
        print(f"错误：第二个路径不是文件夹: {folder2_path}")
        return
    
    # 收集第二个文件夹中的所有文件名（不包含路径）
    folder2_files = set()
    for file_path in folder2.rglob('*'):
        if file_path.is_file():
            folder2_files.add(file_path.name)
    
    # 遍历第一个文件夹中的所有文件
    missing_files = []
    for file_path in folder1.rglob('*'):
        if file_path.is_file():
            file_name = file_path.name
            if file_name not in folder2_files:
                missing_files.append(file_path)
    
    # 输出结果
    if missing_files:
        print(f"在 '{folder1_path}' 中存在但 '{folder2_path}' 中不存在的文件：\n")
        for file_path in sorted(missing_files):
            # 输出相对路径，更易读
            try:
                rel_path = file_path.relative_to(folder1)
                print(str(rel_path))
            except:
                print(str(file_path))
    else:
        print("所有文件都在第二个文件夹中存在。")


def main():
    """主函数"""
    folder1_path = "D:\\Nilou\\src\\Runtime"
    folder2_path = "D:\\Nilou\\Engine"
    
    find_missing_files(folder1_path, folder2_path)


if __name__ == "__main__":
    main()
#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <iostream>

//	===========
//	Shadinclude
//	===========
/*
LICENCE
MIT License

Copyright (c) [2018] [Tahar Meijs]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

INTRODUCTION
The sole purpose of this class is to load a file and extract the text that is in it.
In theory, this class could be used for a variety of text-processing purposes, but
it was initially designed to be used to load shader source code.

USING THIS CLASS
Since the entire class is a static class, you only have to add this single line of
code to your project:

--------------------------------------------------------------------------------------
std::string shaderSource = Shadinclude::load("./path/to/shader.extension");
--------------------------------------------------------------------------------------

This will (recursively) extract the source code from the first shader file.
Now, you might be wondering, what is the point of using your code for something
so trivial as to loading a file and calling the "std::getline()" function on it?

Well, besides loading the shader source code from a single file, the loader also
supports custom keywords that allow you to include external files inside your
shader source code!

PARAMETERS OF THE LOAD FUNCTION
- std::string	path				path to the "main" shader file
- std::string	includeIdentifier		keyword to look for when scanning for files

MISCELLANEOUS
- Author	:	Tahar Meijs
- Date		:	10th - 12th of April 2018
- Language	:	C++ (can easily be converted into other languages)
*/

class Shadinclude
{
public:

	static std::string Preprocess(const std::string &RawCode/*, const std::vector<std::filesystem::path> &IncludeDir, std::string includeIndentifier = "#include"*/);

	// Return the source code of the complete shader
	static std::string load(std::string path, std::string includeIndentifier = "#include");

private:

	static std::string PreprocessInternal(const std::string &RawCode, std::set<std::string> &AlreadyIncludedPathes);

	static void getFilePath(const std::string & fullPath, std::string & pathWithoutFileName)
	{
		// Remove the file name and store the path to this folder
		size_t found = fullPath.find_last_of("/\\");
		pathWithoutFileName = fullPath.substr(0, found + 1);
	}

	static void getFileParentDirPath(const std::string &fullPath, std::string &parentPath)
	{
		// Remove the file name and store the path to this folder
		size_t found = fullPath.find_last_of("/\\");
		std::string pathWithoutFileName = fullPath.substr(0, found);
		found = pathWithoutFileName.find_last_of("/\\");
		parentPath = pathWithoutFileName.substr(0, found + 1);
	}

	// static bool startswith(const std::string &str, const std::string &temp)
	// {
	// 	if (str.size() < temp.size())
	// 		return false;
	// 	int length = std::min(str.size(), temp.size());
	// 	for (int i = 0; i < length; i++)
	// 	{
	// 		if (str[i] != temp[i])
	// 			return false;
	// 	}
	// 	return true;
	// }

};

#pragma once
#include <string>

// ref: https://github.com/CRPropa/CRPropa3

std::string base64_encode(const unsigned char *src, size_t len);
std::string base64_decode(const void* data, const size_t len);
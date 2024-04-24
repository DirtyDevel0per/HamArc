#pragma once
#include <iostream>

std::vector<bool> HamEncode(const std::string& bits);
std::vector<bool> HamDecode(const std::string& bits, const size_t& block_size);
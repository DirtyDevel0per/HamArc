#include "haming.h"

std::vector<bool> HamEncode(const std::string& bits) {
  std::vector<bool> result;
  size_t bits_it = 0;
  size_t second_pow = 1;

  while (bits_it < bits.size()) {
    if (result.size() + 1 == second_pow) {
      result.push_back(false);
      second_pow *= 2;
    } else {
      result.push_back(bits[bits_it] == '1');
      bits_it++;
    }
  }

  result.push_back(false);
  size_t xor_temp = 0;

  for (int i = 0; i < result.size(); i += 2)
    xor_temp ^= result[i];

  result[0] = xor_temp;
  size_t control_bit_index = 2;

  while (control_bit_index < result.size()) {
    xor_temp = 0;
    size_t i = control_bit_index;
    size_t j = 1;

    while (i < result.size()) {
      if (j == control_bit_index) {
        i += control_bit_index;
        j = 0;
      } else {
        xor_temp ^= result[i];
        i++;
        j++;
      }
    }

    result[control_bit_index - 1] = xor_temp;
    control_bit_index *= 2;
  }

  xor_temp = 0;

  for (int i = 0; i < result.size() - 1; ++i)
    xor_temp ^= result[i];
  result[result.size() - 1] = xor_temp;

  return result;
}

std::vector<bool> HamDecode(const std::string& bits, const size_t& block_size) {
  std::vector<bool> bits_for_check;
  const size_t control_bits = log(block_size * 8) / log(2) + 1 + block_size * 8 + 1;

  if (bits.size() < control_bits) {
    uint32_t new_block_size = bits.size() / 8;
    uint32_t new_control_bits = log(new_block_size * 8) / log(2) + 1 + new_block_size * 8 + 1;

    while (new_control_bits >= bits.size()) {
      new_control_bits = log(new_block_size * 8) / log(2) + 1 + new_block_size * 8 + 1;
      new_block_size--;
    }

    for (int i = 0; i < new_control_bits; ++i)
      bits_for_check.push_back(bits[i] == '1');

  } else {
    for (int i = 0; i < control_bits; ++i)
      bits_for_check.push_back(bits[i] == '1');
  }

  int temp_xor = 0;
  for (int i = 2; i < bits_for_check.size() - 1; i += 2)
    temp_xor ^= bits_for_check[i];

  bits_for_check[0] = temp_xor;
  int control_bit_index = 2;

  while (control_bit_index < bits_for_check.size() - 1) {
    temp_xor = 0;
    uint32_t i = control_bit_index;
    uint32_t j = 1;

    while (i < bits_for_check.size() - 1) {
      if (j == control_bit_index) {
        i += control_bit_index;
        j = 0;
      } else {
        temp_xor ^= bits_for_check[i];
        i++;
        j++;
      }
    }

    bits_for_check[control_bit_index - 1] = temp_xor;
    control_bit_index *= 2;
  }

  temp_xor = 0;
  for (int i = 0; i < bits_for_check.size() - 1; i += 1)
    temp_xor ^= bits_for_check[i];
  bits_for_check[bits_for_check.size() - 1] = temp_xor;
  uint32_t mistake_sum = 0;

  for (int i = 0; i < bits_for_check.size() - 1; ++i) {
    if (!bits_for_check[i] == (bits[i] == '0'))
      mistake_sum += i + 1;
  }
  if (mistake_sum != 0 && bits_for_check[bits_for_check.size() - 1] != (bits[bits_for_check.size() - 1] == '0')) {
    bits_for_check[mistake_sum - 1] = bits_for_check[mistake_sum - 1] ^ 1;
  }
  if (mistake_sum != 0 && bits_for_check[bits_for_check.size() - 1] == (bits[bits_for_check.size() - 1] == '0'))
    std::cerr<< "Arc was broken!!!!!" << std::endl;

  std::vector<bool> result;
  for (int i = 0; i < bits_for_check.size() - 1; ++i) {
    if (i + 1 & i) {
      result.push_back(bits_for_check[i]);
    }
  }
  return result;
}
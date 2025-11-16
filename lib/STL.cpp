#include "utils/ADT/SmallVector.hpp"
#include "utils/ADT/StringRef.hpp"
#include "utils/lisp/lisp.hpp"
#include "utils/misc.hpp"
#include <cstdint>
#include <cstdio>
#include <print>
#include <string>

template <typename T, std::size_t N>
using SmallVector = utils::ADT::SmallVector<T, N>;

int main() {
  SmallVector<uint64_t, 4> intVec{114, 514, 1919, 810};

  intVec.push_back(114514);

  SmallVector<std::string, 4> strVec{};
  strVec.emplace_back("114");
  strVec.emplace_back("114");
  strVec.emplace_back("114");
  strVec.emplace_back("114");
  strVec.emplace_back("114514");

  return 0;
}
#include "utils/ADT/SmallVector.hpp"
#include "utils/ADT/StringRef.hpp"
#include "utils/lisp/lisp.hpp"
#include "utils/misc.hpp"
#include <cstdio>
#include <print>

template <typename T, std::size_t N>
using SmallVector = utils::ADT::SmallVector<T, N>;

int main() {
  utils::ADT::StringRef str("   ( auipc x1 %hi ) ( jalr x1 x1 %lo )  ");

  utils::lisp::Lisp lisp(str);

  auto rootNode = lisp.getRoot();

  for (uint32_t sonCnt = 0; sonCnt < rootNode->sonNr; ++sonCnt) {
    auto& son = rootNode->sons[sonCnt];

    /// @note enum the items that may appear in .def file
    for (auto operand : son->sons) {
      if (!operand) {
        continue;
      }

      StringSwitch<bool>(operand->content)
          .Case("rd",
                [](auto&& _) {
                  std::printf("rd ");
                  return true;
                })
          .Case("rt",
                [](auto&& _) {
                  std::printf("rt ");
                  return true;
                })
          .Case("rs",
                [](auto&& _) {
                  std::printf("rs ");
                  return true;
                })
          .Case("%hi",
                [](auto&& _) {
                  std::printf("%%hi ");
                  return true;
                })
          .Case("%lo",
                [](auto&& _) {
                  std::printf("%%lo ");
                  return true;
                })
          .Case("offset",
                [](auto&& _) {
                  std::printf("off ");
                  return true;
                })
          .Case("0",
                [](auto&& _) {
                  std::printf("0 ");
                  return true;
                })
          .Case("1",
                [](auto&& _) {
                  std::printf("1 ");
                  return true;
                })
          .Case("-1",
                [](auto&& _) {
                  std::printf("-1 ");
                  return true;
                })
          .Case("x0",
                [](auto&& _) {
                  std::printf("x0 ");
                  return true;
                })
          .Case("x1",
                [](auto&& _) {
                  std::printf("x1 ");
                  return true;
                })
          .Case("x6",
                [](auto&& _) {
                  std::printf("x6 ");
                  return true;
                })
          .Default([](auto&& _) {
            std::printf("%s ", _.str().c_str());
            return true;
          });
    }
  }

  return 0;
}
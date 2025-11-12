#ifndef MC_PSEUDO
#define MC_PSEUDO

#include "MCExpr.hpp"
#include "MCInst.hpp"
#include "MCOperand.hpp"
#include "mc/MCContext.hpp"
#include "mc/MCOpCode.hpp"
#include "utils/ADT/SmallVector.hpp"
#include "utils/ADT/StringMap.hpp"
#include "utils/lisp/lisp.hpp"
#include <array>
#include <cstdint>

namespace {
template <size_t N>
constexpr std::array<char, N> processPseudo(const char (&pseudo)[N]) {
  std::array<char, N> result{};
  for (size_t i = 0; i < N; ++i) {
    char c = pseudo[i];
    if (c == '_') {
      result[i] = '.';
    } else {
      result[i] = toLower(c);
    }
  }
  return result;
}
} // namespace

namespace mc {
using MCInstPtrs = utils::ADT::SmallVector<MCInst*, 4>;
template <typename T> using StringMap = utils::ADT::StringMap<T>;
using Lisp = utils::lisp::Lisp;
using LispNode = utils::lisp::LispNode;

template <const char*... Ops> struct Pseudo {
public:
  enum OperandKind : uint8_t { Rd, Rt, Rs, Symbol, Offset, Imme };

private:
  struct InstPattern {
    MCOpCode* Op;
    std::array<OperandKind, 4> Operands;
    uint32_t opNr;

    constexpr uint32_t push(OperandKind op) {
      Operands[opNr] = op;
      return ++opNr;
    }
  };

  std::array<InstPattern, 4> InstPatterns;
  uint32_t InstNr;

  bool rd = false, rt = false, rs = false, symbol = false, offset = false,
       imme = false;

  constexpr void parseImpl(LispNode* rootNode) {
    /// @note notice that asm inst wont ref to each other cause they all have
    /// side effets

    for (auto son : rootNode->sons) {
      InstPattern inst{};
      inst.Op =
          const_cast<MCOpCode*>(parser::MnemonicFind(son->content.data()));

      for (auto operand : son->sons) {
        auto op = StringSwitch<OperandKind>(operand->content)
                      .Case("rd",
                            [this](auto&& _) {
                              rd = true;
                              return Rd;
                            })
                      .Case("rt",
                            [this](auto&& _) {
                              rt = true;
                              return Rt;
                            })
                      .Case("rs",
                            [this](auto&& _) {
                              rs = true;
                              return Rs;
                            })
                      .Case("symbol",
                            [this](auto&& _) {
                              symbol = true;
                              return Symbol;
                            })
                      .Case("offset",
                            [this](auto&& _) {
                              offset = true;
                              return Offset;
                            })
                      .Case("imm",
                            [this](auto&& _) {
                              imme = true;
                              return Imme;
                            })
                      .Error();

        inst.push(op);
      }
      InstPatterns[InstNr++] = inst;
    }
  }

public:
  StringRef name;

  constexpr Pseudo(const char* PseudoName, const char* LispPattern)
      : name(PseudoName) {
    Lisp lisp(LispPattern);
    auto rootNode = lisp.getRoot();
    parseImpl(rootNode);
  }

  template <OperandKind Kind, typename Ty>
  void addOperand(MCContext& ctx, MCInstPtrs& Insts, Ty operand) {
    for (uint32_t instCnt = 0; instCnt < InstNr; ++instCnt) {
      const auto& InstPattern = InstPatterns[instCnt];
      for (uint32_t opCnt = 0; opCnt < InstPattern.opNr; ++opCnt) {

        if (InstPattern.Operands[opCnt] != Kind) {
          continue;
        }
        MCOperand newOp = MCOperand::make(operand);
        auto inst = Insts[instCnt];

        inst->addOperand(std::move(newOp));

        /// record symbols
        if (Kind == Symbol) {
          ctx.addReloInst(inst, operand);
        }
      }
    }
  }

  constexpr auto operator()(MCContext& ctx) {

    using Types = decltype(std::make_tuple(Rd, Rt, Rs, Symbol, Offset, Imme));

    constexpr std::array<bool, std::tuple_size_v<Types>> flags = {
        rd, rt, rs, symbol, offset, imme};

    constexpr auto instsBuild = [&ctx, this](auto ArgsTuple) {
      MCInstPtrs insts = ctx.newTextInsts(parser::MnemonicFind(Ops)...);

      [&]<std::size_t... I>(std::index_sequence<I...>) {
        ((flags[I] ? addOperand<std::tuple_element_t<I, Types>>(
                         ctx, insts, std::get<I>(ArgsTuple))
                   : void()),
         ...);
      }(std::make_index_sequence<std::tuple_size_v<Types>>{});

      ctx.commitTextInsts(insts);
    };

    constexpr auto argNormalize = [this](auto... args) {
      auto rawTuple = std::make_tuple(args...);
      constexpr std::size_t argNr = std::tuple_size_v<decltype(rawTuple)>;

      auto ArgTuple = [&]<std::size_t... I>(std::index_sequence<I...>) {
        auto getElem = [&]<std::size_t Idx>() {
          constexpr std::size_t argIdx = [this]() {
            std::size_t count = 0;
            for (std::size_t k = 0; k < Idx; ++k) {
              if (flags[k]) {
                count++;
              }
            }
            return count;
          }();

          if constexpr (flags[Idx] && argIdx < argNr) {
            return std::tuple{std::get<argIdx>(rawTuple)};
          } else {
            return std::tuple{};
          }
        };

        return std::tuple_cat(getElem.template operator()<I>()...);
      }(std::make_index_sequence<std::tuple_size_v<Types>>{});

      return ArgTuple;
    };

    /// return as a callback
    return [this](MCContext& ctx, auto... args) {
      instsBuild(argNormalize(args...));
    };
  }
};

#define PSEUDO_DEF(name, pattern)                                              \
  inline constexpr char _##name[] = #name;                                     \
  inline constexpr char _##name##_Pattern[] = #pattern;                        \
  static constexpr Pseudo name{_##name, _##name##_Pattern};

#define PSEUDO_TLB(name)

/// TODO: more

} // namespace mc

#endif
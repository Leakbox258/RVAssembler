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
#include "utils/misc.hpp"
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
  /// notice that Imme here is ref to defined imme
  /// eg: nop = addi x0, x0, `0`
  enum OperandKind : int8_t {
    Rd,
    Rt,
    Rs,
    Symbol,
    Offset,
    ImmeNeg1,
    Imme0,
    Imme1,
  };

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
       imme0 = false, imme1 = false, immeNeg1 = false;

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
                      .Case("0",
                            [this](auto&& _) {
                              imme0 = true;
                              return Imme0;
                            })
                      .Case("1",
                            [this](auto&& _) {
                              imme1 = true;
                              return Imme1;
                            })
                      .Case("-1",
                            [this](auto&& _) {
                              immeNeg1 = true;
                              return ImmeNeg1;
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

  template <std::size_t I, OperandKind Kind, typename Ty>
  void addOperand(MCContext& ctx, MCInstPtrs& Insts, auto ArgsTuple) {

    if constexpr (utils::in_interval<true, true>(Imme0, ImmeNeg1, Kind)) {
      addOperandImpl(ctx, Insts);
    } else {
      addOperandImpl(ctx, Insts, std::get<I>(ArgsTuple));
    }
  }

  template <OperandKind Kind, typename Ty>
  void addOperandImpl(MCContext& ctx, MCInstPtrs& Insts, Ty operand) {
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

  /// this implement is for `-1` `0` `1`
  template <OperandKind Kind, typename Ty>
  void addOperandImpl(MCContext& ctx, MCInstPtrs& Insts) {

    for (uint32_t instCnt = 0; instCnt < InstNr; ++instCnt) {
      const auto& InstPattern = InstPatterns[instCnt];
      for (uint32_t opCnt = 0; opCnt < InstPattern.opNr; ++opCnt) {

        if (InstPattern.Operands[opCnt] != Kind) {
          continue;
        }

        MCOperand newOp = MCOperand::make((int64_t)Kind - ImmeNeg1 - 1);

        auto inst = Insts[instCnt];

        inst->addOperand(std::move(newOp));
      }
    }
  }

  constexpr auto operator()(MCContext& ctx) {

    /// input args
    constexpr auto argTbl =
        std::make_tuple(Rd, Rt, Rs, Symbol, Offset, ImmeNeg1, Imme0, Imme1);
    using Types = decltype(argTbl);

    constexpr std::array<bool, std::tuple_size_v<Types>> flags = {
        rd, rt, rs, symbol, offset, immeNeg1, imme0, imme1};

    constexpr auto instsBuild = [&ctx, this](auto ArgsTuple) {
      MCInstPtrs insts = ctx.newTextInsts(parser::MnemonicFind(Ops)...);

      [&]<std::size_t... I>(std::index_sequence<I...>) {
        ((flags[I] ? addOperand<std::get<I>(argTbl)>(ctx, insts, ArgsTuple)
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
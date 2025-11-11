#ifndef MC_PSEUDO
#define MC_PSEUDO

#include "MCExpr.hpp"
#include "MCInst.hpp"
#include "MCOperand.hpp"
#include "mc/MCOpCode.hpp"
#include "utils/ADT/SmallVector.hpp"
#include "utils/ADT/StringMap.hpp"
#include "utils/lisp/lisp.hpp"
#include <array>
#include <cstdint>

namespace mc {
using MCInsts = utils::ADT::SmallVector<MCInst, 2>;
template <typename T> using StringMap = utils::ADT::StringMap<T>;
using Lisp = utils::lisp::Lisp;
using LispNode = utils::lisp::LispNode;

struct Pseudo {
private:
  struct InnerInst {
    MCOpCode* Op;
    enum OperandKind { Rd, Rt, Rs, Symbol, Offset, Imme };
    std::array<OperandKind, 4> Operands;
    uint32_t opNr;

    constexpr uint32_t push(OperandKind op) {
      Operands[opNr] = op;
      return ++opNr;
    }
  };

  void parseImpl(LispNode* rootNode) {
    /// @note notice that asm inst wont ref to each other cause they all have
    /// side effets
    for (auto son : rootNode->sons) {
      InnerInst inst{};
      inst.Op =
          const_cast<MCOpCode*>(parser::MnemonicFind(son->content.data()));

      for (auto operand : son->sons) {
        auto op = StringSwitch<InnerInst::OperandKind>(operand->content)
                      .Case("rd", InnerInst::Rd)
                      .Case("rd", InnerInst::Rt)
                      .Case("rd", InnerInst::Rs)
                      .Case("symbol", InnerInst::Symbol)
                      .Case("offset", InnerInst::Offset)
                      .Case("imm", InnerInst::Imme)
                      .Error();

        inst.push(op);
      }
    }
  }

public:
  StringRef name;
  std::array<InnerInst, 4> Insts;
  uint32_t InstNr;

  constexpr Pseudo(const char* PseudoName, const char* LispPattern)
      : name(PseudoName) {
    Lisp lisp(LispPattern);
    auto rootNode = lisp.getRoot();
    parseImpl(rootNode);
  }
};

/// used as call-back
MCInsts ld_rd_symbol(MCReg rd, StringRef symbol);
MCInsts lda_rd_symbol(MCReg rd, StringRef symbol);
MCInsts lla_rd_symbol(MCReg rd, StringRef symbol);
MCInsts lga_rd_symbol(MCReg rd, StringRef symbol);
MCInsts lb_rd_symbol(MCReg rd, StringRef symbol);
MCInsts lh_rd_symbol(MCReg rd, StringRef symbol);
MCInsts lw_rd_symbol(MCReg rd, StringRef symbol);
MCInsts ld_rd_symbol(MCReg rd, StringRef symbol);
MCInsts lbu_rd_symbol(MCReg rd, StringRef symbol);
MCInsts lhu_rd_symbol(MCReg rd, StringRef symbol);
MCInsts lwu_rd_symbol(MCReg rd, StringRef symbol);

MCInsts sb_rd_rt_symbol(MCReg rd, MCReg rt, StringRef symbol);
MCInsts sh_rd_rt_symbol(MCReg rd, MCReg rt, StringRef symbol);
MCInsts sw_rd_rt_symbol(MCReg rd, MCReg rt, StringRef symbol);
MCInsts sd_rd_rt_symbol(MCReg rd, MCReg rt, StringRef symbol);

MCInsts flw_rd_rt_symbol(MCReg rd, MCReg rt, StringRef symbol);
MCInsts fld_rd_rt_symbol(MCReg rd, MCReg rt, StringRef symbol);

MCInsts fsw_rd_rt_symbol(MCReg rd, MCReg rt, StringRef symbol);
MCInsts fsd_rd_rt_symbol(MCReg rd, MCReg rt, StringRef symbol);

MCInsts nop();

MCInsts li_rd_imm(MCReg rd, uint64_t immediate);

MCInsts sext_b_rd_rs(MCReg rd, MCReg rs);
MCInsts sext_h_rd_rs(MCReg rd, MCReg rs);
MCInsts sext_w_rd_rs(MCReg rd, MCReg rs);
MCInsts zext_b_rd_rs(MCReg rd, MCReg rs);
MCInsts zext_h_rd_rs(MCReg rd, MCReg rs);
MCInsts zext_w_rd_rs(MCReg rd, MCReg rs);

MCInsts call_symbol(StringRef symbol); // call_offset
MCInsts tail_symbol(StringRef symbol); // tail_offset

/// TODO: more

} // namespace mc

#endif
#ifndef PARSER_PARSER
#define PARSER_PARSER

#include "Lexer.hpp"
#include "mc/MCContext.hpp"
#include "mc/MCInst.hpp"
#include "mc/MCOpCode.hpp"
#include "utils/ADT/StringMap.hpp"
#include "utils/macro.hpp"
#include <cstdint>

template <typename T> using StringSwitch = utils::ADT::StringSwitch<T>;
template <typename T, std::size_t N>
using SmallVector = utils::ADT::SmallVector<T, N>;
using mc::MCContext;
using mc::MCInst;

namespace parser {

template <typename V> using StringMap = utils::ADT::StringMap<V>;

class Parser {
private:
  mc::MCContext& ctx;
  Lexer& lexer;

  /// speed up mnemnoic class find
  StringMap<const mc::MCOpCode*> CacheLookUpTab;

public:
  Parser(mc::MCContext& _ctx LIFETIME_BOUND, Lexer& _lexer LIFETIME_BOUND)
      : ctx(_ctx), lexer(_lexer) {}

  void parse();

  ~Parser() = default;

private:
  Token token;
  MCInst* curInst = nullptr;
  SmallVector<std::string, 4> DirectiveStack{};
  MCContext::size_ty curOffset = 0;

  void advance();
  uint8_t RegHelper(const StringRef& reg);
  void JrBrHelper(const StringRef& label);

  /// FSM
  void ParseNewLine();
  void ParseComma();
  void ParseLParen();
  void ParseRParen();
  void ParseColon();
  void ParseIdentifier();
  void ParseInteger();
  void ParseHexInteger();
  void ParseFloat();
  void ParseModifier();
  void ParseInstruction();
  void ParseRegister();
  void ParseDirective();
  void ParseLabelDef();
  void ParseString();

  /// this method assume mnemoic is valid
  const mc::MCOpCode* findOpCode(StringRef mnemonic);
};

} // namespace parser

#endif
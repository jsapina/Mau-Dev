/*

    This file is part of the Maude 2 interpreter.

    Copyright 1997-2014 SRI International, Menlo Park, CA 94025, USA.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

*/

//
//	Term* -> ostream& pretty printer.
//

const char*
MixfixModule::computeColor(SymbolType st)
{
  if (interpreter.getPrintFlag(Interpreter::PRINT_COLOR))
    {
      if (st.hasFlag(SymbolType::ASSOC))
	{
	  if (st.hasFlag(SymbolType::COMM))
	    {
	      return Tty(st.hasFlag(SymbolType::LEFT_ID | SymbolType::RIGHT_ID) ?
			 Tty::MAGENTA : Tty::RED).ctrlSequence();
	    }
	  else
	    {
	      return Tty(st.hasFlag(SymbolType::LEFT_ID | SymbolType::RIGHT_ID) ?
			 Tty::CYAN : Tty::GREEN).ctrlSequence();
	    }
	}
      if (st.hasFlag(SymbolType::COMM))
	return Tty(Tty::BLUE).ctrlSequence();
      if (st.hasFlag(SymbolType::LEFT_ID | SymbolType::RIGHT_ID | SymbolType::IDEM))
	return Tty(Tty::YELLOW).ctrlSequence();
    }
  return 0;
}

void
MixfixModule::suffix(ostream& s,
		     Term* term,
		     bool needDisambig,
		     const char* /* color */)
{
  if (needDisambig)
    s << ")." << disambiguatorSort(term);
}

Sort*
MixfixModule::disambiguatorSort(const Term* term)
{
  Symbol* symbol = term->symbol();
  int sortIndex = term->getSortIndex();
  if (sortIndex <= Sort::ERROR_SORT)
    sortIndex = chooseDisambiguator(symbol);
  return symbol->rangeComponent()->sort(sortIndex);
}

bool
MixfixModule::handleIter(ostream& s,
			 Term* term,
			 SymbolInfo& si,
			 bool rangeKnown,
			 const char* color)
{
  if (!(si.symbolType.hasFlag(SymbolType::ITER)))
    return false;
  if (si.symbolType.getBasicType() == SymbolType::SUCC_SYMBOL &&
      interpreter.getPrintFlag(Interpreter::PRINT_NUMBER))
    {
      SuccSymbol* succSymbol = safeCast(SuccSymbol*, term->symbol());
      if (succSymbol->isNat(term))
	{
	  const mpz_class& nat = succSymbol->getNat(term);
	  bool needDisambig = !rangeKnown &&
	    (kindsWithSucc.size() > 1 || overloadedIntegers.count(nat));
	  prefix(s, needDisambig, color);
	  s << succSymbol->getNat(term);
	  suffix(s, term, needDisambig, color);
	  return true;
	}
    }
  S_Term* st = safeCast(S_Term*, term);
  const mpz_class& number = st->getNumber();
  if (number == 1)
    return false;  // do default thing

  // NEED TO FIX: disambig
  string prefixName;
  makeIterName(prefixName, term->symbol()->id(), number);
  if (color != 0)
    s << color << prefixName << Tty(Tty::RESET);
  else
    printPrefixName(s, prefixName.c_str(), si);
  s << '(';
  prettyPrint(s, st->getArgument(),
	      PREFIX_GATHER, UNBOUNDED, 0, UNBOUNDED, 0, rangeKnown);
  s << ')';
  return true;
}

bool
MixfixModule::handleMinus(ostream& s,
			  Term* term,
			  bool rangeKnown,
			  const char* color)
{
  if (interpreter.getPrintFlag(Interpreter::PRINT_NUMBER))
    {
      const MinusSymbol* minusSymbol = safeCast(MinusSymbol*, term->symbol());
      if (minusSymbol->isNeg(term))
	{
	  mpz_class neg;
	  (void) minusSymbol->getNeg(term, neg);
	  bool needDisambig = !rangeKnown &&
	    (kindsWithMinus.size() > 1 || overloadedIntegers.count(neg));
	  prefix(s, needDisambig, color);
	  s << neg;
	  suffix(s, term, needDisambig, color);
	  return true;
	}
    }
  return false;
}

bool
MixfixModule::handleDivision(ostream& s,
			     Term* term,
			     bool rangeKnown,
			     const char* color)
{
  if (interpreter.getPrintFlag(Interpreter::PRINT_RAT))
    {
      const DivisionSymbol* divisionSymbol = safeCast(DivisionSymbol*, term->symbol());
      if (divisionSymbol->isRat(term))
	{
	  pair<mpz_class, mpz_class> rat;
	  rat.second = divisionSymbol->getRat(term, rat.first);
	  bool needDisambig = !rangeKnown &&
	    (kindsWithDivision.size() > 1 || overloadedRationals.count(rat));
	  prefix(s, needDisambig, color);
	  s << rat.first << '/' << rat.second;
	  suffix(s, term, needDisambig, color);
	  return true;
	}
    }
  return false;
}

void
MixfixModule::handleFloat(ostream& s,
			  Term* term,
			  bool rangeKnown,
			  const char* color)
{
  double mfValue = safeCast(FloatTerm*, term)->getValue();
  bool needDisambig = !rangeKnown &&
    (floatSymbols.size() > 1 || overloadedFloats.count(mfValue));
  prefix(s, needDisambig, color);
  s << doubleToString(mfValue);
  suffix(s, term, needDisambig, color);
}

void
MixfixModule::handleString(ostream& s,
			   Term* term,
			   bool rangeKnown,
			   const char* color)
{
  string strValue;
  Token::ropeToString(safeCast(StringTerm*, term)->getValue(), strValue);
  bool needDisambig = !rangeKnown &&
    (stringSymbols.size() > 1 || overloadedStrings.count(strValue));
  prefix(s, needDisambig, color);
  s << strValue;
  suffix(s, term, needDisambig, color);
}

void
MixfixModule::handleQuotedIdentifier(ostream& s,
				     Term* term,
				     bool rangeKnown,
				     const char* color)
{
  int qidCode = safeCast(QuotedIdentifierTerm*, term)->getIdIndex();
  bool needDisambig = !rangeKnown &&
    (quotedIdentifierSymbols.size() > 1 ||
     overloadedQuotedIdentifiers.count(qidCode));
  prefix(s, needDisambig, color);
  s << '\'' << Token::name(qidCode);
  suffix(s, term, needDisambig, color);
}

void
MixfixModule::handleVariable(ostream& s,
			     Term* term,
			     bool rangeKnown,
			     const char* color)
{
  VariableTerm* v = safeCast(VariableTerm*, term);
  Sort* sort = safeCast(VariableSymbol*, term->symbol())->getSort();
  pair<int, int> p(v->id(), sort->id());
  bool needDisambig = !rangeKnown && overloadedVariables.count(p);  // kinds not handled
  prefix(s, needDisambig, color);
  printVariable(s, p.first, sort);
  suffix(s, term, needDisambig, color);
}

void
MixfixModule::handleSMT_Number(ostream& s,
			       Term* term,
			       bool rangeKnown,
			       const char* color)
{
  //
  //	Get value.
  //
  SMT_NumberTerm* n = safeCast(SMT_NumberTerm*, term);
  const mpq_class& value = n->getValue();
  //
  //	Look up the index of our sort.
  //
  Symbol* symbol = term->symbol();
  Sort* sort = symbol->getRangeSort();
  //
  //	Figure out what SMT sort we correspond to.
  //
  SMT_Info::SMT_Type t = getSMT_Info().getType(sort);
  Assert(t != SMT_Info::NOT_SMT, "bad SMT sort " << sort);
  if (t == SMT_Info::INTEGER)
    {
      const mpz_class& integer = value.get_num();
      bool needDisambig = !rangeKnown &&
	(kindsWithSucc.size() > 1 || overloadedIntegers.count(integer));
      prefix(s, needDisambig, color);
      s << integer;
      suffix(s, term, needDisambig, color);
    }
  else
    {
      Assert(t == SMT_Info::REAL, "SMT number sort expected");
      pair<mpz_class, mpz_class> rat(value.get_num(), value.get_den());
      bool needDisambig = !rangeKnown &&
	(kindsWithDivision.size() > 1 || overloadedRationals.count(rat));
      prefix(s, needDisambig, color);
      s << rat.first << '/' << rat.second;
      suffix(s, term, needDisambig, color);
    }
}

void
MixfixModule::prettyPrint(ostream& s,
			  Term* term,
			  int requiredPrec,
			  int leftCapture,
			  const ConnectedComponent* leftCaptureComponent,
			  int rightCapture,
			  const ConnectedComponent* rightCaptureComponent,
			  bool rangeKnown)
{
  if (UserLevelRewritingContext::interrupted())
    return;

  Symbol* symbol = term->symbol();
  SymbolInfo& si = symbolInfo[symbol->getIndexWithinModule()];
  const char* color = computeColor(si.symbolType);
  //
  //	Check for special i/o representation.
  //
  if (handleIter(s, term, si, rangeKnown, color))
    return;
  int basicType = si.symbolType.getBasicType();
  switch (basicType)
    {
    case SymbolType::MINUS_SYMBOL:
      {
	if (handleMinus(s, term, rangeKnown, color))
	  return;
	break;
      }
    case SymbolType::DIVISION_SYMBOL:
      {
	if (handleDivision(s, term, rangeKnown, color))
	  return;
	break;
      }
    case SymbolType::FLOAT:
      {
	handleFloat(s, term, rangeKnown, color);
	return;
      }
    case SymbolType::STRING:
      {
	handleString(s, term, rangeKnown, color);
	return;
      }
    case SymbolType::QUOTED_IDENTIFIER:
      {
	handleQuotedIdentifier(s, term, rangeKnown, color);
	return;
      }
    case SymbolType::VARIABLE:
      {
	handleVariable(s, term, rangeKnown, color);
	return;
      }
    case SymbolType::SMT_NUMBER_SYMBOL:
      {
	handleSMT_Number(s, term, rangeKnown, color);
	return;
      }
    default:
      break;
    }
  //
  //	Default case where no special i/o representation applies.
  //
  int iflags = si.iflags;
  bool needDisambig = !rangeKnown && ambiguous(iflags);
  bool argRangeKnown = rangeOfArgumentsKnown(iflags, rangeKnown, needDisambig);
  int nrArgs = symbol->arity();
  if (needDisambig)
    s << '(';
  if ((interpreter.getPrintFlag(Interpreter::PRINT_MIXFIX) && si.mixfixSyntax.length() != 0) ||
      basicType == SymbolType::SORT_TEST)
    {
      //
      //	Mixfix case.
      //
      bool printWithParens = interpreter.getPrintFlag(Interpreter::PRINT_WITH_PARENS);
      bool needParen = !needDisambig &&
	(printWithParens || requiredPrec < si.prec ||
	 ((iflags & LEFT_BARE) && leftCapture <= si.gather[0] &&
	  leftCaptureComponent == symbol->domainComponent(0)) ||
	 ((iflags & RIGHT_BARE) && rightCapture <= si.gather[nrArgs - 1] &&
	  rightCaptureComponent == symbol->domainComponent(nrArgs - 1)));
      bool needAssocParen = si.symbolType.hasFlag(SymbolType::ASSOC) &&
	(printWithParens || si.gather[1] < si.prec ||
	 ((iflags & LEFT_BARE) && (iflags & RIGHT_BARE) &&
	  si.prec <= si.gather[0]));
      if (needParen)
	s << '(';
      int nrTails = 1;
      int pos = 0;
      ArgumentIterator a(*term);
      int moreArgs = a.valid();
      for (int arg = 0; moreArgs; arg++)
	{
	  Term* t = a.argument();
	  a.next();
	  moreArgs = a.valid();
	  pos = printTokens(s, si, pos, color);
	  if (arg == nrArgs - 1 && moreArgs)
	    {
	      ++nrTails;
	      arg = 0;
	      if (needAssocParen)
		s << '(';
	      pos = printTokens(s, si, 0, color);
	    }
	  int lc = UNBOUNDED;
	  const ConnectedComponent* lcc = 0;
	  int rc = UNBOUNDED;
	  const ConnectedComponent* rcc = 0;
	  if (arg == 0 && (iflags & LEFT_BARE))
	    {
	      rc = si.prec;
	      rcc = symbol->domainComponent(0);
	      if (!needParen && !needDisambig)
		{
		  lc = leftCapture;
		  lcc = leftCaptureComponent;
		}
	    }
	  else if (!moreArgs && (iflags & RIGHT_BARE))
	    {
	      lc = si.prec;
	      lcc = symbol->domainComponent(nrArgs - 1);
	      if (!needParen && !needDisambig)
		{
		  rc = rightCapture;
		  rcc = rightCaptureComponent;
		}
	    }
	  prettyPrint(s, t, si.gather[arg], lc, lcc, rc, rcc, argRangeKnown);
	  if (UserLevelRewritingContext::interrupted())
	    return;
	}
      printTails(s, si, pos, nrTails, needAssocParen, true, color);
      if (UserLevelRewritingContext::interrupted())
	return;
      if (needParen)
	s << ')';
    }
  else
    {
      //
      //	Prefix case.
      //
      const char* prefixName = Token::name(symbol->id());
      if (color != 0)
	s << color << prefixName << Tty(Tty::RESET);
      else
	printPrefixName(s, prefixName, si);
      ArgumentIterator a(*term);
      if (a.valid())
	{
	  int nrTails = 1;
	  s << '(';
	  for (int arg = 0;; arg++)
	    {
	      Term* t = a.argument();
	      a.next();
	      int moreArgs = a.valid();
	      if (arg >= nrArgs - 1 &&
		  !(interpreter.getPrintFlag(Interpreter::PRINT_FLAT)) &&
		  moreArgs)
		{
		  ++nrTails;
		  printPrefixName(s, prefixName, si);
		  s << '(';
		}
	      prettyPrint(s, t,
			  PREFIX_GATHER, UNBOUNDED, 0, UNBOUNDED, 0,
			  argRangeKnown);
	      if (UserLevelRewritingContext::interrupted())
		return;
	      if (!moreArgs)
		break;
	      s << ", ";
	    }
	  while (nrTails-- > 0)
	    {
	      if (UserLevelRewritingContext::interrupted())
		return;
	      s << ')';
	    }
	}
    }
  suffix(s, term, needDisambig, color);
}

/*** BEGIN MAU-DEV ***/
string MixfixModule::getPos(Vector<int>& pos)
{
	if (pos.length() == 0)
		return "";
	stringstream ss;
	copy(pos.begin(), pos.end(), ostream_iterator<int>(ss, "."));
	string str = ss.str();
	str = str.substr(0, str.length()-1);
	return str;
}

void MixfixModule::mapSuffix(Vector<int>& position, ostream& s, Term* term, bool needDisambig, const char* /* color */)
{
	//cout << "\nSUFFIX: " << getPos(position);
	if (needDisambig) {
		stringstream ss;
		ss << disambiguatorSort(term);
		string str = ss.str();
		//s << ")." << disambiguatorSort(term);
        s << "c" << (str.length() + 2) << "p" << getPos(position);
    }
}

bool MixfixModule::mapHandleIter(Vector<int>& position, ostream& s, Term* term, SymbolInfo& si, bool rangeKnown, const char* color)
{
	if (!(si.symbolType.hasFlag(SymbolType::ITER)))
		return false;
	if (si.symbolType.getBasicType() == SymbolType::SUCC_SYMBOL && interpreter.getPrintFlag(Interpreter::PRINT_NUMBER))
    {
		SuccSymbol* succSymbol = safeCast(SuccSymbol*, term->symbol());
		if (succSymbol->isNat(term))
		{
			const mpz_class& nat = succSymbol->getNat(term);
			bool needDisambig = !rangeKnown && (kindsWithSucc.size() > 1 || overloadedIntegers.count(nat));
			mapPrefix(position, s, needDisambig, color);
			//s << succSymbol->getNat(term);
			stringstream ss;
			ss << succSymbol->getNat(term);
			string str = ss.str();
			string mytail = getPos(position).length() > 0?".1":"1";
			s << "c" << str.length() << "p" << getPos(position) << "X" << getPos(position) << mytail;
			mapSuffix(position, s, term, needDisambig, color);
			return true;
		}
	}
	S_Term* st = safeCast(S_Term*, term);
	const mpz_class& number = st->getNumber();
	if (number == 1)
		return false;  // do default thing

	// NEED TO FIX: disambig
	string prefixName;
	makeIterName(prefixName, term->symbol()->id(), number);
	//if (color != 0)
	//	s << color << prefixName << Tty(Tty::RESET);
	//else
		mapPrintPrefixName(position, s, prefixName.c_str(), si);
	//s << '(';
	s << "c1p" << getPos(position);
	mapPrettyPrint(position, s, st->getArgument(),PREFIX_GATHER, UNBOUNDED, 0, UNBOUNDED, 0, rangeKnown);
	//s << ')';
	s << "c1p" << getPos(position);
	return true;
}

bool MixfixModule::mapHandleMinus(Vector<int>& position, ostream& s, Term* term, bool rangeKnown, const char* color)
{
	if (interpreter.getPrintFlag(Interpreter::PRINT_NUMBER))
    {
		const MinusSymbol* minusSymbol = safeCast(MinusSymbol*, term->symbol());
		if (minusSymbol->isNeg(term))
		{
			mpz_class neg;
			(void) minusSymbol->getNeg(term, neg);
			bool needDisambig = !rangeKnown && (kindsWithMinus.size() > 1 || overloadedIntegers.count(neg));
			mapPrefix(position, s, needDisambig, color);
			//s << neg;
			stringstream ss;
			ss << neg;
			string str = ss.str();
			string mytail = getPos(position).length() > 0?".1":"1";
			s << "c1p" << getPos(position) << "c" << str.length()-1 << "p" << getPos(position) << mytail << "X" << getPos(position) << mytail << ".1";
			mapSuffix(position, s, term, needDisambig, color);
			return true;
		}
	}
	return false;
}

bool MixfixModule::mapHandleDivision(Vector<int>& position, ostream& s, Term* term, bool rangeKnown, const char* color)
{
	if (interpreter.getPrintFlag(Interpreter::PRINT_RAT))
    {
		const DivisionSymbol* divisionSymbol = safeCast(DivisionSymbol*, term->symbol());
		if (divisionSymbol->isRat(term))
		{
			pair<mpz_class, mpz_class> rat;
			rat.second = divisionSymbol->getRat(term, rat.first);
			bool needDisambig = !rangeKnown && (kindsWithDivision.size() > 1 || overloadedRationals.count(rat));
			mapPrefix(position, s, needDisambig, color);
			//s << rat.first << '/' << rat.second;
			s << rat.first << '/' << rat.second; //TODO: Map?
			mapSuffix(position, s, term, needDisambig, color);
			return true;
		}
	}
	return false;
}

void MixfixModule::mapHandleFloat(Vector<int>& position, ostream& s, Term* term, bool rangeKnown, const char* color)
{
	double mfValue = safeCast(FloatTerm*, term)->getValue();
	bool needDisambig = !rangeKnown && (floatSymbols.size() > 1 || overloadedFloats.count(mfValue));
	mapPrefix(position, s, needDisambig, color);
	//s << doubleToString(mfValue);
	s << "c" << strlen(doubleToString(mfValue)) << "p" << getPos(position);
	mapSuffix(position, s, term, needDisambig, color);
}

void MixfixModule::mapHandleString(Vector<int>& position, ostream& s, Term* term, bool rangeKnown, const char* color)
{
	string strValue;
	Token::ropeToString(safeCast(StringTerm*, term)->getValue(), strValue);
	bool needDisambig = !rangeKnown && (stringSymbols.size() > 1 || overloadedStrings.count(strValue));
	mapPrefix(position, s, needDisambig, color);
	//s << strValue;
	s << "c" << strValue.length() << "p" << getPos(position);
	mapSuffix(position, s, term, needDisambig, color);
}

void MixfixModule::mapHandleQuotedIdentifier(Vector<int>& position, ostream& s, Term* term, bool rangeKnown, const char* color)
{
	int qidCode = safeCast(QuotedIdentifierTerm*, term)->getIdIndex();
	bool needDisambig = !rangeKnown && (quotedIdentifierSymbols.size() > 1 || overloadedQuotedIdentifiers.count(qidCode));
	mapPrefix(position, s, needDisambig, color);
	//s << '\'' << Token::name(qidCode);
	s << "c" << strlen(Token::name(qidCode))+1 << "p" << getPos(position);
	mapSuffix(position, s, term, needDisambig, color);
}

void MixfixModule::mapHandleVariable(Vector<int>& position, ostream& s, Term* term, bool rangeKnown, const char* color)
{
	VariableTerm* v = safeCast(VariableTerm*, term);
	Sort* sort = safeCast(VariableSymbol*, term->symbol())->getSort();
	pair<int, int> p(v->id(), sort->id());
	bool needDisambig = !rangeKnown && overloadedVariables.count(p);  // kinds not handled
	mapPrefix(position, s, needDisambig, color);
	mapPrintVariable(position, s, p.first, sort);
	mapSuffix(position, s, term, needDisambig, color);
}

void MixfixModule::mapHandleSMT_Number(Vector<int>& position, ostream& s, Term* term, bool rangeKnown, const char* color) // TODO: Map
{
  //
  //	Get value.
  //
  SMT_NumberTerm* n = safeCast(SMT_NumberTerm*, term);
  const mpq_class& value = n->getValue();
  //
  //	Look up the index of our sort.
  //
  Symbol* symbol = term->symbol();
  Sort* sort = symbol->getRangeSort();
  //
  //	Figure out what SMT sort we correspond to.
  //
  SMT_Info::SMT_Type t = getSMT_Info().getType(sort);
  Assert(t != SMT_Info::NOT_SMT, "bad SMT sort " << sort);
  if (t == SMT_Info::INTEGER)
    {
      const mpz_class& integer = value.get_num();
      bool needDisambig = !rangeKnown &&
	(kindsWithSucc.size() > 1 || overloadedIntegers.count(integer));
      prefix(s, needDisambig, color);
      s << integer;
      suffix(s, term, needDisambig, color);
    }
  else
    {
      Assert(t == SMT_Info::REAL, "SMT number sort expected");
      pair<mpz_class, mpz_class> rat(value.get_num(), value.get_den());
      bool needDisambig = !rangeKnown &&
	(kindsWithDivision.size() > 1 || overloadedRationals.count(rat));
      prefix(s, needDisambig, color);
      s << rat.first << '/' << rat.second;
      suffix(s, term, needDisambig, color);
    }
}

void
MixfixModule::mapPrettyPrint(Vector<int>& position, ostream& s, Term* term, int requiredPrec, int leftCapture, const ConnectedComponent* leftCaptureComponent, int rightCapture, const ConnectedComponent* rightCaptureComponent, bool rangeKnown)
{
  if (UserLevelRewritingContext::interrupted())
    return;

  Symbol* symbol = term->symbol();
  SymbolInfo& si = symbolInfo[symbol->getIndexWithinModule()];
  const char* color = computeColor(si.symbolType);
  //
  //	Check for special i/o representation.
  //
  if (mapHandleIter(position, s, term, si, rangeKnown, color))
    return;
  int basicType = si.symbolType.getBasicType();
  switch (basicType)
    {
    case SymbolType::MINUS_SYMBOL:
      {
	if (mapHandleMinus(position, s, term, rangeKnown, color))
	  return;
	break;
      }
    case SymbolType::DIVISION_SYMBOL:
      {
	if (mapHandleDivision(position, s, term, rangeKnown, color))
	  return;
	break;
      }
    case SymbolType::FLOAT:
      {
	mapHandleFloat(position, s, term, rangeKnown, color);
	return;
      }
    case SymbolType::STRING:
      {
	mapHandleString(position, s, term, rangeKnown, color);
	return;
      }
    case SymbolType::QUOTED_IDENTIFIER:
      {
	mapHandleQuotedIdentifier(position, s, term, rangeKnown, color);
	return;
      }
    case SymbolType::VARIABLE:
      {
	mapHandleVariable(position, s, term, rangeKnown, color);
	return;
      }
    case SymbolType::SMT_NUMBER_SYMBOL:
      {
	mapHandleSMT_Number(position, s, term, rangeKnown, color);
	return;
      }
    default:
      break;
    }
  //
  //	Default case where no special i/o representation applies.
  //
  int iflags = si.iflags;
  bool needDisambig = !rangeKnown && ambiguous(iflags);
  bool argRangeKnown = rangeOfArgumentsKnown(iflags, rangeKnown, needDisambig);
  int nrArgs = symbol->arity();
  if (needDisambig)
    //s << '(';
    s << "c1p" << getPos(position);
  if ((interpreter.getPrintFlag(Interpreter::PRINT_MIXFIX) && si.mixfixSyntax.length() != 0) ||
      basicType == SymbolType::SORT_TEST)
    {
      //
      //	Mixfix case.
      //
      bool printWithParens = interpreter.getPrintFlag(Interpreter::PRINT_WITH_PARENS);
      bool needParen = !needDisambig &&
	(printWithParens || requiredPrec < si.prec ||
	 ((iflags & LEFT_BARE) && leftCapture <= si.gather[0] &&
	  leftCaptureComponent == symbol->domainComponent(0)) ||
	 ((iflags & RIGHT_BARE) && rightCapture <= si.gather[nrArgs - 1] &&
	  rightCaptureComponent == symbol->domainComponent(nrArgs - 1)));
      bool needAssocParen = si.symbolType.hasFlag(SymbolType::ASSOC) &&
	(printWithParens || si.gather[1] < si.prec ||
	 ((iflags & LEFT_BARE) && (iflags & RIGHT_BARE) &&
	  si.prec <= si.gather[0]));
      if (needParen)
        //s << '(';
        s << "c1p" << getPos(position);
      int nrTails = 1;
      int pos = 0;
      ArgumentIterator a(*term);
      int moreArgs = a.valid();
      int newpos = 1;
      for (int arg = 0; moreArgs; arg++)
	{
	  Term* t = a.argument();
	  a.next();
	  moreArgs = a.valid();
	  pos = mapPrintTokens(position, s, si, pos, color);
	  if (arg == nrArgs - 1 && moreArgs)
	    {
	      ++nrTails;
	      arg = 0;
	      if (needAssocParen)
            //s << '(';
            s << "c1p" << getPos(position);
	      pos = mapPrintTokens(position, s, si, 0, color);
	    }
	  int lc = UNBOUNDED;
	  const ConnectedComponent* lcc = 0;
	  int rc = UNBOUNDED;
	  const ConnectedComponent* rcc = 0;
	  if (arg == 0 && (iflags & LEFT_BARE))
	    {
	      rc = si.prec;
	      rcc = symbol->domainComponent(0);
	      if (!needParen && !needDisambig)
		{
		  lc = leftCapture;
		  lcc = leftCaptureComponent;
		}
	    }
	  else if (!moreArgs && (iflags & RIGHT_BARE))
	    {
	      lc = si.prec;
	      lcc = symbol->domainComponent(nrArgs - 1);
	      if (!needParen && !needDisambig)
		{
		  rc = rightCapture;
		  rcc = rightCaptureComponent;
		}
	    }
        position.append(newpos);
        mapPrettyPrint(position, s, t, si.gather[arg], lc, lcc, rc, rcc, argRangeKnown);
        position.contractTo(position.length()-1);
        newpos++;
        if (UserLevelRewritingContext::interrupted())
	    return;
	}
      mapPrintTails(position, s, si, pos, nrTails, needAssocParen, true, color);
      if (UserLevelRewritingContext::interrupted())
	return;
      if (needParen)
        //s << ')';
        s << "c1p" << getPos(position);
    }
  else
    {
      //
      //	Prefix case.
      //
      const char* prefixName = Token::name(symbol->id());
      //if (color != 0)
      //s << color << prefixName << Tty(Tty::RESET);
      //else
      mapPrintPrefixName(position, s, prefixName, si);
      ArgumentIterator a(*term);
      if (a.valid())
	{
	  int nrTails = 1;
	  //s << '(';
      s << "c1p" << getPos(position);
      int newpos = 1;
	  for (int arg = 0;; arg++)
	    {
	      Term* t = a.argument();
	      a.next();
	      int moreArgs = a.valid();
	      if (arg >= nrArgs - 1 &&
		  !(interpreter.getPrintFlag(Interpreter::PRINT_FLAT)) &&
		  moreArgs)
		{
		  ++nrTails;
		  mapPrintPrefixName(position, s, prefixName, si);
		  //s << '(';
          s << "c1p" << getPos(position);
		}
        position.append(newpos);
	    mapPrettyPrint(position, s, t, PREFIX_GATHER, UNBOUNDED, 0, UNBOUNDED, 0, argRangeKnown);
	    position.contractTo(position.length()-1);
        newpos++;
        if (UserLevelRewritingContext::interrupted())
            return;
	    if (!moreArgs)
            break;
	      //s << ", ";
          s << "c2p" << getPos(position);
	    }
	  while (nrTails-- > 0)
	    {
	      if (UserLevelRewritingContext::interrupted())
		return;
	      //s << ')';
          s << "c1p" << getPos(position);
	    }
	}
    }
    mapSuffix(position, s, term, needDisambig, color);
}
/*** END MAU-DEV ***/
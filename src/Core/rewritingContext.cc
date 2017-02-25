/*

    This file is part of the Maude 2 interpreter.

    Copyright 1997-2003 SRI International, Menlo Park, CA 94025, USA.

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
//	Implementation for class RewritingContext.
//

//	utility stuff
#include "macros.hh"
#include "vector.hh"

//	forward declarations
#include "interface.hh"
#include "core.hh"

//	interface class definitions
#include "symbol.hh"
#include "dagNode.hh"

//	core class definitions
#include "rewritingContext.hh"

#include "run.cc"

bool RewritingContext::traceFlag = false;

void
RewritingContext::markReachableNodes()
{
  if (rootNode == 0)
    return;  // limited use RewritingContext
  rootNode->mark();
  int nrFragile = nrFragileBindings();
  for (int i = 0; i < nrFragile; i++)
    {
      DagNode* d = value(i);
      if (d != 0)
	d->mark();     
    }
  int stackLength = redexStack.length();
  for (int i = 0; i < stackLength; i++)
    redexStack[i].node()->mark();

  /*** BEGIN MAU-DEV ***/
  for (int i = 0; i < savedSub.length(); i++)
    if (savedSub[i] != 0)
        savedSub[i]->mark();
  if (savedLHS != 0)
      savedLHS->mark();
  if (savedRHS != 0)
      savedRHS->mark();
  /*** END MAU-DEV ***/
}

RewritingContext*
RewritingContext::makeSubcontext(DagNode* root, int /* purpose */)
{
  return new RewritingContext(root);
}

int
RewritingContext::traceBeginEqTrial(DagNode* /* subject */, const Equation* /* equation */)
{
  return 0;
}

int
RewritingContext::traceBeginRuleTrial(DagNode* /* subject */, const Rule* /* rule */)
{
  return 0;
}

int
RewritingContext::traceBeginScTrial(DagNode* /* subject */, const SortConstraint* /* sc */)
{
  return 0;
}

void
RewritingContext::traceEndTrial(int /* trailRef */, bool /* success */)
{
}

void
RewritingContext::traceExhausted(int /* trialRef */)
{
}

void
RewritingContext::tracePreEqRewrite(DagNode* /* redex */,
				    const Equation* /* equation */,
				    int /* type */)
{
}

void
RewritingContext::tracePostEqRewrite(DagNode* /* replacement */)
{
}

void
RewritingContext::tracePreRuleRewrite(DagNode* /* redex */, const Rule* /* rule */)
{
}

void
RewritingContext::tracePostRuleRewrite(DagNode* /* replacement */)
{
}

void
RewritingContext::tracePreScApplication(DagNode* /* subject */, const SortConstraint* /* sc */)
{
}

bool
RewritingContext::traceAbort()
{
  return false;
}

void
RewritingContext::traceBeginFragment(int /* trialRef */,
				     const PreEquation* /* preEquation */,
				     int /* fragmentIndex */,
				     bool /* firstAttempt */)
{
}

void
RewritingContext::traceEndFragment(int /* trialRef */,
				   const PreEquation* /* preEquation */,
				   int /* fragmentIndex */,
				   bool /* success */)
{
}

void
RewritingContext::traceNarrowingStep(Rule* /* rule */,
				     DagNode* /* redex */,
				     DagNode* /* replacement */,
				     const NarrowingVariableInfo* /* variableInfo */,
				     const Substitution* /* substitution */,
				     DagNode* /* newState */)
{
}

void
RewritingContext::traceVariantNarrowingStep(Equation* /* equation */,
					    const Vector<DagNode*>& /* oldVariantSubstitution */,
					    DagNode* /* redex */,
					    DagNode* /* replacement */,
					    const NarrowingVariableInfo& /* variableInfo */,
					    const Substitution* /* substitution */,
					    DagNode* /* newState */,
					    const Vector<DagNode*>& /* newVariantSubstitution */,
					    const NarrowingVariableInfo& /* originalVariables */)
{
}

void
RewritingContext::rebuildUptoRoot()
{
  //  cout << "\nroot was " << rootNode << endl;
  //  cout << "rebuilding from " << currentIndex << endl;
  Assert(currentIndex >= 0, "bad currentIndex");
  //
  //	Locate deepest stack node  with a stale parent.
  //
  int c = currentIndex;  // all staleness guaranteed to be above currentIndex
  for (int i = redexStack[c].parentIndex(); i != staleMarker; i = redexStack[i].parentIndex())
    c = i;
  //
  //	We assume that we only have to rebuild the spine from
  //	staleMarker to root.
  //
  for (int i = staleMarker; i != UNDEFINED; i = redexStack[i].parentIndex())
    {
      remakeStaleDagNode(i, c);
      c = i;
    }
  rootNode = redexStack[0].node();
  staleMarker = ROOT_OK;
  //  cout << "root is " << rootNode << endl;
}

void
RewritingContext::remakeStaleDagNode(int staleIndex, int childIndex)
{
  //
  //	Find first stacked argument of stale dag node.
  //
  int first = childIndex;
  while (redexStack[first - 1].parentIndex() == staleIndex)
    --first;
  //
  //	Find last stacked argument of stale dag node.
  //
  int last = childIndex;
  int stackLength = redexStack.length();
  while (last + 1 < stackLength &&
	 redexStack[last + 1].parentIndex() == staleIndex)
    ++last;
  //
  //	Replace stale dag node with a copy in which stacked arguments
  //	replace corresponding arguments in original.
  //
  DagNode* remade = redexStack[staleIndex].node()->
    copyWithReplacement(redexStack, first, last);
  redexStack[staleIndex].replaceNode(remade);
}

#ifdef DUMP
void
RewritingContext::dumpStack(ostream& s, const Vector<RedexPosition>& stack)
{
  int stackLen = stack.length();
  for (int i = 0; i < stackLen; i++)
    {
      s << stack[i].parentIndex() << '\t' <<
	stack[i].argIndex() << '\t' <<
	stack[i].node() << '\n';
    }
}
#endif

/*** BEGIN MAU-DEV ***/
void RewritingContext::saveContext()
{
	savedContexts.append(makeSubcontext(rootNode->copyReducible(true)));
	/* Save copies of substitution values */
	Substitution& original = *this;
	int nrValues = original.getNrValues();
	for(int i = 0; i < nrValues; i++)
		savedContexts[savedContexts.length()-1]->savedSub.append(original.value(i) != 0?(original.value(i)->copyReducible()):0);
}

void RewritingContext::saveContext(DagNode* redex)
{
	savedContexts.append(makeSubcontext(rootNode->copyReducible(true)));
	savedContexts[savedContexts.length()-1]->savedLHS = redex;
	/* Save copies of substitution values */
	Substitution& original = *this;
	int nrValues = original.getNrValues();
	for(int i = 0; i < nrValues; i++)
		savedContexts[savedContexts.length()-1]->savedSub.append(original.value(i) != 0?(original.value(i)->copyReducible()):0);
}

void RewritingContext::saveEquation(int eqIndex)
{
	savedContexts[savedContexts.length()-1]->savedEquation = eqIndex;
}

void RewritingContext::saveMembership(int scIndex)
{
    savedContexts[savedContexts.length()-1]->savedEquation = -2;
	savedContexts[savedContexts.length()-1]->savedMembership = scIndex;
}

void RewritingContext::addBuiltIn(DagNode* lhs, DagNode* rhs)
{
    //Same as addMemoized
	rhs->setHole();
	saveContext();
	rhs->clearHole();
	savedContexts[savedContexts.length()-1]->savedLHS = lhs;
	savedContexts[savedContexts.length()-1]->savedRHS = rhs->copyReducible(true);
	savedContexts[savedContexts.length()-1]->savedEquation = -1;
}

void RewritingContext::addMemoized(DagNode* lhs, DagNode* rhs)
{
    //Same as addBuiltIn
    rhs->setHole();
    saveContext();
    rhs->clearHole();
    savedContexts[savedContexts.length()-1]->savedLHS = lhs;
    savedContexts[savedContexts.length()-1]->savedRHS = rhs->copyReducible(true);
    savedContexts[savedContexts.length()-1]->savedEquation = -1;
}

void RewritingContext::startRecording(bool flag)
{
	savedContexts.clear();
	membership = flag;
	recording = true;
}

void RewritingContext::stopRecording()
{
	savedContexts.clear();
	recording = false;
    membership = false;
}

int RewritingContext::getEquation()
{
  return savedEquation;
}

int RewritingContext::getMembership()
{
  return savedMembership;
}

bool RewritingContext::isRecording()
{
    return recording;
}

bool RewritingContext::isRecordingMembership()
{
    return (recording && membership);
}
/*** END MAU-DEV ***/

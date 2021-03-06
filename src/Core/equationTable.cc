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
//      Implementation for abstract class EquationTable.
//

//	utility stuff
#include "macros.hh"
#include "vector.hh"

//      forward declarations
#include "interface.hh"
#include "core.hh"

//      interface class definitions
#include "symbol.hh"
#include "dagNode.hh"
#include "term.hh"
#include "subproblem.hh"
#include "extensionInfo.hh"

//      core class definitions
#include "rewritingContext.hh"
#include "lhsAutomaton.hh"
#include "rhsAutomaton.hh"
//#include "variableInfo.hh"
#include "equation.hh"
#include "equationTable.hh"

/*** BEGIN MAU-DEV ***/
void EquationTable::recordNormalize(DagNode* lhs, DagNode* subject, RewritingContext& context)
{
    if (context.isRecording() && (!lhs->equal(subject)))
        context.addBuiltIn(lhs,subject);
}
/*** END MAU-DEV ***/

void
EquationTable::compileEquations()
{
  int nrEquations = equations.length();
  for (int i = 0; i < nrEquations; i++)
    equations[i]->compile(true);
}

bool
EquationTable::applyReplace(DagNode* subject,
			    RewritingContext& context,
			    ExtensionInfo* extensionInfo)
{
  FOR_EACH_CONST(i, Vector<Equation*>, equations)
    {
      Subproblem* sp;
      Equation* eq = *i;
      long nrVariables = eq->fastNrVariables();
      if (nrVariables >= 0)
	{
	  //
	  //	Fast case.
	  //
	  context.clear(nrVariables);
	  if (eq->getLhsAutomaton()->match(subject, context, sp, extensionInfo))
	    {
	      if (sp != 0 || RewritingContext::getTraceStatus())
		goto slowCase;
        /*** BEGIN MAU-DEV ***/
	    if (context.isRecording())
	    {
            if (extensionInfo == 0 || extensionInfo->matchedWhole())
	        {
                DagNode* redex = subject->copyReducible(true);
	            eq->getRhsBuilder().replace(subject, context);
	            subject->setHole();
	            context.saveContext(redex);
	            subject->clearHole();
	        }
	        else
	        {
                DagNode* redex = (extensionInfo->buildMatchedPortion())->copyReducible(true);
	            DagNode* replacement = eq->getRhsBuilder().construct(context);
	            subject->partialReplace(replacement,extensionInfo);
	            replacement->setHole();
	            context.saveContext(replacement); //FIX-CONTEXT
	            replacement->clearHole();
	        }
	        context.saveEquation(eq->getIndexWithinModule());
	    }
	    else
	    {
	    /*** END MAU-DEV ***/
	      if (extensionInfo == 0 || extensionInfo->matchedWhole())
		eq->getRhsBuilder().replace(subject, context);
	      else
		{
		  subject->partialReplace(eq->getRhsBuilder().construct(context),
					  extensionInfo);
		}
        } // MAU-DEV
	      context.incrementEqCount();
	      context.finished();
	      MemoryCell::okToCollectGarbage();
	      return true;
	    }
	}
      else
	{
	  //
	  //	General case.
	  //
	  nrVariables = eq->getNrProtectedVariables();
	  context.clear(nrVariables);
	  if (eq->getLhsAutomaton()->match(subject, context, sp, extensionInfo))
	    {
	    slowCase:
	      DebugAdvisory("EquationTable::applyReplace() slowCase:\nsubject = " <<
			    subject << "\neq = " << eq);
	      if (sp == 0 || sp->solve(true, context))
		{
		  if (!(eq->hasCondition()) || eq->checkCondition(subject, context, sp))
		    {
		      bool trace = RewritingContext::getTraceStatus();
		      if (trace)
			{
			  context.tracePreEqRewrite(subject, eq, RewritingContext::NORMAL);
			  if (context.traceAbort())
			    {
			      delete sp;
			      context.finished();
			      break;
			    }
			}
            /*** BEGIN MAU-DEV ***/
		    if (context.isRecording())
		    {
                if (extensionInfo == 0 || extensionInfo->matchedWhole())
		    	{
		    		DagNode* redex = subject->copyReducible(true);
		    		eq->getRhsBuilder().replace(subject, context);
		    		subject->setHole();
		    		context.saveContext(redex);
		    		subject->clearHole();
		    	}
		    	else
		    	{
		    		DagNode* redex = (extensionInfo->buildMatchedPortion())->copyReducible(true);
		    		DagNode* replacement = eq->getRhsBuilder().construct(context);
		    		subject->partialReplace(replacement,extensionInfo);
		    		replacement->setHole();
		    		context.saveContext(replacement); //FIX-CONTEXT
		    		replacement->clearHole();
		    	}
		    	context.saveEquation(eq->getIndexWithinModule());
		    }
		    else
		    {
		    /*** END MAU-DEV ***/
		      if (extensionInfo == 0 || extensionInfo->matchedWhole())
			eq->getRhsBuilder().replace(subject, context);
		      else
			{
			  subject->partialReplace(eq->getRhsBuilder().construct(context),
						  extensionInfo);
			}
            } //MAU-DEV
		      context.incrementEqCount();
		      if (trace)
			context.tracePostEqRewrite(subject);
		      delete sp;
		      context.finished();
		      MemoryCell::okToCollectGarbage();
		      return true;
		    }
		}
	      delete sp;
	    }
	}
      context.finished();
      MemoryCell::okToCollectGarbage();  // matching process may create new DAG nodes
    }
  return false;
}

bool
EquationTable::applyReplaceNoOwise(DagNode* subject,
				   RewritingContext& context,
				   ExtensionInfo* extensionInfo)
{
  FOR_EACH_CONST(i, Vector<Equation*>, equations)
    {
      Subproblem* sp;
      Equation* eq = *i;
      //
      //	Except for this one if statement, this fuction is identical
      //	to the previous one.
      //
      if (eq->isOwise())
	break;
      long nrVariables = eq->fastNrVariables();
      if (nrVariables >= 0)
	{
	  //
	  //	Fast case.
	  //
	  context.clear(nrVariables);
	  if (eq->getLhsAutomaton()->match(subject, context, sp, extensionInfo))
	    {
	      if (sp != 0 || RewritingContext::getTraceStatus())
		goto slowCase;
        /*** BEGIN MAU-DEV ***/
	    if (context.isRecording())
	    {
	    	if (extensionInfo == 0 || extensionInfo->matchedWhole())
	    	{
	    		DagNode* redex = subject->copyReducible(true);
	    		eq->getRhsBuilder().replace(subject, context);
	    		subject->setHole();
	    		context.saveContext(redex);
	    		subject->clearHole();
	    	}
	    	else
	        {
	    		DagNode* redex = (extensionInfo->buildMatchedPortion())->copyReducible(true);
	    		DagNode* replacement = eq->getRhsBuilder().construct(context);
	    		subject->partialReplace(replacement,extensionInfo);
	    		replacement->setHole();
	    		context.saveContext(replacement); //FIX-CONTEXT
	    		replacement->clearHole();
	        }
	    	context.saveEquation(eq->getIndexWithinModule());
	    }
	    else
	    {
	    /*** END MAU-DEV ***/
	      if (extensionInfo == 0 || extensionInfo->matchedWhole())
		eq->getRhsBuilder().replace(subject, context);
	      else
		{
		  subject->partialReplace(eq->getRhsBuilder().construct(context),
					  extensionInfo);
		}
        } //MAU-DEV
	      context.incrementEqCount();
	      context.finished();
	      MemoryCell::okToCollectGarbage();
	      return true;
	    }
	}
      else
	{
	  //
	  //	General case.
	  //
	  nrVariables = eq->getNrProtectedVariables();
	  context.clear(nrVariables);
	  if (eq->getLhsAutomaton()->match(subject, context, sp, extensionInfo))
	    {
	    slowCase:
	      if (sp == 0 || sp->solve(true, context))
		{
		  if (!(eq->hasCondition()) || eq->checkCondition(subject, context, sp))
		    {
		      bool trace = RewritingContext::getTraceStatus();
		      if (trace)
			{
			  context.tracePreEqRewrite(subject, eq, RewritingContext::NORMAL);
			  if (context.traceAbort())
			    {
			      delete sp;
			      context.finished();
			      break;
			    }
			}
            /*** BEGIN MAU-DEV ***/
		    if (context.isRecording())
		    {
		    	if (extensionInfo == 0 || extensionInfo->matchedWhole())
		    	{
		    		DagNode* redex = subject->copyReducible(true);
		    		eq->getRhsBuilder().replace(subject, context);
		    		subject->setHole();
		    		context.saveContext(redex);
		    		subject->clearHole();
		    	}
		    	else
		    	{
		    		DagNode* redex = (extensionInfo->buildMatchedPortion())->copyReducible(true);
		    		DagNode* replacement = eq->getRhsBuilder().construct(context);
		    		subject->partialReplace(replacement,extensionInfo);
		    		replacement->setHole();
		    		context.saveContext(replacement); //FIX-CONTEXT
		    		replacement->clearHole();
		    	}
		    	context.saveEquation(eq->getIndexWithinModule());
		    }
		    else
		    {
		    /*** END MAU-DEV ***/
		      if (extensionInfo == 0 || extensionInfo->matchedWhole())
			eq->getRhsBuilder().replace(subject, context);
		      else
			{
			  subject->partialReplace(eq->getRhsBuilder().construct(context),
						  extensionInfo);
			}
            } //MAU-DEV
		      context.incrementEqCount();
		      if (trace)
			context.tracePostEqRewrite(subject);
		      delete sp;
		      context.finished();
		      MemoryCell::okToCollectGarbage();
		      return true;
		    }
		}
	      delete sp;
	    }
	}
      context.finished();
      MemoryCell::okToCollectGarbage();  // matching process may create new DAG nodes
    }
  return false;
}

void
EquationTable::resetEachEquation()
{
  FOR_EACH_CONST(e, Vector<Equation*>, equations)
    (*e)->reset();
}

#ifdef DUMP
void
EquationTable::dumpEquationTable(ostream& s, int indentLevel)
{
  for (int i = 0; i < equations.length(); i++)
    {
      LhsAutomaton* a = equations[i]->getLhsAutomaton();
      if (a != 0)
	a->dump(s, *(equations[i]), indentLevel);
    }
}
#endif

/*

    This file is part of the Maude 2 interpreter.

    Copyright 2019 SRI International, Menlo Park, CA 94025, USA.

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
//      Implementation for class ViewCache.
//

//      utility stuff
#include "macros.hh"
#include "vector.hh"

//      forward declarations
#include "interface.hh"
#include "core.hh"
#include "strategyLanguage.hh"
#include "mixfix.hh"

//	front end class definitions
#include "renaming.hh"
#include "view.hh"
#include "viewCache.hh"
#include "fileTable.hh"


ViewCache::ViewCache()
{
}

void
ViewCache::regretToInform(Entity* doomedEntity)
{
  View* doomedView = safeCast(View*, doomedEntity);
  ViewMap::iterator pos = viewMap.find(doomedView->id());
  Assert(pos != viewMap.end(), "could find self-destructing view " << doomedView);
  DebugAdvisory("removing view " << doomedView << " from cache");
  viewMap.erase(pos);
}

bool
ViewCache::viewCompare(const View* v1, const View* v2)
{
  return v1->id() < v2->id();
}

View*
ViewCache::makeViewInstantiation(View* view, const Vector<Argument*>& arguments)
{
  //
  //	Make the name and the cleanName of the view we want.
  //	cleanName looks like name, except:
  //	(1) No []s for bound parameters.
  //	(2) No spaces after commas.
  //	(3) The special characters {,} are backquoted.
  //	(4) We use cleanName of any nested view.
  //	cleanName is intended for forming sort names that contain view names.
  //
  Rope name(Token::name(view->id()));
  Rope cleanName(name);
  
  const char* sep = "{";
  const char* cleanSep = "`{";
  int nrParameters = arguments.size();
  for (int i = 0; i < nrParameters; ++i)
    {
      name += sep;
      sep = ", ";
      cleanName += cleanSep;
      cleanSep = "`,";

      Argument* a = arguments[i];
      if (View* v = dynamic_cast<View*>(a))
	{
	  name += Token::name(v->id());
	  cleanName += Token::name(v->getCleanName());
	}
      else
	{
	  //
	  //	Place brackets around parameter arguments in name so that
	  //	we don't confuse them with views having the same name.
	  //
	  name += '[';
	  name += Token::name(a->id());
	  name += ']';
	  //
	  //	cleanName only makes sense within a module so there is no
	  //	confusion between parameters and view with the same name -
	  //	parameters occlude views.
	  //
	  cleanName += Token::name(a->id());
	}
    }
  name += "}";
  cleanName += "`}";
  //
  //	Now check if a view having our name is already in cache.
  //
  int nameCode = Token::ropeToCode(name);
  ViewMap::const_iterator c = viewMap.find(nameCode);
  if (c != viewMap.end())
    {
      DebugAdvisory("using existing copy of view " << name);
      return c->second;
    }
  //
  //	Create new view; and insert it in cache.
  //
  DebugAdvisory("making view instantiation " << name);
  int cleanNameCode = Token::ropeToCode(cleanName);
  View* copy = view->makeInstantiation(nameCode, cleanNameCode, arguments);
  DebugAdvisory("finished view instantiation " << name);

  if (copy->isBad())
    {
      //
      //	It is possible for the instantiation of a non-bad view to be bad;
      //	for example, instantiating a parameter that has a self-conflict
      //	with a theory-view.
      //
      //	We never want to cache bad views.
      //
      IssueAdvisory(*copy << ": unable to make view instantiation " << QUOTE(name)
		    << " due to earlier errors.");
      copy->removeUser(this);  // since we are not adding a bad module to the cache
      delete copy;
      return 0;
    }
  viewMap[nameCode] = copy;
  return copy;
}

void
ViewCache::destructUnusedViews()
{
  //
  //	This O(n^2) solution to finding unused cached views is slow but
  //	simple. If the number of cached modules grows beyond a few hundred
  //	a more complex O(n) solution based on keeping a linked list of
  //	candidates would be appropriate. We would need a call back from
  //	ImportModule to tell us when a module is down to 1 user (us!).
  //
 restart:
  {
    FOR_EACH_CONST(i, ViewMap, viewMap)
      {
	int nrUsers = i->second->getNrUsers();
	Assert(nrUsers >= 1, "no users");  // we are a user
	if (nrUsers == 1)
	  {
	    DebugAdvisory("view " << i->second << " has no other users");
	    delete i->second;  // invalidates i
	    goto restart;
	  }
      }
  }
}

void
ViewCache::showCreatedViews(ostream& s) const
{
  //
  //	We display true name rather than cleanName.
  //
  FOR_EACH_CONST(i, ViewMap, viewMap)
    s << "view " << Token::name(i->first) << '\n';
}

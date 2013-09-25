/*******************************************************************\

Module: Definition Domain

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

//#define DEBUG

#ifdef DEBUG
#include <iostream>
#endif

#include <util/std_expr.h>

#include "ssa_domain.h"

/*******************************************************************\

Function: ssa_domaint::output

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void ssa_domaint::output(
  std::ostream &out) const
{
  for(def_mapt::const_iterator
      d_it=def_map.begin();
      d_it!=def_map.end();
      d_it++)
  {
    out << "DEF " << d_it->first << ": " << d_it->second << "\n";
  }

  for(phi_nodest::const_iterator
      p_it=phi_nodes.begin();
      p_it!=phi_nodes.end();
      p_it++)
  {
    for(std::map<locationt, deft>::const_iterator
        n_it=p_it->second.begin();
        n_it!=p_it->second.end();
        n_it++)
    {
      // this maps source -> def
      out << "PHI " << p_it->first << ": "
          << (*n_it).second
          << " from " 
          << (*n_it).first->location_number << "\n";
    }
  }
}

/*******************************************************************\

Function: ssa_domaint::transform

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void ssa_domaint::transform(
  locationt from,
  locationt to)
{
  if(from->is_assign())
  {
    const code_assignt &code_assign=to_code_assign(from->code);
    assign(code_assign.lhs(), from);
  }
  else if(from->is_decl())
  {
    const code_declt &code_decl=to_code_decl(from->code);
    assign(code_decl.symbol(), from);
  }
  else if(from->is_dead())
  {
    const code_deadt &code_dead=to_code_dead(from->code);
    const irep_idt &id=code_dead.get_identifier();
    def_map.erase(id);
  }
  
  // update source in all defs
  for(def_mapt::iterator
      d_it=def_map.begin(); d_it!=def_map.end(); d_it++)
    d_it->second.source=from;
}

/*******************************************************************\

Function: ssa_domaint::assign

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void ssa_domaint::assign(const exprt &lhs, locationt from)
{
  if(lhs.id()==ID_symbol)
  {
    const irep_idt &id=to_symbol_expr(lhs).get_identifier();
    def_entryt &def_entry=def_map[id];
    def_entry.def.loc=from;
    def_entry.def.kind=deft::ASSIGNMENT;
    def_entry.source=from;
  }
  else if(lhs.id()==ID_member)
  {
    assign(to_member_expr(lhs).struct_op(), from);
  }
  else if(lhs.id()==ID_index)
  {
    assign(to_index_expr(lhs).array(), from);
  }
  else if(lhs.id()==ID_typecast)
  {
    assign(to_typecast_expr(lhs).op(), from);
  }
  else if(lhs.id()==ID_if)
  {
    assign(to_if_expr(lhs).true_case(), from);
    assign(to_if_expr(lhs).false_case(), from);
  }
  else if(lhs.id()==ID_dereference)
  {
  }
}

/*******************************************************************\

Function: ssa_domaint::merge

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool ssa_domaint::merge(
  const ssa_domaint &b,
  locationt from,
  locationt to)
{
  bool result=false;

  // should traverse both maps simultaneously
  for(def_mapt::const_iterator
      d_it_b=b.def_map.begin();
      d_it_b!=b.def_map.end();
      d_it_b++)
  {
    const irep_idt &id=d_it_b->first;

    // check if we have a phi node for 'id'
  
    phi_nodest::iterator p_it=phi_nodes.find(id);
    if(p_it!=phi_nodes.end())
    {
      // yes, simply add to existing phi node
      std::map<locationt, deft> &phi_node=p_it->second;
      phi_node[d_it_b->second.source]=d_it_b->second.def;      
      // doesn't get propagated, don't set result to 'true'
      continue;
    }

    // have we seen this variable yet?
    def_mapt::iterator d_it_a=def_map.find(id);
    if(d_it_a==def_map.end())
    {
      // no entry in 'this' yet, simply create a new entry
      def_map[id]=d_it_b->second;
      result=true;

      #ifdef DEBUG
      std::cout << "SETTING " << id << ": " << def_map[id] << "\n";
      #endif
      continue;
    }

    // we have two entries, compare
    if(d_it_a->second.def==d_it_b->second.def)
    {
      #ifdef DEBUG
      std::cout << "AGREE " << id << ": " << d_it_b->second << "\n";
      #endif
      continue;
    }

    // Different definitions. Are they coming from the same source?
    if(d_it_a->second.source==d_it_b->second.source)
    {
      // Propagate the new definition for same source.
      d_it_a->second.def=d_it_b->second.def;
      result=true;

      #ifdef DEBUG
      std::cout << "OVERWRITING " << id << ": " << def_map[id] << "\n";
      #endif
    }
    else
    {
      // Arg! Data coming from two sources from two different definitions!
      // We produce a new phi node.
      std::map<locationt, deft> &phi_node=phi_nodes[id];

      phi_node[d_it_a->second.source]=d_it_a->second.def;
      phi_node[d_it_b->second.source]=d_it_b->second.def;
      
      // This phi node is now the new source.
      d_it_a->second.def.loc=to;
      d_it_a->second.def.kind=deft::PHI;
      d_it_a->second.source=to;

      result=true;

      #ifdef DEBUG
      std::cout << "MERGING " << id << ": " << d_it_b->second << "\n";
      #endif
    }
  }
  
  return result;
}

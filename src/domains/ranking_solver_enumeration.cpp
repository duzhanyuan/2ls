#include <iostream>

#include <util/simplify_expr.h>
#include "ranking_solver_enumeration.h"

bool ranking_solver_enumerationt::iterate(invariantt &_inv)
{
  linrank_domaint::templ_valuet &inv = 
    static_cast<linrank_domaint::templ_valuet &>(_inv);

  bool improved = false;

  literalt activation_literal = new_context();

  exprt inv_expr = linrank_domain.to_pre_constraints(inv);
  debug() << "pre-inv: " << from_expr(ns,"",inv_expr) << eom;

#ifndef DEBUG_FORMULA
  solver << or_exprt(inv_expr, literal_exprt(activation_literal));
#else
  debug() << "literal " << activation_literal << eom;
  literalt l = solver.convert(or_exprt(inv_expr, literal_exprt(activation_literal)));
  if(!l.is_constant()) 
  {
    debug() << "literal " << l << ": " << from_expr(ns,"",or_exprt(inv_expr, literal_exprt(activation_literal))) <<eom;
    formula.push_back(l);
  }
#endif

  exprt::operandst strategy_cond_exprs;
  linrank_domain.make_not_post_constraints(inv, 
    strategy_cond_exprs, strategy_value_exprs); 
  
  strategy_cond_literals.resize(strategy_cond_exprs.size());
  
  debug() << "post-inv: ";
  for(unsigned i = 0; i<strategy_cond_exprs.size(); i++)
  {  
    debug() << (i>0 ? " || " : "") << from_expr(ns,"",strategy_cond_exprs[i]) ;

    strategy_cond_literals[i] = solver.convert(strategy_cond_exprs[i]);
    //solver.set_frozen(strategy_cond_literals[i]);
    strategy_cond_exprs[i] = literal_exprt(strategy_cond_literals[i]);
  }
  debug() << eom;


#ifndef DEBUG_FORMULA
  solver << or_exprt(disjunction(strategy_cond_exprs),
		     literal_exprt(activation_literal));
#else

  exprt expr_act=
    or_exprt(disjunction(strategy_cond_exprs),
	       literal_exprt(activation_literal));

  l = solver.convert(expr_act);
  if(!l.is_constant()) 
  {
    debug() << "literal " << l << ": " << 
      from_expr(ns,"", expr_act) <<eom;
    formula.push_back(l);
  }
#endif

  debug() << "solve(): ";

#ifdef DEBUG_FORMULA
  bvt whole_formula = formula;
  whole_formula.insert(whole_formula.end(),activation_literals.begin(),activation_literals.end());
  solver.set_assumptions(whole_formula);
#endif

  if(solve() == decision_proceduret::D_SATISFIABLE) 
  { 
    debug() << "SAT" << eom;
      
    #if 0
    for(unsigned i=0; i<whole_formula.size(); i++) 
    {
      debug() << "literal: " << whole_formula[i] << " " << 
        solver.l_get(whole_formula[i]) << eom;
    }
          
    for(unsigned i=0; i<linrank_domain.template_size(); i++) 
    {
      exprt c = linrank_domain.get_row_constraint(i,inv[i]);
      debug() << "cond: " << from_expr(ns, "", c) << " " << 
          from_expr(ns, "", solver.get(c)) << eom;
      debug() << "guards: " << from_expr(ns, "", linrank_domain.templ.pre_guards[i]) << 
          " " << from_expr(ns, "", solver.get(linrank_domain.templ.pre_guards[i])) << eom;
      debug() << "guards: " << from_expr(ns, "", linrank_domain.templ.post_guards[i]) << " " 
          << from_expr(ns, "", solver.get(linrank_domain.templ.post_guards[i])) << eom; 	     	     }    
          
    for(replace_mapt::const_iterator
          it=renaming_map.begin();
          it!=renaming_map.end();    
          ++it)
          
    {
      debug() << "replace_map (1st): " << from_expr(ns, "", it->first) << " " << 
          from_expr(ns, "", solver.get(it->first)) << eom;
      debug() << "replace_map (2nd): " << from_expr(ns, "", it->second) << " " << 
          from_expr(ns, "", solver.get(it->second)) << eom;
    }
                  
    #endif
      
      
    for(unsigned row=0;row<strategy_cond_literals.size(); row++)
    {
      if(solver.l_get(strategy_cond_literals[row]).is_true()) 
      {
        debug() << "updating row: " << row << eom;

        exprt value = solver.get(strategy_value_exprs[row]);
        linrank_domaint::row_valuet v = simplify_const(value);

        debug() << "raw value; " << from_expr(ns, "", value) << 
          ", simplified value: " << from_expr(ns,"",v) << eom;

        linrank_domain.set_row_value(row,v,inv);
      }
    }
    improved = true;
  }
  else 
  {
    debug() << "UNSAT" << eom;

#ifdef DEBUG_FORMULA
    for(unsigned i=0; i<whole_formula.size(); i++) 
    {
      if(solver.is_in_conflict(whole_formula[i]))
        debug() << "is_in_conflict: " << whole_formula[i] << eom;
      else
        debug() << "not_in_conflict: " << whole_formula[i] << eom;
     }
#endif    
  }
  pop_context();

  return improved;
}

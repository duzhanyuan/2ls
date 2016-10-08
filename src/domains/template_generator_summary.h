/*******************************************************************\

Module: Template Generator for Summaries

Author: Peter Schrammel

\*******************************************************************/

#ifndef CPROVER_2LS_TEMPLATE_GENERATOR_SUMMARY_H
#define CPROVER_2LS_TEMPLATE_GENERATOR_SUMMARY_H

#include "template_generator.h"

class template_generator_summaryt : public template_generatort
{
public:

  explicit template_generator_summaryt(
    optionst &_options,
 		ssa_dbt &_ssa_db,
    ssa_local_unwindert &_ssa_local_unwinder)
    : 
    template_generatort(_options,_ssa_db,_ssa_local_unwinder)
  {
  }  

  virtual void operator()(unsigned _domain_number, 
			  const local_SSAt &SSA, bool forward=true);

  virtual domaint::var_sett inout_vars();
  virtual domaint::var_sett loop_vars();
  virtual domaint::var_sett out_vars();

protected:  
  domaint::var_specst var_specs;

};


#endif

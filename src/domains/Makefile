SRC = tpolyhedra_domain.cpp equality_domain.cpp domain.cpp predabs_domain.cpp\
  ssa_analyzer.cpp util.cpp incremental_solver.cpp \
  strategy_solver_base.cpp strategy_solver_equality.cpp \
  linrank_domain.cpp lexlinrank_domain.cpp\
  ranking_solver_enumeration.cpp lexlinrank_solver_enumeration.cpp \
  strategy_solver_enumeration.cpp strategy_solver_binsearch.cpp \
  template_generator_base.cpp template_generator_summary.cpp \
  template_generator_callingcontext.cpp template_generator_ranking.cpp \
  strategy_solver_binsearch2.cpp strategy_solver_binsearch3.cpp \
  strategy_solver_predabs.cpp
#solver_enumeration.cpp

include ../config.inc
include $(CBMC)/src/config.inc
include $(CBMC)/src/common
CBMC ?= ../..

CP_CXXFLAGS += $(TWOLSFLAGS)

INCLUDES= -I $(CBMC)/src -I ..

CLEANFILES = domains$(LIBEXT)

all: domains$(LIBEXT)

###############################################################################

domains$(LIBEXT): $(OBJ)
	$(LINKLIB)

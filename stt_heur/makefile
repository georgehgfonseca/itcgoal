#######################################################################
# Makefile created by Tulio Toffolo
#######################################################################

#----------------------------------------------------------------------
# General settings
#----------------------------------------------------------------------

EXE = ./stt
BIN = ./bin/
SRC = ./stt_heur/

OBJ = $(BIN)config.o \
      $(BIN)heuristics.o \
      $(BIN)moves.o \
      $(BIN)main.o
      
REFS = $(BIN)khe/*.o

#----------------------------------------------------------------------
# Compiler selection 
#----------------------------------------------------------------------

CCC = g++

#----------------------------------------------------------------------
# Compiler options 
#----------------------------------------------------------------------

CCOPT = -O3

#----------------------------------------------------------------------
# Final flags passed to compiler
#----------------------------------------------------------------------

CCLNFLAGS = -lm -lpthread -lexpat
CCFLAGS   = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) -I$(GUROBIINCDIR) -I$(OPTINCDIR)

#----------------------------------------------------------------------
# Finally compiling and linking files
#----------------------------------------------------------------------

.PHONY: all all-before all-after clean clean-custom

all: all-before $(EXE) all-after

$(BIN)%.o: $(SRC)%.cpp
	@echo Compilando "$<"
	@$(CCC) -c $(CCOPT) $(CCFLAGS) "$<" -o "$@"

$(EXE): $(OBJ)
	@$(CCC) $(CCOPT) $(CCFLAGS) $(OBJ) $(REFS) -o $(EXE) $(CCLNFLAGS) -w

clean: clean-custom
	${RM} $(OBJ) $(EXE)


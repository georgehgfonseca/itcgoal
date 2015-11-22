#ifndef heuristics_h
#define	heuristics_h

#include <ctime>
#include <list>

extern "C" {
#include "khe/khe.h"
}

#include "config.h"

#define MAX_NEIGHBOR        8
#define MEET_SWAP           1
#define TASK_SWAP           2
#define TASK_RESOURCE_SWAP  3
#define MEET_BLOCK_SWAP     4
#define MEET_TIME_CHANGE    5
#define PERMUT_RESOURCES    6
#define KEMPE_TIMES         7
#define MEET_SPLIT          8
#define MEET_MERGE          9
#define MEET_UNASSIGN       10

using namespace std;

//--------------------------------------------------------------------------

// Configura movimentos
void configureMoves(KHE_SOLN soln, KHE_INSTANCE instance, Config &config);
void restartMoves();

// Gera vizinhos
int randomNeighborhood();
bool generateNeighbor(KHE_SOLN soln, KHE_INSTANCE instance, int &neighborhood);

// Vizinhanca Permut
KHE_SOLN permutResource(KHE_SOLN soln, KHE_INSTANCE instance, KHE_RESOURCE resource);

// Vizinhanca Kempe
list< int > bfsConflictsGraph(map< int, map< int, int > > &G, map< int, int > &v, int last, int level);
list< int > generateConflictsGraph(KHE_SOLN soln, KHE_INSTANCE instance, KHE_TIME time1, KHE_TIME time2);

// Heuristicas
KHE_SOLN descent(KHE_SOLN soln, KHE_SOLN bestSoln, KHE_INSTANCE instance, int iterMax, Config &config);
KHE_SOLN simulatedAnnealing(KHE_SOLN soln, KHE_INSTANCE instance, Config &config);
KHE_SOLN ils(KHE_SOLN soln, KHE_INSTANCE instance, Config &config);
KHE_SOLN vns(KHE_SOLN soln, KHE_INSTANCE instance, Config &config);
KHE_SOLN rvns(KHE_SOLN soln, KHE_INSTANCE instance, Config &config);

// Funcoes auxiliares
void printToLog(KHE_SOLN soln, Config &config, int neighborhood, int iter, double temp);
bool isBetterSolution(KHE_SOLN solnA, KHE_SOLN solnB);
bool isBetterSolution(KHE_SOLN solnA, KHE_COST costB);
bool isBetterSolution(KHE_COST costA, KHE_COST costB);

#endif

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <map>
#include <vector>
#include <iostream>
#include <list>
#include <algorithm>

extern "C" {
#include "khe/khe.h"
#include "khe/khe_interns.h"
};

#include "heuristics.h"
#include "moves.h"

MoveSwap swapMeet;
MoveSwap swapMeetBlock;
MoveSwap swapTask;
MoveRealloc reallocMeetTime;
MoveRealloc reallocTaskResource;
MoveRealloc reallocPermutResource;
MoveSwap swapKempeTimes;
Move *moves[8];
int neighbors[8];

//=====================================================
// Configuracao dos Movimentos
//=====================================================

void configureMoves(KHE_SOLN soln, KHE_INSTANCE instance, Config &config) {
    config.assignResourcesConst = false;
    for (int i = 0; (i < KheInstanceConstraintCount(instance)); ++i) {
        KHE_CONSTRAINT constraint = KheInstanceConstraint(instance, i);
        switch (KheConstraintTag(constraint)) {
            case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
                config.assignResourcesConst = true;
                break;
            default:
                break;
        }
    }

    if (config.assignResourcesConst) {
        neighbors[MEET_SWAP] = 2000; // MEET_SWAP
        neighbors[TASK_SWAP] = 4000; // TASK_SWAP
        neighbors[TASK_RESOURCE_SWAP] = 5000; // TASK_RESOURCE_SWAP
        neighbors[MEET_BLOCK_SWAP] = 6000; // MEET_BLOCK_SWAP
        neighbors[MEET_TIME_CHANGE] = 9800; // MEET_TIME_CHANGE
        neighbors[PERMUT_RESOURCES] = 0; // PERMUT_RESOURCES
        neighbors[KEMPE_TIMES] = 10000; // KEMPE_TIMES
        //        neighbors[MEET_SPLIT] = 100000; // MEET_SPLIT
        //        neighbors[MEET_MERGE] = 100000; // MEET_MERGE
        //        neighbors[MEET_UNASSIGN] = 120000; // MEET_MERGE
    } else {
        neighbors[MEET_SWAP] = 4000; // MEET_SWAP
        neighbors[TASK_SWAP] = 0; // TASK_SWAP
        neighbors[TASK_RESOURCE_SWAP] = 0; // TASK_RESOURCE_SWAP
        neighbors[MEET_BLOCK_SWAP] = 6000; // MEET_BLOCK_SWAP
        neighbors[MEET_TIME_CHANGE] = 9800; // MEET_TIME_CHANGE
        neighbors[PERMUT_RESOURCES] = 0; // PERMUT_RESOURCES
        neighbors[KEMPE_TIMES] = 10000; // KEMPE_TIMES
        //        neighbors[MEET_SPLIT] = 100000; // MEET_SPLIT
        //        neighbors[MEET_MERGE] = 100000; // MEET_MERGE
        //       neighbors[MEET_UNASSIGN] = 12000; // MEET_MERGE
    }

    swapMeet.configure(KheSolnMeetCount(soln));
    swapMeetBlock.configure(KheSolnMeetCount(soln));
    swapTask.configure(KheSolnTaskCount(soln));

    reallocMeetTime.configure(KheSolnMeetCount(soln), KheInstanceTimeCount(instance));
    reallocTaskResource.configure(KheSolnTaskCount(soln), KheInstanceResourceCount(instance));

    swapKempeTimes.configure(KheInstanceTimeCount(instance));
    reallocPermutResource.configure(KheInstanceResourceCount(instance), 1);

    moves[MEET_SWAP] = (Move*) & swapMeet;
    moves[MEET_BLOCK_SWAP] = (Move*) & swapMeetBlock;
    moves[TASK_SWAP] = (Move*) & swapTask;
    moves[MEET_TIME_CHANGE] = (Move*) & reallocMeetTime;
    moves[TASK_RESOURCE_SWAP] = (Move*) & reallocTaskResource;
    moves[PERMUT_RESOURCES] = (Move*) & reallocPermutResource;
    moves[KEMPE_TIMES] = (Move*) & swapKempeTimes;
}

void restartMoves() {
    swapMeet.restart();
    swapMeetBlock.restart();
    swapTask.restart();
    reallocMeetTime.restart();
    reallocTaskResource.restart();
    swapKempeTimes.restart();
    reallocPermutResource.restart();
}

int randomNeighborhood() {
    int neighborhood = rand() % 10000; // sorteia a vizinhanca
    for (int i = 1; i <= MAX_NEIGHBOR; i++)
        if (neighborhood <= neighbors[i])
            return i;

    return 0;
}

//=====================================================
// Gerador de Vizinhos
//=====================================================

bool generateNeighbor(KHE_SOLN soln, KHE_INSTANCE instance, int &neighborhood) {
    if (neighborhood == 0)
        neighborhood = randomNeighborhood();


    pair< int, int > move;

    if (neighborhood == MEET_SWAP && swapMeet.hasMove()) {
        move = swapMeet.getMove();
        KheMeetSwap(KheSolnMeet(soln, move.first), KheSolnMeet(soln, move.second));
        return true;
    } else if (neighborhood == TASK_SWAP && swapTask.hasMove()) {
        move = swapTask.getMove();
        if (!KheTaskIsCycle(KheSolnTask(soln, move.first)) && !KheTaskIsCycle(KheSolnTask(soln, move.second)))
            KheTaskSwap(KheSolnTask(soln, move.first), KheSolnTask(soln, move.second));
        return true;
    } else if (neighborhood == TASK_RESOURCE_SWAP && reallocTaskResource.hasMove()) {
        move = reallocTaskResource.getMove();
        if (!KheTaskIsCycle(KheSolnTask(soln, move.first)))
            KheTaskMoveResource(KheSolnTask(soln, move.first), KheInstanceResource(instance, move.second));
        return true;
    } else if (neighborhood == MEET_BLOCK_SWAP && swapMeetBlock.hasMove()) {
        move = swapMeetBlock.getMove();
        KheMeetBlockSwap(KheSolnMeet(soln, move.first), KheSolnMeet(soln, move.second));
        return true;
    } else if (neighborhood == MEET_TIME_CHANGE && reallocMeetTime.hasMove()) {
        move = reallocMeetTime.getMove();
        KheMeetMoveTime(KheSolnMeet(soln, move.first), KheInstanceTime(instance, move.second));
        return true;
    } else if (neighborhood == PERMUT_RESOURCES && reallocPermutResource.hasMove()) {
        //move = reallocPermutResource.getMove();
        //permutResource(soln, instance, KheInstanceResource(instance, move.first));
        return true;
    } else if (neighborhood == KEMPE_TIMES && swapKempeTimes.hasMove()) {
        pair< int, int > move(0, 0);
        while (move.first == move.second)
            move = pair< int, int >(rand() % KheInstanceTimeCount(instance), rand() % KheInstanceTimeCount(instance));

        list< int > conflicts = generateConflictsGraph(soln, instance, KheInstanceTime(instance, move.first), KheInstanceTime(instance, move.second));

        int newTime = move.second;
        for (list< int >::iterator it = conflicts.begin(); it != conflicts.end(); it++) {
            KheMeetMoveTime(KheSolnMeet(soln, *it), KheInstanceTime(instance, newTime));
            newTime = (newTime == move.first) ? move.second : move.first;
        }
        return true;
    } else if (neighborhood == MEET_SPLIT) { //Meet duration split
        //        KHE_COST originalCost = KheSolnCost(soln);
        //printf("Original %f\n", KheCostShow(KheSolnCost(soln)));
        //        for(int i = 0; i < KheSolnMeetCount(soln); ++i) {
        //            if(KheMeetEvent(KheSolnMeet(soln, i)) != NULL)
        //                printf("%d %s %d %s\n", i, KheEventId(KheMeetEvent(KheSolnMeet(soln, i))), KheMeetDuration(KheSolnMeet(soln, i)), KheTimeId(KheMeetAsstTime(KheSolnMeet(soln, i))));
        //        }
        int duration = 0;
        int meetIndex;
        int numTries = 0;
        do {
            meetIndex = rand() % KheSolnMeetCount(soln);
            duration = KheMeetDuration(KheSolnMeet(soln, meetIndex));
            ++numTries;
            if (numTries > KheSolnMeetCount(soln))
                return false;
        } while (duration == 1);
        //Cria um novo meet de duração 1
        KHE_MEET meet1, meet2;
        //int index1, index2;
        //KheSolnAddMeet(soln, meet1, &index1);
        //KheSolnAddMeet(soln, meet2, &index2);
        KheMeetSplit(KheSolnMeet(soln, meetIndex), 1, false, &meet1, &meet2);


        //printf("Selected meet: %d\n", meetIndex);
        //        for(int i = 0; i < KheSolnMeetCount(soln); ++i) {
        //            if(KheMeetEvent(KheSolnMeet(soln, i)) != NULL)
        //                printf("%d %s %d %s\n", i, KheEventId(KheMeetEvent(KheSolnMeet(soln, i))), KheMeetDuration(KheSolnMeet(soln, i)), KheTimeId(KheMeetAsstTime(KheSolnMeet(soln, i))));
        //        }
        //printf("Changed %f\n", KheCostShow(KheSolnCost(soln)));
        //        if(isBetterSolution(KheCostShow(KheSolnCost(soln)), originalCost));
        //            printf("Improved %f %f\n", KheCostShow(originalCost), KheCostShow(KheSolnCost(soln)));
        return true;
    } else if (neighborhood == MEET_MERGE) { //Meet duration merge
        printf("Original %f\n", KheCostShow(KheSolnCost(soln)));
        for (int i = 0; i < KheSolnMeetCount(soln); ++i) {
            if (KheMeetAsstTime(KheSolnMeet(soln, i)) != NULL && KheMeetEvent(KheSolnMeet(soln, i)) != NULL) {
                for (int j = i + 1; j < KheSolnMeetCount(soln); ++j) {
                    if (KheMeetAsstTime(KheSolnMeet(soln, j)) != NULL && KheMeetEvent(KheSolnMeet(soln, j)) != NULL) {
                        //Merges the first possible merge
                        KHE_MEET merged;
                        if (KheMeetMerge(KheSolnMeet(soln, i), KheSolnMeet(soln, j), &merged)) {
                            //KHE error: KheTransactionUndo prevented by unsuitable operations
                            printf("Mergiu! kk\n");
                            printf("Changed %f\n", KheCostShow(KheSolnCost(soln)));
                            return true;
                        }
                    }
                }
            }
        }
    } else if (neighborhood == MEET_UNASSIGN) { //Meet unassign time



        //        int index1 = rand() % KheInstanceResourceCount(instance);
        //        int index2 = rand() % KheInstanceResourceCount(instance);
        //        if(index1 != index2)
        //        KheTwoColourReassign(soln, KheInstanceResource(instance, index1),
        //              KheInstanceResource(instance, index2), true);


        //KheEjectionChainRepairTimes(KheSolnNode(soln, 0));
        printf("%f\n", KheCostShow(KheSolnCost(soln)));
        //Unassign clashes..
        //        printf("Original %f\n", KheCostShow(KheSolnCost(soln)));
        //        
        //        int meetIndex = -1;
        //        do {
        //            meetIndex = rand() % KheSolnMeetCount(soln);
        //        } while(KheMeetAsstTime(KheSolnMeet(soln, meetIndex)) == NULL || 
        //                KheMeetEvent(KheSolnMeet(soln, meetIndex)) == NULL);
        //        //printf("%s\n", KheEventId(KheMeetEvent(KheSolnMeet(soln, meetIndex))));
        //        KheMeetUnAssignTime(KheSolnMeet(soln, meetIndex));
        //        printf("Changed %f\n", KheCostShow(KheSolnCost(soln)));
        //printf("%s\n", KheEventId(KheMeetEvent(KheSolnMeet(soln, meetIndex))));
    }
    return false;
}

//=====================================================
// Movimentos Kempe
//=====================================================

list< int > bfsConflictsGraph(map< int, map< int, int > > &G, map< int, int > &v, int last, int level) {
    list< int > maiorCaminho, caminho;
    v[last] = 1;
    if (level < 10) {
        for (map< int, int >::iterator it = G[last].begin(); it != G[last].end(); it++) {
            if (v[it->first] == 0) {
                list< int > caminho = bfsConflictsGraph(G, v, it->first, level + 1);

                if (caminho.size() > maiorCaminho.size())
                    maiorCaminho = caminho;

                if (level + maiorCaminho.size() == 10) break;
            } else {
                // ciclo detected !!!
            }
        }
    }
    v[last] = 0;
    maiorCaminho.push_front(last);
    return maiorCaminho;
}

list< int > generateConflictsGraph(KHE_SOLN soln, KHE_INSTANCE instance, KHE_TIME time1, KHE_TIME time2) {
    map< int, map< int, int > > G;
    vector< int > meetsTime1;
    vector< int > meetsTime2;

    KHE_MEET meet1, meet2;
    KHE_TASK task1, task2;

    for (int i = 0; i < KheSolnMeetCount(soln); ++i) {
        if (KheMeetAsstTime(KheSolnMeet(soln, i)) && KheTimeIndex(KheMeetAsstTime(KheSolnMeet(soln, i))) == KheTimeIndex(time1))
            meetsTime1.push_back(i);
        if (KheMeetAsstTime(KheSolnMeet(soln, i)) && KheTimeIndex(KheMeetAsstTime(KheSolnMeet(soln, i))) == KheTimeIndex(time2))
            meetsTime2.push_back(i);
    }

    for (int m1 = 0; m1 < meetsTime1.size(); ++m1) {
        meet1 = KheSolnMeet(soln, meetsTime1[m1]);
        for (int m2 = 0; m2 < meetsTime2.size(); ++m2) {
            meet2 = KheSolnMeet(soln, meetsTime2[m2]);
            for (int t1 = 0, t1Max = KheMeetTaskCount(meet1); t1 < t1Max; ++t1) {
                task1 = KheMeetTask(meet1, t1);
                if (!KheTaskAsstResource(task1)) continue;
                for (int t2 = 0, t2Max = KheMeetTaskCount(meet2); t2 < t2Max; ++t2) {
                    task2 = KheMeetTask(meet2, t2);
                    if (!KheTaskAsstResource(task2)) continue;

                    if (strcmp(KheResourceId(KheTaskAsstResource(task1)), KheResourceId(KheTaskAsstResource(task2))) == 0) {
                        G[meetsTime1[m1]][meetsTime2[m2]] = 1;
                        G[meetsTime2[m2]][meetsTime1[m1]] = 1;
                    }
                }
            }
        }
    }

    list< int > conflicts, bestConflicts;
    map< int, int > x;

    int currentHardFitness = -1, neighborHardFitness = -1, bestHardFitness = KheHardCost(KheSolnCost(soln));
    int currentSoftFitness = -1, neighborSoftFitness = -1, bestSoftFitness = KheSoftCost(KheSolnCost(soln));
    float delta;

    bool first = true;
    for (int i = 0; i < meetsTime1.size(); i++) {
        if (!G.count(meetsTime1[i])) continue;

        x.clear();
        conflicts = bfsConflictsGraph(G, x, meetsTime1[i], 0);
        if (conflicts.size() <= 2) continue;

        // realiza o movimento        
        KHE_TRANSACTION t = KheTransactionMake(soln);
        KheTransactionBegin(t);
        KHE_TIME newTime = time2;
        for (list< int >::iterator it = conflicts.begin(); it != conflicts.end(); it++) {
            KheMeetMoveTime(KheSolnMeet(soln, *it), newTime);
            newTime = (newTime == time1) ? time2 : time1;
        }
        KheTransactionEnd(t);

        neighborHardFitness = KheHardCost(KheSolnCost(soln));
        neighborSoftFitness = KheSoftCost(KheSolnCost(soln));

        // valida o movimento: se for melhor, vai para bestConflicts
        if (first) {
            first = false;
            delta = ((neighborHardFitness - bestHardFitness) * 10000.0 + (neighborSoftFitness - bestSoftFitness))
                    / conflicts.size();
            currentHardFitness = neighborHardFitness;
            currentSoftFitness = neighborSoftFitness;
        }

        if (neighborHardFitness < bestHardFitness || (neighborHardFitness == bestHardFitness && neighborSoftFitness < bestSoftFitness)) {
            bestConflicts = conflicts;
            delta = (neighborHardFitness - bestHardFitness) * 10000.0 + (neighborSoftFitness - bestSoftFitness);
            bestHardFitness = neighborHardFitness;
            bestSoftFitness = neighborSoftFitness;
            currentHardFitness = neighborHardFitness;
            currentSoftFitness = neighborSoftFitness;
        } else if (delta > ((neighborHardFitness - bestHardFitness) * 10000.0 + (neighborSoftFitness - bestSoftFitness))
                / conflicts.size()) {
            bestConflicts = conflicts;
            delta = ((neighborHardFitness - bestHardFitness) * 10000.0 + (neighborSoftFitness - bestSoftFitness))
                    / conflicts.size();
            currentHardFitness = neighborHardFitness;
            currentSoftFitness = neighborSoftFitness;
        }

        KheTransactionUndo(t);
        KheTransactionDelete(t);

        //        cout << "Conexoes '" << i << "': ";
        //        for (list< int >::iterator it = c[i].begin(); it != c[i].end(); ++it)
        //            cout << *it << " ( " << KheTimeIndex(KheMeetAsstTime(KheSolnMeet(soln, *it))) << " ) " << " -> ";
        //        cout << endl;    
    }
    //printf("Best Kemp: Hard = %d, Soft = %d  [cadeia de %d meets]\n", currentHardFitness, currentSoftFitness, bestConflicts.size());

    return bestConflicts;
}

//=====================================================
// Movimentos Permut
//=====================================================

KHE_SOLN permutResource(KHE_SOLN soln, KHE_INSTANCE instance, KHE_RESOURCE resource) {
    map< int, map< int, int > > G;
    vector< KHE_MEET > meets;
    vector< KHE_TIME > times;
    vector< KHE_TIME > bestTimes;

    int nTasks = KheResourceAssignedTaskCount(soln, resource);

    for (int i = 0; i < nTasks; ++i) {
        KHE_TASK task = KheResourceAssignedTask(soln, resource, i);
        KHE_MEET meet = KheTaskMeet(task);
        meets.push_back(meet);
        times.push_back(KheMeetAsstTime(meet));
    }
    bestTimes = times;

    KHE_COST costBest;
    bool first = true;

    if (times.size() <= 7) {
        while (next_permutation(times.begin(), times.end())) {
            // executa o movimento        
            KHE_TRANSACTION t = KheTransactionMake(soln);
            KheTransactionBegin(t);
            for (int i = 0; i < meets.size(); ++i) {
                KheMeetMoveTime(meets[i], times[i]);
            }
            KheTransactionEnd(t);

            // analisa se houve melhora
            if (first || isBetterSolution(soln, costBest)) {
                costBest = KheSolnCost(soln);
                bestTimes = times;
            }

            // defaz o movimento
            KheTransactionUndo(t);
            KheTransactionDelete(t);
        }
    } else {
        vector< int > sorteios;
        for (int j = 0; j < times.size(); j++)
            sorteios.push_back(j);

        vector< KHE_TIME > newTimes;
        for (int i = 0; i < 5040; i++) { // fat(7) = 40320
            newTimes.clear();
            while (newTimes.size() < sorteios.size()) {
                int r = (int) (rand() % (sorteios.size() - newTimes.size()));
                int a = sorteios[r];
                sorteios[r] = sorteios[sorteios.size() - newTimes.size() - 1];
                sorteios[sorteios.size() - newTimes.size() - 1] = a;
                newTimes.push_back(times[a]);
            }

            // executa o movimento        
            KHE_TRANSACTION t = KheTransactionMake(soln);
            KheTransactionBegin(t);
            for (int i = 0; i < meets.size(); ++i) {
                KheMeetMoveTime(meets[i], newTimes[i]);
            }
            KheTransactionEnd(t);

            // analisa se houve melhora
            if (first || isBetterSolution(soln, costBest)) {
                costBest = KheSolnCost(soln);
                bestTimes = newTimes;
            }

            // defaz o movimento
            KheTransactionUndo(t);
            KheTransactionDelete(t);
        }
    }

    // executa o melhor movimento encontrado e retorna a solucao
    for (int i = 0; i < meets.size(); ++i) {
        KheMeetMoveTime(meets[i], bestTimes[i]);
    }

    return soln;
}

//=====================================================
// Metaheuristicas
//=====================================================

KHE_SOLN simulatedAnnealing(KHE_SOLN soln, KHE_INSTANCE instance, Config &config) {
    KHE_SOLN bestSoln = soln;
    KHE_COST costAfter, costBefore;
    KHE_TRANSACTION t;
    soln = KheSolnCopy(bestSoln);

    int neighborhood = 0;
    int reheats = -1;
    int iterTemp = 0;
    double currentTemp = config.saTempIni;
    double delta, random;

    while (reheats < config.saReheats && config.getRemainingTime() > 0) {
        restartMoves();
        while (iterTemp < config.saMax && config.getRemainingTime() > 0) {
            iterTemp++;
            neighborhood = 0;

            // Gerando vizinho
            costBefore = KheSolnCost(soln);
            t = KheTransactionMake(soln);
            KheTransactionBegin(t);
            generateNeighbor(soln, instance, neighborhood);
            KheTransactionEnd(t);
            costAfter = KheSolnCost(soln);

            delta = (KheHardCost(costAfter) - KheHardCost(costBefore)) * 10000.0 + (KheSoftCost(costAfter) - KheSoftCost(costBefore))
                    / (KheHardCost(KheSolnCost(bestSoln)) * 10000.0 + KheSoftCost(KheSolnCost(bestSoln)));
            random = (1 + rand() % 100000) / 100000.0;

            if (delta <= 0) {
                if (isBetterSolution(soln, bestSoln)) {
                    KheSolnDelete(bestSoln);
                    bestSoln = KheSolnCopy(soln);
                    printToLog(soln, config, neighborhood, iterTemp, currentTemp);
                    iterTemp = 0;
                    restartMoves();
                }
            } else if (random < exp(-delta / currentTemp)) {
                restartMoves();
            } else {
                KheTransactionUndo(t);
            }
            KheTransactionDelete(t);
        }
        currentTemp = currentTemp * config.saAlpha;
        iterTemp = 0;

        if (currentTemp <= config.saTempMin) {
            reheats++;
            currentTemp = config.saTempIni;
            KheSolnDelete(soln);
            soln = KheSolnCopy(bestSoln);
            printf("Reaquecendo (time: %d)\n", config.getRunTime());
        }
    }

    KheSolnDelete(soln);
    return bestSoln;
}

KHE_SOLN ils(KHE_SOLN soln, KHE_INSTANCE instance, Config &config) {
    soln = descent(soln, soln, instance, config.ilsBlMax, config);
    KHE_SOLN bestSoln = KheSolnCopy(soln);

    KHE_COST cost;
    int perturbationSize = config.ilsPertIni;
    int iters = 0;
    int neighborhood = 0;
    int pertubationChanges = 0;

    while (config.getRemainingTime() > 0 && pertubationChanges < config.ilsIters) {
        restartMoves();
        for (int j = 0; j < perturbationSize; ++j) {
            neighborhood = rand() % 100 < 50 ? 6 : 7;
            generateNeighbor(soln, instance, neighborhood);
        }

        cost = KheSolnCost(soln);
        printf("PERTURBED Level %d Hard cost: %d   Soft cost: %d\n", perturbationSize, KheHardCost(cost), KheSoftCost(cost));
        soln = descent(soln, bestSoln, instance, config.ilsBlMax, config);

        // Houve melhora na solucao?
        if (isBetterSolution(soln, bestSoln)) {
            KheSolnDelete(bestSoln);
            bestSoln = KheSolnCopy(soln);
            perturbationSize = config.ilsPertIni;
            iters = 0;
        } else {
            KheSolnDelete(soln);
            soln = KheSolnCopy(bestSoln);
            iters++;
        }

        if (iters >= config.ilsMax) {
            iters = 0;
            perturbationSize = (config.ilsPertIni + 1) % (config.ilsPertMax + 1);
            pertubationChanges++;
        }
    }

    KheSolnDelete(soln);
    return bestSoln;
}

KHE_SOLN vns(KHE_SOLN soln, KHE_INSTANCE instance, Config &config) {

    int bestHardFitness = KheHardCost(KheSolnCost(soln));
    int bestSoftFitness = KheSoftCost(KheSolnCost(soln));
    int startingNeighborhood = 1;
    int neighborhood = startingNeighborhood;
    
    int neighborHardFitness;
    int neighborSoftFitness;
    bool hasMove;
    bool neighborhoodImprove;

    while (config.getRemainingTime() > 0) {
        restartMoves();
        hasMove = true;
        neighborhoodImprove = false;
        if (neighborhood != PERMUT_RESOURCES &&
                ((neighborhood != TASK_RESOURCE_SWAP && neighborhood != TASK_SWAP) || config.assignResourcesConst == true)) {
            for (int i = 0; i < config.vnsMax && hasMove && config.getRemainingTime() > 0; ++i) {
                KHE_TRANSACTION t = KheTransactionMake(soln);
                KheTransactionBegin(t);
                hasMove = generateNeighbor(soln, instance, neighborhood);
                KheTransactionEnd(t);
                // verifica se houve melhora na solucao
                neighborHardFitness = KheHardCost(KheSolnCost(soln));
                neighborSoftFitness = KheSoftCost(KheSolnCost(soln));
                if (neighborHardFitness < bestHardFitness || (neighborHardFitness == bestHardFitness && neighborSoftFitness < bestSoftFitness)) {
                    bestHardFitness = neighborHardFitness;
                    bestSoftFitness = neighborSoftFitness;
                    printToLog(soln, config, neighborhood, i, 0.0);
                    neighborhoodImprove = true;
                    KheTransactionDelete(t);
                    break;
                } else if (neighborHardFitness > bestHardFitness || neighborSoftFitness > bestSoftFitness) {
                    // caso a solucao seja pior que a anterior
                    KheTransactionUndo(t); // desfaz o movimento
                    KheTransactionDelete(t); // desfaz o movimento
                }
            }
        }
        if (neighborhoodImprove)
            neighborhood = startingNeighborhood;
        else
            if (neighborhood + 1 == MAX_NEIGHBOR)
            neighborhood = startingNeighborhood;
        else
            ++neighborhood;
    }
    return soln;
}

KHE_SOLN rvns(KHE_SOLN soln, KHE_INSTANCE instance, Config &config) {

    //soln = descent(soln, soln, instance, config.ilsBlMax, config);
    KHE_SOLN bestSoln = KheSolnCopy(soln);
    KHE_COST cost;
    int bestHardFitness = KheHardCost(KheSolnCost(soln));
    int bestSoftFitness = KheSoftCost(KheSolnCost(soln));
    int neighborhood = (rand() % (MAX_NEIGHBOR - 1)) + 1;
    
    bool hasMove;
    int neighborHardFitness;
    int neighborSoftFitness;

    while (config.getRemainingTime() > 0) {
        restartMoves();
        hasMove = true;
        if (neighborhood != PERMUT_RESOURCES &&
                ((neighborhood != TASK_RESOURCE_SWAP && neighborhood != TASK_SWAP) || config.assignResourcesConst == true)) {
            for (int i = 0; i < config.vnsMax && hasMove && config.getRemainingTime() > 0; ++i) {
                KHE_TRANSACTION t = KheTransactionMake(soln);
                KheTransactionBegin(t);
                hasMove = generateNeighbor(soln, instance, neighborhood);
                KheTransactionEnd(t);
                // verifica se houve melhora na solucao
                neighborHardFitness = KheHardCost(KheSolnCost(soln));
                neighborSoftFitness = KheSoftCost(KheSolnCost(soln));
                if (neighborHardFitness < bestHardFitness || (neighborHardFitness == bestHardFitness && neighborSoftFitness < bestSoftFitness)) {
                    bestHardFitness = neighborHardFitness;
                    bestSoftFitness = neighborSoftFitness;
                    printToLog(soln, config, neighborhood, i, 0.0);
                    KheTransactionDelete(t);
                    break;
                } else if (neighborHardFitness > bestHardFitness || neighborSoftFitness > bestSoftFitness) {
                    // caso a solucao seja pior que a anterior
                    KheTransactionUndo(t); // desfaz o movimento
                    KheTransactionDelete(t); // desfaz o movimento
                }
            }
        }
        neighborhood = (rand() % (MAX_NEIGHBOR - 1)) + 1;
    }
    return soln;
}

//=====================================================
// Heuristicas
//=====================================================

KHE_SOLN descent(KHE_SOLN soln, KHE_SOLN bestSoln, KHE_INSTANCE instance, int iterMax, Config &config) {
    int bestKnownHardFitness = KheHardCost(KheSolnCost(bestSoln));
    int bestKnownSoftFitness = KheSoftCost(KheSolnCost(bestSoln));

    int bestHardFitness = KheHardCost(KheSolnCost(soln));
    int bestSoftFitness = KheSoftCost(KheSolnCost(soln));
    int iter = 0, neighborhood = 0;

    bool hasMove = true;
    while (hasMove && iter < iterMax && config.getRemainingTime() > 0) {

        // gera o vizinho e executa o movimento
        KHE_TRANSACTION t = KheTransactionMake(soln);
        KheTransactionBegin(t);
        neighborhood = 0;
        hasMove = generateNeighbor(soln, instance, neighborhood);
        KheTransactionEnd(t);

        // verifica se houve melhora na solucao
        int neighborHardFitness = KheHardCost(KheSolnCost(soln));
        int neighborSoftFitness = KheSoftCost(KheSolnCost(soln));
        if (neighborHardFitness < bestHardFitness || (neighborHardFitness == bestHardFitness && neighborSoftFitness < bestSoftFitness)) {
            bestHardFitness = neighborHardFitness;
            bestSoftFitness = neighborSoftFitness;
            if (neighborHardFitness < bestKnownHardFitness || (neighborHardFitness == bestKnownHardFitness && neighborSoftFitness < bestKnownSoftFitness))
                printToLog(soln, config, neighborhood, iter, 0.0);

            restartMoves();
            iter = 0;
        } else if (neighborHardFitness > bestHardFitness || neighborSoftFitness > bestSoftFitness) {
            // caso a solucao seja pior que a anterior
            KheTransactionUndo(t); // desfaz o movimento
        }
        KheTransactionDelete(t);

        iter++;
    }

    return soln;
}

//=====================================================
// Funcoes utilitarias
//=====================================================

void printToLog(KHE_SOLN soln, Config &config, int neighborhood, int iter, double temp) {
    printf("*** time: %-6d fo: %2d, %-6d neighborhood: %-6d iter: %-6d",
            config.getRunTime(),
            KheHardCost(KheSolnCost(soln)), KheSoftCost(KheSolnCost(soln)),
            neighborhood, iter);
    if (temp)
        printf(" temp: %-3.3lf", temp);

    printf("\n");
}

bool isBetterSolution(KHE_SOLN solnA, KHE_SOLN solnB) {
    return isBetterSolution(KheSolnCost(solnA), KheSolnCost(solnB));
}

bool isBetterSolution(KHE_SOLN solnA, KHE_COST costB) {
    return isBetterSolution(KheSolnCost(solnA), costB);
}

bool isBetterSolution(KHE_COST costA, KHE_COST costB) {
    return KheHardCost(costA) < KheHardCost(costB) ||
            (KheHardCost(costA) == KheHardCost(costB) && KheSoftCost(costA) < KheSoftCost(costB));
}


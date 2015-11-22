#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <map>
#include <vector>
#include <iostream>

extern "C" {
#include "khe/khe.h"
#include "khe/khe_interns.h"
};

#include "config.h"
#include "heuristics.h"

using namespace std;

//--------------------------------------------------------------------------

static KHE_ARCHIVE ReadArchive(const char *fname) {
    FILE *fp;  KHE_ARCHIVE res;  KML_ERROR ke;
    fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "khe: cannot open file \"%s\" for reading\n", fname);
        exit( EXIT_FAILURE );
    }
    if (!KheArchiveRead(fp, &res, true, &ke)) {
        fprintf(stderr, "%s:%d:%d: %s\n", fname, KmlErrorLineNum(ke), KmlErrorColNum(ke), KmlErrorString(ke));
        exit( EXIT_FAILURE );
    }
    return res;
}

int main(int argc, char** argv) {
    Config config;
    config.setParameters(argc, argv);
    srand(config.seed);
    
    //___________________________________________________________________________
    /******************************* File read *********************************/
    KHE_ARCHIVE archive = ReadArchive(config.xml);
    if (KheArchiveInstanceCount(archive) > 1) {
        cerr << "Please enter a XML with only one instance." << endl;
        exit(EXIT_FAILURE);
    }
    
    //___________________________________________________________________________
    /*********************** Get elements of instances *************************/
    KHE_INSTANCE instance = KheArchiveInstance(archive, 0);
    printf("instance name is %s\n", KheInstanceName(instance));
    
    //___________________________________________________________________________
    /************************ Get current time/date ****************************/
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char* timeDesctiption = asctime(timeinfo);
    char team[100];
    char remarks[100];
    char title[100];
    
    sprintf(team, "GOAL team: Fonseca, G.H.G., Brito, S., Toffolo, T. and Santos, H.G.");
    sprintf(remarks, "Under development (Random seed = %d)", config.seed);
    sprintf(title, "Hibrid heuristic and IP methods");
    
    KHE_SOLN_GROUP_METADATA md = KheSolnGroupMetaDataMake(
        team,
        timeDesctiption,
        title,
        remarks                                           
    );
    KHE_SOLN_GROUP solg;
    char solnGroupId[256];
    sprintf(solnGroupId, "GOAL team %s", timeDesctiption);
    if (!KheSolnGroupMake(archive, solnGroupId, md, &solg)) {
        fprintf(stderr, "error.\n");
        exit(1);
    }
    //___________________________________________________________________________
    /*********************** Create solution elements **************************/
    KHE_SOLN soln = KheSolnMake(instance, solg);    
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
    
    //soln = KheGeneralSolve(soln);
    //soln = KheParallelSolve(soln, THREADS, &KheGeneralSolve);
    soln = KheSolnGroupSoln(KheArchiveSolnGroup(archive, 0), 0);
//    for(int i = 0; i < KheSolnMeetCount(soln); ++i) {
//        if(KheMeetEvent(KheSolnMeet(soln, i)) != NULL)
//            printf("%s %d\n", KheEventId(KheMeetEvent(KheSolnMeet(soln, i))), KheMeetDuration(KheSolnMeet(soln, i)));
//    }
    //___________________________________________________________________________
    //*************************** Run first heuristic ***************************
    KHE_COST cost = KheSolnCost(soln);
    printf("Initial solution: %d , %d\n", KheHardCost(cost), KheSoftCost(cost));
    fflush(stdout);
    
    configureMoves(soln, instance, config);
    
    printf("Elapsed time: %d of %d\n", config.getRunTime(), config.timeLimit);
    
//    for(int i = 0; i < KheSolnDefectCount(soln); ++i) {
//        for(int j = 0; j < KheMonitorDeviationCount(KheSolnDefect(soln, i)); ++j) {
//            printf("%s\n", KheMonitorDeviationDescription(KheSolnDefect(soln, i), j));
//        }
//    }

//    printf("%d\n", KheSolnDefectCount(soln));
//    printf("%d\n", KheGroupMonitorDefectCount((KHE_GROUP_MONITOR) soln));
//    for(int i = 0; i < KheSolnDefectCount(soln); ++i) {
//        if(strcmp(KheMonitorTagShow(KheMonitorTag(KheSolnDefect(soln, i))), "KHE_AVOID_CLASHES_MONITOR_TAG") == 0)
//            printf("XXXXX %s %s\n", KheMonitorTagShow(KheMonitorTag(KheSolnDefect(soln, i))), KheMonitorAppliesToName(KheSolnDefect(soln, i)));
//    }
    printf("\nStarting Simulated Annealing\n");
    soln = simulatedAnnealing(soln, instance, config);
    
    printf("\nStarting Iterated Local Search (ILS)\n");
    soln = ils(soln, instance, config);
    
//    printf("\nStarting Variable Neighborhood Search (VNS)\n");
//    soln = vns(soln, instance, config);
    //soln = rvns(soln, instance, config);
    
    //__________________________________________________________________________
    //*********************** Run Iterated Local Search ************************
    //___________________________________________________________________________
    /*************************** Evaluate solution *****************************/
    KheSolnEnsureOfficialCost(soln);
    cost = KheSolnCost(soln);
    fflush(stdout);
    printf("Hard cost: %d\nSoft cost: %d\nElapesed time: %ds\n",
           KheHardCost(cost), KheSoftCost(cost), config.getRunTime());
    //___________________________________________________________________________
    /***************************** Write solution ******************************/
escrever:
    //if (soln)
    //KheSolnDelete(soln);
    //KheSolnSetSolnGroup(optimumSoln, solg);
    //FILE *fsol = fopen(argv[1], "w");
    //KheArchiveWrite(archive, true, fsol);
    
                
                
    KheSolnGroupAddSoln(solg, soln);
    //FILE *fsol = fopen((string(config.outPrefix) + ".sol").c_str(), "w");
    FILE *fsol = fopen(config.outPrefix, "w");
    KheSolnGroupWrite(solg, true, KmlMakeFile(fsol, 2, 2));
    fclose(fsol);
    /***************************************************************************/
    return 0;
}


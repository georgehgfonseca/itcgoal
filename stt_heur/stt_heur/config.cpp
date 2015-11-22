#include <iostream>
#include "config.h"

using namespace std;

//--------------------------------------------------------------------------

bool Config::setParameters(int argc, char *argv[]) {
    this->xml = argv[1];
    this->outPrefix = argv[2];
    this->timeLimit = atoi(argv[3]);
    this->seed = atoi(argv[4]);
    
    /*
    
    int value;
    double dvalue;
    char chvalue[1000];
    
    //ITC requiriments
//    sscanf(argv[0], "%s", this->xml);
//    sscanf(argv[1], "%s", this->outPrefix);//this->outPrefix = argv[1];
//    sscanf(argv[2], "%d", this->timeLimit);//this->timeLimit = argv[2];
//    sscanf(argv[3], "%d", this->seed);//this->seed = argv[3];
//    
//    printf("%s %s %d %d\n", this->xml, this->outPrefix, this->timeLimit, this->seed);
    
    // capturando parametros opcionais
    for (int i = 1; i < argc; i++) {        
        if (sscanf(argv[i], "-xml=%s", chvalue) == 1)
            this->xml = argv[i]+5;
        else if (sscanf(argv[i], "-sol=%s", chvalue) == 1)
            this->sol = argv[i]+5;
        else if (sscanf(argv[i], "-out=%s", chvalue) == 1)
            this->outPrefix = argv[i]+5;
        
        else if (sscanf(argv[i], "-seed=%d", &value) == 1)
            this->seed = value;
        else if (sscanf(argv[i], "-threads=%d", &value) == 1)
            this->threads = value;
        else if (sscanf(argv[i], "-time_limit=%d", &value) == 1)
            this->timeLimit = value;
        else if (sscanf(argv[i], "-lb=%d", &value) == 1)
            this->lb = value;
        
        else if (sscanf(argv[i], "-sa_max=%d", &value) == 1)
            this->saMax = value;
        else if (sscanf(argv[i], "-sa_tempini=%lf", &dvalue) == 1)
            this->saTempIni = dvalue;
        else if (sscanf(argv[i], "-sa_tempmin=%lf", &dvalue) == 1)
            this->saTempMin = dvalue;
        else if (sscanf(argv[i], "-sa_alpha=%lf", &dvalue) == 1)
            this->saAlpha = dvalue;
        else if (sscanf(argv[i], "-sa_reheats=%d", &value) == 1)
            this->saReheats = value;
        
        else if (sscanf(argv[i], "-ils_max=%d", &value) == 1)
            this->ilsMax = value;
        else if (sscanf(argv[i], "-ils_pertiter=%d", &value) == 1)
            this->ilsIters = value;
        else if (sscanf(argv[i], "-ils_blmax=%d", &value) == 1)
            this->ilsBlMax = value;
        else if (sscanf(argv[i], "-ils_pertini=%d", &value) == 1)
            this->ilsPertIni = value;
        else if (sscanf(argv[i], "-ils_pertmax=%d", &value) == 1)
            this->ilsPertMax = value;

        else if (sscanf(argv[i], "-vns_max=%d", &value) == 1)
            this->ilsMax = value;
             
        else {
            this->usage(argv[0]);
            cerr << "ERROR: Invalid parameter: " << argv[i] << endl << endl;
            exit(EXIT_FAILURE);
        }
    }
    
    // verificando se os parametros obrigatorios foram fornecidos
    if (!this->xml || !this->outPrefix) {
        this->usage(argv[0]);
        cerr << "ERROR: You must inform the parameters -xml and -out" << endl << endl;
        exit(EXIT_FAILURE);
    }
    */
    return true;
     
}

void Config::usage (const char *progname) {
    cerr << endl;
    cerr << "Usage: " << progname << " [arguments] [options]" << endl;
    cerr << endl;
    cerr << "Program arguments:" << endl;
    cerr << "    -xml=input.xml  : nurse problem" << endl;
    cerr << "    -sol=input.sol  : solution file" << endl;
    cerr << "    -out=output     : prefix of files where the solutions and logs will be saved" << endl;
    cerr << endl;
    cerr << "Optional parameters (example):" << endl;
    cerr << "    -seed=333       : seed for random number generator" << endl;
    cerr << "                      default value = 33" << endl;
    cerr << "    -threads=10     : the program will use up to 10 threads." << endl;
    cerr << "                      default value = 0" << endl;
    cerr << "    -time_limit=100 : the program will execute in up to 100 seconds." << endl;
    cerr << "                      default value = 0 (unlimited)" << endl;
    cerr << "    -lb=0           : value of the best known lower bound (or global optimum)." << endl;
    cerr << "                      default value = 0" << endl;
    cerr << "                    " << endl;
    cerr << "    -sa_time=0      " << endl;
    cerr << "    -sa_max=0       " << endl;
    cerr << "    -sa_tempini=0   " << endl;
    cerr << "    -sa_tempmin=0   " << endl;
    cerr << "    -sa_apha=0      " << endl;
    cerr << "    -sa_reheats=0   " << endl;
    cerr << "                    " << endl;
    cerr << "    -ils_time=0     " << endl;
    cerr << "    -ils_max=0      " << endl;
    cerr << "    -ils_blmax=0    " << endl;
    cerr << "    -ils_pertini=0  " << endl;
    cerr << "    -ils_pertmax=0  " << endl;
    cerr << "    -ils_pertiter=0 " << endl;
    cerr << endl;
}

int Config::getRunTime() {
    return (int) (time(NULL) - this->timeIni);
}

int Config::getRemainingTime() {
    return this->timeLimit - (int) (time(NULL) - this->timeIni);
}

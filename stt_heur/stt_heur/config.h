#ifndef config_h
#define config_h

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

class Config {
public:
    char *xml;       // modelo de entrada
    char *sol;       // arquivo com solucoes
    char *outPrefix; // arquivo(s) de saida
    
    int seed;        // semente de nros aleatorios
    int threads;     // nro de threads
    int timeIni;     // marca tempo de inicio da execucao
    int timeLimit;   // tempo limite de execucao (em minutos)
    int lb;          // melhor lower bound conhecido para a instancia
    
    int saMax;
    int saReheats;
    double saTempIni;
    double saTempMin;
    double saAlpha;
    
    int ilsMax;
    int ilsIters;
    int ilsBlMax;
    int ilsPertIni;
    int ilsPertMax;
    
    int vnsMax;
    
    int assignResourcesConst;
    
    Config() {
        this->xml = NULL;                 
        this->sol = NULL;                 
        this->outPrefix = NULL;           
        
        this->seed = 1;                   
        this->threads = 1;                
        this->timeIni = (int) time(NULL); 
        this->timeLimit = 1000;            
        this->lb = 0;                     
        
        this->saMax = 10000;
        this->saReheats = 5;
        this->saTempIni = 1.0;
        this->saTempMin = 0.1;
        this->saAlpha = 0.97;
        
        this->ilsMax = 10000;
        this->ilsIters = 50;
        this->ilsBlMax = 1000000;
        this->ilsPertIni = 1;
        this->ilsPertMax = 10;
        
        this->vnsMax = 5000;
        
        this->assignResourcesConst = false;
    }
    
    bool setParameters(int argc, char *argv[]);
    void usage(const char *progname);
    int getRunTime();
    int getRemainingTime();
};

#endif

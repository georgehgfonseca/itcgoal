#ifndef swaps_h
#define swaps_h

#include <iostream>
#include <utility>
#include <string>

#define MAX_NEIGHBORHOOD_SIZE 10000000

using namespace std;

class Move {
public:
    int n, total;
    int sizeFirst, sizeSecond;
    virtual bool hasMove();    
};

class MoveSwap : public Move {
public:
    pair< int, int > *moves;
    
    MoveSwap();
    MoveSwap(int size);
    void configure(int size);
    void restart();
    pair< int, int > getMove();
};

class MoveRealloc : public Move {
public:
    pair< int, int > *moves;
    
    MoveRealloc();
    MoveRealloc(int sizeA, int sizeB);
    void configure(int sizeA, int sizeB);
    void restart();
    pair< int, int > getMove();
};


#endif

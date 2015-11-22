#include <iostream>
#include <cstdlib>
#include "moves.h"

using namespace std;

bool Move::hasMove() {
    return this->n != 0;
}

// ------------------------------------------------------------

MoveSwap::MoveSwap() {
    this->moves = NULL;
}

MoveSwap::MoveSwap(int size) {
    this->moves = NULL;
    this->configure(size);
}

void MoveSwap::configure(int size) {
    if (this->moves != NULL)
        free(this->moves);
    
    if (MAX_NEIGHBORHOOD_SIZE > size * size) {
        this->total = this->n = -1;
        this->sizeFirst = size;
        this->sizeSecond = size;
        this->moves = NULL;
        return;
    }
    
    this->total = this->n = 0;    
    this->moves = (pair< int, int> *) malloc(sizeof(pair< int, int >) * size * size);
    
    for (int i = 0; i < size; i++) {
        for (int j = i+1; j < size; j++) {
            this->moves[this->n].first = i;
            this->moves[this->n].second = j;
            
            this->total++; this->n++;
        }
    }
}

void MoveSwap::restart() {
    this->n = this->total;
}

pair< int, int > MoveSwap::getMove() {
    if (this->n == -1) {
        pair< int, int > move(rand() % this->sizeFirst, rand() % this->sizeSecond);
        while (move.first == move.second)
            move = pair< int, int >(rand() % this->sizeFirst, rand() % this->sizeSecond);
        return move;
    }
    
    int p = rand() % this->n;
    pair< int, int > r = this->moves[p];
    this->moves[p] = this->moves[this->n-1];
    this->moves[this->n-1] = r;
    
    this->n--;
    return r;
}

// ------------------------------------------------------------

MoveRealloc::MoveRealloc() {
    this->moves = NULL;
}

MoveRealloc::MoveRealloc(int sizeA, int sizeB) {
    this->moves = NULL;
    this->configure(sizeA, sizeB);
}

void MoveRealloc::configure(int sizeA, int sizeB) {
    if (this->moves != NULL)
        free(this->moves);
    
    if (MAX_NEIGHBORHOOD_SIZE > sizeA * sizeB) {
        this->total = this->n = -1;
        this->sizeFirst = sizeA;
        this->sizeSecond = sizeB;
        this->moves = NULL;
        return;
    }
    
    this->total = this->n = 0;
    this->moves = (pair< int, int> *) malloc(sizeof(pair< int, int >) * sizeA * sizeB);
    
    for (int i = 0; i < sizeA; i++) {
        for (int j = 0; j < sizeB; j++) {
            this->moves[this->n].first = i;
            this->moves[this->n].second = j;
            
            this->total++; this->n++;
        }
    }
}

void MoveRealloc::restart() {
    this->n = this->total;
}

pair< int, int > MoveRealloc::getMove() {
    if (this->n == -1) {
        return pair< int, int >(rand() % this->sizeFirst, rand() % this->sizeSecond);
    }

    int p = rand() % this->n;
    pair< int, int > r = this->moves[p];
    this->moves[p] = this->moves[this->n-1];
    this->moves[this->n-1] = r;
    
    this->n--;
    return r;
}


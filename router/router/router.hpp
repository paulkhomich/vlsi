//
//  router.hpp
//  router
//
//  Created by Павел Хомич on 29/05/2020.
//  Copyright © 2020 Павел Хомич. All rights reserved.
//

#ifndef router_hpp
#define router_hpp

#include <queue>
#include <string>
#include <vector>
#include <stdio.h>

enum Tag {Z, N, S, E, W, U, D};

class CellG {
public:
    int    cost;
    Tag     pred;
    bool    reached;
    
    CellG();
    CellG(int);
    
    void recost(int);
};

class Grid {
public:
    int w;
    int h;
    int bendCost;
    int viaCost;
    CellG** L1;
    CellG** L2;
    
    Grid(int, int, int, int);
    void clear();
};

class CellN {
public:
    int x;
    int y;
    bool onL1;
    
    CellN();
    CellN(int, int, bool);
};

class Net {
public:
    int id;
    CellN p1;
    CellN p2;
    std::vector<CellN> path;
    
    Net();
};

class Nets {
public:
    int size;
    Net* nets;
    
    Nets(int);
};

class CellW {
public:
    int x;
    int y;
    bool onL1;
    int pathcost;
    Tag pred;
    
    CellW();
    CellW(int, int, bool, int, Tag);
};

class CompareCellW {
public:
    bool operator() (CellW a, CellW b) {
        return a.pathcost > b.pathcost;
    }
};

typedef std::priority_queue<CellW, std::vector<CellW>, CompareCellW> Wavefront;

Grid createGrid(std::string);
Nets createNets(std::string);
void save(std::string, Nets&);

void route(Grid&, Net&);
int step(Grid&, Net&, Wavefront&); // Возвращает: Найдена ли цель?
void initWave(Grid&, Net&, Wavefront&);
void addToWave(Grid&, Wavefront&, int, int, bool, int, Tag, Tag);
void backtrace(Grid&, Net&);

#endif /* router_hpp */

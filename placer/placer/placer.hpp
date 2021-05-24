//
//  placer.hpp
//  placer
//
//  Created by Павел Хомич on 03/05/2020.
//  Copyright © 2020 Павел Хомич. All rights reserved.
//

#ifndef placer_hpp
#define placer_hpp

#include <vector>

class Gate;
class Net;

struct Pad {
    float x;
    float y;
};

class Net {
public:
    int id;
    std::vector<int> gates;
    Pad pad;
    
    Net(int);
    void addGate(int);
    void addPad(float, float);
    bool isGateInNet(int);
};

class Gate {
public:
    int id;
    int nNets;
    std::vector<Net*> nets;
    double x;
    double y;
    
    Gate(int, int);
    void addNet(Net*);
};

struct MatrixA {
    int* r;
    int* c;
    double* v;
};

struct MatrixLine {
    double* b;
};

struct compareGatesByX {
    bool operator()(const Gate& a, const Gate& b) {
        return (100000 * a.x + a.y) < (100000 * b.x + b.y);
    }
};

struct compareGatesById {
    bool operator()(const Gate& a, const Gate& b) {
        return a.id < b.id;
    }
};


void initPlacer(std::string, std::vector<Gate>&, std::vector<Net>&);
void createMatrixA(int*, int*, double*, std::vector<Gate>&, std::vector<Net>&);
void createMatrixB(double*, std::vector<Gate>&, std::vector<Net>&, bool);
void createMatrixBHalfL(double*, std::vector<Gate>&, std::vector<Gate>&, std::vector<Net>&, bool);
void createMatrixBHalfR(double*, std::vector<Gate>&, std::vector<Gate>&, std::vector<Net>&, bool);
void solve(int, int*, int*, double*, double*, double*, std::vector<Gate>&, bool);
void saveToFile(std::string, std::vector<Gate>&);

#endif /* placer_hpp */

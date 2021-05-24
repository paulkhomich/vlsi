//
//  placer.cpp
//  placer
//
//  Created by Павел Хомич on 03/05/2020.
//  Copyright © 2020 Павел Хомич. All rights reserved.
//

#include <sstream>
#include <fstream>
#include <vector>
#include <valarray>

#include "placer.hpp"
#include "solver.h"

// Net
Net::Net(int id) : id{id}, gates{}, pad{-1, -1} {}
void Net::addGate(int gId) { gates.push_back(gId); }
void Net::addPad(float x, float y) { pad = {x, y}; }
bool Net::isGateInNet(int gId) {
    for (auto& g : gates) if (g == gId) return true;
    
    return false;
};

// Gate
Gate::Gate(int id, int nNets) : id{id}, nNets{nNets}, nets{}, x{0.0}, y{0.0} {}
void Gate::addNet(Net* nId) { nets.push_back(nId); }

// Functions
void initPlacer(std::string path, std::vector<Gate>& gates, std::vector<Net>& nets) {
    int ng, nn, np; // number of Gates and Nets and Pads
    int id, nNets; // Gate id and number of its nets
    int net; // id for Net
    float x, y;
    
    // Open file
    std::ifstream input;
    input.open(path);
    
    
    // Create scanner for lines in file
    std::string s;
    std::istringstream scanner;
    getline(input, s);
    scanner.str(s);
    scanner >> ng >> nn;
    
    // Create Nets (empty)
    for (int i = 1; i <= nn; i++) nets.emplace_back(i);
    
    // Create Gates
    for (int i = 0; i < ng; i++) {
        getline(input, s);
        scanner.clear();
        scanner.str(s);
        
        scanner >> id >> nNets;
        
        gates.emplace_back(id, nNets);
    
        for (int j = 0; j < nNets; j++) {
            scanner >> net;
            
            gates[id - 1].addNet(&nets[net - 1]);
            nets[net - 1].addGate(id);
        }
    }
    
    // Pads to Nets
    getline(input, s);
    scanner.clear();
    scanner.str(s);
    scanner >> np;
    
    for (int i = 0; i < np; i++) {
        getline(input, s);
        scanner.clear();
        scanner.str(s);
        
        scanner >> id >> net >> x >> y;
        nets[net - 1].addPad(x, y);
    }
    
    input.close();
}


void createMatrixA(int* R, int* C, double* V, std::vector<Gate>& gates, std::vector<Net>& nets) {
    long n = gates.size();
    
    int i = 0;
    
    int gId, rowCount;
    bool found = false;
    for (int k = 0; k < n; k++) {
        gId = gates[k].id;
        rowCount = 0;
        for (int j = 0; j < n; j++) {
            found = false;
            if (gates[j].id == gId) {
                i++;
                continue;
            }

            for (auto& net : gates[k].nets) {
                if (net->isGateInNet(gates[j].id)) {
                    found = true;
                    rowCount++;
                    break;
                }
            }
            
            R[i] = k;
            C[i] = j;
            V[i] = found ? -1 : 0;
            i++;
        }
        
        bool foundPseudoPad = false;
        bool inGates = false;
        for (auto& net : gates[k].nets) {
            foundPseudoPad = false;
            if (net->pad.x != -1) {
                rowCount += 1;
                break;
            } else {
                for (auto& gate : net->gates) {
                    inGates = false;
                    for (auto& g2 : gates) {
                        if (g2.id == gate) {
                            inGates = true;
                            break;
                        }
                    }
                    
                    if (!inGates) {
                        rowCount += 1;
                        foundPseudoPad = true;
                        break;
                    }
                }
            }
            
            if (foundPseudoPad) break;
        }
        
        R[i - (n - k)] = k;
        C[i - (n - k)] = k;
        V[i - (n - k)] = rowCount;
    }
}

void createMatrixB(double* b, std::vector<Gate>& gates, std::vector<Net>& nets, bool forY) {
    int i = 0;
    
    bool found = false;
    for (auto& g : gates) {
        found = false;
        for (auto& net : g.nets) {
            if (net->pad.x != -1) {
                b[i] = forY ? double(net->pad.y) : double(net->pad.x);
                found = true;
                break;
            }
        }
        
        if (!found) b[i] = double(0);
        i++;
    }
}

void createMatrixBHalfL(double* b, std::vector<Gate>& gatesL, std::vector<Gate>& gatesR, std::vector<Net>& nets, bool forY) {
    int i = 0;
    double maxX = 50.0;
    // [2, 8, 9, 13, 10, 6, 7, 4, 11]
    bool found = false;
    for (auto& g : gatesL) {
        found = false;
        for (auto& net : g.nets) {
            if (net->pad.x != -1) {
                b[i] = forY ? double(net->pad.y) : (net->pad.x > maxX) ? maxX : double(net->pad.x);
                found = true;
                break;
            } else {
                for (auto& gate : net->gates) {
                    for (auto& g2 : gatesR) {
                        if (gate == g2.id) {
                            b[i] = forY ? g2.y : maxX;
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
            }
        }
        
        if (!found) b[i] = 0.0;
        i++;
    }
}

void createMatrixBHalfR(double* b, std::vector<Gate>& gatesR, std::vector<Gate>& gatesL, std::vector<Net>& nets, bool forY) {
    int i = 0;
    double maxX = 50.0;

    bool found = false;
    for (auto& g : gatesR) {
        found = false;
        for (auto& net : g.nets) {
            if (net->pad.x != -1) {
                b[i] = forY ? double(net->pad.y) : (net->pad.x < maxX) ? maxX : double(net->pad.x);
                found = true;
                break;
            } else {
                for (auto& gate : net->gates) {
                    for (auto& g2 : gatesL) {
                        if (gate == g2.id) {
                            b[i] = forY ? g2.y : maxX;
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
            }
        }
        
        if (!found) b[i] = 0.0;
        i++;
    }
}

void solve(int size, int* R, int* C, double* V, double* mb, double* mx, std::vector<Gate>& gates, bool forY) {
    coo_matrix A;
    A.n = size;
    A.nnz = size*size;
    
    A.row.resize(A.nnz);
    A.col.resize(A.nnz);
    A.dat.resize(A.nnz);
    
    A.row = valarray<int>(R, A.nnz);
    A.col = valarray<int>(C, A.nnz);
    A.dat = valarray<double>(V, A.nnz);
    
    valarray<double> x(1.0, A.n);
    valarray<double> b(mb, A.n);
    
    A.solve(b, x);
    
    std::copy(begin(x), end(x), mx);

    for (int i = 0; i < gates.size(); ++i) {
        if (forY) gates[i].y = mx[i];
        else gates[i].x = mx[i];
    }
}

void saveToFile(std::string path, std::vector<Gate>& gates) {
    std::ofstream output;
    output.open(path);
    
    for (auto& g : gates) {
        output << g.id << " " << (g.x < 0.0 ? 0.0 : g.x) << " " << (g.y < 0.0 ? 0.0 : g.y) << "\n";
    }
    
    output.close();
}

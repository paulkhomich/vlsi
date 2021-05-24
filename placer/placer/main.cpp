//
//  main.cpp
//  placer
//
//  Created by Павел Хомич on 03/05/2020.
//  Copyright © 2020 Павел Хомич. All rights reserved.
//

#include <iostream>
#include "placer.hpp"

using namespace std;

int main() {
    vector<Gate> gates;
    vector<Net> nets;
    initPlacer("/Users/paulkhomich/Documents/cpp/placer/placer/benchmark/biomed", gates, nets);
    
    int size = int(gates.size());
    
    int* R = new int[size*size];
    int* C = new int[size*size];
    double* V = new double[size*size];
    double Bx[size], By[size], X[size], Y[size];
    
    
    createMatrixA(R, C, V, gates, nets);
    createMatrixB(Bx, gates, nets, false);
    createMatrixB(By, gates, nets, true);

    
    
    solve(size, R, C, V, Bx, X, gates, false);
    solve(size, R, C, V, By, Y, gates, true);

    sort(gates.begin(), gates.end(), compareGatesByX());
    vector<Gate> gatesLeft(gates.begin(), gates.begin() + (gates.size()/2));
    vector<Gate> gatesRight(gates.begin() + (gates.size()/2), gates.end());

    // [2, 8, 9, 13, 10, 6, 7, 4, 11]
    // [8, 2, 9, 6, 4, 7, 10, 13, 18]
    size = int(gatesLeft.size());

    int* RL = new int[size*size];
    int* CL = new int[size*size];
    double* VL = new double[size*size];
    double BxL[size], ByL[size], XL[size], YL[size];

    createMatrixA(RL, CL, VL, gatesLeft, nets);
    createMatrixBHalfL(BxL, gatesLeft, gatesRight, nets, false);
    createMatrixBHalfL(ByL, gatesLeft, gatesRight, nets, true);


    solve(size, RL, CL, VL, BxL, XL, gatesLeft, false);
    solve(size, RL, CL, VL, ByL, YL, gatesLeft, true);
    
//    for (auto& g : gates) {
//        cout << "@: " << g.id << endl;
//    }
//
//    for (int i = 0; i < size*size; i++) {
//        cout << RL[i] << ":" << CL[i] << " = " << VL[i] << endl;
//    }
//
//    for (int i = 0; i < size; i++) {
//        cout << BxL[i] << " # " << ByL[i] << endl;
//    }

    size = int(gatesRight.size());

    int* RR = new int[size*size];
    int* CR = new int[size*size];
    double* VR = new double[size*size];
    double BxR[size], ByR[size], XR[size], YR[size];

    createMatrixA(RR, CR, VR, gatesRight, nets);
    createMatrixBHalfR(BxR, gatesRight, gatesLeft, nets, false);
    createMatrixBHalfR(ByR, gatesRight, gatesLeft, nets, true);

    // [5, 12, 15, 18, 16, 17, 14, 3, 1]
//
//    for (int i = 0; i < size*size; ++i) {
//        cout << RR[i] << ":" << CR[i] << " = " << VR[i] << endl;
//    }
//    for (int i = 0; i < size; i++) {
//        cout << BxR[i] << endl;
//    }

    solve(size, RR, CR, VR, BxR, XR, gatesRight, false);
    solve(size, RR, CR, VR, ByR, YR, gatesRight, true);


    gatesLeft.insert(gatesLeft.end(), gatesRight.begin(), gatesRight.end());
    sort(gatesLeft.begin(), gatesLeft.end(), compareGatesById());
    
    
    saveToFile("/Users/paulkhomich/Documents/cpp/placer/placer/placement/biomed", gatesLeft);
    
    return 0;
}

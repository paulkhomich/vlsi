//
//  router.cpp
//  router
//
//  Created by Павел Хомич on 29/05/2020.
//  Copyright © 2020 Павел Хомич. All rights reserved.
//

#include "router.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

CellG::CellG(int cost) : cost(cost), pred(Z), reached(false) {};
CellG::CellG() : cost(1), pred(Z), reached(false) {};
void CellG::recost(int newCost) { cost = newCost; };

Grid::Grid(int w, int h, int bc, int vc) : w(w), h(h), bendCost(bc), viaCost(vc) {
    L1 = new CellG* [h];
    L2 = new CellG* [h];
    
    for (int i = 0; i < h; i++) {
        L1[i] = new CellG[w];
        L2[i] = new CellG[w];
    }
};
void Grid::clear() {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            L1[y][x].reached = false;
            L1[y][x].pred = Z;
            
            L2[y][x].reached = false;
            L2[y][x].pred = Z;
        }
    }
}

CellW::CellW() : x(0), y(0), onL1(true), pathcost(0), pred(Z) {};
CellW::CellW(int x, int y, bool onL1, int pathcost, Tag pred) : x(x), y(y), onL1(onL1), pathcost(pathcost), pred(pred) {};

CellN::CellN() : x(0), y(0), onL1(true) {};
CellN::CellN(int x, int y, bool onL1) : x(x), y(y), onL1(onL1) {};

Net::Net() : id(0), p1(CellN()), p2(CellN()) {};
Nets::Nets(int size) : size(size), nets(new Net[size]) {};

Grid createGrid(std::string path) {
    int w, h, bc, vc;
    
    std::ifstream input;
    input.open(path);
    std::string s;
    std::istringstream scanner;
    
    getline(input, s);
    scanner.str(s);
    scanner >> w >> h >> bc >> vc;
    
    Grid grid(w,h,bc,vc);
    
    int newCost;
    // For L1
    for (int i = 0; i < h; i++) {
        getline(input, s);
        scanner.clear();
        scanner.str(s);
        
        for (int j = 0; j < w; j++) {
            scanner >> newCost;
            grid.L1[i][j].recost(newCost);
        }
    }
    // For L2 (if exist)
    if (!getline(input, s)) return std::move(grid);
    
    for (int i = 0; i < h; i++) {
        scanner.clear();
        scanner.str(s);
        getline(input, s);
        
        for (int j = 0; j < w; j++) {
            scanner >> newCost;
            grid.L2[i][j].recost(newCost);
        }
    }
    
    return std::move(grid);
}

Nets createNets(std::string path) {
    int size;
    
    std::ifstream input;
    input.open(path);
    std::string s;
    std::istringstream scanner;
    
    getline(input, s);
    scanner.str(s);
    scanner >> size;
    
    Nets nets(size);
    
    int id, l1, x1, y1, l2, x2, y2;
    for (int i = 0; i < size; i++) {
        getline(input, s);
        scanner.clear();
        scanner.str(s);
        
        scanner >> id >> l1 >> x1 >> y1 >> l2 >> x2 >> y2;
        
        nets.nets[i].id = id;
        nets.nets[i].p1.onL1 = l1 == 1;
        nets.nets[i].p1.x = x1;
        nets.nets[i].p1.y = y1;
        nets.nets[i].p2.onL1 = l2 == 1;
        nets.nets[i].p2.x = x2;
        nets.nets[i].p2.y = y2;
    }
    
    return std::move(nets);
}

void save(std::string path, Nets& nets) {
    std::ofstream output;
    output.open(path);
    
    output << nets.size << std::endl;
    
    for (int i = 0; i < nets.size; i++) {
        Net& n = nets.nets[i];
        output << n.id << std::endl; // Начало сети
        
        for (int j = n.path.size() - 1; j >= 0; j--) {
            output << (n.path[j].onL1 ? "1 " : "2 ") << n.path[j].x << " " << n.path[j].y << " " << std::endl;
        }
        
        output << 0 << std::endl; // Конец сети
    }
}

void route(Grid& grid, Net& net) {
    Wavefront wave;
    initWave(grid, net, wave);
    
    int status = 0;
    while (!status && !wave.empty()) {
        status = step(grid, net, wave);
    }
    
    if (status == 1) backtrace(grid, net); // backtrace
    
    grid.clear();
}

int step(Grid& grid, Net& net, Wavefront& wave) {
    CellW top = wave.top();
    
    bool targetFound = false;
    
    // Вверх
    if (top.y <= grid.h - 2) {
        if (top.onL1 && !grid.L1[top.y+1][top.x].reached && grid.L1[top.y+1][top.x].cost > 0) {
            addToWave(grid, wave, top.x, (top.y+1), top.onL1, top.pathcost, S, top.pred);
            if (net.p2.y == top.y + 1 && net.p2.x == top.x) targetFound = true;
        } else if (!top.onL1 && !grid.L2[top.y+1][top.x].reached && grid.L2[top.y+1][top.x].cost > 0) {
            addToWave(grid, wave, top.x, (top.y+1), top.onL1, top.pathcost, S, top.pred);
            if (net.p2.y == top.y + 1 && net.p2.x == top.x) targetFound = true;
        }
    }
    // Вниз
    if (top.y > 0) {
        if (top.onL1 && !grid.L1[top.y-1][top.x].reached && grid.L1[top.y-1][top.x].cost > 0) {
            addToWave(grid, wave, top.x, (top.y-1), top.onL1, top.pathcost, N, top.pred);
            if (net.p2.y == top.y - 1 && net.p2.x == top.x) targetFound = true;
        } else if (!top.onL1 && !grid.L2[top.y-1][top.x].reached && grid.L2[top.y-1][top.x].cost > 0) {
            addToWave(grid, wave, top.x, (top.y-1), top.onL1, top.pathcost, N, top.pred);
            if (net.p2.y == top.y - 1 && net.p2.x == top.x) targetFound = true;
        }
    }
    // Вправо
    if (top.x <= grid.w - 2) {
        if (top.onL1 && !grid.L1[top.y][top.x+1].reached && grid.L1[top.y][top.x+1].cost > 0) {
            addToWave(grid, wave, (top.x+1), top.y, top.onL1, top.pathcost, W, top.pred);
            if (net.p2.y == top.y && net.p2.x == top.x + 1) targetFound = true;
        } else if (!top.onL1 && !grid.L2[top.y][top.x+1].reached && grid.L2[top.y][top.x+1].cost > 0) {
            addToWave(grid, wave, (top.x+1), top.y, top.onL1, top.pathcost, W, top.pred);
            if (net.p2.y == top.y && net.p2.x == top.x + 1) targetFound = true;
        }
    }
    // Влево
    if (top.x > 0) {
        if (top.onL1 && !grid.L1[top.y][top.x-1].reached && grid.L1[top.y][top.x-1].cost > 0) {
            addToWave(grid, wave, (top.x-1), top.y, top.onL1, top.pathcost, E, top.pred);
            if (net.p2.y == top.y && net.p2.x == top.x - 1) targetFound = true;
        } else if (!top.onL1 && !grid.L2[top.y][top.x-1].reached && grid.L2[top.y][top.x-1].cost > 0) {
            addToWave(grid, wave, (top.x-1), top.y, top.onL1, top.pathcost, E, top.pred);
            if (net.p2.y == top.y && net.p2.x == top.x - 1) targetFound = true;
        }
    }

    wave.pop();
    
    return targetFound ? 1 : 0;
}

void initWave(Grid& grid, Net& net, Wavefront& wave) {
    int x = net.p1.x;
    int y = net.p1.y;
    
    if (net.p1.onL1)    grid.L1[y][x].reached = true;
    else                grid.L2[y][x].reached = true;
    
    wave.push({
        x,
        y,
        net.p1.onL1,
        net.p1.onL1 ? grid.L1[y][x].cost : grid.L2[y][x].cost, // Внимательно с y x
        Z
    });
}

void addToWave(Grid& grid, Wavefront& wave, int x, int y, bool onL1, int pathcost, Tag tag, Tag beforeTag) {
    if (onL1) {
        grid.L1[y][x].reached = true;
        grid.L1[y][x].pred = tag;
    } else {
        grid.L2[y][x].reached = true;
        grid.L2[y][x].pred = tag;
    }
    
    bool isBended = beforeTag != Z && tag != beforeTag;
    
    wave.push({
        x,
        y,
        onL1,
        pathcost + (onL1 ? grid.L1[y][x].cost : grid.L2[y][x].cost) + (isBended ? grid.bendCost : 0),
        tag
    });
}

void backtrace(Grid& grid, Net& net) {
    Tag searchTag = S; // S?
    int x = net.p2.x;
    int y = net.p2.y;
    bool onL1 = net.p2.onL1;
    CellG cell = onL1 ? grid.L1[y][x] : grid.L2[y][x];
    
    while (searchTag != Z) {
        net.path.emplace_back(x, y, onL1);

        if (onL1)   grid.L1[y][x].cost = -1;
        else        grid.L2[y][x].cost = -1;

        if (cell.pred == N) {
            y += 1;
            cell = onL1 ? grid.L1[y][x] : grid.L2[y][x];
            searchTag = cell.pred;
        } else if (cell.pred == S) {
            y -= 1;
            cell = onL1 ? grid.L1[y][x] : grid.L2[y][x];
            searchTag = cell.pred;
        } else if (cell.pred == W) {
            x -= 1;
            cell = onL1 ? grid.L1[y][x] : grid.L2[y][x];
            searchTag = cell.pred;
        } else {
            x += 1;
            cell = onL1 ? grid.L1[y][x] : grid.L2[y][x];
            searchTag = cell.pred;
        }
    }
    
    // И первого
    x = net.p1.x;
    y = net.p1.y;
    
    net.path.emplace_back(x, y, onL1);
}

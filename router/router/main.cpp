//
//  main.cpp
//  router
//
//  Created by Павел Хомич on 29/05/2020.
//  Copyright © 2020 Павел Хомич. All rights reserved.
//

#include <iostream>
#include "router.hpp"

using namespace std;

int main() {
    Grid grid = createGrid("/Users/paulkhomich/Documents/cpp/router/router/benchmark/bench3.grid");
    Nets nets = createNets("/Users/paulkhomich/Documents/cpp/router/router/benchmark/bench3.nl");
    
    // Провести каждую сеть
    for (int i = 0; i < nets.size; i++) route(grid, nets.nets[i]);

    save("/Users/paulkhomich/Documents/cpp/router/router/routers/bench3", nets);
    
    return 0;
}

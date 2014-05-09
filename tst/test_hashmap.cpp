#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cstdlib>
#include <ios>
#include <iomanip>
#include <vector>
#include <time.h>
#include <cmath>
#include <functional>

#include "../src/list.h"
#include "../src/coarse_grained_list.h"
#include "../src/fine_grained_list.h"
#include "../src/lock_free_list.h"
#include "../src/coarse_hashmap.h"

using namespace std;

int main(int argc, char** argv)
{
   // CoarseHashMap<int, int, std::hash<int> >* set = new CoarseHashMap<int, int, std::hash<int> >(100);
   // set->insert(5, 3);
    CoarseHashMap<int, int, std::hash<int> > list;
    return 0;
}
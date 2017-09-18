#include <iostream>
#include <vector>
#include <string>
#include "dualslope.h"

using namespace std;
using namespace seal;

int main()
{
    Timing t_all("Overall");
    t_all.start();
    Errors e("testing");
    
    bool debug = false;

    vector<double> samples = get_samples_from_file("MIT_BIH_Records/100_1s.txt", 2, debug);

    if (debug){ 
        print_banner("We bout to run the test, y'all!");
    }

    // Create and run DUALSLOPE algorithm
    DUALSLOPE ds = DUALSLOPE(samples, 360, debug); // Sampling Frequency of MIT-BIH Arrhythmia db = 360HZ
    e = ds.test();
    e.display();

    t_all.end();

    return 0;
}

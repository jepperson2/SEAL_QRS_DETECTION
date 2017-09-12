#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "seal.h"
#include "dualslope.h"

using namespace std;
using namespace seal;

int main()
{
    // Hard-coded ecg voltages from sample 100's first 47 samples. Example of a single calculation
    // TODO - Implement read from file for samples 
    const vector<double> ecg_voltages { -0.145,-0.12,-0.135,-0.145,-0.15,-0.16,-0.155,-0.16,-0.175,-0.18,-0.185,-0.17,-0.155,-0.175,-0.18,-0.19,-0.18,-0.155,-0.135,-0.155,-0.19,-0.205,-0.235,-0.225,-0.245,-0.25,-0.26,-0.275,-0.275,-0.275,-0.265,-0.255,-0.265,-0.275,-0.29,-0.29,-0.29,-0.29,-0.285,-0.295,-0.305,-0.285,-0.275,-0.275,-0.28,-0.285,-0.305 }; // First 47 samples from MIT-BIH sample 100
    
    const vector<double> ecg_voltages_extended { -0.145,-0.12,-0.135,-0.145,-0.15,-0.16,-0.155,-0.16,-0.175,-0.18,-0.185,-0.17,-0.155,-0.175,-0.18,-0.19,-0.18,-0.155,-0.135,-0.155,-0.19,-0.205,-0.235,-0.225,-0.245,-0.25,-0.26,-0.275,-0.275,-0.275,-0.265,-0.255,-0.265,-0.275,-0.29,-0.29,-0.29,-0.29,-0.285,-0.295,-0.305,-0.285,-0.275,-0.275,-0.28,-0.285,-0.305,-0.29,-0.3,-0.28,-0.29 }; // First 51 samples from MIT-BIH sample 100. For testing fs = 400

    bool debug = false;

    Timing t_all("Overall");
    t_all.start();
    Errors e("testing");
    
    if (debug){ 
        print_banner("We bout to run the test, y'all!");
    }

    // Create and run DUALSLOPE algorithm
    DUALSLOPE ds = DUALSLOPE(ecg_voltages, 360, debug); // Sampling Frequency of MIT-BIH Arrhythmia db = 360HZ
    e = ds.test();
    e.display();

    t_all.end();

    return 0;
}

#ifndef DUALSLOPE_H
#define DUALSLOPE_H
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <math.h>
#include "seal.h"
#include "he.h"
#include "helper_functions.h"

using namespace std;
using namespace seal;
using namespace seal::util;

class DUALSLOPE{
    public:
        DUALSLOPE(vector<double> inputs, int sampling_frequency, bool dbg);
        Errors test();

    private:
        const string className();
        bool debug;
        HE he;
        Timing t;

        int a;                  // minimum distance away from considered sample = 0.027*fs
        int b;                  // maximum distance away from considered sample = 0.063*fs
        int n_samples;          // number of samples processed at one time
        int lr_size;            // size of "window" of values being compared to considered sample

        int fs;                 // Sampling Frequency
        vector<double> samples;
        vector<double> sample_difference_widths;

        double diff_threshold;  // Theta_diff
        double min_threshold;   // Theta_min
        double avg_height;      // H_ave

        EncryptionParameters parms;
        MemoryPoolHandle pool = MemoryPoolHandle::acquire_new();

        void set_parms();
        void initialize();
        void t_start();
        void t_end(string name);
        
        vector<Ciphertext> find_qrs();
        vector<Ciphertext> find_qrs(int index_a, int index_b); 

        void calculate_lr_slopes();
        void calculate_lr_minmax();
        void compare_slopes2thresholds();
        void verify_peak();

        bool test_find_qrs();
        bool test_find_qrs(int index_a, int index_b);
};

#endif

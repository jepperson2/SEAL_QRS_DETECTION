#ifndef DUALSLOPE_H
#define DUALSLOPE_H
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "seal.h"
#include "he.h"
#include "helper_functions.h"

using namespace std;
using namespace seal;
using namespace seal::util;

class DUALSLOPE{
    public:
        DUALSLOPE(vector<double> inputs, vector<double> given_thresholds, bool dbg);
        Errors test();

    private:
        const string className();
        bool debug;
        unsigned N_samples;
        HE he;
        Timing t;

        EncryptionParameters parms;
        MemoryPoolHandle pool = MemoryPoolHandle::acquire_new();

        vector<double> samples;
        vector<double> thresholds;
        vector<double> sample_difference_widths;

        double avg_height;

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

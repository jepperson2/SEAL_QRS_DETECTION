#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H
#include <sstream> //string
#include <stdlib.h> //rand
#include <vector>
#include <iostream>
#include <ctime>

using namespace std;

typedef string mkt; // Map Key Type - Key to access a ciphertext in the map
string generate_string(int length);

void print_banner(string title);

class Timing{
        public:
            Timing();
            Timing(string t);
            void start();
            void end();
            double end(string silent);
        private:
            clock_t a,b;
            double duration;
            unsigned measure_id;
            string title;
};

class Errors{
        public:
            Errors(string t);
            void add(string name, bool error);
            void display();
        private:
            string title;
            vector<string> names;
            vector<bool> errors;
};

#endif

#include "helper_functions.h"

string generate_string(int length){
    const string alphanum = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    //"0123456789!@#$%^&*ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    string s;

    for(unsigned int i = 0; i < length; ++i){
        s += alphanum[rand() % (alphanum.size() - 1)];
    }
    return s;
}

void print_banner(string title){
    if (!title.empty()){
        size_t title_length = title.length();
        size_t banner_length = title_length + 2 + 2 * 10;
        string banner_top(banner_length, '*');
        string banner_middle = string(10, '*') + " " + title + " " + string(10, '*');

        cout << endl
            << banner_top << endl
            << banner_middle << endl
            << banner_top << endl
            << endl;
    }
}

Errors::Errors(string t){
    title = t;
}

void Errors::add(string name, bool error){
    names.push_back(name);
    errors.push_back(error);
}

void Errors::display(){
    bool no_error = true;
    for (int i = 0; i < errors.size(); i++){
        if (errors[i]){ 
            no_error = false;
            cout << title << ": Error occured for test " << names[i] << endl;
        } 
    }

    if (no_error){    
        cout << title << ": ALL TESTS PASSED" << endl;
    }   
}

Timing::Timing(){
    title = "";
    measure_id = 0;
}

Timing::Timing(string t){
    title = t + ": ";
    measure_id = 0;
}

void Timing::start(){
    measure_id++;
    a = clock();
}

void Timing::end(){
    b = clock();
    duration = double(b - a)/CLOCKS_PER_SEC;
    cout << title << "Duration of measurement " << measure_id << ": ";
    cout << duration << " seconds" << endl;
}

double Timing::end(string silent){
    b = clock();
    duration = double(b - a)/CLOCKS_PER_SEC;
    return duration;
}

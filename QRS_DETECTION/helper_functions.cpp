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

vector<double> get_samples_from_file(string filename, int channel, bool debug){
        
    ifstream infile(filename);

    string file_heading, file_units;

    vector<int> sample_numbers;
    vector<double> channel_1_values;
    vector<double> channel_2_values;

    int tmp_samp_num;
    double tmp_chan_1_val;
    double tmp_chan_2_val;

    getline(infile, file_heading);
    getline(infile, file_units);

    while (infile >> tmp_samp_num >> tmp_chan_1_val >> tmp_chan_2_val){
        sample_numbers.push_back(tmp_samp_num);
        channel_1_values.push_back(tmp_chan_1_val);
        channel_2_values.push_back(tmp_chan_2_val);
    }   
        
    if (debug){
        cout << file_heading << endl;
        cout << file_units << endl;

        for (int i = 0; i < sample_numbers.size(); i++){
            cout << sample_numbers[i] << "\t\t" << channel_1_values[i] << "\t" << channel_2_values[i] << endl;
        }   
    }   

    if (channel == 1){ 
        return channel_1_values;
    } else {
        return channel_2_values;
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

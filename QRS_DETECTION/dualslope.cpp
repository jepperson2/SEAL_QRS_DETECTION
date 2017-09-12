#include "dualslope.h"

DUALSLOPE::DUALSLOPE(vector<double> inputs, int sampling_frequency, bool dbg){
    samples = inputs;
    fs = sampling_frequency;
    debug = dbg;

    initialize();
    set_parms();
}

const string DUALSLOPE::className(){
    return "DUALSLOPE";
}

void DUALSLOPE::t_start(){
    if (debug){
        t.start();
    }
}

void DUALSLOPE::t_end(string name){
    if (debug){
        double duration = t.end("silent");
        cout << className() << ": " << name << " completed in " << duration << " seconds" << endl;
    }
}

Errors DUALSLOPE::test(){
    Errors e("DUALSLOPE");
    e.add("Find QRS", test_find_qrs());
 //   e.add("Find QRS between a and b", test_find_qrs(5,51));
    return e;
}

void DUALSLOPE::initialize(){
    if (debug){
        cout << className() << ": Initializing..." << endl;
    }
    double double_a = 0.027 * fs;   // 0.027 set as in Wang's original paper
    a = round(double_a);
    double double_b = 0.063 * fs;   // 0.063 set as in Wang's original paper
    b = round(double_b);
    
    n_samples = (2 * b) + 1;        // Number of samples considered in one pass of the algorithm (goes from 0 to -2b)
    lr_size = (b - a) + 1;              // convenience for looping through each side 

    // Constant values of 1/a, 1/(a+1), ... , 1/b. These are the values of the "run" in the rise over run calculation of slope
    double width;
    double rounded_width;

    for (double i = a; i < (b + 1); i++){
        width = 1/i;
        rounded_width = round(100000 * width)/100000;

        sample_difference_widths.push_back(rounded_width);
    }

    avg_height = 0.0; // Set initial height to 0. 
    he.debug_on(debug);
    extern MemoryPoolHandle pool;

    if (debug){
        cout << "a,b = " << a << "," << b << endl;
        cout << "n_samples = " << n_samples << endl;
        cout << "lr_size = " << lr_size << endl;

        for (int i = 0; i < lr_size; i++){
            cout << "sample_difference_widths[" << i << "]: " << sample_difference_widths[i] << endl;
        }
    }
}

void DUALSLOPE::set_parms(){
    if (debug){
        cout << "setting parms..." << endl;
    }
    parms.set_poly_modulus("1x^2048 + 1");
    parms.set_coeff_modulus(ChooserEvaluator::default_parameter_options().at(2048));
    parms.set_plain_modulus(1 << 8);
    parms.validate();

    if (debug){
        cout << "parms.poly_modulus(): " << parms.poly_modulus().to_string() << endl;
        cout << "parms.coeff_modulus(): " << parms.coeff_modulus().to_string() << endl;
        cout << "parms.plain_modulus(): " << parms.plain_modulus().to_string() << endl;
    }
}

bool DUALSLOPE::test_find_qrs(){
    t_start();
   // vector<Ciphertext> results = find_qrs();
    calculate_lr_slopes();
    t_end(__FUNCTION__);

    // TODO - decrypt results and check against plaintext computation
    /* 
        vector<int> locations = find_qrs_plain();
        vector<int> FHE_calculated_locations; 
        for (int i = 0; i < locations.size(); i++){
            if (locations[i] != FHE_calculated_locations[i]){
                return true;
            }
        }
    */ 
    return false;
}

bool DUALSLOPE::test_find_qrs(int index_a, int index_b){
    t_start();
    vector<Ciphertext> results = find_qrs(index_a, index_b);
    t_end(__FUNCTION__);

    // TODO - decrypt results and check against plaintext computation
    /* 
        vector<int> locations = find_qrs_plain(index_a, index_b);
        vector<int> FHE_calculated_locations; 
        for (int i = 0; i < locations.size(); i++){
            if (locations[i] != FHE_calculated_locations[i]){
                return true;
            }
        }
    */ 
    return false;
}

vector<Ciphertext> DUALSLOPE::find_qrs(){
    t_start();
    vector<Ciphertext> qrs_locations; 
   
    if (debug){
        cout << "Beginning to find QRS locations... " << endl; 
        cout << "Number of samples considered per iteration = " << n_samples << endl;

        for (int i = 0; i < n_samples; i++){
            cout << "samples[" << i << "]: " << samples[i] << endl;
        }
    } 
	t_end(__FUNCTION__);

	return qrs_locations;
}

vector<Ciphertext> DUALSLOPE::find_qrs(int index_a, int index_b){
    t_start();
    vector<Ciphertext> qrs_locations;

    if (debug){
        cout << "Beginning to find QRS locations between sample " << index_a << " and sample " << index_b << "... " << endl; 
        cout << "Number of samples considered per iteration = " << n_samples << endl;

        for (int i = 0; i < n_samples; i++){
            cout << "samples[" << i << "]: " << samples[i] << endl;
        }
    } 
    
    t_end(__FUNCTION__);
    return qrs_locations;
}
   
void DUALSLOPE::calculate_lr_slopes(){
   	if (debug){
        print_banner("Calculate LR Slopes");
    }

    // Generate keys
    if (debug){
        cout << "Generating keys ..." << endl;
    }
    KeyGenerator generator(parms);
    generator.generate();
    if (debug){
        cout << "... key generation complete." << endl;
    }
    Ciphertext public_key = generator.public_key();
    Plaintext secret_key = generator.secret_key();

    /* 
     Create fractional encoder. Here using 64 coefficients of the polynomial for the 
     integral part (low-degree terms) and expand the fractional part to 32 terms of 
     precision (base 3) (high-degree terms).
    */

    FractionalEncoder encoder(parms.plain_modulus(), parms.poly_modulus(), 64, 32, 3);

    // Create the rest of the tools
    Encryptor encryptor(parms, public_key);
    Evaluator evaluator(parms);
    Decryptor decryptor(parms, secret_key);

    // Debugging - print out samples and sample_difference_widths
    if (debug){
        cout << "Printing samples ..." << endl;
        for (int i = 0; i < n_samples; i++){
            cout << to_string(samples[i]).substr(0,6) << ((i != (n_samples - 1)) ? ", " : ".\n");
        }
        cout << endl << "Printing sample_difference_widths ..." << endl;
        for (int i = 0; i < lr_size; i++){
            cout << to_string(sample_difference_widths[i]).substr(0,7) << ((i != (lr_size - 1)) ? ", " : ".\n");
        }
    }

    // First encrypt the samples
    if (debug){
        cout << "Encrypting samples ..." << endl;
    }
    vector<Ciphertext> encrypted_samples;
    for (int i = 0; i < n_samples; i++){   
        Plaintext encoded_number = encoder.encode(samples[i]);
        encrypted_samples.emplace_back(encryptor.encrypt(encoded_number));
    }

    // Next we encode the sample_difference_widths. There is no reason to encrypt these since they are not private data. 
    if (debug){
        cout << "Encoding ... " << endl;
    }
    vector<Plaintext> encoded_sample_widths;
    for (int i = 0; i < lr_size; i++){
        encoded_sample_widths.emplace_back(encoder.encode(sample_difference_widths[i]));
    }

    /* 
    Next calculate the slopes between sample z=-b and samples z=-(b-a)...0 to calculate SL min/max
    also calculate the slopes between sample z=-b and samples z=-(b+a)...-2b to calculate SR min/max 
    */

    if (debug){
        cout << "Computing slopes for SL ... " << endl;
    }

    vector<Ciphertext> encrypted_SL_slopes;
    vector<double> unencrypted_SL_slopes(lr_size);

    for (int i = 0; i < lr_size; i++){
        // Offset because 0th sample is actually most recent = encrypted_samples[2b], sample b = mid point
        Ciphertext volt_diff = evaluator.sub(encrypted_samples[b], encrypted_samples[i+(b+a)]); 

        // We use multiply_plain instead of multiply because sample widths aren't encrypted. Less noise growth.
        encrypted_SL_slopes.emplace_back(evaluator.multiply_plain(volt_diff, encoded_sample_widths[i]));
    }
    if (debug){
        for (int i = 0; i < lr_size; i++){
            double res = samples[b] - samples[(b+a)+i];
            res *= sample_difference_widths[i];
            unencrypted_SL_slopes[i] = res;
            cout << "unencrypted_SL_slopes[" << i << "]: " << unencrypted_SL_slopes[i] << endl;
        }
    }

    if (debug){
        cout << "Computing slopes for SR ... " << endl;
    }

    vector<Ciphertext> encrypted_SR_slopes;
    vector<double> unencrypted_SR_slopes(lr_size);

    for (int i = 0; i < lr_size; i++){
        // Offset because 0th sample is actually most recent = encrypted_samples[2b], sample b = mid point
        Ciphertext volt_diff = evaluator.sub(encrypted_samples[b], encrypted_samples[(b-a)-i]);

        // We use multiply_plain instead of multiply because sample widths aren't encrypted. Less noise growth.
        encrypted_SR_slopes.emplace_back(evaluator.multiply_plain(volt_diff, encoded_sample_widths[i]));
    }
    if (debug){
        for (int i = 0; i < lr_size; i++){
            double res = samples[b] - samples[(b-a)-i];
            res *= sample_difference_widths[i];
            unencrypted_SR_slopes[i] = res;
            cout << "unencrypted_SR_slopes[" << i << "]: " << unencrypted_SR_slopes[i] << endl;
        }
    }

	/***************************Practicing decryption/decoding; it works!***************************

    vector<Plaintext> plain_SL_slopes;
    vector<Plaintext> plain_SR_slopes;

    vector<double> SL_slopes(lr_size);
    vector<double> SR_slopes(lr_size);

    for (int i = 0; i < lr_size; i++)
    {
        // Decrypt
        plain_SL_slopes.emplace_back(decryptor.decrypt(encrypted_SL_slopes[i]));
        plain_SR_slopes.emplace_back(decryptor.decrypt(encrypted_SR_slopes[i]));

        // Decode
        SL_slopes[i] = encoder.decode(plain_SL_slopes[i]);
        SR_slopes[i] = encoder.decode(plain_SR_slopes[i]);
        cout << "SL_slopes[" << i << "], SR_slopes[" << i << "]: " << SL_slopes[i] << ", " << SR_slopes[i] << endl;
    }
    */

    if (debug){
	    cout << "we got to the end of slope calculations!" << endl;
    }
}

void DUALSLOPE::calculate_lr_minmax(){
    if (debug){
        print_banner("Calculating LR Min/Max");
    }
}

void DUALSLOPE::compare_slopes2thresholds(){
    if (debug){
        print_banner("Comparing Slopes to Thresholds");
    }
}

void DUALSLOPE::verify_peak(){
    if (debug){
        print_banner("Verifying Peaks");
    }
}

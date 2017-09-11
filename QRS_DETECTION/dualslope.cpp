#include "dualslope.h"

DUALSLOPE::DUALSLOPE(vector<double> inputs, vector<double> given_thresholds, bool dbg){
    samples = inputs;
    thresholds = given_thresholds; 
    debug = dbg;
    set_parms();
    initialize();
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
    e.add("Find QRS between a and b", test_find_qrs(5,124));
    return e;
}

void DUALSLOPE::set_parms(){
    cout << "setting parms..." << endl;
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

void DUALSLOPE::initialize(){
    cout << className() << ": Initializing..." << endl;

    N_samples = 47; // Set number given that fs = 360 samples/sec for MIT-BIH database.
    avg_height = 0.0; // Set initial height to 0. 
    he.debug_on(debug);

    // Constant values of 1/10, 1/11, ... , 1/23. These are the values of the "run" in the rise over run calculation of slope
    sample_difference_widths = { 0.10000,0.09091,0.08333,0.07692,0.07143,0.06667,0.06250,0.05882,0.05556,0.05263,0.05000,0.04762,0.04545,0.04348 };
    
    extern MemoryPoolHandle pool;
}

bool DUALSLOPE::test_find_qrs(){
    t_start();
    vector<Ciphertext> results = find_qrs();
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
        cout << "Number of samples = " << N_samples << endl;

        for (int i = 0; i < N_samples; i++){
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
        cout << "Number of samples = " << N_samples << endl;

        for (int i = 0; i < N_samples; i++){
            cout << "samples[" << i << "]: " << samples[i] << endl;
        }
    } 
    
    t_end(__FUNCTION__);
    return qrs_locations;
}
   
void DUALSLOPE::calculate_lr_slopes(){
   	print_banner("Calculate LR Slopes");

 /*   // Create encryption parameters
    EncryptionParameters parms; 
    parms.set_poly_modulus("1x^2048 + 1");
    parms.set_coeff_modulus(ChooserEvaluator::default_parameter_options().at(2048));
    parms.set_plain_modulus(1 << 8);
    parms.validate();*/

    // Generate keys
    cout << "Generating keys ..." << endl;
    KeyGenerator generator(parms);
    generator.generate();
    cout << "... key generation complete." << endl;
    Ciphertext public_key = generator.public_key();
    Plaintext secret_key = generator.secret_key();

    /* 
     Create fractional encoder. Here using 64 coefficients of the polynomial for the 
     integral part (low-degree terms) and expand the fractional part to 32 terms of 
     precision (base 3)(high-degree terms).
    */

    FractionalEncoder encoder(parms.plain_modulus(), parms.poly_modulus(), 64, 32, 3);

    // Create the rest of the tools
    Encryptor encryptor(parms, public_key);
    Evaluator evaluator(parms);
    Decryptor decryptor(parms, secret_key);

    // First encrypt the samples
    cout << "Encrypting ..." << endl;
    vector<Ciphertext> encrypted_samples;
    for (int i = 0; i < 47; i++)
    {   
        Plaintext encoded_number = encoder.encode(samples[i]);
        encrypted_samples.emplace_back(encryptor.encrypt(encoded_number));
        cout << to_string(samples[i]).substr(0,6) << ((i != 46) ? ", " : ".\n");
    }

    // Next we encode the sample_difference_widths. There is no reason to encrypt these since they are not private data. 
	cout << "Encoding ... ";
    vector<Plaintext> encoded_sample_widths;
    for (int i = 0; i < 14; i++)
    {
        encoded_sample_widths.emplace_back(encoder.encode(sample_difference_widths[i]));
        cout << to_string(sample_difference_widths[i]).substr(0,7) << ((i != 13) ? ", " : ".\n");
    }

    /* 
    Next calculate the slopes between sample z=-23 and samples z=-13...0 to calculate SL min/max
    also calculate the slopes between sample z=-23 and samples z=-33...-46 to calculate SR min/max 
    */

    cout << "Computing slopes for SL ... " << endl;
    vector<Ciphertext> encrypted_SL_slopes;
    vector<double> unencrypted_SL_slopes(14);

    for (int i = 0; i < 14; i++)
    {
        // Offset because 0th sample is actually most recent = encrypted_samples[46], sample 23 = mid point
        Ciphertext volt_diff = evaluator.sub(encrypted_samples[23], encrypted_samples[i+33]);

        // We use multiply_plain instead of multiply because sample widths aren't encrypted. Less noise growth.
        encrypted_SL_slopes.emplace_back(evaluator.multiply_plain(volt_diff, encoded_sample_widths[i]));

        double res = samples[23] - samples[i+33];
        res *= sample_difference_widths[i];
        unencrypted_SL_slopes[i] = res;
        cout << "unencrypted_SL_slopes[" << i << "]: " << unencrypted_SL_slopes[i] << endl;
    }

    cout << "Computing slopes for SR ... " << endl;
    vector<Ciphertext> encrypted_SR_slopes;
    vector<double> unencrypted_SR_slopes(14);

    for (int i = 0; i < 14; i++)
    {
        // Offset because 0th sample is actually most recent = encrypted_samples[46], sample 23 = mid point
        Ciphertext volt_diff = evaluator.sub(encrypted_samples[23], encrypted_samples[13-i]);

        // We use multiply_plain instead of multiply because sample widths aren't encrypted. Less noise growth.
        encrypted_SR_slopes.emplace_back(evaluator.multiply_plain(volt_diff, encoded_sample_widths[i]));

        double res = samples[23] - samples[13-i];
        res *= sample_difference_widths[i];
        unencrypted_SR_slopes[i] = res;
        cout << "unencrypted_SR_slopes[" << i << "]: " << unencrypted_SR_slopes[i] << endl;
    }

	/***************************Practicing decryption/decoding; it works!***************************

    vector<Plaintext> plain_SL_slopes;
    vector<Plaintext> plain_SR_slopes;

    vector<double> SL_slopes(14);
    vector<double> SR_slopes(14);

    for (int i = 0; i < 14; i++)
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
	cout << "we got to the end of slope calculations!" << endl;
}

void DUALSLOPE::calculate_lr_minmax(){
    cout << "You're in calculate_lr_minmax()!" << endl;
}

void DUALSLOPE::compare_slopes2thresholds(){
    cout << "You're in compare_slopes2thresholds()!" << endl;
}

void DUALSLOPE::verify_peak(){
    cout << "You're in verify_peak()!" << endl;
}

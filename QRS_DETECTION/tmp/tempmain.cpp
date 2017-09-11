#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "seal.h"
#include "he.h"
#include "dualslope.h"

using namespace std;
using namespace seal;

void print_example_banner(string title);

void encrypted_xor(Ciphertext k1, Ciphertext k2);
void encrypted_and(Ciphertext k1, Ciphertext k2);
void encrypted_and(Ciphertext k1, Ciphertext k2, Ciphertext k3);
void encrypted_not(Ciphertext k1);
void encrypted_nand(Ciphertext k1, Ciphertext k2);
void encrypted_or(Ciphertext k1, Ciphertext k2);
void encrypted_nor(Ciphertext k1, Ciphertext k2);
void encrypted_xnor(Ciphertext k1, Ciphertext k2);

void calculate_lr_minmax();

int main()
{
    // Calculate the L/R Min/Max
    LR_MINMAX lr_minmax;
    vector<double> minmax = lr_minmax.();

    HE he;

    return 0;
}

void calculate_lr_minmax(){
    print_example_banner("Calculate L/R Min/Max");
    
    // Hard-coded ecg voltages from sample 100's first 47 samples. Example of a single calculation
    const vector<double> ecg_voltages { -0.145,-0.12,-0.135,-0.145,-0.15,-0.16,-0.155,-0.16,-0.175,-0.18,-0.185,-0.17,-0.155,-0.175,-0.18,-0.19,-0.18,-0.155,-0.135,-0.155,-0.19,-0.205,-0.235,-0.225,-0.245,-0.25,-0.26,-0.275,-0.275,-0.275,-0.265,-0.255,-0.265,-0.275,-0.29,-0.29,-0.29,-0.29,-0.285,-0.295,-0.305,-0.285,-0.275,-0.275,-0.28,-0.285,-0.305 };

    // Constant values of 1/10, 1/11, ... , 1/23. These are the values of the "run" in the rise over run calculation of slope
    const vector<double> sample_difference_widths { 0.10000,0.09091,0.08333,0.07692,0.07143,0.06667,0.06250,0.05882,0.05556,0.05263,0.05000,0.04762,0.04545,0.04348 };
    
    // Create encryption parameters
    EncryptionParameters parms; 
    parms.set_poly_modulus("1x^2048 + 1");
    parms.set_coeff_modulus(ChooserEvaluator::default_parameter_options().at(2048));
    parms.set_plain_modulus(1 << 8);
    parms.validate();

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

    // First encrypt the ecg_voltages
    cout << "Encrypting ..." << endl;
    vector<Ciphertext> encrypted_voltages;
    for (int i = 0; i < 47; i++)
    {
        Plaintext encoded_number = encoder.encode(ecg_voltages[i]);
        encrypted_voltages.emplace_back(encryptor.encrypt(encoded_number));
        cout << to_string(ecg_voltages[i]).substr(0,6) << ((i != 46) ? ", " : ".\n");
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
        // Offset because 0th sample is actually most recent = encrypted_voltages[46], sample 23 = mid point
        Ciphertext volt_diff = evaluator.sub(encrypted_voltages[23], encrypted_voltages[i+33]); 

        // We use multiply_plain instead of multiply because sample widths aren't encrypted. Less noise growth.
        encrypted_SL_slopes.emplace_back(evaluator.multiply_plain(volt_diff, encoded_sample_widths[i]));
        
        double res = ecg_voltages[23] - ecg_voltages[i+33];
        res *= sample_difference_widths[i];
        unencrypted_SL_slopes[i] = res; 
        cout << "unencrypted_SL_slopes[" << i << "]: " << unencrypted_SL_slopes[i] << endl;
    } 
    
    cout << "Computing slopes for SR ... " << endl;
    vector<Ciphertext> encrypted_SR_slopes;
    vector<double> unencrypted_SR_slopes(14);

    for (int i = 0; i < 14; i++)
    {
        // Offset because 0th sample is actually most recent = encrypted_voltages[46], sample 23 = mid point
        Ciphertext volt_diff = evaluator.sub(encrypted_voltages[23], encrypted_voltages[13-i]);

        // We use multiply_plain instead of multiply because sample widths aren't encrypted. Less noise growth.
        encrypted_SR_slopes.emplace_back(evaluator.multiply_plain(volt_diff, encoded_sample_widths[i]));

        double res = ecg_voltages[23] - ecg_voltages[13-i];
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



}

void print_example_banner(string title)
{
    if (!title.empty())
    {
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

void encrypted_xor(Ciphertext k1, Ciphertext k2)
{
    
}

void encrypted_and(Ciphertext k1, Ciphertext k2)
{

}

void encrypted_and(Ciphertext k1, Ciphertext k2, Ciphertext k3)
{

}

void encrypted_not(Ciphertext k1)
{

}

void encrypted_nand(Ciphertext k1, Ciphertext k2)
{

}

void encrypted_or(Ciphertext k1, Ciphertext k2)
{

}

void encrypted_nor(Ciphertext k1, Ciphertext k2)
{

}

void encrypted_xnor(Ciphertext k1, Ciphertext k2)
{

}


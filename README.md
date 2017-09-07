Currently in development! 

**End goal: take ECG data as input and output the locations of QRS complexes**

Wang, Deepu, and Lian's Dual-Slope QRS-detection algorithm can be found in their paper: https://www.ncbi.nlm.nih.gov/pubmed/22255619

ECG data taken from MIT-BIH Arrhythmia Database: https://physionet.org/physiobank/database/mitdb/

I chose SEAL because of it's ability to handle Franctional Encoding and it's speed when the plaintext modulus p = 2. 

I chose p = 2 because Wang et al.'s algorithm uses a series of comparisons, which can be implemented homomorphically by being built up using bitwise operators (XOR, AND, etc).

Let me know if you have any questions or want to contribute to this project! 
Jesse Epperson - Epperson.JesseE@gmail.com

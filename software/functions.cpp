#ifndef functions_include
#define functions_include
#include <vector>
#include <math.h>
#include <algorithm>
#include <random>
#include <iostream>
#include <random>
#include <chrono> 
using namespace std;

//Convert a vector of binary data to 
long int b2d(const vector<int>& v){
    long int val = 0;
    for(int i=0;i<v.size();i++)
        if(v[i] == 1)
            val += pow(2,i);
    return(val);
}

//convert and integer value to vector of bits (ints)
vector <int> d2b(unsigned long long int val, int L){
    vector<int> v;
    for(int i=0;i<L;i++){
        v.push_back(val & 1); //Get lsb bit
        val >>= 1; //shift val by 1 (to get next lsb)
    }
    return(v);
}
//overloads
vector <int> d2b(int val, int L){
    return(d2b((unsigned long long int)val,L));
}
//permute a vector
void permute(vector <int>& v){
    auto rng = std::default_random_engine {};
    rng.seed(std::chrono::system_clock::now().time_since_epoch().count());
    std::shuffle(std::begin(v), std::end(v), rng);
}

//print vector
void printVector(vector <int>& v){
    std::cout << "<";
    for(int i=0;i<v.size()-1;i++)
        std::cout << v[i] << ",";
    std::cout << v[v.size()-1] << ">" << std::endl << std::flush;
}

//Allocaters
std::vector <int> range(int start, int stop, int by=1, int MOD=10000000){
    std::vector<int> v;
    for(int i=start;i<stop;i+=by)
        v.push_back(i % MOD);
    return(v);
}

//Deprecated-ish, use only 0 now, logical remapping done with l2l in program
void allocate(int N, int B, vector <int>& a, vector <int>&b, int mode){
    a.clear();
    b.clear();
    switch(mode){
        case(0): //static
        {
            a = range(0,B);
            b = range(B,2*B);   
            break;
        }
        case(1): //bit shuffle
        {
            cout << "allocate with bit shuffle: unnnecessary as this is handled with l2l" << endl;
            vector <int> rows;
            for(int i=0;i<N;i++) rows.push_back(i); //all possible rows
            permute(rows); //permuted
            for(int i=0;i<B;i++) a.push_back(rows[i]);
            for(int i=0;i<B;i++) b.push_back(rows[i+B]);
            break;
        }
        case(2):
        {
            cout << "allocate with byte shift: unnnecessary as this is handled with l2l" << endl;
            static int start = 0;
            a = range(start,start+B);
            b = range(start+B,start+2*B);  
            start += 2*B % N;
            break;
        }
    }
    //In all cases, must be less than N
    for(int i=0;i<B;i++)
        a[i] = a[i] % N;
    for(int i=0;i<B;i++)
        b[i] = b[i] % N;
}

vector<int> gen_l2l(int N, int mode){
    vector <int> v;
    switch(mode){
            case(0): //static
            {
                for(int i=0;i<N;i++)
                    v.push_back(i);
                break;
            }
            case(1): //bit shuffle
            {
                for(int i=0;i<N;i++)
                    v.push_back(i);
                permute(v);
                break;
            }
            case(2): //byte shift
            {
                static int start = 0;
                for(int i=0;i<N;i++)
                    v.push_back((start+i)%N);
                start = (start + 8) % N;
                break;
            }
    }
    return(v);
}
#endif
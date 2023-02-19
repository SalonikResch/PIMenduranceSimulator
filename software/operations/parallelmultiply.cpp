#ifndef bnn_include
#define bnn_include

#include "../functions.cpp"
#include <vector>
#include <algorithm>
#include <random>
#include <stdio.h>

using std::vector;
//cols indicates how many elements there are
//a,b,c are row indices of 2 inputs
void bnn(PROGRAM *prog, const vector <int>& cols, const vector <int>& n, const vector <int>& w, const vector<int>& threshold){
    //XNOR neurons and weights
    vector <int> bits;
    for(int i=0;i<n.size();i++){
        int s = prog->getBit();
        XNOR(prog,cols,n[i],w[i],s);
        bits.push_back(s);
        prog->freeBit(n[i]);
        prog->freeBit(w[i]);
        printf("After XNOR %d, there are %d bits left\n",i,prog->freeSpace());
    }
    //Sum the bits
    vector <int> sum = SUM(prog,cols,bits);
    
    //Compare the sum to threshold
    int c = compare(prog,cols,sum,threshold);
}
#endif
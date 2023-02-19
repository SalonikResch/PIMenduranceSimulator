#ifndef CONV_include
#define CONV_include

#include "../functions.cpp"
#include <vector>
#include <algorithm>
#include <random>
#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

using std::vector;

int section(int mode, int W){
    srand (time(NULL));
    int d;
    if(mode == 0)
        d = 0;
    else
        d = rand() % W;
    return d;
}

vector <int> without(int L, int x){
    vector <int> v;
    for(int i=0;i<L;i++)
        if(i != x)
            v.push_back(i);
    return(v);
}

//cols indicates how many elements there are
//a,b,c are row indices of 2 inputs
void convolution(PROGRAM *prog, const vector <int>& cols, 
    const vector<vector<int>>& in1, 
    const vector<vector<int>>& in2,            
    const vector <int>& thresh, int colSplit=0, int W=3){
    
    //Element-wise multiply first
    vector<vector<int>> out;                   //Get bits for output
    for(int i=0;i<in1.size();i++){
        vector<int> v = {};
        out.push_back(v);
        for(int j=0;j<2*in1[0].size();j++)
            out[i].push_back(prog->getBit());
    }
    
    //Perform initial multiplications
    for(int i=0;i<in1.size();i++)
        multiply(prog,cols,in1[i],in2[i],out[i]);

    //Free input bits because they are not needed anymore
    for(int i=0;i<in1.size();i++)
        for(int j=0;j<in1[i].size();j++){ 
            prog->freeBit(in1[i][j]);
            prog->freeBit(in2[i][j]);
        }

    //Sum the outputs in each column
    vector<int> psum = SUM(prog,cols,out);
    
    //Pick columns to do final sums in
    int d = section(colSplit,W);
    vector<int> s = without(W,d);
    
    vector<int> D;
    vector<vector<int>> S;
    for(int i=0;i<W-1;i++){
        vector<int> v = {};
        S.push_back(v);
    }

    for(int i=0;i<cols.size()/W;i++){
        D.push_back(cols[i*W+d]);
        for(int j=0;j<s.size();j++)
            S[j].push_back(cols[i*W+s[j]]);
    }

    //Get rows in finals columns for moved operands
    vector<vector<int>> finalInputs;
    for(int i=0;i<W-1;i++){
        vector<int> v = {};
        finalInputs.push_back(v);
        for(int j=0;j<psum.size();j++){ 
            finalInputs[i].push_back(prog->getBit());
        }
    }
    
    //Data transfer (read from S[i] columns and write to D columns (from S[i] columns))
    for(int i=0;i<W-1;i++){
        for(int j=0;j<psum.size();j++){   
            prog->READ(S[i],psum[j],S[i]);
            prog->WRITE(D,finalInputs[i][j],S[i]);
        }
    }
    //Add the output in D columns to final inputs
    vector<int> v = {};
    finalInputs.push_back(v);
    for(int i=0;i<psum.size();i++)
        finalInputs[W-1].push_back(psum[i]);
    //Final sum
    SUM(prog,D,finalInputs);
}
#endif
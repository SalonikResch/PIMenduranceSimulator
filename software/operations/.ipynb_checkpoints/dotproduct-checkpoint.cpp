#ifndef DOTPRODUCT_include
#define DOTPRODUCT_include

#include "../functions.cpp"
#include <vector>
#include <algorithm>
#include <random>
#include <stdio.h>

using std::vector;

//first all in a, floor(half) of a moved into b
//mode: 0 linear, 1 random
void splitCols(vector <int>& a, vector <int>& b, int mode){
    int half = (int)a.size()/2;            //half of the size
    int last = a.size();
    if(mode == 1)                          //permute first if random
        permute(a);
    for(int i=half;i<last;i++)         //add second half of a to b
        b.push_back(a[i]);
    for(int i=half;i<last;i++)         //remove them from a
        a.erase(a.begin()+half);
}

//cols indicates how many elements there are
//a,b,c are row indices of 2 inputs
void dotproduct(PROGRAM *prog, const vector <int>& cols, const vector <int>& a, const vector <int>& b, int colSplit=0){
    //Element-wise multiply first
    vector <int> c;                   //Get bits for output
    for(int i=0;i<2*a.size();i++)
        c.push_back(prog->getBit());
    multiply(prog,cols,a,b,c);
    
    //Sum
    vector <int> activeCols;
    for(int i=0;i<cols.size();i++)
        activeCols.push_back(cols[i]);
    vector <int> removedCols;  //Cols which will have elements read out and transfered to activeCols
    vector <int> operand1 = c; //Row indices of the operands
    vector <int> operand2;
    vector <int> output;       //And output
    //Condense until only 1 column left
    while(activeCols.size() > 1){
        //printf("There are %d active columns and %d free rows. Operand size is %d bits\n",(int)activeCols.size(),prog->freeSpace(),operand1.size());
        if(prog->freeSpace() == 0)
            return;
        //Split columns
        splitCols(activeCols,removedCols,colSplit);
        //Allocate row space for 2nd operand
        for(int i=0;i<operand1.size();i++)
            operand2.push_back(prog->getBit());
        //Read and write operands
        for(int i=0;i<operand1.size();i++){
            prog->READ(removedCols,operand1[i],activeCols); //read operand1 from removedCols and store in buffer, at activeCols indices (for every bit of operand)
            prog->WRITE(activeCols,operand2[i],activeCols); //write to operand 2 from same buffer location to activeAcols
        }
        //Allocate row space for output
        for(int i=0;i<operand1.size()+1;i++)
            output.push_back(prog->getBit());
        //Add
        add(prog,activeCols,operand1,operand2,output);
        //Free operand 1 and 2
        for(int i=0;i<operand1.size();i++){
            prog->freeBit(operand1[i]);
            prog->freeBit(operand2[i]);
        }
        //output becomes operand1
        operand1 = output;
        operand2.clear();
        output.clear();
    }
}
#endif
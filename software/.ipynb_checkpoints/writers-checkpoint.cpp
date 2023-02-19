#ifndef writers_include
#define writers_include 

#include <vector>
#include "program.cpp"
#include "functions.cpp"

//Write an integer in a set of rows
void writeInt(PROGRAM *prog, int col, const vector <int>& rows, unsigned long long val){
    vector <int> bits = d2b(val,rows.size()); //convert decimal to vector of bits (integers)
    for(int i=0;i<rows.size();i++)
        prog->WRITE(cols,rows[i],bits[i]);
}

//overloads
void writeInt(PROGRAM *prog, int col, const vector <int>& rows, int val){
    writeInt(prog,col,rows,(unsigned long long int)val);
}
#endif
#ifndef memory_include
#define memory_include 

#include <vector>
#include "program.cpp"
#include "functions.cpp"

//Write an integer in a set of rows (columns are parallel, rows are sequential bits)
void writeInt(PROGRAM *prog, const vector<int>& cols, const vector <int>& rows, unsigned long long val){
    vector <int> bits = d2b(val,rows.size()); //convert decimal to vector of bits (integers)
    for(int i=0;i<rows.size();i++){
        prog->WRITEimmediate(cols,rows[i],bits[i]); //Write (immediate) bit value into row
        prog->takeBit(rows[i]); //Indicate that this bit is taken (manual reserve)
    }
}

//overloads
void writeInt(PROGRAM *prog, const vector<int>& cols, const vector <int>& rows, int val){
    writeInt(prog,cols,rows,(unsigned long long int)val);
}
void writeInt(PROGRAM *prog, int col, const vector <int>& rows, int val){
    vector<int> cols;
    cols.push_back(col);
    writeInt(prog,cols,rows,(unsigned long long int)val);
}

//Read an integer in a set of rows (place in registers)
void readInt(PROGRAM *prog, int col, const vector <int>& rows, const vector <int>& bcols){
    vector <int> cols;
    cols.push_back(col);
    for(int i=0;i<rows.size();i++){
        vector <int> bcol;
        bcol.push_back(bcols[i]);
        prog->READ(cols,rows[i],bcol);
    }
}

//overloads
//void readInt(PROGRAM *prog, int col, const vector <int>& rows, int val){
//    writeInt(prog,col,rows,(unsigned long long int)val);
//}


#endif
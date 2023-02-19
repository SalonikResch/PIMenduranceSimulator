#include "../cram/PIMarray.cpp"
#include "../software/sofware.cpp"
#include "../software/operations/dotproduct.cpp"
#include "../simulator/simulator.cpp"
#include "../statistics/statistics.cpp"
#include "parameters.cpp"
#include <vector>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <algorithm>
//#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <fstream>
#include "../../../SRlib/cpp/mkdir.cpp"
using namespace std;
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

void programLatency(){
    //Write data
    char dirname[200];
    sprintf(dirname,"../data/computationLifetime/latencies");  
    mkdir_r(dirname);
    char filename[200];
    sprintf(filename,"%s/dotproduct",dirname);
    printf("%s\n",filename);
    ofstream oFile(filename);
    oFile << "N\tB\tICOUNT\n";
    
    PROGRAM *prog; //empty program
    vector <int> va;
    vector <int> vb;
    
    int Ns[3] = {256,512,1024};
    int Bs[4] = {4,8,16,32};
    
    for(int n=0;n<3;n++){
        int N = Ns[n];
        for(int b=0;b<4;b++){
            int B = Bs[b];
            vector <int> cols = range(0,N); //use all N columns
            //Create program
            prog = new PROGRAM(0,N);
            //Allocate space for operands (and arrange logical addresses)
            allocate(N,B,va,vb,0); //static allocate
            //Write in values
            writeInt(prog,cols,va,3);
            writeInt(prog,cols,vb,4);
            //Peform dotproduct
            dotproduct(prog,cols,va,vb,0);
            //Write data     
            oFile << N << '\t' << B << '\t' << prog->ICOUNT() << std::endl;
            //End create program
            delete prog;
        }
    }
    oFile.close();
}



int main(int argc, const char* argv[]){
    programLatency();
    return(0);
}
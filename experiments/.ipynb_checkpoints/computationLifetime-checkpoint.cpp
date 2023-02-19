#include "../cram/PIMarray.cpp"
#include "../software/sofware.cpp"
#include "../software/operations/dotproduct.cpp"
#include "../software/operations/bnn.cpp"
#include "../software/operations/convolution.cpp"
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

//rowMove: 0 static, 1 bit shuffle, 2 byteshift
STATISTICS* computationLifetime(char *benchmark, int N, int B, unsigned long switchEvery, uint32_t switches, int rowMove, int colMove, 
    bool rowRenaming=false, int rowRenamingPeriod=-1, int rowRenamingSize=0,
    bool colRenaming=false, int colRenamingPeriod=-1, int colRenamingSize=0,
    int nRepititions=-1){
    
    //Initialize permanent objects
    PIMarray *pimArray = new PIMarray(N,N,
        rowRenaming,rowRenamingSize,rowRenamingPeriod,
        colRenaming,colRenamingSize,colRenamingPeriod,switches); //Create PIM array
    SIMULATOR *sim = new SIMULATOR(pimArray); //create simulator
    PROGRAM *prog; //empty program
    STATISTICS *stats = new STATISTICS; //holds statistics about the experient
    
    stats->nOps = 0;
    
    int nRows = pimArray->remap(); //Number of rows available for computation
    printf("There are %d rows available for computation after initial mapping\n",nRows);
    vector <int> a;
    vector <int> b;
    vector <int> c;
    
    unsigned  long sampleSize = 1000000000;
    
    //c vector length is variable depending on benchmark
    int len_c;
    if(strcmp(benchmark,"bnn") == 0) len_c = 14;
    if(strcmp(benchmark,"multiply") == 0) len_c = 2*B;
    
    int nReps = 0;
    
    //Until not enough space
    //while( nRows > (4*B+2*B+1) ){
    while(1){
        printf("After %d operations, there are %d rows left for computation, with an average of %u switches left\n",stats->nOps,nRows,pimArray->avgSwitchesLeft());
        
        //Create program
        prog = new PROGRAM(0,nRows);
        
        //Allocate columns
        vector <int> cols = range(0,N); //use all N columns
        if(colMove == 1)
            permute(cols);
        if(colMove == 2){
            static int shift = 0;
            for(int i=0;i<cols.size();i++)
                cols[i] = (cols[i]+shift) % N;
            shift += 8;
        }
        
        //Setup for convolution, highly different from other benhmarks
        vector<vector<int>> in1;
        vector<vector<int>> in2;
        vector<int> thresh;
        int W;
        if(strcmp(benchmark,"convolution") != 0){ 
        //Allocate space for operands (and arrange logical addresses)
        allocate(nRows,B,a,b,0); //static allocate
        c.clear(); for(int i=0;i<len_c;i++) c.push_back(2*B+i); //Hack - Allocate a c vector for threshold for BNN benchmark
        }
        vector<int> l2l = gen_l2l(prog->spaceRequired(),rowMove); //static , bit shuffle, or byte shift
        prog->set_l2l(l2l);

        //Write in values
        if(strcmp(benchmark,"convolution") == 0){
            W = 4;
            int H = 3;
            int idx = 0;
            for(int i=0;i<H;i++){
                vector<int> v = range(idx,idx+B);
                writeInt(prog,cols,v,0);
                in1.push_back(v);
                idx += B;
            }
            for(int i=0;i<H;i++){
                vector<int> v = range(idx,idx+B);
                writeInt(prog,cols,v,0);
                in2.push_back(v);
                idx += B;
            }
            thresh = range(idx,idx+2*B);  
            writeInt(prog,cols,thresh,0);
        }else{ 
            writeInt(prog,cols,a,3);
            writeInt(prog,cols,b,4);
        }
            
        //Peform computation
        if(strcmp(benchmark,"dotproduct") == 0) dotproduct(prog,cols,a,b,0); //colMoves now all 0, covered by col re-arranging
        //if(strcmp(benchmark,"bnn") == 0) bnn(prog,cols,a,b,c);
        if(strcmp(benchmark,"multiply") == 0) multiply(prog,cols,a,b,c);
        printf("W = %d\n",W);
        if(strcmp(benchmark,"convolution") == 0) convolution(prog,cols,in1,in2,thresh,0,W);
        if(strcmp(benchmark,"singleMultiply") == 0){
            vector <int> col;
            col.push_back(cols[0]);
            multiply(prog,col,a,b,c);
        }
        //End create program
        //prog->printInstructions();
        
        //Check if program is valid
        if(!prog->isValid())
            break;
        
        //Set program to run on the array
        sim->setProgram(prog); //load instructions
        if(nRows < prog->spaceRequired()){
            cout << "PIM array does not have sufficient space to run program\n";
            break;
        }
        
        //Report ICOUNT
        cout << "This " << benchmark << " has " << prog->ICOUNT() << " instructions\n";
        
        //Run the program on the array
        for(int i=0;i<switchEvery;i++){ //As many times before changing program, or just a sample of
            if(i % 100 == 0) printf("Iteration %d of %d (%.2f%%)\n",i,switchEvery,(float)i/(float)switchEvery*100.);
            sim->runProgram();
            //cout << "Average # cols " << pimArray->avg_cols << endl << flush;
            if(pimArray->nPristineLogicalRows() < prog->spaceRequired() ){ //error out
                cout << "Program hit a hardware error running, quitting early\n";
                delete prog;
                goto done;
            }
            if(nReps+i >= nRepititions){
                cout << "Hit limit of number of repititions at " << nReps+i << " and now exiting\n";
                delete prog;
                goto done;
            }
            stats->nOps++;
        }
        delete prog;
        round:  
        nRows = pimArray->remap();
        
        
        
        nReps += switchEvery;
        if(nReps >= nRepititions && nRepititions > 0){
            cout << "Hit limit of number of repititions at " << nReps << " and now exiting\n";
            goto done;
        }
    }
    
    done: 
    //Transfer data
    stats->pimArray = pimArray;
    cout << "Completed " << stats->nOps << " operations" << endl;
    return(stats);
}



int main(int argc, const char* argv[]){
    char benchmark[50];
    int idx, size, switches;
    if(argc > 4){
        sprintf(benchmark,"%s",argv[1]);
        idx = atoi(argv[2]); //get index
        size = atoi(argv[3]);
        switches = atoi(argv[4]);
    }else{
        cout << "Not enough arguments" << endl;
        cout << "benchmark index size(0/1) switches" << endl;
        return(-1);
    }
    if( strcmp(benchmark,"dotproduct") != 0 && 
        strcmp(benchmark,"convolution") != 0 &&
        strcmp(benchmark,"multiply") != 0 &&
        strcmp(benchmark,"singleMultiply") != 0){
        cout << "Benchmark " << benchmark << " not found" << endl;
        return(-1);
    }
    cout << "Running benchmark " << benchmark << endl;
    
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    
    parameters p; //parameters struct
    setParameters(&p,idx,size,switches);
    
    STATISTICS *stats = computationLifetime(benchmark,p.N,p.B,p.switchEvery,p.switches,p.rowMove,p.colMove,
        p.rowRenaming,p.rowRenamingPeriod,p.rowRenamingSize,
        p.colRenaming,p.colRenamingPeriod,p.colRenamingSize,
        p.nRepititions);
    
    //Write data
    char dirname[200];
    sprintf(dirname,"../data/computationLifetime/%s/%u",benchmark,p.switches);     
    char filename[200];
    sprintf(filename,"%s/%dx%d_%dbit_%s_%s%s",dirname,p.N,p.N,p.B,p.rowMoveText,p.colMoveText,p.renamingText);
    printf("%s\n",filename);
    mkdir_r(dirname);
    ofstream MyFile(filename,MyFile.out | MyFile.app);
    MyFile << p.switchEvery << "\t" << stats->nOps << endl;
    MyFile.close();
    
    sprintf(dirname,"../data/writes/computationLifetime/%s/%u",benchmark,p.switches);
    sprintf(filename,"%s/%dx%d_%dbit_%u_%s_%s%s",dirname,p.N,p.N,p.B,p.switchEvery,p.rowMoveText,p.colMoveText,p.renamingText);
    mkdir_r(dirname);
    stats->write_writes(filename);
    delete stats;
    
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Experiment took " << std::chrono::duration_cast<std::chrono::minutes>(end - begin).count() << " minutes" << std::endl;

    return(0);
}
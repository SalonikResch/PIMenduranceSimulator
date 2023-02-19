#ifndef STATISTICS_include
#define STATISTICS_include

#include "../cram/PIMarray.cpp"
#include <fstream>
using namespace std;
//Class to hold statistics about an experiment
class STATISTICS{
    public:
    
    PIMarray *pimArray;
    
    unsigned long long latency; //In nanoseconds
    unsigned long long energy; //In picoJoules
    
    unsigned long long nGates;
    unsigned long long nOps;
    
    //write S out
    void write_S(char *f){
        fstream oFile;
        oFile.open(f);
        for(int i=0;i<pimArray->Np;i++){
            for(int j=0;j<pimArray->Mp-1;j++)
                oFile << pimArray->S[i*pimArray->Mp+j] << "\t"
            oFile << pimArray->S[i*pimArray->Mp+pimArray->Mp-1] << endl;
        }
        oFile.close();
    }
}
#endif
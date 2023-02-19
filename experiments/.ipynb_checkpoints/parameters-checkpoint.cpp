#include <vector>
#include <math.h>
#include <stdio.h>
#include "../../../SRlib/cpp/arguments.cpp"

struct parameters{
    int N = 128;
    int B = 16;
    unsigned long switchEvery = 10;
    uint32_t switches = 10000000;
    int rowMove = 0;
    int colMove = 0;
    bool renaming = false;
    bool rowRenaming = false;
    int rowRenamingPeriod = -1;
    int rowRenamingSize = 0;
    bool colRenaming = false;
    int colRenamingPeriod = -1;
    int colRenamingSize = 0;
    char rowMoveText[20];
    char colMoveText[20];
    char renamingText[20];
    int nRepititions = -1;
};

void setParameters(parameters *p, int idx, int size = 1, int switches = 1000000){
    vector <unsigned long> switchEverys;
    vector <int> Ns;
    vector <int> rowMoves;
    vector <int> colMoves;
    vector <bool> renamings;
    vector <int> Bs;
    
    //switchEverys = { (unsigned long)pow(10,5), (unsigned long)pow(10,4), (unsigned long)pow(10,3), (unsigned long)pow(10,2), (unsigned long)pow(10,1) };
    //switchEverys = { (unsigned long)pow(10,3), (unsigned long)pow(10,2), (unsigned long)pow(10,1) };
    //switchEverys = { (unsigned long)pow(10,5), (unsigned long)pow(10,4), (unsigned long)pow(10,3), (unsigned long)pow(10,2), (unsigned long)pow(10,1) };
    switchEverys = { (unsigned long)10000, (unsigned long)1000, (unsigned long)500, (unsigned long)100, (unsigned long)50, (unsigned long)10};
    
    switch(size){
            case(0): //small dot-product
            {
                Ns = {1024,512,256,128};
                Bs = {16,8,4,2};
                break;
            }
            case(1): //medium dot-product
            {
                Ns = {1024,512,256};
                Bs = {8,16,32};
                break;
            }
            case(2): //dot-product and multiply and single multiply
            {
                Ns = {1024};
                Bs = {32};
                break;
            }
            case(3): //bnn
            {
                Ns = {1024};
                Bs = {256};
                break;
            }
            case(4): //conv
            {
                Ns = {1024};
                Bs = {8};
                break;
            }
    }
    //Constant
    rowMoves = {0,1,2};
    colMoves = {0,1,2};
    renamings = {false,true};
    
    vector <int> dimensions;
    dimensions.push_back(switchEverys.size());
    dimensions.push_back(rowMoves.size());
    dimensions.push_back(colMoves.size());
    dimensions.push_back(renamings.size());
    dimensions.push_back(Ns.size());
    dimensions.push_back(Bs.size());
    for(int i=0;i<dimensions.size();i++)
       cout << dimensions[i] << "," << flush;
        
    vector <int> ids = int2ints(idx,dimensions);
    printf("%d: %d,%d,%d,%d,%d\n",idx,ids[0],ids[1],ids[2],ids[3],ids[4],ids[5]);
  
    p->switchEvery = switchEverys[ids[0]];
    p->rowMove = rowMoves[ids[1]];
    p->colMove = colMoves[ids[2]];
    p->renaming = renamings[ids[3]];
    p->N = Ns[ids[4]];
    p->B = Bs[ids[5]];
    if(p->renaming){
        p->rowRenaming = true;
        p->rowRenamingSize = 1; //(int)(0.1*(float)p->N);
        p->rowRenamingPeriod = 1;
        p->colRenaming = false;
        p->colRenamingSize = 0; //(int)(0.1*(float)p->N);
        p->colRenamingPeriod = 10;
    }
    p->switches = (uint32_t)switches;
    p->nRepititions = 100000; //100,000 for most, more for singleMultiply
    
    char FT[2][10] = { {"FALSE"},{"TRUE"} };
    char rowMoveTexts[3][15] = {{"Static"},{"Random"},{"ByteShift"}};
    char colMoveTexts[3][15] = {{"Static"},{"Random"},{"ByteShift"}};
    char renamingTexts[2][15] = {{""},"renaming"};
    
    sprintf(p->rowMoveText,"%s",rowMoveTexts[p->rowMove]);
    sprintf(p->colMoveText,"%s",colMoveTexts[p->colMove]);
    sprintf(p->renamingText,"%s",renamingTexts[p->renaming]);
    
    
    printf("N: %d, B: %d, switchEvery: %u, rowMove: %d (%s), colMove: %d (%s)\nnRepititions: %d\n",
          p->N,p->B,p->switchEvery,p->rowMove,p->rowMoveText,p->colMove,p->colMoveText,p->nRepititions);
    
    printf("rowRenaming: %s, rowRenamingPeriod: %d, rowRenamingSize: %d\n",FT[p->rowRenaming],p->rowRenamingPeriod,p->rowRenamingSize);
    printf("colRenaming: %s, colRenamingPeriod: %d, colRenamingSize: %d\n",FT[p->colRenaming],p->colRenamingPeriod,p->colRenamingSize);
}
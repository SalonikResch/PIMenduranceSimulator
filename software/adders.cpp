#ifndef adders_include
#define adders_include 

#include <vector>
#include "program.cpp"

int2 halfAdd(PROGRAM *prog, const vector <int>& cols, int A, int B, int S=-1, int Cout=-1){
    int t1 = prog->getBit();
    prog->NAND(cols,A,B,t1);
    int t2 = prog->getBit();
    prog->NAND(cols,A,t1,t2);
    int t3 = prog->getBit();
    prog->NAND(cols,B,t1,t3);
    
    if(S < 0)
        S = prog->getBit();
    
    prog->NAND(cols,t2,t3,S);
    prog->freeBit(t2);
    prog->freeBit(t3);
    
    if(Cout < 0)
        Cout = prog->getBit();
    
    prog->NOT(cols,t1,Cout);
    prog->freeBit(t1);
    
    //Return sum and carry bit locations
    int2 sc;
    sc.int1 = S;
    sc.int2 = Cout;
    return(sc);
}

int2 fullAdd(PROGRAM *prog, const vector <int>& cols, int A, int B, int Cin, int S=-1, int Cout=-1){
    int t1 = prog->getBit();
    prog->NAND(cols,A,B,t1);
    int t2 = prog->getBit();
    prog->NAND(cols,A,t1,t2);
    int t3 = prog->getBit();
    prog->NAND(cols,B,t1,t3);
    int t4 = prog->getBit();
    prog->NAND(cols,t2,t3,t4);
    
    prog->freeBit(t2);
    prog->freeBit(t3);

    int t5 = prog->getBit();
    prog->NAND(cols,t4,Cin,t5);
    
    if(Cout < 0)
        Cout = prog->getBit();
    
    prog->NAND(cols,t5,t1,Cout);
    prog->freeBit(t1);
    
    int t6 = prog->getBit();
    prog->NAND(cols,t4,t5,t6);
    prog->freeBit(t4);
    
    int t7 = prog->getBit();
    prog->NAND(cols,t5,Cin,t7);
    prog->freeBit(t5);
    
    if(S < 0)
        S = prog->getBit();
    
    prog->NAND(cols,t6,t7,S);
    prog->freeBit(t6);
    prog->freeBit(t7);
    
    int2 sc;
    sc.int1 = S;
    sc.int2 = Cout;
    return(sc);
}

int halfBorrow(PROGRAM *prog, const vector <int>& cols, int A, int B, int Bout=-1){
    int t1 = prog->getBit();
    prog->NOT(cols,A,t1);
    if(Bout < 0)
        Bout = prog->getBit();
    prog->AND(cols,t1,B,Bout);
    prog->freeBit(t1);
    return(Bout);
}

int fullBorrow(PROGRAM *prog, const vector <int>& cols, int A, int B, int Bin, int Bout=-1){
    int notA = prog->getBit();
    prog->NOT(cols,A,notA);
    int P1 = prog->getBit();
    prog->NAND(cols,notA,Bin,P1);
    int P2 = prog->getBit();
    prog->NAND(cols,notA,B,P2);
    prog->freeBit(notA);
    int P3 = prog->getBit();
    prog->NAND(cols,B,Bin,P3);
    
    int P1P2 = prog->getBit();
    prog->WRITEimmediate(cols,P1P2,1);
    prog->AND(cols,P1,P2,P1P2);
    prog->freeBit(P1);
    prog->freeBit(P2);
    if(Bout < 0)
        Bout = prog->getBit();
    prog->NAND(cols,P1P2,P3,Bout);
    prog->freeBit(P1P2);
    prog->freeBit(P3);
    return(Bout);
}

void XNOR(PROGRAM *prog, const vector <int>& cols, int a, int  b, int c){
    int t1 = prog->getBit();
    prog->NAND(cols,a,b,t1);
    int t2 = prog->getBit();
    prog->NAND(cols,a,t1,t2);
    int t3 = prog->getBit();
    prog->NAND(cols,b,t1,t3);
    prog->freeBit(t1);
    prog->AND(cols,t2,t3,c);
    prog->freeBit(t2);
    prog->freeBit(t3);        
}

// int XNOR_withNOR(PROGRAM *prog, const vector <int>& cols, int a, int b, int c=-1){
//     int t1 = prog->getBit();
//     prog->NOR(cols,a,b,t1);
//     int t2 = prog->getBit();
//     prog->NOR(cols,a,t1,t2);
//     if(c < 0)
//         c = prog->getBit();
//     prog->NOR(cols,t1,t2,c);
//     prog->freeBit(t1);
//     prog->freeBit(t2);
// }
#endif
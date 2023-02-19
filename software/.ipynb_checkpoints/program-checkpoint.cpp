#ifndef PROGRAM_include
#define PROGRAM_include
#include "../cram/PIMarray.cpp"
#include <stdio.h>
#include <vector>
using std::vector;

//Struct for instructions
struct instruction{
    int opcode; //Defines instruction (opcodes define in PIMarray.cpp)
    int in1; //First input of logic gate, or bit to be read on read on operation
    int in2; //Second input (only for logic)
    int out; //Output, target of logic gate or bit written on write operation
    vector <int> cols; //columns to apply instruction to
    vector <int> vals; //Values to write for write immediate, or register # source for write
};

struct int2{
    int int1;
    int int2;
};

//A program is a class which holds instructions for a program, it works entirely on logical addresses (no knowledge of underlying physical hardware)
//It also contains software-level information to aid in writing a program, such as which logical bits are in use and which are free
class PROGRAM{
    private:
        //Parameters for keeping track of bits
        int L;  //length of workspace
        int *w; //workspace for program
        bool *needed; //booleans indicating whether values in w are still needed
        bool *free; //booleans indicating whether space is free
    
        bool outOfWorkspace = false; //flag for out of workspace (program can't run)
        bool invalidInstructions = false; //flag for invalid instructions
        bool outOfBoundsAllocations = false; //flag for taking invalid bits
        
        //Vectors to hold program
        vector <instruction> instructions;
    
        //Logical to logical mapping (to allow software to elegantly shift logical addresses)
        int *l2l; 
    
        //CPU memory (memory in the controlling hardware)
        //int nRegisters;
        //int *registers;
        
    //Function to allocate needed and free
    void allocateAuxiliary(int L){
        needed = new bool[L];
        free = new bool[L];
        for(int i=0;i<L;i++){
            needed[i] = false;
            free[i] = true;
        }
        l2l = new int[L];
        for(int i=0;i<L;i++)
            l2l[i] = i;
    }
    
    public:
        //Constructor with start/stop
        PROGRAM(int start, int stop){
            L = stop-start;
            w = new int[L];
            for(int i=0;i<L;i++)
                w[i] = start+i;
            allocateAuxiliary(L);
        }

        //Constructor with vector
        PROGRAM(std::vector<int>& workspace){
            L = workspace.size();
            w = new int[L];
            for(int i=0;i<L;i++)
                w[i] = workspace[i];
            allocateAuxiliary(L);
        }
    
        //Destructor
        ~PROGRAM(){
            delete[] w;
            delete[] free;
            delete[] needed;
            delete[] l2l;
            instructions.clear();
        }
    
    private:
        //Free up workspace
        void freeWorkspace(){
            for(int i=0;i<L;i++){
                if(!needed[i] && !free[i])
                    free[i] = true;
            }
        }

        //Search for a free bit
        int bitsearch(){
            //Search for bit
            for(int i=0;i<L;i++){
                if(free[i]){
                    free[i] = false;
                    needed[i] = true;
                    return(i);
                }
            }
            return(-1);
        }
    
    public:
        //Allocate a free bit (request any bit)
        int getBit(){
            int b = bitsearch(); //search for 1
            if(b >= 0)           //return if found
                return(b);
            //Not found, free space
            freeWorkspace();
            b = bitsearch(); // try again
            if(b >= 0)
                return(b);
            //Was not able to free workspace, ran out of space
            //cout << "Ran out of workspace" << endl << flush;
            outOfWorkspace = true; //flag to indicate program is invalid
            return(-1);
        }
    
        //De-allocate a bit
        void freeBit(int b){
            //if(b < 0 || b >= L)
             //   cout << "Freeing invalid bit " << b << endl;
            needed[b] = false; //Indicate is is no longer needed
        } //free[b] not set to true, because that only happens on clear workspace (to prevent re-using bits frequently)
    
        //Indicate a bit is unavailable manually
        void takeBit(int b){
            if(b < 0 || b >= L)
                outOfBoundsAllocations = true;
                //cout << "Taking invalid bit " << b << endl;
            needed[b] = true;
            free[b] = false;
        }
    
        //Reserve a sequnce of bits
        void takeBits(const vector <int>& b){
            for(int i=0;i<b.size();i++)
                takeBit(b[i]);
        }
    
        //Add instructions
    private:
        bool bound(int x){
            if( x < 0 || x >= L)
                return(true);
            return(false);
        }
    
        //validate an instruction is valid when adding it
        void check(const vector <int>& cols, int opcode, int in1, int in2, int out, const vector <int>& vals){
            switch(opcode){
                    case(gateNOT):
                        if(bound(in1) || bound(out)){
                            //cout << "Invalid NOT, in1=" << in1 << " out=" << out << endl;
                            invalidInstructions = true;
                        }
                        break;
                    case(gateNAND):
                        if(bound(in1) || bound(in2) || bound(out)){
                            //cout << "Invalid NAND, in1=" << in1 << " in2=" << in2 << " out=" << out << endl;
                            invalidInstructions = true;
                        }
                        break;
                    case(gateAND):
                        if(bound(in1) || bound(in2) || bound(out)){
                            //cout << "Invalid AND, in1=" << in1 << " in2=" << in2 << " out=" << out << endl;
                            invalidInstructions = true;
                        }
                        break;
                    case(gateWRITE):
                        if(bound(out)){
                            //cout << "Invalide WRITE, out=" << out << endl;
                            invalidInstructions = true;
                        }
                        break;
                    case(gateWRITEimmediate):
                        if(bound(out)){
                            //cout << "Invalide WRITEimmediate, out=" << out << endl;
                            invalidInstructions = true;
                        }
                        break;
                    case(gateREAD):
                        if(bound(in1)){
                            //cout << "Invalide READ, in1=" << in1 << endl;
                            invalidInstructions = true;
                        }
                        break;
                    case(gateREADimmediate):
                        if(bound(in1)){
                            //cout << "Invalide READimmediate, in1=" << in1 << endl;
                            invalidInstructions = true;
                        }
                        break;
            }
            //for(int i=0;i<cols.size();i++){
            //    if( cols[i] < 0 || cols[i] >= L)
            //}
        }
    
        //works for all instructions, after gate is converted to an opcode (opcodes defined in PIMarray.cpp)
        void addGate(vector <int> cols, int opcode, int in1, int in2, int out, vector <int> vals){
            check(cols,opcode,in1,in2,out,vals);
            //l2l conversions
            if(in1 >= 0) in1 = l2l[in1];
            if(in2 >= 0) in2 = l2l[in2];
            if(out >= 0) out = l2l[out];
            instruction ins; //Instatiate instruction struct
            ins.opcode = opcode; //fill in entries
            ins.in1 = in1;
            ins.in2 = in2;
            ins.out = out;
            ins.cols = cols;
            ins.vals = vals;
            instructions.push_back(ins); //append to instructions vector
        }
        
        //Overload to add empty vector to end
        void addGate(vector <int> cols, int opcode, int in1, int in2, int out){
            vector <int> v;
            addGate(cols,opcode,in1,in2,out,v);
        }
    
    public:
        void NOT(const vector <int>& cols, int in1, int out){
            addGate(cols,gateNOT,in1,-1,out);
        }
    
        void NAND(const vector <int>& cols, int in1, int in2, int out){
            addGate(cols,gateNAND,in1,in2,out);
        }
        
        void AND(const vector <int>& cols, int in1, int in2, int out){
            addGate(cols,gateAND,in1,in2,out);
        }
    
        void WRITE(const vector <int>& cols, int row, const vector <int>& vals){
            addGate(cols,gateWRITE,-1,-1,row,vals);
        }
    
        void WRITEimmediate(const vector <int>& cols, int row, const vector <int>& vals){
            addGate(cols,gateWRITEimmediate,-1,-1,row,vals); //row is place in instrution.out,others don't care
        }
    
        void READ(const vector <int>& cols, int row, const vector <int>& vals){
            addGate(cols,gateREAD,row,-1,-1,vals);
        }
    
        void READimmediate(const vector <int>& cols, int row){
            addGate(cols,gateREADimmediate,row,-1,-1); //row is placed in instruction.in1, others don't care
        }
    
        //overload for write imediate, single value
        void WRITEimmediate(const vector <int>& cols, int row, int val){
            vector <int> vals;
            for(int i=0;i<cols.size();i++)
                vals.push_back(val);
            addGate(cols,gateWRITEimmediate,-1,-1,row,vals); //row is place in instrution.out,others don't care
        }
    
        //Set new l2l
        void set_l2l(vector <int>& l){
            for(int i=0;i<L;i++){
                l2l[i] = l[i];
                if(l2l[i] < 0 && l2l[i] >= L){
                    cout << "Error, l2l value is out of range";
                }
            }
        }
    
        bool isValid(){
            bool valid = true;
            if(outOfWorkspace){
                valid = false;
                cout << "Program ran out of workspace\n";
            }
            if(!outOfWorkspace && invalidInstructions){
                valid = false;
                cout << "Program contains invalid instructions\n";
            }
            if(!outOfWorkspace && outOfBoundsAllocations){
                valid = false;
                cout << "Program has out of bounds memory requests\n";
            }
            return(valid);
        }
        
            
        vector <instruction> getInstructions(){
            return(instructions);
        }
    
        void printInstruction(instruction ins){
            printf("%s\tin1: %d, in2: %d, out: %d  (%d cols)\n",ISA[ins.opcode],ins.in1,ins.in2,ins.out,ins.cols.size());
        }
    
        void printInstructions(){
            for(int i=0;i<instructions.size();i++)
                printInstruction(instructions[i]);
        }
    
        //Report amount of free space
        int freeSpace(){
            int n = 0;
            for(int i=0;i<L;i++)
                if(free[i] || !needed[i])
                    n++;
            return(n);
        }
    
        //Space required by program
        int spaceRequired(){
            return(L);
        }
    
        //report l2l
        void print_l2l(){
            for(int i=0;i<L;i++)
                printf("%d: %d\n",i,l2l[i]);
        }
    
        //Report instruction count
        int ICOUNT(){
            return((int)instructions.size());
        }
};
#endif

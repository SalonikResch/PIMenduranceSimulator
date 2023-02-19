#ifndef PIMarray_include
#define PIMarray_include

#include <vector>
#include <stdint.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>

using namespace std;
using std::vector;

#define gateNOT 0
#define gateNAND 1
#define gateAND 2
#define gateWRITE 3
#define gateREAD 4
#define gateWRITEimmediate 5
#define gateREADimmediate 6

char ISA[6][10] = { {"NOT"},{"NAND"},{"AND"},{"WRITE"},{"READ"},{"WRITE_I"} };

#if BOUNDCHECKING
#include "boundchecking.cpp"
#endif

class PIMarray{
    public:
        int Nl; //Number logical of rows
        int Ml; //Number logical of columns
        int Np; //Number physical of rows
        int Mp; //Number physical of columns
        
        int *r_l2p; //Logical to physical mapping for rows (Nl entries)
        int *c_l2p; //Logical to physical mapping for columns (Ml entries)
    
        int *buffer; //Buffer for holding data between reads and writes
    
        int *v;   //Values of array (NpxMp entries)
        uint32_t *S;   //Switches left for each cell (NpxMp entries)
        
        uint32_t *writes; //Number of writes to each cell (NpxMp entries)
        uint32_t *reads;  //Number of reads to each cell (NpxMp entries)
    
        bool rowRenaming; //Indicates whether renaming is being performed or not
        vector<int> freeRows; //"free" columns, those not mapped to logical addresses (Mp-Ml entries)
        bool colRenaming;
        vector<int> freeCols;
    
        int rowRenamingPeriod;
        int colRenamingPeriod;
    
        int *colCounters;
    
        int pgate_count;
        double avg_cols;
    
    //Constructor
    PIMarray(int N, int M, bool rnaming, int rowSpace, int rowPeriod,
             bool cnaming, int colSpace, int colPeriod, uint32_t switches){
        pgate_count = 0;
        double avg_cols = 0.0;
        
        //Set array dimensions and configuration
        Np = N;
        Mp = M;         
        Nl = Np - rowSpace;
        Ml = Mp - colSpace;  
        
        rowRenaming = rnaming;
        colRenaming = cnaming;
        
        rowRenamingPeriod = rowPeriod;
        colRenamingPeriod = colPeriod;
        
        colCounters = new int[Ml](); //not used yet
        
        //Renaming structures
        r_l2p = new int[Nl];      //l2p is 1:1
        for(int i=0;i<Nl;i++)
            r_l2p[i] = i;
        for(int i=Nl;i<Np;i++)  
            freeRows.push_back(i);
        c_l2p = new int[Ml];
        for(int i=0;i<Ml;i++)
            c_l2p[i] = i;
        printf("Ml=%d Mp=%d\n",Ml,Mp);
        for(int i=Ml;i<Mp;i++){  
            printf("freecols %d\n",i);
            freeCols.push_back(i);
        }
        //Values of array
        int nIntegers = Np*Mp/(8*sizeof(int)); //Store bits in integers
        v = new int[nIntegers];
        for(int i=0;i<nIntegers;i++) //size of rows or cols with need be multiple of 32
            v[i] = 0;
        
        //Buffer
        buffer = new int[Mp]; //number of physical columns
        
        //Switches
        S = new uint32_t[Np*Mp];
        for(int i=0;i<Np*Mp;i++)
            S[i] = switches;
        
        //Writes
        writes = new uint32_t[Np*Mp];
        for(int i=0;i<Np*Mp;i++)
            writes[i] = 0;
        
        //Reads 
        reads = new uint32_t[Np*Mp];
        for(int i=0;i<Np*Mp;i++)
            reads[i] = 0;
    }
    
    //Destructor
    ~PIMarray(){
        delete[] r_l2p;
        delete[] c_l2p;
        delete[] v;
        delete[] S;
        delete[] writes;
        delete[] reads;
    }
    
    private:
        //Find bit value in array (from physical address)
        inline unsigned int bitR(int row, int col){
            if(row < 0 || row >= Np || col < 0 || col >= Np)
                std::cout <<  "Error in bitR, row = " << row << " col = " << col << "\n" << std::flush;
            int Aidx = row*Mp + col;   //(row,col) to single index
            int Iidx = Aidx/(8*sizeof(int));  //Integer index 
            int Bidx = Aidx%(8*sizeof(int));  //Bit index
            unsigned int val = ( v[Iidx] & (1u << Bidx) ) >> Bidx; //Read int with bit in it, AND w/ 1 bit at Bidx, shift back to LSB
            return(val);
        }

        //Set bit value in array (from physical address)
        inline void bitW(int row, int col, unsigned int val){
            if(row < 0 || row >= Np || col < 0 || col >= Np)
                std::cout <<  "Error in bitW, row = " << row << " col = " << col << "\n" << std::flush;
            int Aidx = row*Mp + col;   //(row,col) to single index
            int Iidx = Aidx/(8*sizeof(int));  //Integer index 
            int Bidx = Aidx%(8*sizeof(int));  //Bit index
            if(val == 0){
                int mask = ~(1 << Bidx); //All 1's except at Bidx
                v[Iidx] &= mask; //AND with mask
            }else{
                int mask = (1 << Bidx); //All 0's except at Bidx
                v[Iidx] |= mask; //AND with mask
            }
        }

        //Logic definitions (would be handled by electronics), a and b must be 1 or 0
        inline unsigned int logic(int gate, int a, int b){
            switch(gate){
                case gateNOT:
                    return(1-a);
                    break;
                case gateNAND:
                    return(1 - (a & b));
                    break;
                case gateAND:
                    return(a & b);
                    break;
            }
            cout << "Error: Invalid ype of logic not specified for PIMarray" << endl;
            return(-1);
        };

        //Perform gate in array (physical address)
        void pgate(const vector <int>& cols, int in1, int in2, int out, int gate){
            pgate_count++;
            double dpgate_count = (double)pgate_count;
            double ncols = (double)cols.size();
            avg_cols = ncols/dpgate_count + (avg_cols*(dpgate_count-1))/dpgate_count;
                        
            for(int i=0;i<cols.size();i++){
                //Update switches
                if(S[out*Mp+cols[i]] > 0)
                    S[out*Mp+cols[i]]--;
                
                //Increment read/write counters
                reads[in1*Mp+cols[i]]++; 
                if(in2 >= 0)
                    reads[in2*Mp+cols[i]]++;
                writes[out*Mp+cols[i]]++;

                //Compute logic
                unsigned int a = bitR(in1,cols[i]);
                unsigned int b;
                if(in2 >= 0)
                    b = bitR(in2,cols[i]);
                unsigned int val = logic(gate,a,b);

                //Write the output bit
                bitW(out,cols[i],val);   //Set output bit to value
            }
        }
    
        //Write a sequence of bits (physical addresses)
        void pwrite(const vector <int>& cols, int row, const vector <int>& vals){
            for(int i=0;i<cols.size();i++){
                bitW(row,cols[i],vals[i]);
                if(S[row*Mp+cols[i]] > 0)
                S[row*Mp+cols[i]]--;
                //Increment read/write counters
                writes[row*Mp+cols[i]]++;
            }
        }
    
        //Read a sequence of bits (phyiscal addresses)
        vector <int> pread(const vector <int>& cols, int row){
            vector <int> v;        
            for(int i=0;i<cols.size();i++)
                v.push_back(bitR(row,cols[i]));
            return(v);
        }
    
        //Logical to physical addressing
    
        //Logical to physical translation for columns
        vector <int> col_l2p(const vector <int>& cols){        
            if(colRenaming){
                vector <int> pcols; //Physical columns
                for(int i=0;i<cols.size();i++)
                    pcols.push_back(c_l2p[cols[i]]);
                return(pcols);
            }else{
                return(cols);
            }
        }
    
        //Hardware based renaming
        inline void row_rename(int lrow){
            static int opCount = 0;
            opCount++;
            if(opCount >= rowRenamingPeriod){
                opCount = 0;
                int p1 = r_l2p[lrow];
                int p2 = freeRows[0];
                freeRows.erase(freeRows.begin());
                freeRows.push_back(p1);
                r_l2p[lrow] = p2;
            }
        }

        inline void col_rename(){
            static int opCount = 0;
            opCount++;
            if(opCount >= colRenamingPeriod){
                opCount = 0;
                int cSwap = rand() % Ml;
                int p1 = c_l2p[cSwap];
                int p2 = freeCols[0];
                freeCols.erase(freeCols.begin());
                freeCols.push_back(p1);
                c_l2p[cSwap] = p2;
                //Data transfer
                for(int i=0;i<Nl;i++){
                    int v = bitR(i,p1);
                    bitW(i,p2,v);
                }
            }
        }
    
        //Logical to physical translation for columns
        inline int row_l2p(int row){
            if(rowRenaming && row >= 0)
                return(r_l2p[row]);
            else
                return(row);
        }
    
        //Swap physical addresses of 2 logical rows
        inline void rowSwap(int r1, int r2){
            int p1 = r_l2p[r1];
            int p2 = r_l2p[r2];
            r_l2p[r1] = p2;
            r_l2p[r2] = p1;
        }

        //Logical level operations
    
        //Perform gate in array (logical address)
        inline void lgate(const vector <int>& cols, int in1, int in2, int out, int gate){
            //Check for renaming opportunity
            if(rowRenaming) //if renaming
                if(cols.size() >= Ml) //if all columns are participating
                    row_rename(out);  //check if rename row
            if(colRenaming) //if col renaming
                col_rename(); //call every time
            //Logical to physical mapping
            vector<int> pcols = col_l2p(cols); //The physical columns
            in1 = row_l2p(in1);
            in2 = row_l2p(in2);
            out = row_l2p(out);
            //Call physical gate
            pgate(cols,in1,in2,out,gate);
        }
    
        inline void lwrite(const vector <int>& cols, int row, const vector <int>& vals){
            //Check for renaming opportunity
            if(rowRenaming) //if renaming
                if(cols.size() >= Ml) //if all columns are participating
                    row_rename(row);  //check if rename row
            vector<int> pcols = col_l2p(cols);
            int prow = row_l2p(row);
            pwrite(pcols,prow,vals);
        }
    
        vector <int> lread(const vector <int>& cols, int row){
            vector<int> pcols = col_l2p(cols);
            int prow = row_l2p(row);
            return(pread(pcols,prow));
        }
    
    public:
        //Instruction Set (Logical addresses)
        void NOT(vector <int>& cols, int in1, int out){
            lgate(cols,in1,-1,out,gateNOT);
        }

        void NAND(vector <int>& cols, int in1, int in2, int out){
            lgate(cols,in1,in2,out,gateNAND);
        }

        void AND(vector <int>& cols, int in1, int in2, int out){
            lgate(cols,in1,in2,out,gateAND);
        }
    
        void WRITEimmediate(const vector <int>& cols, int row, const vector <int>& vals){
            lwrite(cols,row,vals);
        }
    
        void WRITE(const vector <int>& cols, int row, const vector <int>& vals){
            vector <int> bvals;
            for(int i=0;i<vals.size();i++)
                bvals.push_back(buffer[vals[i]]); //get values from buffer and then write
            lwrite(cols,row,bvals);
        }
    
        //Overload of WRITE with a single value
        void WRITEimmediate(const vector <int>& cols, int row, int val){
            vector <int> vals;
            vals.push_back(val);
            lwrite(cols,row,vals);
        }
    
        //READ and store in buffer
        void READ(const vector <int>& cols, int row, const vector <int>& vals){ 
            vector <int> values = lread(cols,row);
            for(int i=0;i<vals.size();i++)
                buffer[vals[i]] = values[i];
        }
    
        //READ and return value in function
        vector <int> READimmediate(const vector <int>& cols, int row){  
            return(lread(cols,row));                          //useful for reporting/debugging but not for simulation
        }
    
        //Row management
        //Check whether a row is pristine or not
        inline bool isPristine(int r){
            for(int i=0;i<Mp;i++) //Check each column
                if(S[r*Mp+i] <= 0) //if any columns are out, the row is not pristine
                    return(false);
            return(true);
        }
    
        //Valuable info functions
        vector <int> pristineRows(){
            vector <int> rows;
            for(int i=0;i<Np;i++){    //Check each row
                if(isPristine(i))     //if it is pristine, save it
                    rows.push_back(i);
            }
            return(rows);
        }
    
        //Number of rows which have all bit cells still working
        int nPristineRows(){
            return((int)pristineRows().size());
        }
    
        //Number of available pristine rows
        int nAvailablePristineRows(){
            return(nPristineRows()-(Np-Nl)); //pristine rows minus space for renaming
        }
    
        //Change logical to phsical addresses so perturbed rows are not used
        int remap(){
            //Get arrays of prstine/not for logical rows and renaming space
            bool *pristine = new bool[Nl];
            for(int i=0;i<Nl;i++)
                pristine[i] = isPristine(r_l2p[i]);
            //Put invalid (not pristine) rows at the highest logical addresses
            int h_idx = Nl-1;
            //First, ensure that renaming space has valid rows
            for(int i=0;i<freeRows.size();i++){
                if(!isPristine(freeRows[i])){ //if free row is not pristine
                    int t = freeRows[i];
                    while(!isPristine(r_l2p[h_idx])) //Find first pristine logical row
                        h_idx--;
                    freeRows[i] = r_l2p[h_idx]; //give free row the phyiscal row of logical row
                    r_l2p[h_idx] = t; //map this logical row to the previous (invalid) free row
                }
            }
            //Second swap low logical address invalid with high logical address
            int l_idx = 0;
            while(true){
                while(isPristine(r_l2p[l_idx]) && l_idx < Nl-1) //ensure pristine from 0
                    l_idx++;
                while(!isPristine(r_l2p[h_idx]) && h_idx > 1) //look for invalid back from Nl
                    h_idx--;
                if(l_idx < h_idx)
                    rowSwap(l_idx,h_idx);
                else
                    break;
            }
            //Return how many logical columns are available after remapping
            int nL = Nl;
            for(int i=0;i<Nl;i++){
                if(!isPristine(r_l2p[i])){
                    nL = i; //past 1 (0 indexing)
                    break;
                }
            }
            return(nL); 
        }
    
        //Check how many logical rows are fully intact (as done in remap, but without remap)
        int nPristineLogicalRows(){
            for(int i=0;i<Nl;i++)
                if(!isPristine(r_l2p[i]))
                    return(i);
            return(Nl);
        }
    
        uint32_t avgSwitchesLeft(){
            unsigned long long sw = 0;
            for(int i=0;i<Np*Mp;i++)
                sw += S[i];
            sw /= (unsigned long long)(Np*Mp);
            return((uint32_t)sw);
        }

        //Reporting functions
        void printValues(){
            cout << "Array Values" << endl;
            for(int i=0;i<Np;i++){
                for(int j=0;j<Mp;j++)
                    cout << bitR(i,j);
                cout << endl;
            }
        }

        void printIntegers(){
            cout << "Integer Values" << endl;
            for(int i=0;i<Np*Mp/(8*sizeof(int));i++)
                cout << v[i] << endl;
        }
    
        vector <int> f(){
            vector<int> v;
            v.push_back(5);
            return(v);
        }
    
        unsigned long long valueAt(int col, const vector<int>& rows){
            unsigned long long val = 0;
            vector <int> cols;
            cols.push_back(col);
            vector<int> vals;
            for(int i=0;i<rows.size();i++){
                vals = READimmediate(cols,rows[i]);
                if(vals[0] == 1)
                    val += pow(2,i);
            }
            return(val);
        }
    
        void print_r_l2p(){
            printf("Computation Rows\n");
            for(int i=0;i<Nl;i++)
                printf("%d: %d\n",i,r_l2p[i]);
            printf("Renaming Rows\n");
            for(int i=0;i<freeRows.size();i++)
                printf("%d\n",(int)freeRows[i]);
        }
    
        //Get r_l2p and free rows and also be able to set them
        vector<int> get_r_l2p(){
            vector<int> rl2p;
            for(int i=0;i<Nl;i++)
                rl2p.push_back(r_l2p[i]);
            return(rl2p);
        }
        
        vector<int> get_freeRows(){
            vector<int> fR;
            for(int i=0;i<freeRows.size();i++)
                fR.push_back(freeRows[i]);
            return(fR);
        }
    
        void set_r_l2p(vector<int> rl2p){
            for(int i=0;i<Nl;i++)
                r_l2p[i] = rl2p[i];
        }
    
        void set_freeRows(vector<int> fR){
            for(int i=0;i<freeRows.size();i++)
                freeRows[i] = fR[i];
        }
    
        //Report number of writes in a column
        void print_writesCol(int col){
            for(int i=0;i<Np;i++){
                printf("Row %d, Col %d, writes: %u, S: %u\n",i,col,writes[i*Mp+col],S[i*Mp+col]);
            }
        }
    
        uint32_t sum_writes(){
            uint32_t sum = 0;
            for(int i=0;i<Np;i++)
                for(int j=0;j<Mp;j++)
                    sum += writes[i*Mp+j];
            return(sum);
        }
    
        void print_S(){
            for(int i=0;i<Np;i++){
                for(int j=0;j<Mp;j++)
                    printf("|%u|",S[i*Mp+j]);
                printf("\n");
            }
        }
        void print_Ssymbols(){
            string symbols = "ZYXWVUT";
            for(int i=0;i<Np;i++){
                for(int j=0;j<Mp;j++){
                    uint32_t s = S[i*Mp+j];
                    uint32_t t = 1;
                    bool printed = false;
                    for(int k=0;k<symbols.length();k++){
                        if(s < t ){
                            printf("%c",symbols.at(k));
                            printed = true;
                            break;
                        }
                        t += 100;
                    }
                    if(!printed)
                        printf("_");
                }
                printf("\n");
            }
        }
};

#endif
#ifndef operations_include
#define operations_include

#include "adders.cpp"
#include <algorithm>
#include <stdio.h>

void printVector(const vector<int>& v){
    for(int i=0;i<v.size()-1;i++)
        printf("%d,",v[i]);
    printf("%d\n",v[v.size()-1]);
}

void add(PROGRAM *prog, const vector <int>& cols, const vector <int>& a, const vector <int>& b, const vector <int>& c){
    if(a.size() != b.size())
        cout << "Error: Addition of numbers of unequal length" << endl;
    if(c.size() != a.size() + 1)
        cout << "Error: Addition sum is not 1 bit longer" << endl;
    
    int2 sc = halfAdd(prog,cols,a[0],b[0],c[0]);
    int Cout = sc.int2;
    for(int i=1;i<a.size()-1;i++){
        sc = fullAdd(prog,cols,a[i],b[i],Cout,c[i]);
        prog->freeBit(Cout);
        Cout = sc.int2;
    }
    fullAdd(prog,cols,a[a.size()-1],b[a.size()-1],Cout,c[a.size()-1],c[c.size()-1]);
}

int compare(PROGRAM *prog, const vector <int>& cols, const vector <int>& a, const vector <int>& b){
    int Bout = halfBorrow(prog,cols,a[0],b[0]);
    for(int i=1;i<a.size();i++){
        int tBout = fullBorrow(prog,cols,a[i],b[i],Bout);
        prog->freeBit(Bout);
        Bout = tBout;
    }
    return(Bout);
}

void multiply(PROGRAM *prog, const vector <int>& cols, const vector<int>& a, const vector <int>& b, const vector <int>& c){
    if(a.size() != b.size())
        cout << "Error: Multiplication of numbers of unequal length" << endl;
    if(c.size() != 2*a.size())
        cout << "Error: Multiplication sum is not twice as long" << endl;
    
    //First bit is automatic
    prog->WRITEimmediate(cols,c[0],1); //Preset 0th bit of c to 1
    prog->AND(cols,a[0],b[0],c[0]);
    //For loop from LSB to MSB
    vector <int> bits_c; //"Current" bits, bits at "this" significance
    vector <int> bits_n; //"Next" bits, bits at 1 higher significance
    for(int B=2;B<max((int)c.size(),3);B++){ //iterate over bits, up to c.size(), at least once (if c is 2 bits)
        bits_c.clear();  //clear out vector
        bits_c = bits_n; //Move "next" bits to "current" bits
        bits_n.clear();  //clear out vector
        
        //Add "current" bits from a and b (combos of a[i], b[j] where B=i+j)
        int low = max(0,B-(int)a.size());    //can't use bits less than 0
        int high = min(B-1,(int)a.size()-1); //can't use bits beyond length of a or b
        if(low < a.size()){
            for(int i=low;i<=high;i++){
                int j = B - 1 - i;
                int n = prog->getBit(); //get a bit to AND a[i] and b[j] into
                prog->WRITEimmediate(cols,n,1); //preset
                prog->AND(cols,a[i],b[j],n);  //AND
                bits_c.push_back(n);          //add it to current bits
            }
        }

        //Reduce with adders until only 1 bit left
        while(bits_c.size() > 2){ //full adds if 3 or more bits
            int S = -1; //don't care where S goes
            if(bits_c.size() == 3)
                S = c[B-1]; //unless there are 3 bits left in current, then sum goes to c[B]
            int Cout = -1; //don't care where Cout goes
            if(B == c.size()-1)//unless this is 2nd MSB, then Cout goes to MSB
                Cout = c[c.size()-1];
            int2 sc = fullAdd(prog,cols,bits_c[0],bits_c[1],bits_c[2],S,Cout); //do full add
            S = sc.int1;    //extract S and Cout
            Cout = sc.int2;
            //Remove and add relevant bits
            prog->freeBit(bits_c[0]); //free first 3 bits
            prog->freeBit(bits_c[1]);
            prog->freeBit(bits_c[2]);
            for(int i=0;i<3;i++)
                bits_c.erase(bits_c.begin()); //remove from current bits
            bits_c.push_back(S); //Add S back to current
            bits_n.push_back(Cout); //Add Cout to next
        }
        if(bits_c.size() == 2){ //do an extra half-add if necessary
            int S = c[B-1]; //Since only 2 bits left, sum goes to c[B]
            int Cout = -1; //don't care where Cout goes
            if(B == c.size()-1)//unless this is 2nd MSB, then Cout goes to MSB
                Cout = c[c.size()-1];
            int2 sc = halfAdd(prog,cols,bits_c[0],bits_c[1],S,Cout); //do half add
            S = sc.int1;
            Cout = sc.int2;
            //Add and remove relevant bits
            prog->freeBit(bits_c[0]);
            prog->freeBit(bits_c[1]);
            for(int i=0;i<2;i++)
                bits_c.erase(bits_c.begin()); //remove from current bits
            //Only 1 bit left, don't need to add
            bits_n.push_back(Cout); //Add Cout to next
        }
    }
}

vector <int> SUM(PROGRAM *prog, const vector <int>& cols, vector<vector<int>> operands){
    
    vector <vector <int>> toperands;
    printf("There are %d operands of length %d\n",(int)operands.size(),(int)(operands[0].size()));
    //Operand length
    int L = (int)(operands[0].size());
    //While there is more than one, summ together
    while(operands.size() > 1){
        //printf("There are %d operands of %d-bits\n",(int)operands.size(),L);
        //Sum operands, storing in toperands
        while(operands.size() > 1){
            //Get output space
            vector <int> output;
            for(int i=0;i<L+1;i++)
                output.push_back(prog->getBit());
            //Add two operands and put in output
            //printf("Adding %d-bit and %d-bit\n",(int)operands[0].size(),(int)operands[1].size());
            add(prog,cols,operands[0],operands[1],output);
            //Add output to new operands
            toperands.push_back(output);
            //Free input bits
            for(int i=0;i<L;i++){
                prog->freeBit(operands[0][i]);
                prog->freeBit(operands[1][i]);
            }
            //Delete inputs
            operands.erase(operands.begin());
            operands.erase(operands.begin());
        }
        //Check for odd case
        if(operands.size() > 0){
            toperands.push_back(operands[0]);
            for(int i=0;i<L;i++)
                prog->freeBit(operands[0][i]);
        }
        L++;
        operands.clear();
        operands = toperands;
        toperands.clear();
    }
    return(operands[0]);
}
vector <int> SUM(PROGRAM *prog, const vector <int>& cols, const vector <int>& bits){
    //printf("Begin summation\n");
    //Vector of vectors to hold operands
    vector <vector <int>> operands;
    for(int i=0;i<bits.size();i++){
        vector <int> operand = {bits[i]};
        operands.push_back(operand);
    }
    return SUM(prog,cols,operands);
}
#endif

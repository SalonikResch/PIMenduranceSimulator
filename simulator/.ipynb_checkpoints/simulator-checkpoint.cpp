#ifndef SIMULATOR_include
#define SIMULATOR_include 

#include "../cram/PIMarray.cpp"
#include "../software/program.cpp"
#include <vector>
using std::vector;

//Simulator to run a program on a PIM array
class SIMULATOR{
    private:
        //Program to run
        PROGRAM *program;

        //PIM array to run program on 
        PIMarray *pimArray;
        
        //CPU memory
        int nReg;
        int *reg;

    public:
        //Constructor
        SIMULATOR(PIMarray *parray, PROGRAM *prog){
            program = prog;
            pimArray = parray;
        }
    
        SIMULATOR(PIMarray *parray){
            pimArray = parray;
        }
        
        void setProgram(PROGRAM *prog){
            program = prog;
        }
        
        void runProgram(){
            uint32_t write_sum = 0;
            //std::cout << "Running" << std::endl;
            vector <instruction> I = program->getInstructions();
            //Iterate over the instructions
            for(int i=0;i<I.size();i++){
                instruction ins = I[i]; //Load instruction
                //Use switch statement to issue instructions
                //program->printInstruction(ins);
                switch(ins.opcode){
                    case(gateNOT):
                        pimArray->NOT(ins.cols,ins.in1,ins.out);
                        break;
                    case(gateNAND):
                        pimArray->NAND(ins.cols,ins.in1,ins.in2,ins.out);
                        break;
                    case(gateAND):
                        pimArray->AND(ins.cols,ins.in1,ins.in2,ins.out);
                        break;
                    case(gateWRITE):
                        pimArray->WRITE(ins.cols,ins.out,ins.vals);
                        break;
                    case(gateWRITEimmediate):
                        pimArray->WRITEimmediate(ins.cols,ins.out,ins.vals);
                        break;
                    case(gateREAD):
                        pimArray->READ(ins.cols,ins.in1,ins.vals);
                        break;
                    case(gateREADimmediate):
                        pimArray->READimmediate(ins.cols,ins.in1);
                        break;
                }
//                 uint32_t write_sum_new = pimArray->sum_writes();
//                 //std::cout << "After instruction " << i << " there are " << write_sum_new - write_sum << " more writes\n" << std::flush;
//                 if(write_sum_new < (write_sum+1024)){
//                     std::cout << "Instruction had only " << write_sum_new - write_sum << " more writes\n" << std::flush;
//                     program->printInstruction(ins);
//                 }
//                 write_sum = write_sum_new;
            }
        }
};
#endif
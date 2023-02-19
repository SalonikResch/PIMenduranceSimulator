//Class to hold statistics about an experiment
class statistics{
    unsigned long long latency; //In nanoseconds
    unsigned long long energy; //In picoJoules
    
    unsigned long long nGates;
    unsigned long long nOps;
    
    uint32_t **switches;
}
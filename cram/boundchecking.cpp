#ifndef BOUNDCHECKING_include
#define BOUNDCHECKING_include

//Boundary checking for debugging
inline bool bc(int a, int limit){
    if(a < 0 || a >= limit)
        return(false);
    return(true);
}

inline bool bc(const vector<int> cols, int climit, int a, int limit){
    if(!bc(a,limit))
        return(false);
    for(int i=0;i<cols.size();i++)
        if(!bc(cols[i],climit))
            return(false);
    return(true);
}

inline bool bc(const vector<int> cols, int climit, int a, int b, int limit){
    if(!bc(b,limit))
        return(false);
    return(bc(cols,climit,a,limit));
}

inline bool bc(const vector<int> cols, int climit, int a, int b, int c, int limit){
    if(!bc(c,limit))
        return(false);
    return(bc(cols,climit,a,b,limit));
}

void bcReport(char *gate,)

inline void boundaryCheck(const vector<int> cols, int climit, int a, int limit)
inline void boundaryCheck(const vector<int> cols, int climit, int a, int b, int limit)
inline void boundaryCheck(const vector<int> cols, int climit, int a, int b, int c, int limit)

#endif
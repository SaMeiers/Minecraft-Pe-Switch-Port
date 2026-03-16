#include "switch_utils.h"
#include <cstdio>
#include <cstring>

extern "C" {

int getRemainingFileSize(FILE* fp) {
    if (!fp) return 0;
    
    long currentPos = ftell(fp);
    if (currentPos == -1) return 0;
    
    if (fseek(fp, 0, SEEK_END) != 0) {
        fseek(fp, currentPos, SEEK_SET);
        return 0;
    }
    
    long size = ftell(fp);
    fseek(fp, currentPos, SEEK_SET);
    
    return (size > 0) ? size : 0;
}

} 

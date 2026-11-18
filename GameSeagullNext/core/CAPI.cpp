#include "Seagull.hpp"
#include <cstdlib>
#include <cstring>
extern "C" {
// Small C ABI for prerecorded/streamed validation. Caller frees returned JSON with sg_free.
const char* sg_analyze(const unsigned char*p,int w,int h,int stride){
 auto data=sg::json(sg::detectBGRA(p,w,h,stride));
 char* result=(char*)std::malloc(data.size()+1);if(result)std::memcpy(result,data.c_str(),data.size()+1);return result;
}
void sg_free(const char*p){std::free((void*)p);}
}

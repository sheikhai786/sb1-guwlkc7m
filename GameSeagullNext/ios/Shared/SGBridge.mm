#import "SGBridge.h"
#include "../../core/Seagull.hpp"
@implementation SGBridge
+ (NSString *)analyze:(CVPixelBufferRef)buffer group:(NSInteger)group advanced:(BOOL)advanced eightAllowed:(BOOL)eightAllowed {
 if(CVPixelBufferGetPixelFormatType(buffer)!=kCVPixelFormatType_32BGRA)return @"{\"valid\":false,\"reason\":\"Unsupported pixel format\"}";
 CVPixelBufferLockBaseAddress(buffer,kCVPixelBufferLock_ReadOnly);
 auto s=sg::detectBGRA((const uint8_t*)CVPixelBufferGetBaseAddress(buffer),(int)CVPixelBufferGetWidth(buffer),(int)CVPixelBufferGetHeight(buffer),(int)CVPixelBufferGetBytesPerRow(buffer));
 CVPixelBufferUnlockBaseAddress(buffer,kCVPixelBufferLock_ReadOnly);
 sg::Options o;o.group=group==2?sg::solid:group==3?sg::stripe:sg::unknown;o.banks=advanced;o.combinations=advanced;o.eightAllowed=eightAllowed;
 return [NSString stringWithUTF8String:sg::json(s,o).c_str()];
}
@end

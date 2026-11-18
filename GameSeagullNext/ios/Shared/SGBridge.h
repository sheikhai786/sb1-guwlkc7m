#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>
NS_ASSUME_NONNULL_BEGIN
@interface SGBridge : NSObject
+ (NSString *)analyze:(CVPixelBufferRef)buffer group:(NSInteger)group advanced:(BOOL)advanced eightAllowed:(BOOL)eightAllowed;
@end
NS_ASSUME_NONNULL_END

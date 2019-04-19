#import "GPUImageTextureInput.h"

@implementation GPUImageTextureInput

#pragma mark -
#pragma mark Initialization and teardown

- (id)initWithTexture:(GLuint)newInputTexture size:(CGSize)newTextureSize;
{
    if (!(self = [super init]))
    {
        return nil;
    }

    runSynchronouslyOnVideoProcessingQueue(^{
        [GPUImageContext useImageProcessingContext];
    });
    
    textureSize = newTextureSize;

    runSynchronouslyOnVideoProcessingQueue(^{
        self->outputFramebuffer = [[GPUImageFramebuffer alloc] initWithSize:newTextureSize overriddenTexture:newInputTexture];
    });
    
    return self;
}

#pragma mark -
#pragma mark Image rendering

- (void)processTextureWithFrameTime:(CMTime)frameTime;
{
    runAsynchronouslyOnVideoProcessingQueue(^{
        for (id<GPUImageInput> currentTarget in self->targets)
        {
            NSInteger indexOfObject = [self->targets indexOfObject:currentTarget];
            NSInteger targetTextureIndex = [[self->targetTextureIndices objectAtIndex:indexOfObject] integerValue];
            
            [currentTarget setInputSize:self->textureSize atIndex:targetTextureIndex];
            [currentTarget setInputFramebuffer:self->outputFramebuffer atIndex:targetTextureIndex];
            [currentTarget newFrameReadyAtTime:frameTime atIndex:targetTextureIndex];
        }
    });
}

@end

#import "GPUImageTwoPassTextureSamplingFilter.h"

@implementation GPUImageTwoPassTextureSamplingFilter

@synthesize verticalTexelSpacing = _verticalTexelSpacing;
@synthesize horizontalTexelSpacing = _horizontalTexelSpacing;

#pragma mark -
#pragma mark Initialization and teardown

- (id)initWithFirstStageVertexShaderFromString:(NSString *)firstStageVertexShaderString firstStageFragmentShaderFromString:(NSString *)firstStageFragmentShaderString secondStageVertexShaderFromString:(NSString *)secondStageVertexShaderString secondStageFragmentShaderFromString:(NSString *)secondStageFragmentShaderString
{
    if (!(self = [super initWithFirstStageVertexShaderFromString:firstStageVertexShaderString firstStageFragmentShaderFromString:firstStageFragmentShaderString secondStageVertexShaderFromString:secondStageVertexShaderString secondStageFragmentShaderFromString:secondStageFragmentShaderString]))
    {
		return nil;
    }
    
    runSynchronouslyOnVideoProcessingQueue(^{
        [GPUImageContext useImageProcessingContext];

        self->verticalPassTexelWidthOffsetUniform = [self->filterProgram uniformIndex:@"texelWidthOffset"];
        self->verticalPassTexelHeightOffsetUniform = [self->filterProgram uniformIndex:@"texelHeightOffset"];
        
        self->horizontalPassTexelWidthOffsetUniform = [self->secondFilterProgram uniformIndex:@"texelWidthOffset"];
        self->horizontalPassTexelHeightOffsetUniform = [self->secondFilterProgram uniformIndex:@"texelHeightOffset"];
    });
    
    self.verticalTexelSpacing = 1.0;
    self.horizontalTexelSpacing = 1.0;
    
    return self;
}

- (void)setUniformsForProgramAtIndex:(NSUInteger)programIndex;
{
    [super setUniformsForProgramAtIndex:programIndex];
    
    if (programIndex == 0)
    {
        glUniform1f(verticalPassTexelWidthOffsetUniform, verticalPassTexelWidthOffset);
        glUniform1f(verticalPassTexelHeightOffsetUniform, verticalPassTexelHeightOffset);
    }
    else
    {
        glUniform1f(horizontalPassTexelWidthOffsetUniform, horizontalPassTexelWidthOffset);
        glUniform1f(horizontalPassTexelHeightOffsetUniform, horizontalPassTexelHeightOffset);
    }
}

- (void)setupFilterForSize:(CGSize)filterFrameSize;
{
    runSynchronouslyOnVideoProcessingQueue(^{
        // The first pass through the framebuffer may rotate the inbound image, so need to account for that by changing up the kernel ordering for that pass
        if (GPUImageRotationSwapsWidthAndHeight(self->inputRotation))
        {
            self->verticalPassTexelWidthOffset = self->_verticalTexelSpacing / filterFrameSize.height;
            self->verticalPassTexelHeightOffset = 0.0;
        }
        else
        {
            self->verticalPassTexelWidthOffset = 0.0;
            self->verticalPassTexelHeightOffset = self->_verticalTexelSpacing / filterFrameSize.height;
        }
        
        self->horizontalPassTexelWidthOffset = self->_horizontalTexelSpacing / filterFrameSize.width;
        self->horizontalPassTexelHeightOffset = 0.0;
    });
}

#pragma mark -
#pragma mark Accessors

- (void)setVerticalTexelSpacing:(CGFloat)newValue;
{
    _verticalTexelSpacing = newValue;
    [self setupFilterForSize:[self sizeOfFBO]];
}

- (void)setHorizontalTexelSpacing:(CGFloat)newValue;
{
    _horizontalTexelSpacing = newValue;
    [self setupFilterForSize:[self sizeOfFBO]];
}

@end

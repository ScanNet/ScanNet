#import "GPUImageTwoInputCrossTextureSamplingFilter.h"

NSString *const kGPUImageTwoInputNearbyTexelSamplingVertexShaderString = SHADER_STRING
(
 attribute vec4 position;
 attribute vec4 inputTextureCoordinate;
 attribute vec4 inputTextureCoordinate2;
 
 uniform float texelWidth;
 uniform float texelHeight;
 
 varying vec2 textureCoordinate;
 varying vec2 leftTextureCoordinate;
 varying vec2 rightTextureCoordinate;
 varying vec2 topTextureCoordinate;
 varying vec2 bottomTextureCoordinate;
 
 varying vec2 textureCoordinate2;
 varying vec2 leftTextureCoordinate2;
 varying vec2 rightTextureCoordinate2;
 varying vec2 topTextureCoordinate2;
 varying vec2 bottomTextureCoordinate2;
 
 void main()
 {
     gl_Position = position;
     
     vec2 widthStep = vec2(texelWidth, 0.0);
     vec2 heightStep = vec2(0.0, texelHeight);
     
     textureCoordinate = inputTextureCoordinate.xy;
     leftTextureCoordinate = inputTextureCoordinate.xy - widthStep;
     rightTextureCoordinate = inputTextureCoordinate.xy + widthStep;
     topTextureCoordinate = inputTextureCoordinate.xy - heightStep;
     bottomTextureCoordinate = inputTextureCoordinate.xy + heightStep;
     
     textureCoordinate2 = inputTextureCoordinate2.xy;
     leftTextureCoordinate2 = inputTextureCoordinate2.xy - widthStep;
     rightTextureCoordinate2 = inputTextureCoordinate2.xy + widthStep;
     topTextureCoordinate2 = inputTextureCoordinate2.xy - heightStep;
     bottomTextureCoordinate2 = inputTextureCoordinate2.xy + heightStep;
 }
);

@implementation GPUImageTwoInputCrossTextureSamplingFilter

@synthesize texelWidth = _texelWidth;
@synthesize texelHeight = _texelHeight;

#pragma mark -
#pragma mark Initialization and teardown

- (id)initWithFragmentShaderFromString:(NSString *)fragmentShaderString;
{
    if (!(self = [super initWithVertexShaderFromString:kGPUImageTwoInputNearbyTexelSamplingVertexShaderString fragmentShaderFromString:fragmentShaderString]))
    {
        return nil;
    }
    
    texelWidthUniform = [filterProgram uniformIndex:@"texelWidth"];
    texelHeightUniform = [filterProgram uniformIndex:@"texelHeight"];
    
    return self;
}

- (void)setupFilterForSize:(CGSize)filterFrameSize;
{
    if (!hasOverriddenImageSizeFactor)
    {
        _texelWidth = 1.0 / filterFrameSize.width;
        _texelHeight = 1.0 / filterFrameSize.height;
        
        runSynchronouslyOnVideoProcessingQueue(^{
            [GPUImageContext setActiveShaderProgram:self->filterProgram];
            if (GPUImageRotationSwapsWidthAndHeight(self->inputRotation))
            {
                glUniform1f(self->texelWidthUniform, self->_texelHeight);
                glUniform1f(self->texelHeightUniform, self->_texelWidth);
            }
            else
            {
                glUniform1f(self->texelWidthUniform, self->_texelWidth);
                glUniform1f(self->texelHeightUniform, self->_texelHeight);
            }
        });
    }
}

#pragma mark -
#pragma mark Accessors

- (void)setTexelWidth:(CGFloat)newValue;
{
    hasOverriddenImageSizeFactor = YES;
    _texelWidth = newValue;
    
    [self setFloat:_texelWidth forUniform:texelWidthUniform program:filterProgram];
}

- (void)setTexelHeight:(CGFloat)newValue;
{
    hasOverriddenImageSizeFactor = YES;
    _texelHeight = newValue;
    
    [self setFloat:_texelHeight forUniform:texelHeightUniform program:filterProgram];
}

@end

#ifndef __IMAGE_BUFFER_H__
#define __IMAGE_BUFFER_H__

#include <glm/glm.hpp>
#include <string.h>
#include <pthread.h>

struct Sample;

class ImageBuffer
{
public:
    explicit ImageBuffer(const int width, const int height);
    ~ImageBuffer();

    void commit(const Sample& sample, const glm::vec4& color);
    void write(const std::string& filename) const;

private:
    
    float *mPixels;
    const int mWidth, mHeight;
    
    pthread_mutex_t mBufferLock;
};

#endif


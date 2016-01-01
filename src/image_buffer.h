#ifndef __IMAGE_BUFFER_H__
#define __IMAGE_BUFFER_H__

#include <glm/glm.hpp>
#include <string>

struct Sample;

class ImageBuffer
{
public:
    explicit ImageBuffer(unsigned width, unsigned height);
    ~ImageBuffer();

    void commit(const Sample& sample, const glm::vec4& color);
    void write(const std::string& filename) const;

private:
    float* mPixels;
    const unsigned mWidth, mHeight;
};

#endif


#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include "uplinksimple_image-codecs.h"
#include "uplinksimple_shift2depth.h"

void saveAsPGM(const std::string& outFile, unsigned int width, unsigned int height, unsigned short* data, bool binary = true) {
  if (binary) {
    std::ofstream of(outFile, std::ios::binary);
    std::stringstream ss;
    ss << "P5\n";
    ss << "# data values are 16-bit each" << "\n";
    ss << width << " " << height << "\n";
    ss << std::numeric_limits<unsigned short>::max() << "\n";
    of << ss.str();
    unsigned char* data_c = (unsigned char*)data;
    for (unsigned int i = 0; i < width*height; i++) {
      std::swap(data_c[2 * i + 0], data_c[2 * i + 1]);
    }
    of.write((const char*)data, width*height*sizeof(unsigned short));
  } else {
    std::stringstream ss;
    ss << "P2\n";
    ss << width << " " << height << "\n";
    ss << std::numeric_limits<unsigned short>::max() << "\n";
    for (unsigned int y = 0; y < height; y++) {
      for (unsigned int x = 0; x < width; x++) {
        unsigned int idx = y*width + x;
        ss << data[idx] << " ";
      }
      ss << "\n";
    }
    std::ofstream of(outFile);
    of << ss.str();
  }
}

void process(const std::string& depthFile, const std::string& outBasename,
             size_t numDepthFrames, size_t depthWidth = 640, size_t depthHeight = 480) {
  std::ifstream inDepth(depthFile, std::ios::binary);
  int frame = 0;

  while (frame < numDepthFrames) {
    assert(inDepth.good());

    // read depth frame
    uint32_t byteSize;
    inDepth.read((char*)&byteSize, sizeof(uint32_t));
    char* depthCompressed = new char[byteSize];
    inDepth.read((char*)depthCompressed, byteSize);

    // decode and convert from shift to depth
    unsigned short* depth = (unsigned short*) std::malloc(depthWidth*depthHeight * 2);
    uplinksimple::decode((unsigned char*)depthCompressed, byteSize, depthWidth*depthHeight, depth);
    uplinksimple::shift2depth(depth, depthWidth*depthHeight);

    //check for invalid values and set to 0
    for (int i = 0; i < depthWidth*depthHeight; i++) {
      if (depth[i] >= uplinksimple::shift2depth(0xffff)) {
        depth[i] = 0;
      }
    }

    // save to PGM
    const std::string filename = outBasename + "_" + std::to_string(frame) + ".pgm";
    saveAsPGM(filename, depthWidth, depthHeight, depth);

    delete[] depthCompressed;
    std::free(depth);
    std::cout << "frame " << frame << ": read " << byteSize << " [bytes] " << std::endl;
    frame++;
  }

  // read timestamps
  for (int i = 0; i < numDepthFrames; i++) {
    double timeStampDouble;
    inDepth.read((char*)&timeStampDouble, sizeof(double));
    uint64_t timeStamp = (uint64_t)(timeStampDouble*1000*1000);  // seconds to microseconds
    // TODO do something with timestamps
  }
}


int main(int argc, char* argv[]) {
  if (argc >= 3) {
    std::string depthFile(argv[1]);
    std::string outBasename(argv[2]);
    int numDepthFrames = 1;
    if (argc >= 4) {
      numDepthFrames = std::atoi(argv[3]);
    }
    process(depthFile, outBasename, numDepthFrames);
  } else {
    std::cerr << "Usage: depth2pgm path/to/file.depth pgm_seq_basename [numDepthFrames]" << std::endl;
  }
  return 0;
}

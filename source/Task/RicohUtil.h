#pragma once

#include "ZBlend.h"
#include "PinnedMemoryPool.h"
#include "ConcurrentQueue.h"
#include "opencv2/core.hpp"
#include <memory>
#include <string>

class RicohPanoramaRender
{
public:
    RicohPanoramaRender() : success(0) {};
    bool prepare(const std::string& path, const cv::Size& srcSize, const cv::Size& dstSize);
    void render(const cv::Mat& src, cv::Mat& dst);
private:
    cv::Size srcFullSize;
    cv::Mat dstSrcMap1, dstSrcMap2;
    cv::Mat from1, from2, intersect;
    cv::Mat weight1, weight2;
    int success;
};

class DetuPanoramaRender
{
public:
    DetuPanoramaRender() : success(0) {};
    bool prepare(const std::string& path, const cv::Size& srcSize, const cv::Size& dstSize);
    void render(const cv::Mat& src, cv::Mat& dst);
private:
    cv::Size srcFullSize;
    cv::Mat dstSrcMap;
    int success;
};

class PanoramaRender
{
public:
    enum BlendType 
    {
        BlendTypeLinear, 
        BlendTypeMultiband
    };
    virtual ~PanoramaRender() {};
    virtual bool prepare(const std::string& path, int blendType, const cv::Size& srcSize, const cv::Size& dstSize) = 0;
    virtual bool render(const std::vector<cv::Mat>& src, cv::Mat& dst) = 0;
};

class DualGoProPanoramaRender : public PanoramaRender
{
public:
    DualGoProPanoramaRender() : success(0) {};
    ~DualGoProPanoramaRender() {};
    bool prepare(const std::string& path, int blendType, const cv::Size& srcSize, const cv::Size& dstSize);
    bool render(const cv::Mat& src1, const cv::Mat& src2, cv::Mat& dst);
    bool render(const std::vector<cv::Mat>& src, cv::Mat& dst);
private:
    cv::Size srcFullSize;
    cv::Mat dstSrcMap1, dstSrcMap2;
    cv::Mat mask1, mask2;
    cv::Mat from1, from2, intersect;
    cv::Mat weight1, weight2;
    int success;
};

class CPUMultiCameraPanoramaRender : public PanoramaRender
{
public:
    CPUMultiCameraPanoramaRender() : success(0) {};
    ~CPUMultiCameraPanoramaRender() {};
    bool prepare(const std::string& path, int blendType, const cv::Size& srcSize, const cv::Size& dstSize);
    bool render(const std::vector<cv::Mat>& src, cv::Mat& dst);
private:
    cv::Size srcFullSize;
    std::vector<cv::Mat> dstSrcMaps;
    std::vector<cv::Mat> masks;
    std::vector<cv::Mat> reprojImages;
    TilingMultibandBlendFastParallel blender;
    int numImages;
    int success;
};

class CudaMultiCameraPanoramaRender : public PanoramaRender
{
public:
    CudaMultiCameraPanoramaRender(): success(0) {};
    ~CudaMultiCameraPanoramaRender() {};
    bool prepare(const std::string& path, int blendType, const cv::Size& srcSize, const cv::Size& dstSize);
    bool render(const std::vector<cv::Mat>& src, cv::Mat& dst);
private:
    cv::Size srcFullSize;
    std::vector<cv::cuda::GpuMat> dstSrcXMapsGPU, dstSrcYMapsGPU;
    std::vector<cv::cuda::HostMem> srcMems;
    std::vector<cv::Mat> srcImages;
    std::vector<cv::cuda::GpuMat> srcImagesGPU;
    std::vector<cv::cuda::GpuMat> reprojImagesGPU;
    cv::cuda::GpuMat blendImageGPU;
    cv::Mat blendImage;
    std::vector<cv::cuda::Stream> streams;
    CudaTilingMultibandBlendFast blender;
    int numImages;
    int success;
};

// render accepts pinned memory cv::Mat
class CudaMultiCameraPanoramaRender2 : public PanoramaRender
{
public:
    CudaMultiCameraPanoramaRender2() : success(0) {};
    ~CudaMultiCameraPanoramaRender2() {};
    bool prepare(const std::string& path, int blendType, const cv::Size& srcSize, const cv::Size& dstSize);
    bool render(const std::vector<cv::Mat>& src, cv::Mat& dst);
private:
    cv::Size srcFullSize;
    std::vector<cv::cuda::GpuMat> dstSrcXMapsGPU, dstSrcYMapsGPU;
    std::vector<cv::cuda::GpuMat> srcImagesGPU;
    std::vector<cv::cuda::GpuMat> reprojImagesGPU;
    cv::cuda::GpuMat blendImageGPU;
    cv::Mat blendImage;
    std::vector<cv::cuda::Stream> streams;
    int blendType;
    CudaTilingMultibandBlendFast mbBlender;
    CudaTilingLinearBlend lBlender;
    int numImages;
    int success;
};

// add gain adjust function compared with CudaMultiCameraPanoramaRender2
class CudaMultiCameraPanoramaRender3 : public PanoramaRender
{
public:
    CudaMultiCameraPanoramaRender3() : success(0) {};
    ~CudaMultiCameraPanoramaRender3() {};
    bool prepare(const std::string& path, int blendType, const cv::Size& srcSize, const cv::Size& dstSize);
    bool render(const std::vector<cv::Mat>& src, cv::Mat& dst);
private:
    cv::Size srcFullSize;
    std::vector<std::vector<unsigned char> > luts;
    std::vector<cv::cuda::GpuMat> dstSrcXMapsGPU, dstSrcYMapsGPU;
    std::vector<cv::cuda::GpuMat> srcImagesGPU;
    std::vector<cv::cuda::GpuMat> reprojImagesGPU;
    cv::cuda::GpuMat blendImageGPU;
    cv::Mat blendImage;
    std::vector<cv::cuda::Stream> streams;
    MultibandBlendGainAdjust adjuster;
    CudaTilingLinearBlend blender;
    int numImages;
    int success;
};

// multithread version of CudaMultiCameraPanoramaRender2
class CudaMultiCameraPanoramaRender4
{
public:
    enum BlendType
    {
        BlendTypeLinear,
        BlendTypeMultiband
    };
    CudaMultiCameraPanoramaRender4() : success(0) {};
    ~CudaMultiCameraPanoramaRender4() { clear(); };
    bool prepare(const std::string& path, int blendType, const cv::Size& srcSize, const cv::Size& dstSize);
    bool render(const std::vector<cv::Mat>& src, long long int timeStamp);
    bool getResult(cv::Mat& dst, long long int& timeStamp);
    void stop();
    void resume();
    void clear();
private:
    cv::Size srcFullSize;
    std::vector<cv::cuda::GpuMat> dstSrcXMapsGPU, dstSrcYMapsGPU;
    std::vector<cv::cuda::GpuMat> srcImagesGPU;
    std::vector<cv::cuda::GpuMat> reprojImagesGPU;
    PinnedMemoryPool pool;
    typedef ForceWaitRealTimeQueue<std::pair<cv::cuda::HostMem, long long int> > StampedMatQueue;
    StampedMatQueue queue;
    std::vector<cv::cuda::Stream> streams;
    int blendType;
    CudaTilingMultibandBlendFast mbBlender;
    CudaTilingLinearBlend lBlender;
    std::vector<cv::cuda::GpuMat> weightsGPU;
    cv::cuda::GpuMat accumGPU;
    int numImages;
    int success;
};
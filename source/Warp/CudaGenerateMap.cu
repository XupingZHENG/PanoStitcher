#include <opencv2/core/core.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/gpu/stream_accessor.hpp>
#include <opencv2/gpu/device/common.hpp>
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <device_functions.h>
#include "ZReproject.h"

typedef double CalcType;

struct CudaRemapParam
{
    CalcType srcTX, srcTY;
    CalcType destTX, destTY;
    CalcType scale[2];
    CalcType shear[2];
    CalcType rot[2];
    void *perspect[2];
    CalcType rad[6];
    CalcType mt[3][3];
    CalcType distance;
    CalcType horizontal;
    CalcType vertical;
    CalcType PI;
    CalcType width;
    CalcType height;
    CalcType centx;
    CalcType centy;
    CalcType sqrDist;
    int type;
};

void copyParam(const Remap& src, CudaRemapParam& dst, 
    CalcType width, CalcType height, CalcType centx, CalcType centy, CalcType sqrDist, CalcType type)
{
    dst.srcTX = src.srcTX;
    dst.srcTY = src.srcTY;
    dst.destTX = src.destTX;
    dst.destTY = src.destTY;
    dst.scale[0] = src.mp.scale[0];
    dst.scale[1] = src.mp.scale[1];
    dst.shear[0] = src.mp.shear[0];
    dst.shear[1] = src.mp.shear[1];
    dst.rot[0] = src.mp.rot[0];
    dst.rot[1] = src.mp.rot[1];
    dst.perspect[0] = src.mp.perspect[0];
    dst.perspect[1] = src.mp.perspect[1];
    for (int i = 0; i < 6; i++)
        dst.rad[i] = src.mp.rad[i];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
            dst.mt[i][j] = src.mp.mt[i][j];
    }    
    dst.distance = src.mp.distance;
    dst.horizontal = src.mp.horizontal;
    dst.vertical = src.mp.vertical;
    dst.PI = 3.1415926535898;
    dst.width = width;
    dst.height = height;
    dst.centx = centx;
    dst.centy = centy;
    dst.sqrDist = sqrDist;
    dst.type = type;
}

__constant__ CudaRemapParam param;

__global__ void remapKernel(unsigned char* xMapData, int xMapStep, 
    unsigned char* yMapData, int yMapStep, int mapWidth, int mapHeight)
{
    int x = threadIdx.x + blockIdx.x * blockDim.x;
    int y = threadIdx.y + blockIdx.y * blockDim.y;
    if (x >= mapWidth || y >= mapHeight)
        return;

    CalcType x_src = x, y_src = y;

    x_src -= param.srcTX - 0.5;
    y_src -= param.srcTY - 0.5;

    CalcType tx_dest, ty_dest;

    //rotate_erect  中心归一化
    tx_dest = x_src + param.rot[1];

    while (tx_dest < -param.rot[0])
        tx_dest += 2 * param.rot[0];

    while (tx_dest >   param.rot[0])
        tx_dest -= 2 * param.rot[0];

    ty_dest = y_src;

    x_src = tx_dest;
    y_src = ty_dest;

    //sphere_tp_erect 球面坐标转化为现实坐标
    CalcType phi, theta, r;
    CalcType v[3];
    phi = x_src / param.distance; //
    theta = -y_src / param.distance + param.PI / 2; //
    if (theta < 0)
    {
        theta = -theta;
        phi += param.PI;
    }
    if (theta > param.PI)
    {
        theta = param.PI - (theta - param.PI);
        phi += param.PI;
    }

    v[0] = sin(theta) * sin(phi);
    v[1] = cos(theta);
    v[2] = sin(theta) * cos(phi);

    //摄像机外参
    CalcType v0 = v[0];
    CalcType v1 = v[1];
    CalcType v2 = v[2];

    for (int i = 0; i<3; i++)
    {
        v[i] = param.mt[0][i] * v0 + param.mt[1][i] * v1 + param.mt[2][i] * v2;
    }

    r = sqrt(v[0] * v[0] + v[1] * v[1]);
    if (r == 0.0)
        theta = 0.0;
    else
        theta = param.distance * atan2(r, v[2]) / r;
    tx_dest = theta * v[0];
    ty_dest = theta * v[1];
    x_src = tx_dest;
    y_src = ty_dest;

    //摄像机内参
    //SetDesc(  stack[i],   resize,                 param.scale       ); i++; // Scale image
    tx_dest = x_src * param.scale[0];
    ty_dest = y_src * param.scale[1];

    x_src = tx_dest;
    y_src = ty_dest;

    CalcType rt, scale;

    rt = (sqrt(x_src*x_src + y_src*y_src)) / param.rad[4];
    if (rt < param.rad[5])
    {
        scale = ((param.rad[3] * rt + param.rad[2]) * rt +
            param.rad[1]) * rt + param.rad[0];
    }
    else
        scale = 1000.0;

    tx_dest = x_src * scale;
    ty_dest = y_src * scale;

    x_src = tx_dest;
    y_src = ty_dest;

    //摄像机水平竖直矫正
    if (param.vertical != 0.0)
    {
        //SetDesc(stack[i],   vert,                   &(param.vertical));   i++;
        tx_dest = x_src;
        ty_dest = y_src + param.vertical;
        x_src = tx_dest;
        y_src = ty_dest;
    }

    if (param.horizontal != 0.0)
    {
        //SetDesc(stack[i],   horiz,                  &(param.horizontal)); i++;
        tx_dest = x_src + param.horizontal;
        ty_dest = y_src;
        x_src = tx_dest;
        y_src = ty_dest;
    }

    if (param.shear[0] != 0 || param.shear[1] != 0)
    {
        //SetDesc( stack[i],  shear,                  param.shear       ); i++;
        tx_dest = x_src + param.shear[0] * y_src;
        ty_dest = y_src + param.shear[1] * x_src;
    }

    tx_dest += param.destTX - 0.5;
    ty_dest += param.destTY - 0.5;

    if (param.type == 2 || param.type == 3)
    {
        float diffx = tx_dest - param.centx;
        float diffy = ty_dest - param.centy;
        if (tx_dest >= 0 && tx_dest < param.width && ty_dest >= 0 && ty_dest < param.height &&
            diffx * diffx + diffy * diffy < param.sqrDist)
        {
            *((float*)(xMapData + y * xMapStep) + x) = tx_dest;
            *((float*)(yMapData + y * yMapStep) + x) = ty_dest;
        }
        else
        {
            *((float*)(xMapData + y * xMapStep) + x) = -1.0F;
            *((float*)(yMapData + y * yMapStep) + x) = -1.0F;
        }
    }
    else
    {
        *((float*)(xMapData + y * xMapStep) + x) = tx_dest;
        *((float*)(yMapData + y * yMapStep) + x) = ty_dest;
    }
}

void cudaGenerateReprojectMap(const PhotoParam& photoParam,
    const cv::Size& srcSize, const cv::Size& dstSize, cv::gpu::GpuMat& xmap, cv::gpu::GpuMat& ymap)
{
    int dstWidth = dstSize.width, dstHeight = dstSize.height;
    int srcWidth = srcSize.width, srcHeight = srcSize.height;
    CalcType centx = photoParam.cropX + photoParam.cropWidth / 2;
    CalcType centy = photoParam.cropY + photoParam.cropHeight / 2;
    CalcType sqrDist = photoParam.cropWidth * photoParam.cropWidth * 0.25;

    Remap remap;
    remap.init(photoParam, dstWidth, dstHeight);
    CudaRemapParam cudaParam;
    copyParam(remap, cudaParam, srcWidth, srcHeight, centx, centy, sqrDist, photoParam.imageType);
    cudaSafeCall(cudaMemcpyToSymbol(param, &cudaParam, sizeof(CudaRemapParam)));

    xmap.create(dstHeight, dstWidth, CV_32FC1);
    ymap.create(dstHeight, dstWidth, CV_32FC1);

    dim3 block(16, 16);
    dim3 grid((dstSize.width + block.x - 1) / block.x, (dstSize.height + block.y - 1) / block.y);
    remapKernel<<<grid, block>>>(xmap.data, xmap.step, ymap.data, ymap.step, dstWidth, dstHeight);
    cudaSafeCall(cudaGetLastError());
}

void cudaGenerateReprojectMaps(const std::vector<PhotoParam>& params,
    const cv::Size& srcSize, const cv::Size& dstSize, std::vector<cv::gpu::GpuMat>& xmaps, std::vector<cv::gpu::GpuMat>& ymaps)
{
    int num = params.size();
    xmaps.resize(num);
    ymaps.resize(num);
    for (int i = 0; i < num; i++)
        cudaGenerateReprojectMap(params[i], srcSize, dstSize, xmaps[i], ymaps[i]);
}
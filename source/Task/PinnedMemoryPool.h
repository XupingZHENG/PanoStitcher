#pragma once

#include "IntelOpenCLMat.h"
#include "opencv2/core/cuda.hpp"
#include <vector>
#include <mutex>

class PinnedMemoryPool
{
public:
    PinnedMemoryPool() :
        rows(0), cols(0), type(0), flag(cv::cuda::HostMem::PAGE_LOCKED), hasInit(0)
    {
    }

    bool init(int rows_, int cols_, int type_, 
        cv::cuda::HostMem::AllocType flag_ = cv::cuda::HostMem::PAGE_LOCKED)
    {
        clear();

        std::lock_guard<std::mutex> lock(mtx);

        cv::cuda::HostMem test(flag_);
        try
        {
            test.create(rows_, cols_, type_);
        }
        catch (...)
        {
            return false;
        }
        if (!test.data)
            return false;

        rows = rows_;
        cols = cols_;
        type = type_;
        flag = flag_;

        hasInit = 1;
        return true;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mtx);
        pool.clear();
    }

    bool get(cv::cuda::HostMem& mem)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!hasInit)
        {
            mem = cv::cuda::HostMem(flag);
            return false;
        }

        int size = pool.size();
        int index = -1;
        for (int i = 0; i < size; i++)
        {
            if (pool[i].refcount && *pool[i].refcount == 1)
            {
                index = i;
                break;
            }
        }
        if (index >= 0)
        {
            mem = pool[index];
            return true;
        }

        cv::cuda::HostMem newMem(rows, cols, type, flag);
        if (!newMem.data)
        {
            mem = cv::cuda::HostMem(flag);
            return false;
        }

        mem = newMem;
        pool.push_back(newMem);
        return true;
    }
private:
    int rows, cols, type;
    std::vector<cv::cuda::HostMem> pool;
    std::mutex mtx;
    cv::cuda::HostMem::AllocType flag;
    int hasInit;
};

class GpuMemoryPool
{
public:
    GpuMemoryPool() :
        rows(0), cols(0), type(0), hasInit(0)
    {
    }

    bool init(int rows_, int cols_, int type_)
    {
        clear();

        std::lock_guard<std::mutex> lock(mtx);

        cv::cuda::GpuMat test;
        try
        {
            test.create(rows_, cols_, type_);
        }
        catch (...)
        {
            return false;
        }
        if (!test.data)
            return false;

        rows = rows_;
        cols = cols_;
        type = type_;

        hasInit = 1;
        return true;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mtx);
        pool.clear();
    }

    bool get(cv::cuda::GpuMat& mem)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!hasInit)
        {
            mem = cv::cuda::GpuMat();
            return false;
        }

        int size = pool.size();
        int index = -1;
        for (int i = 0; i < size; i++)
        {
            if (pool[i].refcount && *pool[i].refcount == 1)
            {
                index = i;
                break;
            }
        }
        if (index >= 0)
        {
            mem = pool[index];
            return true;
        }

        cv::cuda::GpuMat newMem(rows, cols, type);
        if (!newMem.data)
        {
            mem = cv::cuda::GpuMat();
            return false;
        }

        mem = newMem;
        pool.push_back(newMem);
        return true;
    }
private:
    int rows, cols, type;
    std::vector<cv::cuda::GpuMat> pool;
    std::mutex mtx;
    int hasInit;
};

class CPUMemoryPool
{
public:
    CPUMemoryPool() :
        rows(0), cols(0), type(0), hasInit(0)
    {
    }

    bool init(int rows_, int cols_, int type_)
    {
        clear();

        std::lock_guard<std::mutex> lock(mtx);

        cv::Mat test;
        try
        {
            test.create(rows_, cols_, type_);
        }
        catch (...)
        {
            return false;
        }
        if (!test.data)
            return false;

        rows = rows_;
        cols = cols_;
        type = type_;

        hasInit = 1;
        return true;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mtx);
        pool.clear();
    }

    bool get(cv::Mat& mem)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!hasInit)
        {
            mem = cv::Mat();
            return false;
        }

        int size = pool.size();
        int index = -1;
        for (int i = 0; i < size; i++)
        {
            if (pool[i].u && pool[i].u->refcount == 1)
            {
                index = i;
                break;
            }
        }
        if (index >= 0)
        {
            mem = pool[index];
            return true;
        }

        cv::Mat newMem(rows, cols, type);
        if (!newMem.data)
        {
            mem = cv::Mat();
            return false;
        }

        mem = newMem;
        pool.push_back(newMem);
        return true;
    }
private:
    int rows, cols, type;
    std::vector<cv::Mat> pool;
    std::mutex mtx;
    int hasInit;
};

class IntelMemoryPool
{
public:
    IntelMemoryPool() :
        rows(0), cols(0), type(0), hasInit(0)
    {
    }

    bool init(int rows_, int cols_, int type_, cl_context ctx_)
    {
        clear();

        std::lock_guard<std::mutex> lock(mtx);

        IOclMat test;
        try
        {
            test.create(rows_, cols_, type_, ctx_);
        }
        catch (...)
        {
            return false;
        }
        if (!test.data)
            return false;

        rows = rows_;
        cols = cols_;
        type = type_;
        ctx = ctx_;

        hasInit = 1;
        return true;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mtx);
        pool.clear();
    }

    bool get(IOclMat& mem)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!hasInit)
        {
            mem = IOclMat();
            return false;
        }

        int size = pool.size();
        int index = -1;
        for (int i = 0; i < size; i++)
        {
            if (pool[i].sdata.use_count() == 1)
            {
                index = i;
                break;
            }
        }
        if (index >= 0)
        {
            mem = pool[index];
            return true;
        }

        IOclMat newMem(rows, cols, type, ctx);
        if (!newMem.data)
        {
            mem = IOclMat();
            return false;
        }

        mem = newMem;
        pool.push_back(newMem);
        return true;
    }
private:
    int rows, cols, type;
    std::vector<IOclMat> pool;
    std::mutex mtx;
    cl_context ctx;
    int hasInit;
};
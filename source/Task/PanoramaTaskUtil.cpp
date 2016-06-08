#include "PanoramaTaskUtil.h"
#include "Image.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <stdarg.h>

bool prepareSrcVideos(const std::vector<std::string>& srcVideoFiles, bool bgr24, const std::vector<int>& offsets,
    int tryAudioIndex, std::vector<avp::AudioVideoReader>& readers, int& audioIndex, cv::Size& srcSize, int& validFrameCount)
{
    readers.clear();
    srcSize = cv::Size();
    validFrameCount = 0;

    if (srcVideoFiles.empty())
        return false;

    int numVideos = srcVideoFiles.size();
    bool hasOffsets = !offsets.empty();
    if (hasOffsets && offsets.size() != numVideos)
        return false;

    readers.resize(numVideos);

    if (tryAudioIndex < 0 || tryAudioIndex >= numVideos)
    {
        ptlprintf("Info in %s, no audio will be opened\n", __FUNCTION__);
        audioIndex = -1;
    }

    bool ok = false;
    double fps = -1;
    for (int i = 0; i < numVideos; i++)
    {
        if (i == tryAudioIndex)
        {
            ok = readers[i].open(srcVideoFiles[i], true, true, bgr24 ? avp::PixelTypeBGR24 : avp::PixelTypeBGR32);
            if (ok)
                audioIndex = tryAudioIndex;
            else
            {
                ok = readers[i].open(srcVideoFiles[i], false, true, bgr24 ? avp::PixelTypeBGR24 : avp::PixelTypeBGR32);
                audioIndex = -1;
            }
        }
        else
            ok = readers[i].open(srcVideoFiles[i], false, true, bgr24 ? avp::PixelTypeBGR24 : avp::PixelTypeBGR32);
        if (!ok)
            break;

        if (srcSize == cv::Size())
        {
            srcSize.width = readers[i].getVideoWidth();
            srcSize.height = readers[i].getVideoHeight();
        }
        if (srcSize.width != readers[i].getVideoWidth() ||
            srcSize.height != readers[i].getVideoHeight())
        {
            ok = false;
            ptlprintf("Error in %s, video size unmatch\n", __FUNCTION__);
            break;
        }

        if (fps < 0)
            fps = readers[i].getVideoFps();
        if (abs(fps - readers[i].getVideoFps()) > 0.1)
        {
            ptlprintf("Error in %s, video fps not consistent\n", __FUNCTION__);
            ok = false;
            break;
        }

        int count = hasOffsets ? offsets[i] : 0;
        int currValidFrameCount = readers[i].getVideoNumFrames();
        if (currValidFrameCount <= 0)
            validFrameCount = -1;
        else
        {
            currValidFrameCount -= count;
            if (currValidFrameCount <= 0)
            {
                ptlprintf("Error in %s, video not long enough\n", __FUNCTION__);
                ok = false;
                break;
            }
        }

        if (validFrameCount == 0)
            validFrameCount = currValidFrameCount;
        if (validFrameCount > 0)
            validFrameCount = validFrameCount > currValidFrameCount ? currValidFrameCount : validFrameCount;

        if (hasOffsets)
        {
            if (!readers[i].seek(1000000.0 * count / fps + 0.5, avp::VIDEO))
            {
                ptlprintf("Error in %s, cannot seek to target frame\n", __FUNCTION__);
                ok = false;
                break;
            }
            if (!ok)
                break;
        }
    }

    if (!ok)
    {
        readers.clear();
        audioIndex = -1;
        srcSize = cv::Size();
        validFrameCount = 0;
    }

    return ok;
}

static void alphaBlend(cv::Mat& image, const cv::Mat& logo)
{
    CV_Assert(image.data && (image.type() == CV_8UC3 || image.type() == CV_8UC4) &&
        logo.data && logo.type() == CV_8UC4 && image.size() == logo.size());

    int rows = image.rows, cols = image.cols, channels = image.channels();
    for (int i = 0; i < rows; i++)
    {
        unsigned char* ptrImage = image.ptr<unsigned char>(i);
        const unsigned char* ptrLogo = logo.ptr<unsigned char>(i);
        for (int j = 0; j < cols; j++)
        {
            if (ptrLogo[3])
            {
                int val = ptrLogo[3];
                int comp = 255 - ptrLogo[3];
                ptrImage[0] = (comp * ptrImage[0] + val * ptrLogo[0] + 254) / 255;
                ptrImage[1] = (comp * ptrImage[1] + val * ptrLogo[1] + 254) / 255;
                ptrImage[2] = (comp * ptrImage[2] + val * ptrLogo[2] + 254) / 255;
            }
            ptrImage += channels;
            ptrLogo += 4;
        }
    }
}

bool LogoFilter::init(int width_, int height_, int type_)
{
    initSuccess = false;
    if (width_ < 0 || height_ < 0 || (type_ != CV_8UC3 && type_ != CV_8UC4))
        return false;

    width = width_;
    height = height_;
    type = type_;

    cv::Mat origLogo(logoHeight, logoWidth, CV_8UC4, logoData);

    int blockWidth = 512, blockHeight = 512;
    rects.clear();
    if (width < logoWidth || height < logoHeight)
    {
        cv::Rect logoRect(logoWidth / 2 - width / 2, logoHeight / 2 - height / 2, width, height);
        logo = origLogo(logoRect);
        rects.push_back(cv::Rect(0, 0, width, height));
    }
    else
    {
        logo = origLogo;
        int w = (width + blockWidth - 1) / blockWidth, h = (height + blockHeight - 1) / blockHeight;
        cv::Rect full(0, 0, width, height);
        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                cv::Rect thisRect = cv::Rect(j * blockWidth + blockWidth / 2 - logoWidth / 2, 
                                             i * blockHeight + blockHeight / 2 - logoHeight / 2, 
                                             logoWidth, logoHeight) & 
                                    full;
                if (thisRect.area())
                    rects.push_back(thisRect);
            }
        }
    }

    initSuccess = true;
    return true;
}

bool LogoFilter::addLogo(cv::Mat& image)
{
    if (!initSuccess || !image.data || image.rows != height || image.cols != width || image.type() != type)
        return false;

    int size = rects.size();
    for (int i = 0; i < size; i++)
    {
        cv::Mat imagePart(image, rects[i]);
        cv::Mat logoPart(logo, cv::Rect(0, 0, rects[i].width, rects[i].height));
        alphaBlend(imagePart, logoPart);
    }

    return true;
}

void ptLogDefaultCallback(const char* format, va_list vl)
{
    vprintf(format, vl);
}

PanoTaskLogCallbackFunc ptLogCallback = ptLogDefaultCallback;

void ptlprintf(const char* format, ...)
{
    if (ptLogCallback)
    {
        va_list vl;
        va_start(vl, format);
        ptLogCallback(format, vl);
        va_end(vl);
    }    
}

PanoTaskLogCallbackFunc setPanoTaskLogCallback(PanoTaskLogCallbackFunc func)
{
    PanoTaskLogCallbackFunc oldFunc = ptLogCallback;
    ptLogCallback = func;
    return oldFunc;
}

void CustomIntervaledMasks::reset()
{
    clearAllMasks();
    width = 0;
    height = 0;
    initSuccess = 0;
}

bool CustomIntervaledMasks::init(int width_, int height_)
{
    clearAllMasks();

    initSuccess = 0;
    if (width_ < 0 || height_ < 0)
        return false;

    width = width_;
    height = height_;
    initSuccess = 1;
    return true;
}

bool CustomIntervaledMasks::getMask(long long int time, cv::Mat& mask) const
{
    if (!initSuccess)
    {
        mask = cv::Mat();
        return false;
    }

    int size = masks.size();
    for (int i = 0; i < size; i++)
    {
        const IntervaledMask& currMask = masks[i];
        if (time >= currMask.begInc && time < currMask.endExc)
        {
            mask = currMask.mask;
            return true;
        }
    }
    mask = cv::Mat();
    return false;
}

bool CustomIntervaledMasks::addMask(long long int begInc, long long int endExc, const cv::Mat& mask)
{
    if (!initSuccess)
        return false;
    
    if (!mask.data || mask.type() != CV_8UC1 || mask.cols != width || mask.rows != height)
        return false;

    masks.push_back(IntervaledMask(begInc, endExc, mask.clone()));
    return true;
}

void CustomIntervaledMasks::clearMask(long long int begInc, long long int endExc, long long int precision)
{
    if (precision < 0)
        precision = 0;
    for (std::vector<IntervaledMask>::iterator itr = masks.begin(); itr != masks.end();)
    {
        if (abs(itr->begInc - begInc) <= precision &&
            abs(itr->endExc - endExc) <= precision)
            itr = masks.erase(itr);
        else
            ++itr;
    }
}

void CustomIntervaledMasks::clearAllMasks()
{
    masks.clear();
}

bool loadIntervaledContours(const std::string& fileName, std::vector<std::vector<IntervaledContour> >& contours)
{
    return false;
}

bool cvtMaskToContour(const IntervaledMask& mask, IntervaledContour& contour)
{
    if (!mask.mask.data || mask.mask.type() != CV_8UC1)
    {
        contour = IntervaledContour();
        return false;
    }

    contour.begIncInMilliSec = mask.begInc * 0.001;
    contour.endExcInMilliSec = mask.endExc * 0.001;

    int rows = mask.mask.rows, cols = mask.mask.cols;
    int pad = 4;
    std::vector<std::vector<cv::Point> > contours;
    if (cv::countNonZero(mask.mask.row(0)) || cv::countNonZero(mask.mask.row(rows - 1)) ||
        cv::countNonZero(mask.mask.col(0)) || cv::countNonZero(mask.mask.col(cols - 1)))
    {
        cv::Mat extendMask(rows + 2 * pad, cols + 2 * pad, CV_8UC1);
        extendMask.setTo(0);
        cv::Mat roi = extendMask(cv::Rect(pad, pad, cols, rows));
        mask.mask.copyTo(roi);
        cv::findContours(extendMask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        int numCountors = contours.size();
        cv::Point offset(pad, pad);
        for (int i = 0; i < numCountors; i++)
        {
            int len = contours[i].size();
            for (int j = 0; j < len; j++)
                contours[i][j] -= offset;
        }
    }
    else
    {
        cv::Mat nonExtendMask;
        mask.mask.copyTo(nonExtendMask);
        cv::findContours(nonExtendMask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    }
    contour.contours = contours;
    contour.width = cols;
    contour.height = rows;

    return true;
}

bool cvtContourToMask(const IntervaledContour& contour, const cv::Mat& boundedMask, IntervaledMask& customMask)
{
    if (contour.width <= 0 || contour.height <= 0 || 
        !boundedMask.data || boundedMask.type() != CV_8UC1)
    {
        customMask = IntervaledMask();
        return false;
    }

    customMask.begInc = (long long int)contour.begIncInMilliSec * 1000LL;
    customMask.endExc = (long long int)contour.endExcInMilliSec * 1000LL;
    customMask.mask.create(boundedMask.size(), CV_8UC1);
    customMask.mask.setTo(0);
    if (contour.contours.size())
    {
        if (contour.width == boundedMask.cols &&
            contour.height == boundedMask.rows)
            cv::drawContours(customMask.mask, contour.contours, -1, 255, CV_FILLED);
        else
        {
            cv::Mat temp;
            cv::drawContours(temp, contour.contours, -1, 255, CV_FILLED);
            cv::resize(temp, customMask.mask, customMask.mask.size());
            customMask.mask.setTo(255, customMask.mask);
        }
        customMask.mask &= boundedMask;
    }
    return true;
}

bool cvtContoursToMasks(const std::vector<std::vector<IntervaledContour> >& contours,
    const std::vector<cv::Mat>& boundedMasks, std::vector<CustomIntervaledMasks>& customMasks)
{
    customMasks.clear();
    if (contours.size() != boundedMasks.size())
        return false;

    if (boundedMasks.empty())
        return true;

    int size = boundedMasks.size();
    int width = boundedMasks[0].cols, height = boundedMasks[0].rows;
    for (int i = 1; i < size; i++)
    {
        if (boundedMasks[i].cols != width || boundedMasks[i].rows != height)
            return false;
    }

    bool success = true;
    customMasks.resize(size);
    IntervaledMask currItvMask;
    for (int i = 0; i < size; i++)
    {
        customMasks[i].init(width, height);
        int num = contours[i].size();
        for (int j = 0; j < num; j++)
        {
            if (!cvtContourToMask(contours[i][j], boundedMasks[i], currItvMask))
            {
                success = false;
                break;
            }
            customMasks[i].addMask(currItvMask.begInc, currItvMask.endExc, currItvMask.mask);
        }
        if (!success)
            break;
    }
    if (!success)
        customMasks.clear();

    return success;
}

bool setIntervaledContoursToPreviewTask(const std::vector<std::vector<IntervaledContour> >& contours,
    CPUPanoramaPreviewTask& task)
{
    std::vector<cv::Mat> boundedMasks;
    if (!task.getMasks(boundedMasks))
        return false;

    if (contours.size() != boundedMasks.size())
        return false;

    int size = boundedMasks.size();
    bool success = true;
    IntervaledMask currItvMask;
    for (int i = 0; i < size; i++)
    {
        int num = contours[i].size();
        for (int j = 0; j < num; j++)
        {
            if (!cvtContourToMask(contours[i][j], boundedMasks[i], currItvMask))
            {
                success = false;
                break;
            }
            task.setCustomMaskForOne(i, currItvMask.begInc, currItvMask.endExc, currItvMask.mask);
        }
        if (!success)
            break;
    }

    return success;
}

bool getIntervaledContoursFromPreviewTask(const CPUPanoramaPreviewTask& task,
    std::vector<std::vector<IntervaledContour> >& contours)
{
    contours.clear();
    if (!task.isValid())
        return false;

    int size = task.getNumSourceVideos();
    contours.resize(size);
    std::vector<long long int> begIncs, endExcs;
    std::vector<cv::Mat> masks;
    bool success = true;
    for (int i = 0; i < size; i++)
    {
        if (task.getAllCustomMasksForOne(i, begIncs, endExcs, masks))
        {
            int len = begIncs.size();
            contours[i].resize(len);
            for (int j = 0; j < len; j++)
            {
                if (!cvtMaskToContour(IntervaledMask(begIncs[j], endExcs[j], masks[j]), contours[i][j]))
                {
                    success = false;
                    break;
                }
            }
            if (!success)
                break;
        }
        else
        {
            success = false;
            break;
        }
    }
    if (!success)
        contours.clear();
    return success;
}

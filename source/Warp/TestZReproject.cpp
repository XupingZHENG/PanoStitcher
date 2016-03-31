#include "ZReproject.h"
#include <opencv2/core/core.hpp>
#include <opencv2/core/gpumat.hpp>
#include <opencv2/highgui/highgui.hpp>

void getEquirectToCubeInverseMap(cv::Mat& map, int equirectWidth, int equirectHeight, int cubeWidth, int cubeHeight);

int main()
{
    //{
    //    cv::Mat sphere = cv::imread("F:\\panoimage\\detuoffice\\blendmultiband.bmp");
    //    cv::Mat cubic = cv::Mat(600, 3600, CV_8UC3);
    //    cv::Mat map;
    //    getEquirectToCubeInverseMap(map, sphere.cols, sphere.rows, cubic.cols, cubic.rows);
    //    reprojectParallel(sphere, cubic, map);
    //    cv::imshow("cubic", cubic);
    //    cv::imwrite("cubic.bmp", cubic);
    //    cv::waitKey(0);
    //    return 0;
    //}

    //{
    //    std::vector<PhotoParam> params;
    //    loadPhotoParamFromPTS("C:\\Users\\zhengxuping\\Desktop\\all___.pts", params);
    //}

    //{
    //    std::vector<PhotoParam> params;
    //    loadPhotoParamFromXML("left.xml", params);
    //}

    cv::Size dstSize = cv::Size(1400, 700);

    {
        std::vector<std::string> paths;
        paths.push_back("F:\\panoimage\\circular\\park1.JPG");
        paths.push_back("F:\\panoimage\\circular\\park2.JPG");
        paths.push_back("F:\\panoimage\\circular\\park3.JPG");

        int numImages = paths.size();
        std::vector<cv::Mat> src(numImages);
        for (int i = 0; i < numImages; i++)
            src[i] = cv::imread(paths[i]);

        std::vector<PhotoParam> params;
        //loadPhotoParamFromPTS("F:\\panoimage\\outdoor\\Panorama.pts", params);
        //loadPhotoParamFromXML("F:\\panoimage\\outdoor\\outdoor.xml", params);
        loadPhotoParamFromXML("F:\\panoimage\\circular\\param.xml", params);
        rotatePhotoParamInXML("F:\\panoimage\\circular\\param.xml", "F:\\panoimage\\circular\\new.xml", 1.57, 0.0, 0.0);
        //rotateCameras(params, 0, 3.1415926536 / 2 * 0.65, 0);

        std::vector<cv::Mat> maps, masks;
        getReprojectMapsAndMasks(params, src[0].size(), dstSize, maps, masks);

        std::vector<cv::Mat> dst(numImages);
        for (int i = 0; i < 1; i++)
            reprojectParallelTo16S(src, dst, maps);
        //for (int i = 0; i < numImages; i++)
        //{
        //    char buf[64];
        //    sprintf(buf, "mask%d.bmp", i);
        //    //cv::imwrite(buf, masks[i]);
        //    cv::imshow("mask", masks[i]);
        //    reproject(src[i], dst[i], maps[i]);
        //    sprintf(buf, "reprojimage%d.bmp", i);
        //    //cv::imwrite(buf, dst[i]);
        //    cv::imshow("dst", dst[i]);
        //    cv::waitKey(0);
        //}
    }
    return 0;

    {
        std::vector<std::string> paths;
        paths.push_back("F:\\panoimage\\outdoor\\1.MOV.tif");
        paths.push_back("F:\\panoimage\\outdoor\\2.MOV.tif");
        paths.push_back("F:\\panoimage\\outdoor\\3.MOV.tif");
        paths.push_back("F:\\panoimage\\outdoor\\4.MOV.tif");

        int numImages = paths.size();
        std::vector<cv::Mat> src(numImages);
        for (int i = 0; i < numImages; i++)
            src[i] = cv::imread(paths[i]);

        std::vector<PhotoParam> params;
        //loadPhotoParamFromPTS("F:\\panoimage\\outdoor\\Panorama.pts", params);
        //loadPhotoParamFromXML("F:\\panoimage\\outdoor\\outdoor.xml", params);
        loadPhotoParamFromXML("F:\\panoimage\\outdoor\\213.xml", params);
        //rotateCameras(params, 0, 3.1415926536 / 2 * 0.65, 0);

        std::vector<cv::Mat> maps, masks;
        getReprojectMapsAndMasks(params, src[0].size(), dstSize, maps, masks);

        std::vector<cv::Mat> dst(numImages);
        for (int i = 0; i < numImages; i++)
        {
            char buf[64];
            sprintf(buf, "mask%d.bmp", i);
            //cv::imwrite(buf, masks[i]);
            cv::imshow("mask", masks[i]);
            reproject(src[i], dst[i], maps[i]);
            sprintf(buf, "reprojimage%d.bmp", i);
            //cv::imwrite(buf, dst[i]);
            cv::imshow("dst", dst[i]);
            cv::waitKey(0);
        }
    }
    return 0;

    //{
    //    std::vector<PhotoParam> params;
    //    loadPhotoParamFromXML("F:\\panovideo\\test\\detu\\outdoor.xml", params);

    //    std::vector<cv::Mat> maps, masks;
    //    getReprojectMapsAndMasks(params, cv::Size(1080, 1920), dstSize, maps, masks);
    //
    //    for (int i = 0; i < params.size(); i++)
    //    {
    //        cv::imshow("mask", masks[i]);
    //        cv::waitKey(0);
    //    }

    //    ReprojectParam rp;
    //    rp.LoadConfig("F:\\panovideo\\test\\detu\\outdoor.xml");
    //    rp.SetPanoSize(cv::Size(2048, 1024));
    //    getReprojectMapsAndMasks(rp, cv::Size(1080, 1920), maps, masks);

    //    for (int i = 0; i < masks.size(); i++)
    //    {
    //        cv::imshow("mask", masks[i]);
    //        cv::waitKey(0);
    //    }

    //    //ReprojectParam rp;
    //    rp.LoadConfig("E:\\Projects\\PanoVideo\\build\\GeneratePanoVideo\\5builtinleft.xml");
    //    rp.SetPanoSize(cv::Size(2048, 1024));
    //    getReprojectMapsAndMasks(rp, cv::Size(1920, 1080), maps, masks);

    //    for (int i = 0; i < masks.size(); i++)
    //    {
    //        cv::imshow("mask", masks[i]);
    //        cv::waitKey(0);
    //    }
    //}
    //return 0;

    {
        std::vector<std::string> paths;
        paths.push_back("F:\\panoimage\\beijing\\image0.bmp");
        paths.push_back("F:\\panoimage\\beijing\\image1.bmp");
        paths.push_back("F:\\panoimage\\beijing\\image2.bmp");
        paths.push_back("F:\\panoimage\\beijing\\image3.bmp");
        paths.push_back("F:\\panoimage\\beijing\\image4.bmp");
        paths.push_back("F:\\panoimage\\beijing\\image5.bmp");

        int numImages = paths.size();
        std::vector<cv::Mat> src(numImages);
        for (int i = 0; i < numImages; i++)
            src[i] = cv::imread(paths[i]);

        std::vector<PhotoParam> params;
        loadPhotoParamFromXML("F:\\panoimage\\beijing\\temp_camera_param.xml", params);
        rotateCameras(params, 0, 3.1415926536 / 2 * 0.65, 0);

        std::vector<cv::Mat> maps, masks;
        getReprojectMapsAndMasks(params, src[0].size(), dstSize, maps, masks);

        std::vector<cv::Mat> dst(numImages);
        for (int i = 0; i < numImages; i++)
        {
            char buf[64];
            sprintf(buf, "mask%d.bmp", i);
            cv::imwrite(buf, masks[i]);
            reproject(src[i], dst[i], maps[i]);
            sprintf(buf, "reprojimage%d.bmp", i);
            cv::imwrite(buf, dst[i]);
            cv::imshow("dst", dst[i]);
            cv::waitKey(0);
        }

        //const double PI = 3.1415926535898;
        //rotateCameras(params, 0, -35.264 / 180 * PI, -PI / 4);

        //reproject(src, dst, masks, params, dstSize);
        //for (int i = 0; i < numImages; i++)
        //{
        //    cv::imshow("dst", dst[i]);
        //    cv::waitKey(0);
        //}
    }

    return 0;

    {
        std::vector<std::string> paths;
        paths.push_back("F:\\panoimage\\detuoffice2\\input-01.jpg");
        paths.push_back("F:\\panoimage\\detuoffice2\\input-00.jpg");
        paths.push_back("F:\\panoimage\\detuoffice2\\input-03.jpg");
        paths.push_back("F:\\panoimage\\detuoffice2\\input-02.jpg");

        int numImages = paths.size();
        std::vector<cv::Mat> src(numImages);
        for (int i = 0; i < numImages; i++)
            src[i] = cv::imread(paths[i]);

        std::vector<PhotoParam> params;
        loadPhotoParamFromPTS("F:\\panoimage\\detuoffice2\\4port.pts", params);
        //loadPhotoParamFromXML("F:\\panoimage\\detuoffice2\\detu.xml", params);
        //rotateCameras(params, 0, 0, 3.1415926 / 2);

        std::vector<cv::Mat> maps, masks;
        getReprojectMapsAndMasks(params, src[0].size(), dstSize, maps, masks);

        std::vector<cv::Mat> dst(numImages);
        for (int i = 0; i < numImages; i++)
        {
            char buf[64];
            sprintf(buf, "mask%d.bmp", i);
            cv::imwrite(buf, masks[i]);
            reproject(src[i], dst[i], maps[i]);
            sprintf(buf, "image%d.bmp", i);
            cv::imwrite(buf, dst[i]);
            cv::imshow("dst", dst[i]);
            cv::waitKey(0);
        }

        //const double PI = 3.1415926535898;
        //rotateCameras(params, 0, -35.264 / 180 * PI, -PI / 4);

        //reproject(src, dst, masks, params, dstSize);
        //for (int i = 0; i < numImages; i++)
        //{
        //    cv::imshow("dst", dst[i]);
        //    cv::waitKey(0);
        //}
    }

    return 0;

    {
        cv::Size srcSize = cv::Size(1440, 1080);

        PhotoParam param;
        loadPhotoParamFromXML("F:\\panovideo\\detu\\param.xml", param);
        rotateCamera(param, 0, 3.14 / 2, 0);

        cv::Mat dstMask;
        cv::Mat dstSrcMap;
        getReprojectMapAndMask(param, srcSize, dstSize, dstSrcMap, dstMask);

        cv::Mat origImage = cv::imread("F:\\panovideo\\detu\\aab.png");
        cv::Mat src;
        src.push_back(origImage);

        cv::Mat dst;
        reproject(src, dst, dstSrcMap);
        cv::imshow("dst", dst);
        cv::waitKey(0);
    }

    //return 0;

    {
        cv::Size srcSizeLeft = cv::Size(960, 1080);
        cv::Size srcSizeRight = cv::Size(960, 1080);

        PhotoParam paramLeft, paramRight;
        loadPhotoParamFromXML("F:\\panovideo\\ricoh\\5builtinleft.xml", paramLeft);
        loadPhotoParamFromXML("F:\\panovideo\\ricoh\\5builtinright.xml", paramRight);

        cv::Mat dstMaskLeft, dstMaskRight;
        cv::Mat dstSrcMapLeft, dstSrcMapRight;
        getReprojectMapAndMask(paramLeft, srcSizeLeft, dstSize, dstSrcMapLeft, dstMaskLeft);
        getReprojectMapAndMask(paramRight, srcSizeRight, dstSize, dstSrcMapRight, dstMaskRight);

        cv::Mat origImage = cv::imread("F:\\panovideo\\ricoh\\R0010005\\image1.bmp");
        cv::Mat leftImage, rightImage;
        leftImage = origImage(cv::Rect(0, 0, 960, 1080));
        rightImage = origImage(cv::Rect(960, 0, 960, 1080));

        cv::Mat dstLeft, dstRight;
        reproject(leftImage, dstLeft, dstSrcMapLeft);
        reproject(rightImage, dstRight, dstSrcMapRight);
        cv::imshow("dst left", dstLeft);
        cv::imshow("dst right", dstRight);
        cv::waitKey(0);
    }

    {
        std::vector<std::string> paths;
        paths.push_back("F:\\panoimage\\2\\1\\1.jpg");
        paths.push_back("F:\\panoimage\\2\\1\\2.jpg");
        paths.push_back("F:\\panoimage\\2\\1\\3.jpg");
        paths.push_back("F:\\panoimage\\2\\1\\4.jpg");
        paths.push_back("F:\\panoimage\\2\\1\\5.jpg");
        paths.push_back("F:\\panoimage\\2\\1\\6.jpg");

        int numImages = paths.size();
        std::vector<cv::Mat> src(numImages);
        for (int i = 0; i < numImages; i++)
            src[i] = cv::imread(paths[i]);

        std::vector<PhotoParam> params;
        loadPhotoParamFromXML("F:\\panoimage\\2\\1\\distort.xml", params);

        std::vector<cv::Mat> maps, masks;
        getReprojectMapsAndMasks(params, src[0].size(), dstSize, maps, masks);

        std::vector<cv::Mat> dst(numImages);
        for (int i = 0; i < numImages; i++)
        {
            reproject(src[i], dst[i], maps[i]);
            cv::imshow("dst", dst[i]);
            cv::waitKey(0);
        }

        const double PI = 3.1415926535898;
        rotateCameras(params, 0, -35.264 / 180 * PI, -PI / 4);

        reproject(src, dst, masks, params, dstSize);
        for (int i = 0; i < numImages; i++)
        {
            cv::imshow("dst", dst[i]);
            cv::waitKey(0);
        }
    }
}
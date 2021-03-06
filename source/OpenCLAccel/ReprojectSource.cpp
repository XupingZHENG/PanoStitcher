const char* sourceReproject = R"(

__constant int BILINEAR_INTER_SHIFT = 10;
__constant int BILINEAR_INTER_BACK_SHIFT = 10 * 2;
__constant int BILINEAR_UNIT = 1 << 10;

__kernel void reprojectLinearKernel(__global const unsigned char* srcData, int srcWidth, int srcHeight, int srcStep,
    __global unsigned char* dstData, int dstWidth, int dstHeight, int dstStep,
    __global const unsigned char* xmapData, int xmapStep, __global const unsigned char* ymapData, int ymapStep)
{
    int dstx = get_global_id(0);
    int dsty = get_global_id(1);
    if (dstx >= dstWidth || dsty >= dstHeight)
        return;

    float srcx = *((__global const float*)(xmapData + dsty * xmapStep) + dstx);
    float srcy = *((__global const float*)(ymapData + dsty * ymapStep) + dstx);

    __global unsigned char* ptrDst = (__global unsigned char*)(dstData + dsty * dstStep) + dstx * 4;
    if (srcx < 0 || srcx >= srcWidth || srcy < 0 || srcy >= srcHeight)
        ptrDst[3] = ptrDst[2] = ptrDst[1] = ptrDst[0] = 0;
    else
    {
        int x0 = srcx, y0 = srcy;
        int x1 = x0 + (x0 < srcWidth - 1), y1 = y0 + (y0 < srcHeight - 1);
        int deltax0 = (srcx - x0) * BILINEAR_UNIT, deltax1 = BILINEAR_UNIT - deltax0;
        int deltay0 = (srcy - y0) * BILINEAR_UNIT, deltay1 = BILINEAR_UNIT - deltay0;
        int b = 0, g = 0, r = 0, w = 0;
        uchar4 val;
        
        val = *((__global const uchar4*)(srcData + srcStep * y0) + x0);
        w = deltax1 * deltay1;
        b += val.x * w;
        g += val.y * w;
        r += val.z * w;
        
        val = *((__global const uchar4*)(srcData + srcStep * y0) + x1);
        w = deltax0 * deltay1;
        b += val.x * w;
        g += val.y * w;
        r += val.z * w;

        val = *((__global const uchar4*)(srcData + srcStep * y1) + x0);
        w = deltax1 * deltay0;
        b += val.x * w;
        g += val.y * w;
        r += val.z * w;

        val = *((__global const uchar4*)(srcData + srcStep * y1) + x1);
        w = deltax0 * deltay0;
        b += val.x * w;
        g += val.y * w;
        r += val.z * w;

        ptrDst[0] = b >> BILINEAR_INTER_BACK_SHIFT;
        ptrDst[1] = g >> BILINEAR_INTER_BACK_SHIFT;
        ptrDst[2] = r >> BILINEAR_INTER_BACK_SHIFT;
        ptrDst[3] = 0;
    }
}

__kernel void reprojectLinearTo16SKernel(__global const unsigned char* srcData, int srcWidth, int srcHeight, int srcStep,
    __global unsigned char* dstData, int dstWidth, int dstHeight, int dstStep,
    __global const unsigned char* xmapData, int xmapStep, __global const unsigned char* ymapData, int ymapStep)
{
    int dstx = get_global_id(0);
    int dsty = get_global_id(1);
    if (dstx >= dstWidth || dsty >= dstHeight)
        return;

    float srcx = *((__global const float*)(xmapData + dsty * xmapStep) + dstx);
    float srcy = *((__global const float*)(ymapData + dsty * ymapStep) + dstx);

    __global short* ptrDst = (__global short*)(dstData + dsty * dstStep) + dstx * 4;
    if (srcx < 0 || srcx >= srcWidth || srcy < 0 || srcy >= srcHeight)
        ptrDst[3] = ptrDst[2] = ptrDst[1] = ptrDst[0] = 0;
    else
    {
        int x0 = srcx, y0 = srcy;
        int x1 = x0 + (x0 < srcWidth - 1), y1 = y0 + (y0 < srcHeight - 1);
        int deltax0 = (srcx - x0) * BILINEAR_UNIT, deltax1 = BILINEAR_UNIT - deltax0;
        int deltay0 = (srcy - y0) * BILINEAR_UNIT, deltay1 = BILINEAR_UNIT - deltay0;
        int b = 0, g = 0, r = 0, w = 0;
        uchar4 val;
        
        val = *((__global const uchar4*)(srcData + srcStep * y0) + x0);
        w = deltax1 * deltay1;
        b += val.x * w;
        g += val.y * w;
        r += val.z * w;
        
        val = *((__global const uchar4*)(srcData + srcStep * y0) + x1);
        w = deltax0 * deltay1;
        b += val.x * w;
        g += val.y * w;
        r += val.z * w;

        val = *((__global const uchar4*)(srcData + srcStep * y1) + x0);
        w = deltax1 * deltay0;
        b += val.x * w;
        g += val.y * w;
        r += val.z * w;

        val = *((__global const uchar4*)(srcData + srcStep * y1) + x1);
        w = deltax0 * deltay0;
        b += val.x * w;
        g += val.y * w;
        r += val.z * w;

        ptrDst[0] = b >> BILINEAR_INTER_BACK_SHIFT;
        ptrDst[1] = g >> BILINEAR_INTER_BACK_SHIFT;
        ptrDst[2] = r >> BILINEAR_INTER_BACK_SHIFT;
        ptrDst[3] = 0;
    }
}

void reprojectLinearLocal(__global const unsigned char* srcData, int srcWidth, int srcHeight, int srcStep,
    float srcx, float srcy, unsigned char* result)
{
    int x0 = srcx, y0 = srcy;
    int x1 = x0 + (x0 < srcWidth - 1), y1 = y0 + (y0 < srcHeight - 1);
    int deltax0 = (srcx - x0) * BILINEAR_UNIT, deltax1 = BILINEAR_UNIT - deltax0;
    int deltay0 = (srcy - y0) * BILINEAR_UNIT, deltay1 = BILINEAR_UNIT - deltay0;
    int b = 0, g = 0, r = 0, w = 0;
    uchar4 val;
        
    val = *((__global const uchar4*)(srcData + srcStep * y0) + x0);
    w = deltax1 * deltay1;
    b += val.x * w;
    g += val.y * w;
    r += val.z * w;
        
    val = *((__global const uchar4*)(srcData + srcStep * y0) + x1);
    w = deltax0 * deltay1;
    b += val.x * w;
    g += val.y * w;
    r += val.z * w;

    val = *((__global const uchar4*)(srcData + srcStep * y1) + x0);
    w = deltax1 * deltay0;
    b += val.x * w;
    g += val.y * w;
    r += val.z * w;

    val = *((__global const uchar4*)(srcData + srcStep * y1) + x1);
    w = deltax0 * deltay0;
    b += val.x * w;
    g += val.y * w;
    r += val.z * w;

    result[0] = b >> BILINEAR_INTER_BACK_SHIFT;
    result[1] = g >> BILINEAR_INTER_BACK_SHIFT;
    result[2] = r >> BILINEAR_INTER_BACK_SHIFT;
    result[3] = 0;
}

inline uint bicubic(const uint rgb[4], const float w[4]) 
{
	int res = (int)(rgb[0] * w[0] + rgb[1] * w[1] + rgb[2] * w[2] + rgb[3] * w[3] + 0.5F);
	res = res > 255 ? 255 : res;
	res = res < 0 ? 0 : res;
	return res;
}

inline void calcWeight(float deta, float weight[4])
{
	weight[3] = (deta * deta * (-1.0F + deta));								
	weight[2] = (deta * (1.0F + deta * (1.0F - deta)));						
	weight[1] = (1.0F + deta * deta * (-2.0F + deta)) ;		
	weight[0] = (-deta * (1.0F + deta * (-2.0F + deta))) ;  
}

void resamplingGlobal(__global const unsigned char* data, int width, int height, int step, float x, float y, __global unsigned char* result) 
{
	int x2 = (int)x;
	int y2 = (int)y;
	int nx[4];
	int ny[4];

	for (int i = 0; i < 4;++i)
	{
		nx[i] = (x2 - 1 + i);
		ny[i] = (y2 - 1 + i);
		if (nx[i] < 0) nx[i] = 0;
		if (nx[i] > width - 1) nx[i] = width - 1;
		if (ny[i] < 0) ny[i] = 0;
		if (ny[i] > height - 1) ny[i] = height - 1;
	}

	float u = (x - nx[1]);
	float v = (y - ny[1]);
	//u,v vertical while /100 horizontal
    float tweight1[4], tweight2[4];
	calcWeight(u, tweight1);//weight
	calcWeight(v, tweight2);//weight

    uchar4 val[4][4];
    for (int j = 0; j < 4; j++)
    {
        for (int i = 0; i < 4; i++)
            val[j][i] = *((__global uchar4*)(data + ny[j] * step) + nx[i]);
    }
    uint temp0[4], temp1[4];
    for (int j = 0; j < 4; j++)
    {
		// 按行去每个通道
		for (int i = 0; i < 4; i++)
		{
			temp0[i] = val[j][i].x;
		}
		//4*4区域的三个通道
		temp1[j] = bicubic(temp0, tweight1);
	}
	result[0] = bicubic(temp1, tweight2);
    for (int j = 0; j < 4; j++)
    {
		// 按行去每个通道
		for (int i = 0; i < 4; i++)
		{
			temp0[i] = val[j][i].y;
		}
		//4*4区域的三个通道
		temp1[j] = bicubic(temp0, tweight1);
	}
	result[1] = bicubic(temp1, tweight2);
    for (int j = 0; j < 4; j++)
    {
		// 按行去每个通道
		for (int i = 0; i < 4; i++)
		{
			temp0[i] = val[j][i].z;
		}
		//4*4区域的三个通道
		temp1[j] = bicubic(temp0, tweight1);
	}
	result[2] = bicubic(temp1, tweight2);
    result[3] = 0;
}

void resamplingLocal(__global const unsigned char* data, int width, int height, int step, float x, float y, unsigned char* result) 
{
	int x2 = (int)x;
	int y2 = (int)y;
	int nx[4];
	int ny[4];

	for (int i = 0; i < 4;++i)
	{
		nx[i] = (x2 - 1 + i);
		ny[i] = (y2 - 1 + i);
		if (nx[i] < 0) nx[i] = 0;
		if (nx[i] > width - 1) nx[i] = width - 1;
		if (ny[i] < 0) ny[i] = 0;
		if (ny[i] > height - 1) ny[i] = height - 1;
	}

	float u = (x - nx[1]);
	float v = (y - ny[1]);
	//u,v vertical while /100 horizontal
    float tweight1[4], tweight2[4];
	calcWeight(u, tweight1);//weight
	calcWeight(v, tweight2);//weight

    uchar4 val[4][4];
    for (int j = 0; j < 4; j++)
    {
        for (int i = 0; i < 4; i++)
            val[j][i] = *((uchar4*)(data + ny[j] * step) + nx[i]);
    }
    uint temp0[4], temp1[4];
    for (int j = 0; j < 4; j++)
    {
		// 按行去每个通道
		for (int i = 0; i < 4; i++)
		{
			temp0[i] = val[j][i].x;
		}
		//4*4区域的三个通道
		temp1[j] = bicubic(temp0, tweight1);
	}
	result[0] = bicubic(temp1, tweight2);
    for (int j = 0; j < 4; j++)
    {
		// 按行去每个通道
		for (int i = 0; i < 4; i++)
		{
			temp0[i] = val[j][i].y;
		}
		//4*4区域的三个通道
		temp1[j] = bicubic(temp0, tweight1);
	}
	result[1] = bicubic(temp1, tweight2);
    for (int j = 0; j < 4; j++)
    {
		// 按行去每个通道
		for (int i = 0; i < 4; i++)
		{
			temp0[i] = val[j][i].z;
		}
		//4*4区域的三个通道
		temp1[j] = bicubic(temp0, tweight1);
	}
	result[2] = bicubic(temp1, tweight2);
    result[3] = 0;
}

__kernel void reprojectCubicKernel(__global const unsigned char* srcData, int srcWidth, int srcHeight, int srcStep,
    __global unsigned char* dstData, int dstWidth, int dstHeight, int dstStep,
    __global const unsigned char* xmapData, int xmapStep, __global const unsigned char* ymapData, int ymapStep)
{
    int dstx = get_global_id(0);
    int dsty = get_global_id(1);
    if (dstx >= dstWidth || dsty >= dstHeight)
        return;

    float srcx = *((__global const float*)(xmapData + dsty * xmapStep) + dstx);
    float srcy = *((__global const float*)(ymapData + dsty * ymapStep) + dstx);

    __global unsigned char* ptrDst = (__global unsigned char*)(dstData + dsty * dstStep) + dstx * 4;
    if (srcx < 0 || srcx >= srcWidth || srcy < 0 || srcy >= srcHeight)
        ptrDst[3] = ptrDst[2] = ptrDst[1] = ptrDst[0] = 0;
    else
        resamplingGlobal(srcData, srcWidth, srcHeight, srcStep, srcx, srcy, ptrDst);
}

__kernel void reprojectWeightedAccumulateTo32FKernel(__global const unsigned char* srcData, int srcWidth, int srcHeight, int srcStep,
    __global unsigned char* dstData, int dstWidth, int dstHeight, int dstStep,
    __global const unsigned char* xmapData, int xmapStep, __global const unsigned char* ymapData, int ymapStep,
    __global const unsigned char* weightData, int weightStep)
{
    int dstx = get_global_id(0);
    int dsty = get_global_id(1);
    if (dstx >= dstWidth || dsty >= dstHeight)
        return;

    float srcx = *((__global const float*)(xmapData + dsty * xmapStep) + dstx);
    float srcy = *((__global const float*)(ymapData + dsty * ymapStep) + dstx);
    
    if (srcx < 0 || srcx >= srcWidth || srcy < 0 || srcy >= srcHeight)
        ;
    else
    {        
        unsigned char temp[4];
        reprojectLinearLocal(srcData, srcWidth, srcHeight, srcStep, srcx, srcy, temp);
        //resamplingLocal(srcData, srcWidth, srcHeight, srcStep, srcx, srcy, temp);
        float w = *((__global const float*)(weightData + dsty * weightStep) + dstx);
        __global float* ptrDst = (__global float*)(dstData + dsty * dstStep) + dstx * 4;
        ptrDst[0] += temp[0] * w;
        ptrDst[1] += temp[1] * w;
        ptrDst[2] += temp[2] * w;
        ptrDst[3] = 0;
    }        
}

)";
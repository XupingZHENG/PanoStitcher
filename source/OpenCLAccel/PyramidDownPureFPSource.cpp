const char* sourcePyramidDownPureFP = R"(

int borderInterpolateWrap(int p, int len)
{
    if( (unsigned)p < (unsigned)len )
        ;
    else
    {
        if( p < 0 )
            p -= ((p-len+1)/len)*len;
        if( p >= len )
            p %= len;
    }
    return p;
}

int borderInterpolateReflect( int p, int len)
{
    if( (unsigned)p < (unsigned)len )
        ;
    else
    {
        if( len == 1 )
            return 0;
        do
        {
            if( p < 0 )
                p = -p;
            else
                p = len - 1 - (p - len) - 1;
        }
        while( (unsigned)p >= (unsigned)len );
    }
    return p;
}

#ifdef HORI_WRAP
#define horiBorder borderInterpolateWrap
#elif defined HORI_REFLECT
#define horiBorder borderInterpolateReflect
#else
#error No horizontal interpolate method
#endif

#ifdef VERT_WRAP
#define vertBorder borderInterpolateWrap
#elif defined VERT_REFLECT
#define vertBorder borderInterpolateReflect
#else
#error No vertical interpolate method
#endif

#define PYR_DOWN_BLOCK_SIZE 256

#define NUM_1_DEN_16 0.0625F
#define NUM_4_DEN_16 0.25F
#define NUM_6_DEN_16 0.375F

inline TYPE getSourceElem(__global const unsigned char* data, int step, int row, int col)
{
    return ((__global TYPE*)(data + step * row))[col];
}

inline __global TYPE* getDestRowPtr(__global unsigned char* data, int step, int row)
{
    return (__global TYPE*)(data + step * row);
}

__kernel void pyrDownKernel(__global const unsigned char* srcData, int srcRows, int srcCols, int srcStep,
    __global unsigned char* dstData, int dstRows, int dstCols, int dstStep)
{
    __local TYPE smem[PYR_DOWN_BLOCK_SIZE + 4];

    const int x = get_global_id(0);
    const int y = get_global_id(1);
	const int localx = get_local_id(0);

    const int srcy = 2 * y;

    if (srcy >= 2 && srcy < srcRows - 2 && x >= 2 && x < srcCols - 2)
    {
        {
            TYPE sum;
            sum =       NUM_1_DEN_16 * getSourceElem(srcData, srcStep, srcy - 2, x);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, srcy - 1, x);
            sum = sum + NUM_6_DEN_16 * getSourceElem(srcData, srcStep, srcy,     x);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, srcy + 1, x);
            sum = sum + NUM_1_DEN_16 * getSourceElem(srcData, srcStep, srcy + 2, x);
            smem[2 + localx] = sum;
        }

        if (localx < 2)
        {
            const int leftx = x - 2;
            TYPE sum;
            sum =       NUM_1_DEN_16 * getSourceElem(srcData, srcStep, srcy - 2, leftx);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, srcy - 1, leftx);
            sum = sum + NUM_6_DEN_16 * getSourceElem(srcData, srcStep, srcy,     leftx);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, srcy + 1, leftx);
            sum = sum + NUM_1_DEN_16 * getSourceElem(srcData, srcStep, srcy + 2, leftx);
            smem[localx] = sum;
        }

        if (localx > PYR_DOWN_BLOCK_SIZE - 3)
        {
            const int rightx = x + 2;
            TYPE sum;
            sum =       NUM_1_DEN_16 * getSourceElem(srcData, srcStep, srcy - 2, rightx);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, srcy - 1, rightx);
            sum = sum + NUM_6_DEN_16 * getSourceElem(srcData, srcStep, srcy,     rightx);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, srcy + 1, rightx);
            sum = sum + NUM_1_DEN_16 * getSourceElem(srcData, srcStep, srcy + 2, rightx);
            smem[4 + localx] = sum;
        }
    }
    else
    {
        {
            TYPE sum;
			int middlex = horiBorder(x, srcCols);
            sum =       NUM_1_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy - 2, srcRows), middlex);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy - 1, srcRows), middlex);
            sum = sum + NUM_6_DEN_16 * getSourceElem(srcData, srcStep, srcy,                          middlex);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy + 1, srcRows), middlex);
            sum = sum + NUM_1_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy + 2, srcRows), middlex);
            smem[2 + localx] = sum;
        }

        if (localx < 2)
        {
            const int leftx = horiBorder(x - 2, srcCols);
            TYPE sum;
            sum =       NUM_1_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy - 2, srcRows), leftx);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy - 1, srcRows), leftx);
            sum = sum + NUM_6_DEN_16 * getSourceElem(srcData, srcStep, srcy,                          leftx);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy + 1, srcRows), leftx);
            sum = sum + NUM_1_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy + 2, srcRows), leftx);
            smem[localx] = sum;
        }

        if (localx > PYR_DOWN_BLOCK_SIZE - 3)
        {
            const int rightx = horiBorder(x + 2, srcCols);
            TYPE sum;
            sum =       NUM_1_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy - 2, srcRows), rightx);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy - 1, srcRows), rightx);
            sum = sum + NUM_6_DEN_16 * getSourceElem(srcData, srcStep, srcy,                          rightx);
            sum = sum + NUM_4_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy + 1, srcRows), rightx);
            sum = sum + NUM_1_DEN_16 * getSourceElem(srcData, srcStep, vertBorder(srcy + 2, srcRows), rightx);
            smem[4 + localx] = sum;
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if (localx < PYR_DOWN_BLOCK_SIZE / 2)
    {
        const int tid2 = localx * 2;
        TYPE sum;
        sum =       NUM_1_DEN_16 * smem[2 + tid2 - 2];
        sum = sum + NUM_4_DEN_16 * smem[2 + tid2 - 1];
        sum = sum + NUM_6_DEN_16 * smem[2 + tid2    ];
        sum = sum + NUM_4_DEN_16 * smem[2 + tid2 + 1];
        sum = sum + NUM_1_DEN_16 * smem[2 + tid2 + 2];

        const int dstx = (x + localx) / 2;

        if (dstx < dstCols)
            getDestRowPtr(dstData, dstStep, y)[dstx] = sum;
    }
}

)";
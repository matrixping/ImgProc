/******************************************************************************
模块名	    ：Resize                 
文件名	    ：resize
相关文件	：resize.h
文件实现功能：图像缩放
作者		：
版本		：v001
-------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		走读人    修改内容
2014-03-11	v001        赵平                  创建
******************************************************************************/
#include "xmmintrin.h"
#include "emmintrin.h"
#include "imgproc_resize.h"

/*====================================================================
函 数 名：  BilinearZoomFloat
功    能：  双线性缩放
算法实现: 
全局变量:	无
参    数：  pu8SrcImage   输入图像[in]
            pu8DstImage   输出图像[in/out]
			u32SrcWidth   输入图像宽[in]
			u32SrcHeight  输入图像高[in]
			u32SrcStride  输入图像步长[in]
			u32DstWidth   输出图像宽[in]
			u32DstHeight  输出图像高[in]
			u32DstStride  输出图像步长[in]
返 回 值：  无
====================================================================*/
void BilinearZoomFloat(u8 *pu8SrcImage, u8 *pu8DstImage,
					   u32 u32SrcWidth, u32 u32SrcHeight, u32 u32SrcStride, 
					   u32 u32DstWidth, u32 u32DstHeight, u32 u32DstStride)
{
    u8 *pu8SrcLine1 = NULL;
    u8 *pu8SrcLine2 = NULL;
    u8 *pu8Dst = NULL;
    f32 f32StrideX = (f32)u32SrcWidth / u32DstWidth;
    f32 f32StrideY = (f32)u32SrcHeight / u32DstHeight;
    f32 f32WY1 = 0, f32WY2 = 0;
    f32 f32WX1 = 0, f32WX2 = 0;
    f32 f32Vn0 = 0, f32Vn1 = 0;
    f32 f32Temp = 0;
    u32 u32XSrc = 0, u32YSrc = 0;
    u32 u32Row = 0, u32Col = 0;

    for(u32Row = 0; u32Row < u32DstHeight; u32Row++)
    {
        f32Temp = (u32Row + 0.5f) * f32StrideY - 0.5f;
        u32YSrc = (u32)f32Temp;
        
        //垂直插值权重
        f32WY2 = f32Temp - u32YSrc;
        f32WY1 = 1.0f - f32WY2;
        pu8SrcLine1 = pu8SrcImage + u32YSrc * u32SrcStride;
        pu8SrcLine2 = u32YSrc > u32SrcHeight - 1 ? pu8SrcLine1 : pu8SrcLine1 + u32SrcStride;
        pu8Dst = pu8DstImage + u32Row * u32DstStride;
        for(u32Col = 0; u32Col < u32DstWidth; u32Col++)
        {
            f32Temp = (u32Col + 0.5) * f32StrideX - 0.5;
            u32XSrc = (u32)f32Temp;

            //水平插值权重
            f32WX2 = f32Temp - u32XSrc;
            f32WX1 = 1.0f - f32WX2;

            //水平线性滤波
            f32Vn0 = pu8SrcLine1[u32XSrc] * f32WX1 + pu8SrcLine1[u32XSrc + 1] * f32WX2;
            f32Vn1 = pu8SrcLine2[u32XSrc] * f32WX1 + pu8SrcLine2[u32XSrc + 1] * f32WX2;

            //垂直线性滤波
            *pu8Dst++ = (u8)(f32Vn0 * f32WY1 + f32Vn1 * f32WY2 + 0.5);
        }
    }
}

/*====================================================================
函 数 名：  BilinearZoomFix
功    能：  双线性缩放
算法实现: 
全局变量:	无
参    数：  pu8SrcImage   输入图像[in]
            pu8DstImage   输出图像[in/out]
			u32SrcWidth   输入图像宽[in]
			u32SrcHeight  输入图像高[in]
			u32SrcStride  输入图像步长[in]
			u32DstWidth   输出图像宽[in]
			u32DstHeight  输出图像高[in]
			u32DstStride  输出图像步长[in]
返 回 值：  无
====================================================================*/
void BilinearZoomFix(u8 *pu8SrcImage, u8 *pu8DstImage,
					 u32 u32SrcWidth, u32 u32SrcHeight, u32 u32SrcStride, 
					 u32 u32DstWidth, u32 u32DstHeight, u32 u32DstStride)
{
	u8 *pu8SrcLine1;
	u8 *pu8SrcLine2;
	u8 *pu8Dst;

	u32 u32WY1,u32WY2;
	u32 u32WX1,u32WX2;

	u32 u32XPosition,u32YPosition;
	u32 u32XSrc, u32YSrc;
	u32 u32XStride, u32YStride;
	u32 u32Vn0,u32Vn1;

	u32 u32RowIndex;
	u32 u32LineIndex;
	u32 u32YPositionInit = 0;
	u32 u32XPositionInit = 0;

	u32XStride = ((u32SrcWidth - 1) << 16) / (u32DstWidth - 1);//pow(2,16)*Src_width/Dst_width
	u32YStride = ((u32SrcHeight - 1) << 16) / (u32DstHeight - 1);//pow(2,16)*Src_height/Dst_height

	//判断是否需要进行起始点偏移
	if(0 == ((u32XStride << 16) >> 27))
	{
		u32XStride = ((u32SrcWidth - 2) << 16) / (u32DstWidth - 1);//pow(2,16)*Src_width/Dst_width
		u32XPositionInit = 1 << 15;
	}

	if((u32SrcHeight != u32DstHeight) && (0 == ((u32YStride << 16) >> 27)))
	{
		u32YStride = ((u32SrcHeight - 2) << 16) / (u32DstHeight - 1);//pow(2,16)*Src_height/Dst_height
		u32YPositionInit = 1 << 15;
	}

	u32YPosition = u32YPositionInit;
	pu8Dst = pu8DstImage;

	for(u32RowIndex = 0; u32RowIndex < u32DstHeight; u32RowIndex++)
	{
		u32YSrc = u32YPosition >> 16;

		//垂直方向权值
		u32WY2 = (u32YPosition << 16) >> 29;
		u32WY1 = 8 - u32WY2;

		//放大权值
		u32WY2 *= 1024;
		u32WY1 *= 1024;

		pu8SrcLine1 = pu8SrcImage + u32YSrc * u32SrcStride;
		pu8SrcLine2 = u32WY2 == 0 ? pu8SrcLine1 : pu8SrcLine1 + u32SrcStride; //在最后一行的时候，读的数据是不参与运算的，但VC不允许越界读，因此这里加判断防止读越界

		pu8Dst = pu8DstImage + u32RowIndex * u32DstStride;//u32DstWidth;

		u32XPosition = u32XPositionInit;
		for(u32LineIndex = 0; u32LineIndex < u32DstWidth; u32LineIndex++)
		{
			//定点数的高16位就是整数部分
			u32XSrc = u32XPosition >> 16;

			//定点数的低16位就是小数部分，我们使用其高3位作为权值(范围0-8)
			u32WX2 = (u32XPosition << 16) >> 29;
			u32WX1 = 8 - u32WX2;

			//水平线性滤波
			u32Vn0 = pu8SrcLine1[u32XSrc] * u32WX1 + pu8SrcLine1[u32XSrc + 1] * u32WX2;
			u32Vn1 = pu8SrcLine2[u32XSrc] * u32WX1 + pu8SrcLine2[u32XSrc + 1] * u32WX2;

			//垂直线性滤波
			*pu8Dst++ = (u8)((u32Vn0 * u32WY1 + u32Vn1 * u32WY2 + 0x8000) >> 16);

			u32XPosition += u32XStride;
		}
		u32YPosition += u32YStride;
	}
}

/*====================================================================
函 数 名：  BilinearZoomSSE
功    能：  优化后的双线性缩放
算法实现: 
全局变量:	无
参    数：  pu8SrcImage   输入图像[in]
            pu8DstImage   输出图像[in/out]
			u32SrcWidth   输入图像宽[in]
			u32SrcHeight  输入图像高[in]
			u32SrcStride  输入图像步长[in]
			u32DstWidth   输出图像宽[in]
			u32DstHeight  输出图像高[in]
			u32DstStride  输出图像步长[in]
			pu32Temp      临时buff[in]
返 回 值：  无
====================================================================*/
void BilinearZoomSSE(u8 *pu8SrcImage, u8 *pu8DstImage, 
					u32 u32SrcWidth, u32 u32SrcHeight, u32 u32SrcStride, 
					u32 u32DstWidth,  u32 u32DstHeight, u32 u32DstStride, u32 *pu32Temp)
{
	u8 *pu8SrcLine1;
	u8 *pu8SrcLine2;
	u8 *pu8Dst;

	u32 u32WY1,u32WY2;
	u32 u32WX1,u32WX2;

	u32 u32XPosition,u32YPosition;
	u32 u32XSrc, u32YSrc;
	u32 u32XStride, u32YStride;

	u32 u32RowIndex;
	u32 u32LineIndex;
	u32 u32YPositionInit = 0;
	u32 u32XPositionInit = 0;

	//应优化需要所添加的变量
	u32 *pu32WXBuffer = NULL;
	u32 u32Aligned4DstWidth = 0;
	u32 u32InitAlignedAddrOffset = 0;
	u32 u32RemainWidth = 0;
	__m64 mmxDValue1, mmxDValue2, mmxWYTemp, mmxProTemp0, mmxProTemp1;
	__m128i xmmCvtTemp0, xmmCvtTemp1, xmmTemp1, xmmTemp2, xmmInterValue, xmmWY;
	u8 *pu8ImageLastLine = NULL;
	u32 u32Temp = 0;

	u32XStride = ((u32SrcWidth - 1) << 16) / (u32DstWidth - 1);//pow(2,16)*Src_width/Dst_width
	u32YStride = ((u32SrcHeight - 1) << 16) / (u32DstHeight - 1);//pow(2,16)*Src_height/Dst_height

	//判断是否需要进行起始点偏移
	if(0 == ((u32XStride << 16) >> 27))
	{
		u32XStride = ((u32SrcWidth - 2) << 16) / (u32DstWidth - 1);//pow(2,16)*Src_width/Dst_width
		u32XPositionInit = 1 << 15;
	}

	if((u32SrcHeight != u32DstHeight) && (0 == ((u32YStride << 16) >> 27)))
	{
		u32YStride = ((u32SrcHeight - 2) << 16) / (u32DstHeight - 1);//pow(2,16)*Src_height/Dst_height
		u32YPositionInit = 1 << 15;
	}

	u32YPosition = u32YPositionInit;
	pu8Dst = pu8DstImage;

	//将目的图像宽度按照四字节对齐（把二进制数的最低两位清零）
	u32Aligned4DstWidth = u32DstWidth & (~0x3);
	u32RemainWidth = u32DstWidth - u32Aligned4DstWidth;
    u32InitAlignedAddrOffset = (u32)pu32Temp;
	u32InitAlignedAddrOffset = ((u32InitAlignedAddrOffset + 15) & (~0xf)) - u32InitAlignedAddrOffset;
    pu32WXBuffer = pu32Temp + (u32InitAlignedAddrOffset / sizeof(u32));
	pu8ImageLastLine = pu8SrcImage + (u32SrcHeight - 1) * u32SrcStride;

	//统一计算水平方向上插值的权重值，存放到数组buffer里面，避免每行都要重新计算
	//权重值保留了小数部分高三位
	for(u32LineIndex = 0, u32XPosition = u32XPositionInit; u32LineIndex < u32DstWidth; u32LineIndex++)
	{
		//对小数部分进行四舍五入处理，比对十进制数，如果我们把小数部分本来8位只保留3位也会进行四舍五入处理的
		//针对某一行数据的每个像素点的像素值，都是由四个点的插值得到，这四个点对应着4个权重值分别是WX1 WX2 WX1 WX2
		//两个WX1相同，两个WX2相同
		//经过下面的逻辑处理后，u32WX2中存放的数据由低到高表示为WX1 WX2 WX1 WX2，该变量存放了计算一个像素值所需要的四个点的权重            
		u32WX2 = (u32XPosition << 16) >> 29;
		u32WX1 = 8 - u32WX2;
		u32WX2 = (u32WX2 << 8) | u32WX1;
		u32WX2 = (u32WX2 << 16) | u32WX2;
		pu32WXBuffer[u32LineIndex] = u32WX2;
		u32XPosition += u32XStride;
	}
	//定义临时数据，准备对计算结果处理
	//           l32a0 l32a1 l32a2 l32a3 | u8a0 u8a1 u8a2 u8a3
	xmmCvtTemp0 = _mm_cvtsi32_si128(0x20);
	xmmCvtTemp0 = _mm_shuffle_epi32(xmmCvtTemp0, 0);
	xmmCvtTemp1 = _mm_cvtsi32_si128(0x0c080400);
	mmxProTemp0 = _mm_movepi64_pi64(xmmCvtTemp0);
	mmxProTemp1 = _mm_cvtsi32_si64(0x0400);

	for(u32RowIndex = 0; u32RowIndex < u32DstHeight; u32RowIndex++)
	{
		u32YSrc = u32YPosition >> 16;

		//垂直方向权值
		u32WY2 = (u32YPosition << 16) >> 29;
		u32WY1 = 8 - u32WY2;
		u32WY2 = (u32WY2 << 16) | u32WY1;
		//将u32权重值转换为128位数据，便于后面扩展使用
		xmmWY = _mm_cvtsi32_si128(u32WY2);
		xmmWY = _mm_shuffle_epi32(xmmWY, 0);

		pu8SrcLine1 = pu8SrcImage + u32YSrc * u32SrcStride;
		//在最后一行的时候，读的数据是不参与运算的，但VC不允许越界读，因此这里加判断防止读越界
		pu8SrcLine2 = (pu8SrcLine1 >= pu8ImageLastLine) ? pu8SrcLine1 : pu8SrcLine1 + u32SrcStride;

		//预取指令
		_mm_prefetch((s8*)pu8SrcLine1 + 24, _MM_HINT_T0);
		_mm_prefetch((s8*)pu8SrcLine2 + 24, _MM_HINT_T0);		

		//先读取要计算4个点的所需要的原图数据，准备计算
		//计算第一个点所需要的原图中的四个点
		mmxDValue1 = _mm_cvtsi32_si64(*(l32 *)pu8SrcLine1);
		mmxDValue1 = _mm_insert_pi16(mmxDValue1, *(l32 *)pu8SrcLine2, 1);

		//计算第二个点所需要的原图中的四个点
		u32XPosition = u32XStride;
		u32XSrc = u32XPosition >> 16;
		mmxDValue1 = _mm_insert_pi16(mmxDValue1, *(l32 *)&pu8SrcLine1[u32XSrc], 2);
		mmxDValue1 = _mm_insert_pi16(mmxDValue1, *(l32 *)&pu8SrcLine2[u32XSrc], 3);

		//计算第三个点所需要的原图中的四个点
		u32XPosition += u32XStride;
		u32XSrc = u32XPosition >> 16;
		mmxDValue2 = _mm_cvtsi32_si64(*(l32 *)&pu8SrcLine1[u32XSrc]);
		mmxDValue2 = _mm_insert_pi16(mmxDValue2, *(l32 *)&pu8SrcLine2[u32XSrc], 1);

		//计算第四个点所需要的原图中的四个点
		u32XPosition += u32XStride;
		u32XSrc = u32XPosition >> 16;
		mmxDValue2 = _mm_insert_pi16(mmxDValue2, *(l32 *)&pu8SrcLine1[u32XSrc], 2);
		mmxDValue2 = _mm_insert_pi16(mmxDValue2, *(l32 *)&pu8SrcLine2[u32XSrc], 3);

		for(u32LineIndex = 0; u32LineIndex < u32Aligned4DstWidth; u32LineIndex += 4)
		{
			//将mmxDValue1和mmxDValue2转换为128位数据，并打包在一起，运算后xmmInterValue数据形式如下：
			//Line1CNT10 Line1CNT11 Line2CNT10 Line2CNT11 Line1CNT20 Line1CNT21 Line2CNT20 Line2CNT21 Line1CNT30 Line1CNT31 Line2CNT30 Line2CNT31 Line1CNT40 Line1CNT41 Line2CNT40 Line2CNT41
			xmmTemp1 = _mm_movpi64_epi64(mmxDValue1);
			xmmTemp2 = _mm_movpi64_epi64(mmxDValue2);
			xmmInterValue = _mm_unpacklo_epi64(xmmTemp1, xmmTemp2);

			//读取第一组数据，每组数据有4个，如下表示：
			//                                        Line1CNT10   Line1CNT11
			//                                        Line2CNT10   Line2CNT11
			//在第一行从u32XSrc位置开始连续读取4个数据(4x8=32),转换为64位，高32位是0
			//mmxDValue1:   a0 a1 a2 a3 a4 a5 a6 a7   字节
			//                w0   w1    w2    w3      字
			//执行下面的操作后mmxDValue1的值由低到高是Line1CNT10 Line1CNT11 Line2CNT10 Line2CNT11 0 0 0 0
			u32XPosition += u32XStride;
			u32XSrc = u32XPosition >> 16;
			mmxDValue1 = _mm_cvtsi32_si64(*(l32 *)&pu8SrcLine1[u32XSrc]);
			mmxDValue1 = _mm_insert_pi16(mmxDValue1, *(l32 *)&pu8SrcLine2[u32XSrc], 1);     

			//xmmInterValue先进行一次乘加运算得到水平方向的插值
			//再进行一次乘加运算得到垂直方向的插值  pu32WXBuffer
			//要检查存放权值的方式跟取用权值的方式是否一致，保证输入的正确性
			xmmInterValue = _mm_maddubs_epi16(xmmInterValue, *(__m128i *)(pu32WXBuffer+u32LineIndex));
			xmmInterValue = _mm_madd_epi16(xmmInterValue, xmmWY);

			//经过下面的操作mmxDValue1的值由低到高是Line1CNT10 Line1CNT11 Line2CNT10 Line2CNT11 Line1CNT20 Line1CNT21 Line2CNT20 Line2CNT21
			u32XPosition += u32XStride;
			u32XSrc = u32XPosition >> 16;
			mmxDValue1 = _mm_insert_pi16(mmxDValue1, *(l32 *)(pu8SrcLine1+u32XSrc), 2);
			mmxDValue1 = _mm_insert_pi16(mmxDValue1, *(l32 *)(pu8SrcLine2+u32XSrc), 3);

			//把计算得到的4个结果分别四舍五入并取整
			xmmInterValue = _mm_add_epi32(xmmInterValue, xmmCvtTemp0);
			xmmInterValue = _mm_srli_epi32(xmmInterValue, 6);

			//重复上面的操作，再取两组数据存放在mmxDValue2中
			u32XPosition += u32XStride;
			u32XSrc = u32XPosition >> 16;
			mmxDValue2 = _mm_cvtsi32_si64(*(l32 *)&pu8SrcLine1[u32XSrc]);
			mmxDValue2 = _mm_insert_pi16(mmxDValue2, *(l32 *)&pu8SrcLine2[u32XSrc], 1);

			//取四个32位数据中的低八位存放在整个数据的低32位
			xmmInterValue = _mm_shuffle_epi8(xmmInterValue, xmmCvtTemp1);

			u32XPosition += u32XStride;
			u32XSrc = u32XPosition >> 16;
			mmxDValue2 = _mm_insert_pi16(mmxDValue2, *(l32 *)&pu8SrcLine1[u32XSrc], 2);
			mmxDValue2 = _mm_insert_pi16(mmxDValue2, *(l32 *)&pu8SrcLine2[u32XSrc], 3);

			//取出低32位，高位舍弃掉
			*(l32 *)&pu8Dst[u32LineIndex] = _mm_cvtsi128_si32(xmmInterValue);
		}		
		//因为前面循环的是4字节对齐的宽度，当原本宽度不是按照四字节对齐的话会有些像素点没有进行插值计算
		//下面需要进行单独计算
		if(u32RemainWidth != 0)
		{
			switch(u32RemainWidth)
			{
			case 1:
				//mmxDValue1中保存了最后一个像素点需要的四个点以及是我们不需要的四个点，这四个点需要删除
				mmxDValue1 = _mm_maddubs_pi16(mmxDValue1, *(__m64 *)&pu32WXBuffer[u32LineIndex]);
				mmxWYTemp = _mm_movepi64_pi64(xmmWY);
				mmxDValue1 = _mm_madd_pi16(mmxDValue1, mmxWYTemp);
				mmxDValue1 = _mm_add_pi32(mmxDValue1, mmxProTemp0);
				mmxDValue1 = _mm_srli_pi32(mmxDValue1, 6);
				pu8Dst[u32LineIndex] = (u8)_mm_cvtsi64_si32(mmxDValue1);
				break;
			case 2:
				//mmxDValue1中保存了最后两个像素点需要的八个点
				mmxDValue1 = _mm_maddubs_pi16(mmxDValue1, *(__m64 *)&pu32WXBuffer[u32LineIndex]);
				mmxWYTemp = _mm_movepi64_pi64(xmmWY);
				mmxDValue1 = _mm_madd_pi16(mmxDValue1, mmxWYTemp);
				mmxDValue1 = _mm_add_pi32(mmxDValue1, mmxProTemp0);
				mmxDValue1 = _mm_srli_pi32(mmxDValue1, 6);
				mmxDValue1 = _mm_shuffle_pi8(mmxDValue1, mmxProTemp1);
				*(u16 *)&pu8Dst[u32LineIndex] = (u16)_mm_cvtsi64_si32(mmxDValue1);
				break;
			case 3:
				xmmTemp1 = _mm_movpi64_epi64(mmxDValue1);
				xmmTemp2 = _mm_movpi64_epi64(mmxDValue2);
				xmmInterValue = _mm_unpacklo_epi64(xmmTemp1, xmmTemp2);
				xmmInterValue = _mm_maddubs_epi16(xmmInterValue, *(__m128i *)&pu32WXBuffer[u32LineIndex]);
				xmmInterValue = _mm_madd_epi16(xmmInterValue, xmmWY);
				xmmInterValue = _mm_add_epi32(xmmInterValue, xmmCvtTemp0);
				xmmInterValue = _mm_srli_epi32(xmmInterValue, 6);
				xmmInterValue = _mm_shuffle_epi8(xmmInterValue, xmmCvtTemp1);
				u32Temp = _mm_cvtsi128_si32(xmmInterValue);
				*(u16 *)&pu8Dst[u32LineIndex] = (u16)u32Temp;
				pu8Dst[u32LineIndex + 2] = (u8)(u32Temp >> 16);
				break;
			}
		}
		u32YPosition += u32YStride;
		pu8Dst += u32DstStride;   
	}

    _mm_empty();
}

/*====================================================================
函 数 名：  ImgResize
功    能：  图像缩放
算法实现: 
全局变量:	无
参    数：  ptSrcImage      输入图像[in]
			ptDstImage      输出图像[in/out]
			pu32TempBuff    临时buff[in]
返 回 值：  无
====================================================================*/
void ImgResize(TImage *ptSrcImage, TImage *ptDstImage, u32 *pu32TempBuff)
{
	l32 l32Channels = ptSrcImage->u32Type == AI_Y ? 1 : 3;
	l32 l32Index = 0;

	for(l32Index = 0; l32Index < l32Channels; l32Index++)
	{
        //浮点计算
        if(BILINEAR_COMPUTE_MODE == 0)
        {
            BilinearZoomFloat((u8 *)ptSrcImage->atPlane[l32Index].pvBuffer, (u8 *)ptDstImage->atPlane[l32Index].pvBuffer, 
                (u32)ptSrcImage->atPlane[l32Index].l32Width, (u32)ptSrcImage->atPlane[l32Index].l32Height, (u32)ptSrcImage->atPlane[l32Index].l32Stride, 
                (u32)ptDstImage->atPlane[l32Index].l32Width, (u32)ptDstImage->atPlane[l32Index].l32Height, (u32)ptDstImage->atPlane[l32Index].l32Stride);
        }
        //定点化
        else if(BILINEAR_COMPUTE_MODE == 1)
        {
            BilinearZoomFix((u8 *)ptSrcImage->atPlane[l32Index].pvBuffer, (u8 *)ptDstImage->atPlane[l32Index].pvBuffer, 
                (u32)ptSrcImage->atPlane[l32Index].l32Width, (u32)ptSrcImage->atPlane[l32Index].l32Height, (u32)ptSrcImage->atPlane[l32Index].l32Stride, 
                (u32)ptDstImage->atPlane[l32Index].l32Width, (u32)ptDstImage->atPlane[l32Index].l32Height, (u32)ptDstImage->atPlane[l32Index].l32Stride);
        }
        //SSE优化
        else
        {
            BilinearZoomSSE((u8 *)ptSrcImage->atPlane[l32Index].pvBuffer, (u8 *)ptDstImage->atPlane[l32Index].pvBuffer, 
                (u32)ptSrcImage->atPlane[l32Index].l32Width, (u32)ptSrcImage->atPlane[l32Index].l32Height, (u32)ptSrcImage->atPlane[l32Index].l32Stride, 
                (u32)ptDstImage->atPlane[l32Index].l32Width, (u32)ptDstImage->atPlane[l32Index].l32Height, (u32)ptDstImage->atPlane[l32Index].l32Stride, pu32TempBuff);
        }
	}
}

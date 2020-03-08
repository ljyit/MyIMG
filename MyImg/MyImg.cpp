#include <windows.h>
#include "VfpLib/pro_ext.h"
#include "MyIMg.h"
//zlib
#include "ziplib/zlib.h"
#pragma comment(lib,"ziplib/zlib.lib")
//CXimage
#include "../CxImg/CxImage/ximage.h"
#pragma comment(lib,"../CxImg/CxImage/Release/cximage.lib")
#pragma comment(lib,"../CxImg/jpeg/Release/jpeg.lib")
#pragma comment(lib,"../CxImg/png/Release/png.lib")
#pragma comment(lib,"../CxImg/tiff/Release/tiff.lib")
//////////////////////////////////////////////////////////////////////////
#define P0 parm->p[0].val
#define P1 parm->p[1].val
#define P2 parm->p[2].val
#define P3 parm->p[3].val
#define P4 parm->p[4].val
#define msg(x)  MessageBox(0,x,0,0)

#define PGetImgType(PP,nType) \
		if(PP == _IMG_BMP ) nType = CXIMAGE_FORMAT_BMP;\
		if(PP == _IMG_GIF ) nType = CXIMAGE_FORMAT_GIF;\
		if(PP == _IMG_JPG ) nType = CXIMAGE_FORMAT_JPG;\
		if(PP == _IMG_PNG ) nType = CXIMAGE_FORMAT_PNG;\
		if(PP == _IMG_TIF ) nType = CXIMAGE_FORMAT_TIF;
//////////////////////////////////////////////////////////////////////////
//打开
void FAR ImgOpen(ParamBlk FAR *parm)
{
	LPCTSTR szFileName=NULL;
	CxImage * img=new CxImage;
	
	//初始化返回值
	Value vRet;
	vRet.ev_type='C';
	vRet.ev_length=4;
	vRet.ev_handle=_AllocHand(4);
	memcpy(_HandToPtr(vRet.ev_handle),&img,vRet.ev_length);

	if(parm->pCount == 3 )  //三个参数,第三个参数指定解码方式
	{
		img->SetFrame(P2.ev_long-1);
	}
	if(parm->pCount == 2 )  //两个参数,内存解码
	{
		DWORD nType=CXIMAGE_FORMAT_BMP;
		PGetImgType(P1.ev_long,nType);
		img->Decode((BYTE*)_HandToPtr(P0.ev_handle),P0.ev_length,nType);
		goto LBL253453245;
	}
	//只有一个参数
	if(P0.ev_type == 'I')
	{
		switch(P0.ev_long) 
		{
		case _FROM_SCREEN ://屏幕取图
			HDC         hScrDC, hMemDC;		// 屏幕和内存设备描述表   
			int         nWidth, nHeight;	// 位图宽度和高度   
			HBITMAP     hBitmap;//位图句柄
			
			//选择内存DC，绘制到其中，这样的 hBitmap 就是一DDB形式的抓屏结果了
			hScrDC  = GetDC(NULL);						//屏幕DC
			nWidth =GetDeviceCaps(hScrDC, HORZRES);		//宽
			nHeight=GetDeviceCaps(hScrDC, VERTRES);		//高
			
			hMemDC  = CreateCompatibleDC(NULL);
			hBitmap = CreateCompatibleBitmap(hScrDC, nWidth,nHeight);//抓取屏幕
			
			SelectObject(hMemDC,hBitmap);
			BitBlt( hMemDC, 0, 0, nWidth, nHeight, hScrDC, 0, 0, SRCCOPY);	//得到DDB
			
			DeleteDC(hScrDC);
			DeleteDC(hMemDC); 
			
			//hBitmap是一个内存图
			img->CreateFromHBITMAP( hBitmap );
			DeleteObject(hBitmap);
			break;
		case _FROM_CLIPBORD://剪贴板
			OpenClipboard(NULL);
			img->CreateFromHANDLE( GetClipboardData(CF_DIB) );
			CloseClipboard();
			break;
		default:
			break;
		}
	}
	if(P0.ev_type == 'C') //文件
	{
		
		LPCTSTR szFileName=GetStr(parm,1);
		img->Load(szFileName);
	}

LBL253453245:
	if(!img->IsValid())
	{
		delete img;
		vRet.ev_length=0;//没有返回值
	}
	_RetVal(&vRet);
	_FreeHand(vRet.ev_handle);
}
//////////////////////////////////////////////////////////////////////////
//关闭
void FAR ImgClose(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	delete img;
}
//////////////////////////////////////////////////////////////////////////
//获取DPI
void FAR ImgGetXDpi(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	_RetInt(img->GetXDPI(),11);
}
void FAR ImgGetYDpi(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	_RetInt(img->GetYDPI(),11);
}
//////////////////////////////////////////////////////////////////////////
//设置灰度
void FAR ImgSetGray(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	img->GrayScale();

}
//////////////////////////////////////////////////////////////////////////
//保存
void FAR ImgSave(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);

	LPCSTR szFileName=GetStr(parm,2);
	
	DWORD nType = CXIMAGE_FORMAT_BMP;
	PGetImgType(P2.ev_long,nType);
	
	if(nType==CXIMAGE_FORMAT_GIF || nType == CXIMAGE_FORMAT_TIF)
		img->DecreaseBpp(8,false);//Gif tif 都要设置深度
	if(nType == CXIMAGE_FORMAT_JPG)
		if(!img->IsGrayScale())img->IncreaseBpp(24);  //jpeg要设置24

	_RetLogical( img->Save(szFileName,nType) );
}
//////////////////////////////////////////////////////////////////////////
//裁剪
void FAR ImgCrop(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	RECT rc;
	rc.left=P1.ev_long;
	rc.top=P2.ev_long;
	rc.right=parm->pCount > 3 ? rc.left+P3.ev_long : img->GetWidth();
	rc.bottom=parm->pCount > 4 ? rc.top+P4.ev_long : img->GetHeight();

	_RetLogical( img->Crop(rc) );
}
//////////////////////////////////////////////////////////////////////////
//缩放
void FAR ImgZoom(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	_RetLogical( img->Resample(P1.ev_long,P2.ev_long) );
}
//////////////////////////////////////////////////////////////////////////
//旋转
void FAR ImgRotate(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);

	_RetLogical( img->Rotate((float)P1.ev_real) );

}
//////////////////////////////////////////////////////////////////////////
//设置jpg的品质
void FAR ImgSetJpegQuality(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	img->SetJpegQuality((BYTE)P1.ev_long);
}
//////////////////////////////////////////////////////////////////////////
//复制到剪贴板
void FAR ImgCopyToClipbord(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	BOOL isOK=FALSE;

	HANDLE hDIB = img->CopyToHandle();
	if ( OpenClipboard(NULL) )
	{
		if( EmptyClipboard() )
		{
			if ( SetClipboardData(CF_DIB,hDIB) != NULL )
				isOK=TRUE;
		}
		CloseClipboard();
	}
	_RetLogical(isOK);
}
//////////////////////////////////////////////////////////////////////////
//取得内容
void FAR ImgGetPtr(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	Value vRet;
	vRet.ev_type='C';
	vRet.ev_length=0;


	DWORD nType = CXIMAGE_FORMAT_BMP;
	PGetImgType(P1.ev_long,nType);

	if(nType==CXIMAGE_FORMAT_GIF || nType == CXIMAGE_FORMAT_TIF)
		img->DecreaseBpp(8,false);//Gif tif 都要设置深度
	if(nType == CXIMAGE_FORMAT_JPG)
		if(!img->IsGrayScale())img->IncreaseBpp(24);  //jpeg要设置24
		

	BYTE * pBuff=NULL;
	long nSize=0;
	
	if(! img->Encode(pBuff,nSize,nType))
	{
		//解码失败!
		_RetVal(&vRet);
		return;
	}
	
	vRet.ev_length=nSize;
	vRet.ev_handle=_AllocHand(vRet.ev_length);
	memcpy(_HandToPtr(vRet.ev_handle),pBuff,vRet.ev_length);
	
	_RetVal(&vRet);
	_FreeHand(vRet.ev_handle);
	free(pBuff);
	
}
//////////////////////////////////////////////////////////////////////////
void FAR ImgGetWidth(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	_RetInt(img->GetWidth(),11);
}
//////////////////////////////////////////////////////////////////////////
void FAR ImgGetHeight(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);

	_RetInt(img->GetHeight(),11);
}
//////////////////////////////////////////////////////////////////////////
void FAR ImgGetLastError(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	_RetChar((char*)img->GetLastError());
}	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//取得Frame数量
void FAR ImgGetFramesCount(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	_RetInt(img->GetNumFrames(),11);
}

//////////////////////////////////////////////////////////////////////////
FoxInfo myFoxInfo[] =
{
	{"ImgOpen",						(FPFI) ImgOpen,						3, "?.I.I"},
	{"ImgGetWidth",					(FPFI) ImgGetWidth,					1, "C"},
	{"ImgGetHeight",				(FPFI) ImgGetHeight,				1, "C"},
	{"ImgClose",					(FPFI) ImgClose,					1, "C"},
	{"ImgSave",						(FPFI) ImgSave,						3, "CCI"},
	{"ImgCopyToClipbord",			(FPFI) ImgCopyToClipbord,			1, "C"},
	{"ImgCrop",						(FPFI) ImgCrop,						5, "CII.I.I"},
	{"ImgZoom",						(FPFI) ImgZoom,						3, "CII"},
	{"ImgRotate",					(FPFI) ImgRotate,					2, "CN"},
	{"ImgSetJpegQuality",			(FPFI) ImgSetJpegQuality,			2, "CI"},
	{"ImgGetPtr",					(FPFI) ImgGetPtr,					2, "CI"},
	{"ImgGetLastError",				(FPFI) ImgGetLastError,				1, "C"},
	{"ImgGetXDpi",					(FPFI) ImgGetXDpi,					1, "C"},
	{"ImgGetYDpi",					(FPFI) ImgGetYDpi,					1, "C"},
	{"ImgSetGray",					(FPFI) ImgSetGray,					1, "C"},
	{"ImgGetFramesCount",			(FPFI) ImgGetFramesCount,			1, "C"},
};

#ifdef __cplusplus
extern "C" {
#endif
	FoxTable _FoxTable = 
	{
		(FoxTable FAR *) 0, sizeof(myFoxInfo)/sizeof(FoxInfo), myFoxInfo
	};
#ifdef __cplusplus
}
#endif
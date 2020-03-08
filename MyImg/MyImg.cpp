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
//��
void FAR ImgOpen(ParamBlk FAR *parm)
{
	LPCTSTR szFileName=NULL;
	CxImage * img=new CxImage;
	
	//��ʼ������ֵ
	Value vRet;
	vRet.ev_type='C';
	vRet.ev_length=4;
	vRet.ev_handle=_AllocHand(4);
	memcpy(_HandToPtr(vRet.ev_handle),&img,vRet.ev_length);

	if(parm->pCount == 3 )  //��������,����������ָ�����뷽ʽ
	{
		img->SetFrame(P2.ev_long-1);
	}
	if(parm->pCount == 2 )  //��������,�ڴ����
	{
		DWORD nType=CXIMAGE_FORMAT_BMP;
		PGetImgType(P1.ev_long,nType);
		img->Decode((BYTE*)_HandToPtr(P0.ev_handle),P0.ev_length,nType);
		goto LBL253453245;
	}
	//ֻ��һ������
	if(P0.ev_type == 'I')
	{
		switch(P0.ev_long) 
		{
		case _FROM_SCREEN ://��Ļȡͼ
			HDC         hScrDC, hMemDC;		// ��Ļ���ڴ��豸������   
			int         nWidth, nHeight;	// λͼ��Ⱥ͸߶�   
			HBITMAP     hBitmap;//λͼ���
			
			//ѡ���ڴ�DC�����Ƶ����У������� hBitmap ����һDDB��ʽ��ץ�������
			hScrDC  = GetDC(NULL);						//��ĻDC
			nWidth =GetDeviceCaps(hScrDC, HORZRES);		//��
			nHeight=GetDeviceCaps(hScrDC, VERTRES);		//��
			
			hMemDC  = CreateCompatibleDC(NULL);
			hBitmap = CreateCompatibleBitmap(hScrDC, nWidth,nHeight);//ץȡ��Ļ
			
			SelectObject(hMemDC,hBitmap);
			BitBlt( hMemDC, 0, 0, nWidth, nHeight, hScrDC, 0, 0, SRCCOPY);	//�õ�DDB
			
			DeleteDC(hScrDC);
			DeleteDC(hMemDC); 
			
			//hBitmap��һ���ڴ�ͼ
			img->CreateFromHBITMAP( hBitmap );
			DeleteObject(hBitmap);
			break;
		case _FROM_CLIPBORD://������
			OpenClipboard(NULL);
			img->CreateFromHANDLE( GetClipboardData(CF_DIB) );
			CloseClipboard();
			break;
		default:
			break;
		}
	}
	if(P0.ev_type == 'C') //�ļ�
	{
		
		LPCTSTR szFileName=GetStr(parm,1);
		img->Load(szFileName);
	}

LBL253453245:
	if(!img->IsValid())
	{
		delete img;
		vRet.ev_length=0;//û�з���ֵ
	}
	_RetVal(&vRet);
	_FreeHand(vRet.ev_handle);
}
//////////////////////////////////////////////////////////////////////////
//�ر�
void FAR ImgClose(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	delete img;
}
//////////////////////////////////////////////////////////////////////////
//��ȡDPI
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
//���ûҶ�
void FAR ImgSetGray(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	img->GrayScale();

}
//////////////////////////////////////////////////////////////////////////
//����
void FAR ImgSave(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);

	LPCSTR szFileName=GetStr(parm,2);
	
	DWORD nType = CXIMAGE_FORMAT_BMP;
	PGetImgType(P2.ev_long,nType);
	
	if(nType==CXIMAGE_FORMAT_GIF || nType == CXIMAGE_FORMAT_TIF)
		img->DecreaseBpp(8,false);//Gif tif ��Ҫ�������
	if(nType == CXIMAGE_FORMAT_JPG)
		if(!img->IsGrayScale())img->IncreaseBpp(24);  //jpegҪ����24

	_RetLogical( img->Save(szFileName,nType) );
}
//////////////////////////////////////////////////////////////////////////
//�ü�
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
//����
void FAR ImgZoom(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	_RetLogical( img->Resample(P1.ev_long,P2.ev_long) );
}
//////////////////////////////////////////////////////////////////////////
//��ת
void FAR ImgRotate(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);

	_RetLogical( img->Rotate((float)P1.ev_real) );

}
//////////////////////////////////////////////////////////////////////////
//����jpg��Ʒ��
void FAR ImgSetJpegQuality(ParamBlk FAR *parm)
{
	CxImage * img;
	memcpy(&img,_HandToPtr(P0.ev_handle),4);
	
	img->SetJpegQuality((BYTE)P1.ev_long);
}
//////////////////////////////////////////////////////////////////////////
//���Ƶ�������
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
//ȡ������
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
		img->DecreaseBpp(8,false);//Gif tif ��Ҫ�������
	if(nType == CXIMAGE_FORMAT_JPG)
		if(!img->IsGrayScale())img->IncreaseBpp(24);  //jpegҪ����24
		

	BYTE * pBuff=NULL;
	long nSize=0;
	
	if(! img->Encode(pBuff,nSize,nType))
	{
		//����ʧ��!
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
//ȡ��Frame����
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
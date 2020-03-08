//////////////////////////////////////////////////////////////////////////
//   ѹ��zlib������

#pragma once

#include <windows.h>
#include <stdio.h>
//zlib define
#include "zlib.h"
#pragma comment(lib,"ziplib\\zlib.lib")
//minizip define
#include "zip.h"
#include "unzip.h"
#pragma comment(lib,"ziplib\\minizip.lib")


//////////////////////////////////////////////////////////////////////////
//��ȡ�ļ���zip��ʽ����
BOOL GetFileZipInfo(const char * pFileName,			//�ļ���
					zip_fileinfo * zipFileInfo);	//Ҫ���������

					
//////////////////////////////////////////////////////////////////////////
//��ȡ�ļ���crcֵ
DWORD GetFileCRC32(const char * cFileName);


//////////////////////////////////////////////////////////////////////////
//���ļ�ѹ��
BOOL zipAddFile(void * hZip,					//zip�ļ����
				const char * cFileName,		//Ҫѹ�����ļ���
				const char * cPassword=NULL,	//����
				 int nCompressLevel=-1);		//ѹ������

				 
//////////////////////////////////////////////////////////////////////////
BOOL zipAddFolder(void * hZip,					//zip�ļ����
				  const char* cFolderName,		//�ļ��� ���ܰ������һ��б��
				  const char* cPassword=NULL,	//����
				  int nCompressLevel=-1);		//ѹ������

				  
////////////////////////////////////////////////////////////////////////////
//�����ļ�����
void SetFileZipInfo(const char * cFileName,unz_file_info * unzInfo);


//////////////////////////////////////////////////////////////////////////
//�����ļ���
BOOL unzCreateFolder(char * pFolder);


//////////////////////////////////////////////////////////////////////////
//��ѹ��ǰ�ļ�
BOOL unzCreateCurrentFile(void * hZip,
						  char * cFileName,
						  const char * cPassword=NULL,
						  DWORD dwFileAttrib=FILE_ATTRIBUTE_NORMAL);


//////////////////////////////////////////////////////////////////////////
//��ѹ�ļ�
BOOL unzUnzipFile(const char * cZipFile, const char * cDestFolder, const char * cPassword=NULL);


//////////////////////////////////////////////////////////////////////////
//���һ���ļ��Ƿ������zip�ļ�
BOOL unzFileInZip(const char * cZipFile,const char * cCheckFile);

//////////////////////////////////////////////////////////////////////////
//��λһ���ļ�
BOOL unzSeekFile(void * hZip,char * cFileName);
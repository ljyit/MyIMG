//////////////////////////////////////////////////////////////////////////
//
//���ߣ�ľ��
//���ܣ�����minizip��zlib
//
//////////////////////////////////////////////////////////////////////////

#include "ziplib.h"
//////////////////////////////////////////////////////////////////////////
//																		//
//��ȡ�ļ���zip��ʽ����
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL GetFileZipInfo(const char * pFileName,		//�ļ���
					zip_fileinfo * zipFileInfo)		//Ҫ���������
{
	BOOL isOk=FALSE;
	FILETIME ftLocal;
	HANDLE hFind;
	WIN32_FIND_DATA  fInfo;
	
	hFind = FindFirstFile(pFileName,&fInfo);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FileTimeToLocalFileTime(&(fInfo.ftLastWriteTime),&ftLocal);
		FileTimeToDosDateTime(&ftLocal,
							((LPWORD) & zipFileInfo->dosDate)+1,
							((LPWORD) & zipFileInfo->dosDate)+0);
		zipFileInfo->external_fa =fInfo.dwFileAttributes;//�ļ�����
		FindClose(hFind);
		isOk=TRUE;
	}
	return isOk;
}
//////////////////////////////////////////////////////////////////////////
//																		//
//��ȡ�ļ���crcֵ
//																		//
//////////////////////////////////////////////////////////////////////////

DWORD GetFileCRC32(const char * cFileName)
{
	DWORD dwCrc=0,nLen=0;
	HANDLE hFile=CreateFile(cFileName,
							GENERIC_READ,FILE_SHARE_READ,
							0,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_READONLY,
							NULL);
	if(hFile==NULL){return dwCrc;}
	
	unsigned char buffer[1024];
	dwCrc = crc32(0L, Z_NULL, 0);
	while(ReadFile(hFile,buffer,sizeof(buffer),&nLen,NULL))
	{
		if(nLen==0)break;
		dwCrc = crc32(dwCrc,buffer, nLen);
	}
	CloseHandle(hFile);
	return dwCrc;
}
//////////////////////////////////////////////////////////////////////////
//																		//
//���ļ�ѹ��
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL zipAddFile(void * hZip,					//zip�ļ����
				 const char * cFileName,		//Ҫѹ�����ļ���
				 const char * cPassword,		//����
				 int nCompressLevel)			//ѹ������
{
	DWORD crcFile=0;
	zip_fileinfo zipFileInfo;
	BOOL isOk=FALSE;
	DWORD dwLen;
	char buffer[10240];
	ZeroMemory(&zipFileInfo,sizeof(zip_fileinfo));
	
	//ָ���ļ���ʧ�ܣ�����
	HANDLE hFile=CreateFile(cFileName,
							GENERIC_READ,FILE_SHARE_READ,
							0,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_READONLY,
							NULL);
	if(hFile==NULL){return FALSE;}
	
	
	//��ȡ�ռ��ʽ
	GetFileZipInfo(cFileName, &zipFileInfo);
	//������ʱ��Ҫ����CRCֵ
	if(cPassword!=NULL) crcFile=GetFileCRC32(cFileName);

	if(ZIP_OK!=zipOpenNewFileInZip3(hZip,cFileName,&zipFileInfo,
									NULL,0,NULL,0,NULL,			//��ע��Ϣ
									(nCompressLevel != 0) ? Z_DEFLATED : 0,
									nCompressLevel,0,
									-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
									cPassword,crcFile))
	{
		CloseHandle(hFile);
		return FALSE;
	}
	
	//��ʼѹ��
	isOk=TRUE;
	while(ReadFile(hFile,buffer,10240,&dwLen,NULL))
	{
		if(dwLen==0)break;
		if(0 > zipWriteInFileInZip (hZip,buffer,dwLen))
		{
			isOk=FALSE;
			break;
		}
	}
	CloseHandle(hFile);
	zipCloseFileInZip(hZip);
	
	return isOk;

}
//////////////////////////////////////////////////////////////////////////
//																		//
//���һ���ļ��е��ļ�
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL zipAddFolder(void * hZip,					//zip�ļ����
				  const char* cFolderName,		//�ļ��� ���ܰ������һ��б��
				  const char* cPassword,	//����
				  int nCompressLevel)		//ѹ������
{
	BOOL isOK=TRUE;
	WIN32_FIND_DATA info;	//�����ļ����ö���
	HANDLE hFind;			//�����ļ���������
	char cTmpFile[MAX_PATH+1];
	sprintf(cTmpFile,"%s\\*.*",cFolderName);
	
	hFind = FindFirstFile(cTmpFile, &info); 
	if(hFind==INVALID_HANDLE_VALUE) return FALSE;
	do 
	{
		if (info.cFileName[0] == '.') continue; // ����������Ŀ¼ 
		sprintf(cTmpFile,"%s\\%s",cFolderName,info.cFileName);
		if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(!zipAddFolder(hZip,cTmpFile,cPassword,nCompressLevel))
			{
				isOK=FALSE;
			}
		}
		//ѹ���ļ�
		else
		{
			if(!zipAddFile(hZip,cTmpFile,cPassword,nCompressLevel))
			{
				isOK=FALSE;
			}
		}
	} while(FindNextFile(hFind, &info));
	FindClose(hFind);
	
	//���ļ��мӽ�ȥ
	hFind = FindFirstFile(cFolderName, &info); 
	FindClose(hFind);
	if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		zip_fileinfo zipFileInfo;
		ZeroMemory(&zipFileInfo,sizeof(zip_fileinfo));
		//��ȡ�ռ��ʽ����Ϣ
		GetFileZipInfo(cFolderName, &zipFileInfo);
		if(ZIP_OK!=zipOpenNewFileInZip3(hZip,cFolderName,&zipFileInfo,
										NULL,0,NULL,0,NULL,			//��ע��Ϣ
										(nCompressLevel != 0) ? Z_DEFLATED : 0,
										nCompressLevel,0,
										-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
										cPassword,0))	//�ļ��е�����
		{
			return FALSE;
		}
		zipCloseFileInZip(hZip);
	}
	return isOK;
}
//////////////////////////////////////////////////////////////////////////
//																		//
//�����ļ�����
//																		//
//////////////////////////////////////////////////////////////////////////

void SetFileZipInfo(const char * cFileName,unz_file_info * unzInfo)
{
	HANDLE hFile;
	FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

	hFile = CreateFile(cFileName ,GENERIC_READ | GENERIC_WRITE,
		0,NULL,OPEN_EXISTING,0,NULL);
	GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
	DosDateTimeToFileTime(  (WORD)( unzInfo->dosDate >>16),
							(WORD) unzInfo->dosDate ,
							&ftLocal);
	LocalFileTimeToFileTime(&ftLocal,&ftm);
	SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
	CloseHandle(hFile);
}
//////////////////////////////////////////////////////////////////////////
//																		//
//�����ļ���
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL unzCreateFolder(char * pFolder)
{
	if (!pFolder || !lstrlen(pFolder))	return FALSE;
	
	DWORD dwAttrib;
	BOOL isOk=TRUE;
	dwAttrib = GetFileAttributes(pFolder);
	// ����ļ����Ƿ����
	if (dwAttrib != 0xffffffff)
		return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
	
	// ���������
	char* pParent = strdup(pFolder);
	char* p = strrchr(pParent, '\\');
	if(p)
	{
		//����ϼ����ڵĻ��������ϼ�·��
		*p = '\0';
		//�����ϲ�
		isOk=unzCreateFolder(pParent);
	}
	free(pParent);
	if (!isOk) return FALSE;
	//�����ļ���
	if (!CreateDirectory(pFolder,NULL))	return FALSE;
	return TRUE;
}
//��ʽ��·����ɾ���ظ���
void unzFormatPath(char * cPathBuffer)
{
	char * p=cPathBuffer;
	while (*p !='\0' )
	{
		if(*p=='/') *p ='\\';
		p++;
	}

	p=cPathBuffer;
	char * pNew=cPathBuffer;

	while (*p !='\0' )
	{
		*pNew=*p;
		if(*pNew=='\\' && *(p+1)=='\\')p++;
		pNew++;p++;
	}
	*(pNew)='\0';
}
//////////////////////////////////////////////////////////////////////////
//																		//
//��ѹ��ǰ�ļ�
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL unzCreateCurrentFile(void * hZip,
						   char * cFileName,
						   const char * cPassword,
						   DWORD dwFileAttrib)
{
	char* pFile =strrchr(cFileName, '\\');
	if(pFile)
	{
		//�������·��,�ȼ��·���Ƿ����
		*pFile='\0';
		if(!unzCreateFolder(cFileName)) return FALSE;		//�޷������ļ���
		*pFile='\\';
	}
	if (unzOpenCurrentFilePassword(hZip,cPassword)!= UNZ_OK)	return FALSE;	//�޷��򿪵�ǰ�ļ�
	
	HANDLE hFile=CreateFile(cFileName, 
							GENERIC_WRITE,
							0,
							NULL,
							CREATE_ALWAYS,
							dwFileAttrib,
							NULL);
	if(hFile==NULL) return FALSE;							//�޷�д���ļ�
	
	DWORD	dwReadLen=0,
			dwBytesWritten=0;
	char*	pBuffer= new char[1024*1024];
	BOOL isOk=TRUE;
	while(TRUE)
	{
		dwReadLen = unzReadCurrentFile(hZip, pBuffer, 1024*1024);
		if (dwReadLen <= 0) break;
		if (!WriteFile(hFile, pBuffer, dwReadLen, &dwBytesWritten, NULL) ||
			dwBytesWritten != (DWORD)dwReadLen)
		{
			isOk=FALSE;
			break;
		}
	}
	//����
	delete pBuffer;
	CloseHandle(hFile);
	unzCloseCurrentFile(hZip);

	return isOk;
}
//////////////////////////////////////////////////////////////////////////
//																		//
//��ѹ�ļ�																//
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL unzUnzipFile(const char * cZipFile, const char * cDestFolder, const char * cPassword)
{
	unzFile hZip=NULL;
	hZip=unzOpen(cZipFile);
	if(hZip==NULL)return FALSE;//�޷���zip�ļ�
	
	//��ȡ�ļ���Ϣ
	unz_global_info unzGlobal;
	if(UNZ_OK!=unzGetGlobalInfo(hZip,&unzGlobal))
	{
		unzClose(hZip);
		return FALSE;
	}

	unz_file_info unzFileInfo;
	char cFileName[MAX_PATH];
	char buffer[MAX_PATH*2];
	
	BOOL isOK=TRUE;
	for( DWORD x=0; x < unzGlobal.number_entry; x++)
	{
		unzGetCurrentFileInfo(hZip,&unzFileInfo,
							cFileName,sizeof(cFileName),
							NULL,0,NULL,0);
		//�ϲ�
		sprintf(buffer,"%s\\%s",cDestFolder,cFileName);
		unzFormatPath(buffer);//��ʽ��·��

		if((unzFileInfo.external_fa & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			isOK=unzCreateFolder(buffer);//������ļ��У��򴴽��ļ���
		}
		else
		{
			isOK=unzCreateCurrentFile(hZip,buffer,cPassword);//���򴴽��ļ�
			SetFileZipInfo(buffer,&unzFileInfo);//�����ļ�����
		}
		if(!isOK) break;
		unzGoToNextFile(hZip);
	}

	unzClose(hZip);
	return isOK;
}
//////////////////////////////////////////////////////////////////////////
//																		//
//���һ���ļ��Ƿ������zip�ļ�											//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL unzFileInZip(const char * cZipFile,const char * cCheckFile)
{
	void * hZip;
	char pFile[MAX_PATH];
	strcpy(pFile,cCheckFile);

	BOOL isOk=FALSE;	//����ֵ
	hZip=unzOpen(cZipFile);
	if(hZip)
	{
		isOk=!unzLocateFile(hZip,pFile,0);
		if(!isOk)//���û���ҵ���ת��б���ټ��
		{
			for(int x=0; pFile[x]!='\0'; x++)
			{
				if(pFile[x]=='\\')pFile[x]='/';
			}
			//����zip��ʹ�÷�б��
			isOk=!unzLocateFile(hZip,pFile,0);
		}
		unzClose(hZip);
	}
	return isOk;
}
//////////////////////////////////////////////////////////////////////////
//
//��λһ���ļ�
//
//////////////////////////////////////////////////////////////////////////
BOOL unzSeekFile(void * hZip,char * cFileName)
{
	BOOL isOk= !unzLocateFile(hZip,cFileName,2);
	if(!isOk)
	{
		//���û���ҵ���ת��б���ټ��
		char * pStr=new char(strlen(cFileName));
		for(int x=0; pStr[x]!='\0'; x++)
		{
			if(pStr[x]=='\\')pStr[x]='/';
		}
		//����zip��ʹ�÷�б��
		isOk=!unzLocateFile(hZip,pStr,0);
		delete pStr;
	}
	return isOk;
}
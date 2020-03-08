//////////////////////////////////////////////////////////////////////////
//
//作者：木瓜
//功能：整合minizip和zlib
//
//////////////////////////////////////////////////////////////////////////

#include "ziplib.h"
//////////////////////////////////////////////////////////////////////////
//																		//
//读取文件的zip格式日期
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL GetFileZipInfo(const char * pFileName,		//文件名
					zip_fileinfo * zipFileInfo)		//要处理的类型
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
		zipFileInfo->external_fa =fInfo.dwFileAttributes;//文件属性
		FindClose(hFind);
		isOk=TRUE;
	}
	return isOk;
}
//////////////////////////////////////////////////////////////////////////
//																		//
//读取文件的crc值
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
//将文件压缩
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL zipAddFile(void * hZip,					//zip文件句柄
				 const char * cFileName,		//要压缩的文件名
				 const char * cPassword,		//密码
				 int nCompressLevel)			//压缩级别
{
	DWORD crcFile=0;
	zip_fileinfo zipFileInfo;
	BOOL isOk=FALSE;
	DWORD dwLen;
	char buffer[10240];
	ZeroMemory(&zipFileInfo,sizeof(zip_fileinfo));
	
	//指定文件打开失败，返回
	HANDLE hFile=CreateFile(cFileName,
							GENERIC_READ,FILE_SHARE_READ,
							0,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_READONLY,
							NULL);
	if(hFile==NULL){return FALSE;}
	
	
	//获取日间格式
	GetFileZipInfo(cFileName, &zipFileInfo);
	//带密码时，要计算CRC值
	if(cPassword!=NULL) crcFile=GetFileCRC32(cFileName);

	if(ZIP_OK!=zipOpenNewFileInZip3(hZip,cFileName,&zipFileInfo,
									NULL,0,NULL,0,NULL,			//备注信息
									(nCompressLevel != 0) ? Z_DEFLATED : 0,
									nCompressLevel,0,
									-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
									cPassword,crcFile))
	{
		CloseHandle(hFile);
		return FALSE;
	}
	
	//开始压缩
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
//添加一个文件夹到文件
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL zipAddFolder(void * hZip,					//zip文件句柄
				  const char* cFolderName,		//文件夹 不能包含最后一个斜杠
				  const char* cPassword,	//密码
				  int nCompressLevel)		//压缩级别
{
	BOOL isOK=TRUE;
	WIN32_FIND_DATA info;	//查找文件所用对像
	HANDLE hFind;			//查找文件所用名柄
	char cTmpFile[MAX_PATH+1];
	sprintf(cTmpFile,"%s\\*.*",cFolderName);
	
	hFind = FindFirstFile(cTmpFile, &info); 
	if(hFind==INVALID_HANDLE_VALUE) return FALSE;
	do 
	{
		if (info.cFileName[0] == '.') continue; // 过滤这两个目录 
		sprintf(cTmpFile,"%s\\%s",cFolderName,info.cFileName);
		if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(!zipAddFolder(hZip,cTmpFile,cPassword,nCompressLevel))
			{
				isOK=FALSE;
			}
		}
		//压缩文件
		else
		{
			if(!zipAddFile(hZip,cTmpFile,cPassword,nCompressLevel))
			{
				isOK=FALSE;
			}
		}
	} while(FindNextFile(hFind, &info));
	FindClose(hFind);
	
	//把文件夹加进去
	hFind = FindFirstFile(cFolderName, &info); 
	FindClose(hFind);
	if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		zip_fileinfo zipFileInfo;
		ZeroMemory(&zipFileInfo,sizeof(zip_fileinfo));
		//获取日间格式等信息
		GetFileZipInfo(cFolderName, &zipFileInfo);
		if(ZIP_OK!=zipOpenNewFileInZip3(hZip,cFolderName,&zipFileInfo,
										NULL,0,NULL,0,NULL,			//备注信息
										(nCompressLevel != 0) ? Z_DEFLATED : 0,
										nCompressLevel,0,
										-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
										cPassword,0))	//文件夹的密码
		{
			return FALSE;
		}
		zipCloseFileInZip(hZip);
	}
	return isOK;
}
//////////////////////////////////////////////////////////////////////////
//																		//
//设置文件日期
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
//建立文件夹
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL unzCreateFolder(char * pFolder)
{
	if (!pFolder || !lstrlen(pFolder))	return FALSE;
	
	DWORD dwAttrib;
	BOOL isOk=TRUE;
	dwAttrib = GetFileAttributes(pFolder);
	// 检查文件夹是否存在
	if (dwAttrib != 0xffffffff)
		return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
	
	// 如果不存在
	char* pParent = strdup(pFolder);
	char* p = strrchr(pParent, '\\');
	if(p)
	{
		//如果上级存在的话，创建上级路径
		*p = '\0';
		//创建上层
		isOk=unzCreateFolder(pParent);
	}
	free(pParent);
	if (!isOk) return FALSE;
	//创建文件夹
	if (!CreateDirectory(pFolder,NULL))	return FALSE;
	return TRUE;
}
//格式化路径：删除重复的
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
//解压当前文件
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
		//如果包含路径,先检查路径是否存在
		*pFile='\0';
		if(!unzCreateFolder(cFileName)) return FALSE;		//无法创建文件夹
		*pFile='\\';
	}
	if (unzOpenCurrentFilePassword(hZip,cPassword)!= UNZ_OK)	return FALSE;	//无法打开当前文件
	
	HANDLE hFile=CreateFile(cFileName, 
							GENERIC_WRITE,
							0,
							NULL,
							CREATE_ALWAYS,
							dwFileAttrib,
							NULL);
	if(hFile==NULL) return FALSE;							//无法写入文件
	
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
	//清理
	delete pBuffer;
	CloseHandle(hFile);
	unzCloseCurrentFile(hZip);

	return isOk;
}
//////////////////////////////////////////////////////////////////////////
//																		//
//解压文件																//
//																		//
//////////////////////////////////////////////////////////////////////////

BOOL unzUnzipFile(const char * cZipFile, const char * cDestFolder, const char * cPassword)
{
	unzFile hZip=NULL;
	hZip=unzOpen(cZipFile);
	if(hZip==NULL)return FALSE;//无法打开zip文件
	
	//获取文件信息
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
		//合并
		sprintf(buffer,"%s\\%s",cDestFolder,cFileName);
		unzFormatPath(buffer);//格式化路径

		if((unzFileInfo.external_fa & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			isOK=unzCreateFolder(buffer);//如果是文件夹，则创建文件夹
		}
		else
		{
			isOK=unzCreateCurrentFile(hZip,buffer,cPassword);//否则创建文件
			SetFileZipInfo(buffer,&unzFileInfo);//设置文件属性
		}
		if(!isOK) break;
		unzGoToNextFile(hZip);
	}

	unzClose(hZip);
	return isOK;
}
//////////////////////////////////////////////////////////////////////////
//																		//
//检查一下文件是否存在于zip文件											//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL unzFileInZip(const char * cZipFile,const char * cCheckFile)
{
	void * hZip;
	char pFile[MAX_PATH];
	strcpy(pFile,cCheckFile);

	BOOL isOk=FALSE;	//返回值
	hZip=unzOpen(cZipFile);
	if(hZip)
	{
		isOk=!unzLocateFile(hZip,pFile,0);
		if(!isOk)//如果没有找到，转换斜杠再检查
		{
			for(int x=0; pFile[x]!='\0'; x++)
			{
				if(pFile[x]=='\\')pFile[x]='/';
			}
			//部分zip是使用反斜杠
			isOk=!unzLocateFile(hZip,pFile,0);
		}
		unzClose(hZip);
	}
	return isOk;
}
//////////////////////////////////////////////////////////////////////////
//
//定位一个文件
//
//////////////////////////////////////////////////////////////////////////
BOOL unzSeekFile(void * hZip,char * cFileName)
{
	BOOL isOk= !unzLocateFile(hZip,cFileName,2);
	if(!isOk)
	{
		//如果没有找到，转换斜杠再检查
		char * pStr=new char(strlen(cFileName));
		for(int x=0; pStr[x]!='\0'; x++)
		{
			if(pStr[x]=='\\')pStr[x]='/';
		}
		//部分zip是使用反斜杠
		isOk=!unzLocateFile(hZip,pStr,0);
		delete pStr;
	}
	return isOk;
}
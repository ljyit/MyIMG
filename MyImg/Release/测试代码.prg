* MyImg.fll
* ��Ҫ���ܣ�ץ�����ü������š���ת����ʽת�������ɵ����������Ƶ�������
* ���ߣ�ľ�ϣ�
* 2007-07-14
* ����˳��ImgOpen()  -->  �������� -->ImgClose() ����������ʱ��ʹ��ImgGetLastError��ȡ����

#define _FROM_SCREEN		1
#define _FROM_CLIPBORD		2

#define _IMG_BMP			1
#define _IMG_GIF			2
#define _IMG_JPG			3
#define _IMG_PNG			4
#define _IMG_TIF			5


Clear 
Set Library To myImg

* 1. ��ͼ����Ļ��������
 hImg = ImgOpen(_FROM_SCREEN)		&&����Ļ
* hImg = ImgOpen(_FROM_CLIPBORD)	&&�򿪼��а�
* hImg = ImgOpen( cFileName )		&&��ͼ��
If hImg==""
	MessageBox("��ͼ��ʧ�ܣ�")
	Return 
EndIf 
ImgSave(hImg,"1ԭʼͼ.bmp",_IMG_BMP)


* 2. ȡ��ͼ���С
?"��ȣ�",ImgGetWidth(hImg)
?"�߶ȣ�",ImgGetHeight(hImg)


* 3. �ü� ImgCrop(��������������ϣ�����)
If not ImgCrop(hImg,10,10,500,300) &&(��(10,10)���꿪ʼ������һ����500����300��ͼ�� 
	MessageBox(ImgGetLastError(hImg),"�ü�ʧ�ܣ�")
EndIf 
ImgSave(hImg,"3�ü���.bmp",1)

* 4. ���š�ImgZoom(������¿�ȣ��¸߶�)

If not ImgZoom(hImg,400,400)  &&���ŵ�400��400��ͼ��
	MessageBox(ImgGetLastError(hImg),"����ʧ�ܣ�")
EndIf 
ImgSave(hImg,"4���ź�.bmp",1)

* 5.��ת ImgRotate(���,�Ƕ�)
If not imgRotate(hImg,90)
	MessageBox(ImgGetLastError(hImg),"��תʧ�ܣ�")
EndIf 
ImgSave(hImg,"5��ת��.bmp",1)

* 6.��ʽת��
If not ImgSave(hImg,"6��ʽת��.png",_IMG_PNG)  &&�ڶ�������Ϊ�ļ�����������ΪͼƬ����
	MessageBox(ImgGetLastError(hImg),"��ʽת��ʧ�ܣ�")
EndIf 

* 7. ���ҪתJPG��������jpg��Ʒ�� 1-100
ImgSetJpegQuality(hImg,70)	&&����jpg��Ʒ��
If not ImgSave(hImg,"7��ʽת��.jpg",_IMG_JPG)
	MessageBox(ImgGetLastError(hImg),"���Ϊʧ�ܣ�")
EndIf 

* 8. ���Ƶ�������
If ImgCopyToClipbord(hImg) 
	MessageBox("�Ѹ��Ƶ�������,���Դ򿪻�ͼ����ճ��")
Else
	MessageBox(ImgGetLastError(hImg),"���Ƶ�������ʧ��!") 
EndIf 

* 9.ֱ��ȡ��ͼ�����ݵ����������ص���һ����������ֱ�Ӵ������ݿ⣩
vImgSrc = ImgGetPtr(hImg,_IMG_GIF)  &&��ȡGif��ʽ��ͼ������
StrToFile(vImgSrc,"9ʹ���ڴ����.gif")

* 10.�ر�ͼ��
ImgClose(hImg)

* 11.���ڴ�����д��� ( vImgSrc��ǰ�����ɵ�,Ҳ������FileToStrȡ��
hImg2=ImgOpen(vImgSrc,_IMG_GIF)
If hImg2==""
	MessageBox(ImgGetLastError(hImg),"�ӱ�����ͼ��ʧ��!")
	Return 
EndIf 

*  12.��Vfp9��PictureVal���ʹ��,ImgGetPtr()�ķ���ֵ��ֱ�Ӹ���PictureVal
If Val(_vfp.Version) >=9  
	Local oForm as Form 
	oForm=CreateObject("form")
	oForm.AddObject("image","image")
	With oForm.image as Image 
		.Visible=.t.
		.PictureVal = ImgGetPtr(hImg2,_IMG_TIF)  && �Ѹ�ʽתΪtif������image�ؼ���PictureVal
	EndWith 
	oForm.Show(1)
EndIf 


*13.��ȡDPI
?"DPI��",ImgGetXDpi(hImg),ImgGetYDpi(hImg)

ImgClose(hImg2)


* 14 �Ҷ�

hImg=ImgOpen(_FROM_SCREEN)
ImgSetGray(hImg)
ImgSave(hImg,"�Ҷȴ�������Ļ.gif",_IMG_TIF)
ImgClose(hImg)


* 15 ֡������ҳtif�Ͷ���gif������ô������
cFile="fox.gif"  &&����һ������

hImg=ImgOpen(cFile)
If hImg==""
	MessageBox("�޷���ͼ��")
	Return 
EndIf 
*ȡ��֡��
nFrames=ImgGetFramesCount(hImg)
?"���ļ�����֡����",nFrames
ImgClose(hImg) &&�ر�

*ȡ��ÿһ֡
For x=1 to nFrames
	hImg=ImgOpen(cFile,0,x)  &&�򿪵� x ֡���ڶ����������Ժ���
	If hImg==""
		MessageBox("�޷��򿪵�"+Transform(x)+"֡��")
		Loop 
	EndIf 
	?"�������ɵ�",x,"֡����"
	ImgSave(hImg,"Frame"+Transform(x)+".gif",_IMG_GIF)
	ImgClose(hImg)  &&ÿһ֡��һ�������ľ������Ҫ�ر�
EndFor 



Set Library To 
# MyIMG
a image lib based on CXImage 

这是一个VC6开发的用于VFP的FLL项目，主要功能：抓屏、裁剪、缩放、旋转、格式转换、生成到变量、复制到剪贴板
这也是一个开发FLL扩展库的模板项目，可以用于你自己的函数库开发。


* MyImg.fll
* 主要功能：抓屏、裁剪、缩放、旋转、格式转换、生成到变量、复制到剪贴板
* 作者：木瓜：
* 2007-07-14
* 调用顺序：ImgOpen()  -->  其它函数 -->ImgClose() ，发生错误时，使用ImgGetLastError读取错误

#define _FROM_SCREEN		1
#define _FROM_CLIPBORD		2

#define _IMG_BMP			1
#define _IMG_GIF			2
#define _IMG_JPG			3
#define _IMG_PNG			4
#define _IMG_TIF			5


Clear 
Set Library To myImg

* 1. 打开图像　屏幕、剪贴板
 hImg = ImgOpen(_FROM_SCREEN)		&&打开屏幕
* hImg = ImgOpen(_FROM_CLIPBORD)	&&打开剪切板
* hImg = ImgOpen( cFileName )		&&打开图像
If hImg==""
	MessageBox("打开图像失败！")
	Return 
EndIf 
ImgSave(hImg,"1原始图.bmp",_IMG_BMP)


* 2. 取得图像大小
?"宽度：",ImgGetWidth(hImg)
?"高度：",ImgGetHeight(hImg)


* 3. 裁剪 ImgCrop(参数：句柄，左，上，宽，高)
If not ImgCrop(hImg,10,10,500,300) &&(从(10,10)坐标开始，生成一个宽500，高300的图像 
	MessageBox(ImgGetLastError(hImg),"裁剪失败！")
EndIf 
ImgSave(hImg,"3裁剪后.bmp",1)

* 4. 缩放　ImgZoom(句柄，新宽度，新高度)

If not ImgZoom(hImg,400,400)  &&缩放到400×400的图像
	MessageBox(ImgGetLastError(hImg),"缩放失败！")
EndIf 
ImgSave(hImg,"4缩放后.bmp",1)

* 5.旋转 ImgRotate(句柄,角度)
If not imgRotate(hImg,90)
	MessageBox(ImgGetLastError(hImg),"旋转失败！")
EndIf 
ImgSave(hImg,"5旋转后.bmp",1)

* 6.格式转换
If not ImgSave(hImg,"6格式转换.png",_IMG_PNG)  &&第二个参数为文件名，第三个为图片类型
	MessageBox(ImgGetLastError(hImg),"格式转换失败！")
EndIf 

* 7. 如果要转JPG，可以设jpg的品质 1-100
ImgSetJpegQuality(hImg,70)	&&设置jpg的品质
If not ImgSave(hImg,"7格式转换.jpg",_IMG_JPG)
	MessageBox(ImgGetLastError(hImg),"另存为失败！")
EndIf 

* 8. 复制到剪贴版
If ImgCopyToClipbord(hImg) 
	MessageBox("已复制到剪贴板,可以打开画图程序粘贴")
Else
	MessageBox(ImgGetLastError(hImg),"复制到剪贴板失败!") 
EndIf 

* 9.直接取得图像内容到变量（返回的是一个变量，可直接存入数据库）
vImgSrc = ImgGetPtr(hImg,_IMG_GIF)  &&获取Gif格式的图像内容
StrToFile(vImgSrc,"9使用内存变量.gif")

* 10.关闭图像
ImgClose(hImg)

* 11.从内存变量中创建 ( vImgSrc是前面生成的,也可以用FileToStr取得
hImg2=ImgOpen(vImgSrc,_IMG_GIF)
If hImg2==""
	MessageBox(ImgGetLastError(hImg),"从变量打开图像失败!")
	Return 
EndIf 

*  12.与Vfp9的PictureVal结合使用,ImgGetPtr()的返回值可直接赋给PictureVal
If Val(_vfp.Version) >=9  
	Local oForm as Form 
	oForm=CreateObject("form")
	oForm.AddObject("image","image")
	With oForm.image as Image 
		.Visible=.t.
		.PictureVal = ImgGetPtr(hImg2,_IMG_TIF)  && 把格式转为tif，赋给image控件的PictureVal
	EndWith 
	oForm.Show(1)
EndIf 


*13.获取DPI
?"DPI：",ImgGetXDpi(hImg),ImgGetYDpi(hImg)

ImgClose(hImg2)


* 14 灰度

hImg=ImgOpen(_FROM_SCREEN)
ImgSetGray(hImg)
ImgSave(hImg,"灰度处理后的屏幕.gif",_IMG_TIF)
ImgClose(hImg)


* 15 帧操作多页tif和动画gif均可这么操作：
cFile="fox.gif"  &&这是一个动画

hImg=ImgOpen(cFile)
If hImg==""
	MessageBox("无法打开图像！")
	Return 
EndIf 
*取得帧数
nFrames=ImgGetFramesCount(hImg)
?"此文件共有帧数：",nFrames
ImgClose(hImg) &&关闭

*取得每一帧
For x=1 to nFrames
	hImg=ImgOpen(cFile,0,x)  &&打开第 x 帧，第二个参数可以忽略
	If hImg==""
		MessageBox("无法打开第"+Transform(x)+"帧！")
		Loop 
	EndIf 
	?"正在生成第",x,"帧……"
	ImgSave(hImg,"Frame"+Transform(x)+".gif",_IMG_GIF)
	ImgClose(hImg)  &&每一帧是一个单独的句柄，都要关闭
EndFor 



Set Library To 

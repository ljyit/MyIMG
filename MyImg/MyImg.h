#define _FROM_SCREEN		1
#define _FROM_CLIPBORD		2


#define _IMG_BMP			1
#define _IMG_GIF			2
#define _IMG_JPG			3
#define _IMG_PNG			4
#define _IMG_TIF			5


char FAR * GetStr(ParamBlk FAR *parm,int x)
{
	if (!_SetHandSize(parm->p[x-1].val.ev_handle,parm->p[x-1].val.ev_length+1))_Error(182); // "ÄÚ´æ²»×ã"
	((char *)_HandToPtr(parm->p[x-1].val.ev_handle))[parm->p[x-1].val.ev_length] = '\0';
	return (char FAR *) _HandToPtr(parm->p[x-1].val.ev_handle);
}
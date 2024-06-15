#ifndef	UNICODE
#define	UNICODE
#endif	/* UNICODE */
#ifndef	_UNICODE
#define	_UNICODE
#endif	/* _UNICODE */
#define	STRICT
#include <windows.h>Å@
#include <commctrl.h> 
#include	<commdlg.h>
#include <wingdi.h>
#include "resource.h" 
#include <Winbase.h>
#include "jznovel.h"
 
static int	tap_x,tap_y;

int	pic_color[2][3]={{128,128,128},{255,255,255}};
int bg_color[2][3]={{255,255,255},{0,0,0}};
int	text_color[2][3]={{0,0,0},{255,255,255}};

BYTE	bw_pic_color[4]={2,2,3,3};
BYTE	bw_text_color[2]={0,3};
BYTE	bw_bg_color[2]={3,0};

int		rotate_offset=480;

void	get_pixel24(BYTE *buf,int x,int y,int *r,int *g,int *b)
{
	if (x<0 || winWidth<=x || y<0 || winHeight<=y) return;

	BYTE *bb=&buf[((winHeight-1-y)*winWidth+x)*3];
	*b=*bb++;
	*g=*bb++;
	*r=*bb++;
}

void set_pixel24(BYTE *buf,int x,int y,int r,int g,int b)
{
	BYTE *bb;
	if (x<0 || winWidth<=x || y<0 || winHeight<=y) return;
	bb=&buf[((winHeight-1-y)*winWidth+x)*3];
	*bb++=b;
	*bb++=g;
	*bb++=r;
}

void set_pixel24_r(BYTE *buf,int x,int y,int r,int g,int b)
{
	BYTE *bb;
	int xx,yy;
	if (x<0 || winWidth<=x || y<0 || winHeight<=y) return;
	if (rotate_flag) {
		xx=rotate_offset-y;
		yy=x;
	} else {
		xx=x;
		yy=y;
	}
	if (xx<0 || winWidth_dev<=xx || yy<0 || winHeight_dev<=yy) return;
	bb=&buf[((winHeight_dev-1-yy)*winWidth_dev+xx)*3];
	*bb++=b;
	*bb++=g;
	*bb++=r;
}

BYTE get_pixel(BYTE *buf,int x,int y)
{
	static BYTE shift[4]={6,4,2,0};
	return (buf[((winHeight-1-y)*winWidth+x)/4]>>shift[x%4])&3; 
}


void set_pixel(BYTE *buf,int x,int y,BYTE c)
{
	static BYTE and_mask[4]={0x3f,0xcf,0xf3,0xfc};
	static BYTE shift[4]={6,4,2,0};
	if (x<0 || winWidth<=x || y<0 || winHeight<=y) return;
	int k=((winHeight-1-y)*winWidth+x)/4;
	buf[k]&=and_mask[x%4];
	buf[k]|=(c&3)<<shift[x%4];
}

void set_pixel_r(BYTE *buf,int x,int y,BYTE c)
{
	static BYTE and_mask[4]={0x3f,0xcf,0xf3,0xfc};
	static BYTE shift[4]={6,4,2,0};
	int		xx,yy;
	if (x<0 || winWidth<=x || y<0 || winHeight<=y) return;
	if (rotate_flag) {
		xx=rotate_offset-y;
		yy=x;
	} else {
		xx=x;
		yy=y;
	}
	if (xx<0 || winWidth_dev<=xx || yy<0 || winHeight_dev<=yy) return;
	int k=((winHeight_dev-1-yy)*winWidth_dev+xx)/4;
	buf[k]&=and_mask[xx%4];
	buf[k]|=(c&3)<<shift[xx%4];
}

void fill(BYTE *buf,BYTE c)
{
/*
	int i,j;
	for (j=0;j<winHeight;j++)
		for (i=0;i<winWidth;i++)
			set_pixel(buf,i,j,c);
*/
	int		col=c*0x55555555;
	int		i;
	int		*b=(int *)buf;
	for (i=0;i<winWidth*winHeight/16;i++)
		*b++=col;
}
void fill_r(BYTE *buf,BYTE c)
{
	int i,j;
	for (j=0;j<winHeight;j++)
		for (i=0;i<winWidth;i++)
			set_pixel_r(buf,i,j,c);
}

void fill24(BYTE *buf,int r,int g,int b)
{
/*
	int i,j;
	for (j=0;j<winHeight;j++)
		for (i=0;i<winWidth;i++)
			set_pixel24(buf,i,j,r,g,b);
/*/
	int		i; 
	BYTE	*buf2=buf;
	for (i=winWidth*winHeight;i>0;i--) {
		*buf2++=b;
		*buf2++=g;
		*buf2++=r;
	}
}
void fill24_r(BYTE *buf,int r,int g,int b)
{
	int i,j;
	for (j=0;j<winHeight;j++)
		for (i=0;i<winWidth;i++)
			set_pixel24_r(buf,i,j,r,g,b);
}

void put_pic()
{
	int i,j;

	if (bitspixel==2) {
		int *b1,*b2;
		b1=(int *)picBuf;
		b2=(int *)baseBuf;

		if (!rotate_flag) {
			for (i=winWidth*winHeight/4/4;i>0;i--)
				*b2++=*b1++;
		} else {
			for (j=0;j<winHeight;j++) {
				for (i=0;i<winWidth;i++) {
					BYTE c;
					c=get_pixel(picBuf,i,j);
					set_pixel_r(baseBuf,i,j,c);
				}
			}
		}
	} else {
		if (!rotate_flag) {
			int *b1,*b2;
			b1=(int *)picBuf;
			b2=(int *)baseBuf;
			for (i=winWidth*winHeight*3/4;i>0;i--)
				*b2++=*b1++;
		} else {
			for (j=0;j<winHeight;j++) {
				for (i=0;i<winWidth;i++) {
					int r,g,b;
					get_pixel24(picBuf,i,j,&r,&g,&b);
					set_pixel24_r(baseBuf,i,j,r,g,b);
				} 
			}
		}
	}
}				 

BYTE mono_color(int r,int g,int b)
{
	int c;
	BYTE cc;
	c=(r+b+g)/3;
	cc=0;
	if (c>64) cc=1;
	if (c>128) cc=2;
	if (c>192) cc=3; 
	return cc;
}
BYTE mono_color255(int r,int g,int b)
{
	int c;
	c=(r+b+g)/3; 
	return (BYTE)c;
}

void conv_pic()  
{
	int i,j;

	if (key_kaisou_flag==TRUE) { 
		if (bitspixel==2) {
			if (!rotate_flag)	fill(baseBuf,bw_bg_color[1]);
			else				fill_r(baseBuf,bw_bg_color[1]);

		} else {
			if (!rotate_flag)	fill24(baseBuf,bg_color[1][0],bg_color[1][1],bg_color[1][2]);
			else				fill24_r(baseBuf,bg_color[1][0],bg_color[1][1],bg_color[1][2]);
		}
		return;
	}
	if (action_mode==1) {
		put_pic();
		return;
	}
	if (bitspixel==2) {
		BYTE	c;
		for (j=0;j<winHeight;j++) 
			for (i=0;i<winWidth;i++) {
				c=get_pixel(picBuf,i,j);
				c=bw_pic_color[c];
				if (!rotate_flag)	set_pixel(baseBuf,i,j,c);
				else				set_pixel_r(baseBuf,i,j,c);
			}
	} else {
		for (j=0;j<winHeight;j++) 
			for (i=0;i<winWidth;i++) {
				int	r,g,b;
				get_pixel24(picBuf,i,j,&r,&g,&b);
				r=pic_color[0][0]+(pic_color[1][0]-pic_color[0][0])*r/256;
				g=pic_color[0][1]+(pic_color[1][1]-pic_color[0][1])*g/256;
				b=pic_color[0][2]+(pic_color[1][2]-pic_color[0][2])*b/256;
				if (r<0) r=0;
				if (g<0) g=0;
				if (b<0) b=0;
				if (255<r) r=255;
				if (255<g) g=255;
				if (255<b) b=255;
				if (!rotate_flag)	set_pixel24(baseBuf,i,j,r,g,b);
				else				set_pixel24_r(baseBuf,i,j,r,g,b);
			}
	}
}

void overlap_text(HWND hWnd,int sx,int sy,int ex,int ey)
{
	int		i,j,k;
	RECT	rc;

	k=(key_kaisou_flag==FALSE) ? 0:1;
	if (bitspixel==2) {
		int m=bw_text_color[k];
		for (j=sy;j<ey;j++) 
			for (i=sx;i<ex;i++) 
				if (get_pixel(textBuf,i,j)==0) {
					if (!rotate_flag)	set_pixel(baseBuf,i,j,m);
					else				set_pixel_r(baseBuf,i,j,m);
				}
	} else {
		for (j=sy;j<ey;j++) 
			for (i=sx;i<ex;i++) 
				if (get_pixel(textBuf,i,j)==0) {
					if (!rotate_flag)	set_pixel24(baseBuf,i,j,text_color[k][0],text_color[k][1],text_color[k][2]);
					else				set_pixel24_r(baseBuf,i,j,text_color[k][0],text_color[k][1],text_color[k][2]);
				}
	}
				
	rc.left=sx;
	rc.top=sy;
	rc.right=ex;
	rc.bottom=ey+cbh;
	InvalidateRect(hWnd,&rc,FALSE);
}

void locate_tap(int x,int y)
{
	tap_x=x;
	tap_y=y;
}

void put_tap_pattern(HWND hWnd,PAT_PIC	n,int mode)
{
	int		i,j;
	RECT	rc;

	if (bitspixel==2) {
		for (j=0;j<8;j++) {
			for (i=0;i<8;i++) {
				byte c;
				if (tap_pattern[n][mode][j][i]==1) {
					int k;
					k=(key_kaisou_flag==FALSE) ? 0 : 1;
					c=bw_text_color[k];
				} else {
					if (key_kaisou_flag==TRUE) {
						c=bw_bg_color[1];
					} else {
						c=get_pixel(picBuf,tap_x+i,tap_y+j);
						if (action_mode==0 || action_mode==2) 
							c=bw_pic_color[c];
					}
				}
				if (!rotate_flag)	set_pixel(baseBuf,tap_x+i,tap_y+j,c);
				else				set_pixel_r(baseBuf,tap_x+i,tap_y+j,c);
			}
		}
	} else {
		for (j=0;j<8;j++) {
			for (i=0;i<8;i++) {
				int r,g,b;
				if (tap_pattern[n][mode][j][i]==1) {
					int k;
					k=(key_kaisou_flag==FALSE) ? 0 : 1;
					r=text_color[k][0];
					g=text_color[k][1];
					b=text_color[k][2];
				} else {
					if (key_kaisou_flag==TRUE) {
						r=bg_color[1][0];
						g=bg_color[1][1];
						b=bg_color[1][2];
					} else {
						get_pixel24(picBuf,tap_x+i,tap_y+j,&r,&g,&b);
						if (action_mode==0 || action_mode==2) {
							r=pic_color[0][0]+(pic_color[1][0]-pic_color[0][0])*r/256;
							g=pic_color[0][1]+(pic_color[1][1]-pic_color[0][1])*g/256;
							b=pic_color[0][2]+(pic_color[1][2]-pic_color[0][2])*b/256;
							if (r<0) r=0;
							if (g<0) g=0;
							if (b<0) b=0;
							if (255<r) r=255;
							if (255<g) g=255;
							if (255<b) b=255;
						}
					}
				}
				if (!rotate_flag)	set_pixel24(baseBuf,tap_x+i,tap_y+j,r,g,b);
				else				set_pixel24_r(baseBuf,tap_x+i,tap_y+j,r,g,b);
			}
		}
	}
	if (!rotate_flag) {
		rc.left=tap_x;
		rc.top=tap_y+cbh;
		rc.right=rc.left+8;
		rc.bottom=rc.top+8;
	} else {
		rc.left=rotate_offset-(tap_y)-7;
		rc.top=tap_x+cbh;
		rc.right=rc.left+8;
		rc.bottom=rc.top+8;
	}
	InvalidateRect(hWnd,&rc,FALSE);
}

int put_button(TCHAR *str,int x,int y,int dx,BOOL sflag)
{
	RECT	rc;
	int		i,j,h;

	rc.left=3;
	rc.right=rc.left+dx-4;
	rc.top=2;
	rc.bottom=winHeight;
	h=DrawText(tmpDC,str,-1,&rc, DT_CENTER | DT_TOP | DT_WORDBREAK | DT_CALCRECT);
	fill(tmpBuf,1);
	rc.left=3;
	rc.right=rc.left+dx-4;
	rc.top=2;
	rc.bottom=winHeight;
	DrawText(tmpDC,str,-1,&rc, DT_CENTER | DT_TOP | DT_WORDBREAK);
	
	for (j=0;j<h+4;j++)
		for (i=0;i<dx;i++)
			if (get_pixel(tmpBuf,i,j)!=0)
				set_pixel(tmpBuf,i,j,2);
	for (j=0;j<h+4;j++) {
		set_pixel(tmpBuf,0,j,1);
		set_pixel(tmpBuf,dx-1,j,1);
	}
	for (i=0;i<dx;i++) {
		set_pixel(tmpBuf,i,0,1);
		set_pixel(tmpBuf,i,h+4-1,1);
	}
	if (sflag) {
		for (j=2;j<h+4-2;j++) {
			if (j%2==1) continue;
			set_pixel(tmpBuf,2,j,0);
			set_pixel(tmpBuf,dx-1-2,j,0);
		}
		for (i=2;i<dx-2;i++) {
			if (i%2==1) continue;
			set_pixel(tmpBuf,i,2,0);
			set_pixel(tmpBuf,i,h+4-1-2,0);
		}		
	}

	for (j=0;j<h+4;j++)
		for (i=0;i<dx;i++) {
			BYTE c=get_pixel(tmpBuf,i,j);
			if (bitspixel==2) {
				if (!rotate_flag)
					set_pixel(baseBuf,x+i,y+j,c);
				else
					set_pixel_r(baseBuf,x+i,y+j,c);
			} else {
				int r,g,b;
				if (c==0) r=g=b=0;
				if (c==1) r=g=b=128;
				if (c==2) r=g=b=192;
				if (c==3) r=g=b=255;
				if (!rotate_flag)
					set_pixel24(baseBuf,x+i,y+j,r,g,b);
				else
					set_pixel24_r(baseBuf,x+i,y+j,r,g,b);
			}
		}

	return h+4;
}
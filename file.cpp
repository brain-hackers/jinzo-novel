#ifndef	UNICODE
#define	UNICODE
#endif	/* UNICODE */
#ifndef	_UNICODE
#define	_UNICODE
#endif	/* _UNICODE */
#define	STRICT 
#include <windows.h> 
#include <commctrl.h> 
#include	<commdlg.h>
#include <wingdi.h>
#include "resource.h" 
#include <Winbase.h>
#include <WINDOWSX.H>
#include "jznovel.h" 

BOOL	CALLBACK select_scenarioDlgProc(HWND, UINT, WPARAM, LPARAM);

BMI bmi ;
BITMAPFILEHEADER	BmpFH ;
BITMAPINFOHEADER	BmpIH ;

TCHAR	picture_name[MAX_PICTURE][MAX_PATH];
int		picture_x[MAX_PICTURE],picture_y[MAX_PICTURE];
int		picture[MAX_PICTURE][4];
BOOL	picture_transparent_flag[MAX_PICTURE];
BYTE	picture_transparent[MAX_PICTURE];

TCHAR	path[MAX_PATH];
TCHAR	save_data_fn[MAX_PATH];
TCHAR	scenario_filename[MAX_PATH];
TCHAR	*sc_fn[256];

int		cnf_width=0,cnf_height=0;
int		cnf_language;

BOOL	compiled_scenario_flag;
HANDLE	hCompiledFile;

void get_file_line(HANDLE h,TCHAR *buf)
{
	DWORD	readsize;
	int		i;
	BYTE	str[256],c;
	for (i=0;;i++) {
		ReadFile( h, &c, 1, &readsize, NULL ) ;
		if (c==0x0d && i==0) {
			ReadFile( h, &c, 1, &readsize, NULL ) ;
			i=0;
			continue;
		}
		if (c==0x0d || readsize==0) {
			ReadFile( h, &c, 1, &readsize, NULL ) ;
			str[i]='\0';
			MultiByteToWideChar(CP_ACP, 0, (const char *)str, -1, buf, 1000);
			return;
		} else {
			str[i]=c;
		}
	}
}

void compile_scenario()
{
	HANDLE	hFile,hFile2;
	DWORD	writesize;
	TCHAR	fn[MAX_PATH],*s;
	int		i,j;
	BYTE	buf[16];
	int		intbuf[100];
	int		p=0;

	wsprintf(fn,TEXT("%s%s"),path,scenario_filename);
	for (s=fn;;s++) 
		if ((*s)=='\0') break;
	*(s-3)='j';
	*(s-2)='z';
	*(s-1)='n';

	hFile = CreateFile( fn, GENERIC_WRITE, 0, 0,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) {
		MessageBox( NULL, fn, TEXT("File cannot Create (compile)"), MB_OK ) ;
		return;
	}
	buf[0]='J';
	buf[1]='Z';
	buf[2]='N';
	buf[3]='\0';
	buf[4]=cnf_width%256;	
	buf[5]=cnf_width/256;	
	buf[6]=cnf_height%256;	
	buf[7]=cnf_height/256;	
	buf[8]=cnf_language;
	WriteFile( hFile, buf, 16, &writesize, NULL ) ; 
	p+=16;

	for (j=0;j<100;j++)
		for (i=0;i<4;i++) {
			BYTE data=picture[j][i];
			WriteFile( hFile, &data, 1, &writesize, NULL ) ; 
			p+=1;
		}

	WriteFile( hFile, intbuf, sizeof(int)*100, &writesize, NULL ) ; 
	p+=sizeof(int)*100;

	buf[0]=scenario_size%256;
	buf[1]=(scenario_size/256)%256;
	buf[2]=(scenario_size/256/256)%256;
	buf[3]=(scenario_size/256/256/256)%256;
	WriteFile( hFile, buf, 4, &writesize, NULL ) ; 
	p+=4;
	for (i=0;i<scenario_size;i++)
		scenario[i]^=255;
	WriteFile( hFile, scenario, scenario_size, &writesize, NULL ) ; 
	for (i=0;i<scenario_size;i++)
		scenario[i]^=255;
	p+=scenario_size;

	for (j=0;j<MAX_PICTURE;j++) {
		int		size;
		BYTE	*b;
		TCHAR	gfn[MAX_PATH];
		intbuf[j]=0;
		if (picture_name[j][0]=='\0') continue;
		intbuf[j]=p;
		buf[0]=picture_x[j]%256;
		buf[1]=picture_x[j]/256;
		buf[2]=picture_y[j]%256;
		buf[3]=picture_y[j]/256;
		buf[4]=picture_transparent_flag[j];
		buf[5]=picture_transparent[j];
		WriteFile( hFile, buf, 6, &writesize, NULL ) ; 
		p+=6;
		wsprintf(gfn,TEXT("%s%s"),path,picture_name[j]);
		hFile2 = CreateFile(gfn, GENERIC_READ, FILE_SHARE_READ,
					0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
		if ( hFile2 == INVALID_HANDLE_VALUE ) {
			MessageBox( NULL, picture_name[j], TEXT("File not Open (graphic)"), MB_OK ) ;
			return;
		}
		size=GetFileSize(hFile2,NULL);
		b=(BYTE *)malloc(size);
		ReadFile( hFile2, b, size, &writesize, NULL ) ;
		CloseHandle(hFile2);

		buf[0]=size%256;
		buf[1]=(size/256)%256;
		buf[2]=(size/256/256)%256;
		buf[3]=(size/256/256/256)%256;
		WriteFile( hFile, buf, 4, &writesize, NULL ) ; 
		p+=4;
		WriteFile( hFile, b, size, &writesize, NULL ) ; 
		p+=size;
		free(b);
	}

	SetFilePointer(hFile,16+4*100,NULL,FILE_BEGIN);
	WriteFile( hFile, intbuf, sizeof(int)*100, &writesize, NULL ) ; 
	CloseHandle(hFile);	

	MessageBox( NULL, TEXT("Compiling Scneario has Done."), TEXT("Compile Done"), MB_OK ) ;

/*
	コンパイルシナリオフォーマット
	00-03   'J','Z','N','\0'
	04-05	表示画面サイズ　横幅
	06-07	表示画面サイズ　縦幅
	08		言語
	10-19F  グラフィック重ね合わせ (4*100)
	1A0-32F	グラフィックファイルオフセット (4*100)
	330-333	シナリオデータのファイルサイズ(4)
	334-	シナリオデータ
	x-		グラフィック位置 x (2)
	(x+2)-	グラフィック位置 y (2)
	(x+4)   透明色の有り(1)/無し(0)
	(x+5)   透明色
	(x+6)-	グラフィックデータサイズ (4)
	(x+10)-	グラフィックデータ
*/
}

void load_cnf_file()
{
	TCHAR	buf[256],fn[MAX_PATH],graphic_filename[MAX_PATH];
	int		i,j;
	for (j=0;j<MAX_PICTURE;j++) {
		picture_name[j][0]='0';
		picture[j][0]=-1;
		picture[j][1]=-1;
		picture[j][2]=-1;
		picture[j][3]=-1;
		picture_name[j][0]='\0';
		picture_transparent_flag[j]=FALSE;
		picture_transparent[j]=255;
	}

	wsprintf(fn,TEXT("%s%s"),path,scenario_filename);

	if (compiled_scenario_flag==TRUE) {
		DWORD	readsize;
		BYTE	buf[16];

		hCompiledFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
					0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
		if ( hCompiledFile == INVALID_HANDLE_VALUE ) {
			MessageBox( NULL, fn, TEXT("File not Open (config)"), MB_OK ) ;
			return;
		}

		ReadFile( hCompiledFile, buf, 16, &readsize, NULL ) ;
		if (!(buf[0]=='J' && buf[1]=='Z' && buf[2]=='N' && buf[3]=='\0')) {
			MessageBox( NULL, fn, TEXT("File irregal (config)"), MB_OK ) ;
			return;
		}
		cnf_width=buf[4]+buf[5]*256;
		cnf_height=buf[6]+buf[7]*256;
		cnf_language=buf[8];
		
		for (j=0;j<MAX_PICTURE;j++) 
			for (i=0;i<4;i++) {
				BYTE	data;
				ReadFile( hCompiledFile, &data, 1, &readsize, NULL ) ;
				picture[j][i]=data;
				if (data>=MAX_PICTURE)	picture[j][i]=-1;
			}
		SetFilePointer(hCompiledFile,4*MAX_PICTURE,NULL,FILE_CURRENT);
		ReadFile( hCompiledFile, buf, 4, &readsize, NULL ) ;
		scenario_size=((buf[3]*256+buf[2])*256+buf[1])*256+buf[0];
		scenario=(BYTE *)malloc(scenario_size);
		ReadFile( hCompiledFile, scenario, scenario_size, &readsize, NULL ) ;
		for (i=0;i<scenario_size;i++)
			scenario[i]^=255;
		init_scenario_file();
	} else {
		HANDLE	hFile;
		HANDLE	hTextFile;
		DWORD	readsize;
		hFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
					0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
		if ( hFile == INVALID_HANDLE_VALUE ) {
			MessageBox( NULL, fn, TEXT("File not Open (config)"), MB_OK ) ;
			return;
		}

		get_file_line(hFile,buf);
		wsprintf(fn,TEXT("%s%s"),path,buf);
		hTextFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
					0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
		if ( hTextFile == INVALID_HANDLE_VALUE ) {
			MessageBox( NULL, fn, TEXT("File not Open"), MB_OK ) ;
			return;
		}
		scenario_size=GetFileSize(hTextFile,NULL);
		scenario=(BYTE *)malloc(scenario_size);
		ReadFile( hTextFile, scenario, scenario_size, &readsize, NULL ) ;
		CloseHandle(hTextFile);

		init_scenario_file();
	
		get_file_line(hFile,graphic_filename);
	
		get_file_line(hFile,buf);
		cnf_width =(buf[0]-'0')*100+(buf[1]-'0')*10+(buf[2]-'0');
		cnf_height=(buf[4]-'0')*100+(buf[5]-'0')*10+(buf[6]-'0');
	
		if (cnf_width>winWidth || cnf_height>winHeight) {
			if (cnf_width!=0) {
				TCHAR str[256];
				wsprintf(str,TEXT("Scenario display area is larger than this machine.\n\n  scenario : %d x %d\n  machine : %d x %d"),
					cnf_width,cnf_height,winWidth,winHeight);
				MessageBox( NULL, str, TEXT("Warning"), MB_OK ) ;
			}
		}
	
		get_file_line(hFile,buf);
		cnf_language=hex_char(buf[0]);
	
		CloseHandle( hFile ) ;
	
		wsprintf(fn,TEXT("%s%s"),path,graphic_filename);
		hFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
					0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
		if ( hFile == INVALID_HANDLE_VALUE ) {
			MessageBox( NULL, buf, TEXT("File not Open (graphic)"), MB_OK ) ;
			return;
		}
		for (;;) {
			int	n,i,j;
			TCHAR	*s=buf,c;
			get_file_line(hFile,buf);
			if (*s=='*') break;
			n=(*(s+1)-'0')*10+(*(s+2)-'0');
			if (*s=='#') {
				s+=4;
				for (i=0;;i++) {
					if (*s==',') break;
					picture_name[n][i]=*s++;
				}
				picture_name[n][i]='\0';
				s++;
				i=0;
				for (;;) {
					c=*s++;
					if (!('0'<=c && c<='9')) break;
					i=i*10+(c-'0');
				}
				picture_x[n]=i;
				i=0;
				for (;;) {
					c=*s++;
					if (!('0'<=c && c<='9')) break;
					i=i*10+(c-'0');
				}
				picture_y[n]=i;
				if (c=='\0') continue;
				i=0;
				for (;;) {
					c=*s++;
					if (!('0'<=c && c<='9')) break;
					i=i*10+(c-'0');
					picture_transparent_flag[n]=TRUE;
				}
				if (picture_transparent_flag[n])
					picture_transparent[n]=i;
			}
			if (*s=='%') {
				s+=4;
				for (j=0;j<4;j++) {
					i=0;
					for (;;) {
						c=*s++;
						if (!('0'<=c && c<='9')) break;
						i=i*10+(c-'0');
					}
					picture[n][j]=i;
					if (!('0'<=*s && *s<='9')) break;
				}
			}
		}
		CloseHandle( hFile ) ;
	}

	wsprintf(save_data_fn,TEXT("%s%s"),path,scenario_filename);
	for (i=0;;i++)
		if (save_data_fn[i]=='\0') break;
	save_data_fn[i-3]='s';
	save_data_fn[i-2]='a';
	save_data_fn[i-1]='v';
}

void load_picture(HWND hWnd,int n)
{
	HANDLE	hFile;
	DWORD	readsize ;
	BYTE	buf[640*3];
	TCHAR	fn[MAX_PATH];
	int		i,j;  
	int		x_file_offset; 
	int		wmax,wmin,hmax,hmin;
	int		bmp_width,bmp_height,bmp_pixel;
	int		px,py;  
	int		filesize;

	if (compiled_scenario_flag==TRUE) {
		wsprintf(fn,TEXT("%s%s"),path,scenario_filename);
		SetFilePointer(hCompiledFile,16+4*MAX_PICTURE+4*n,NULL,FILE_BEGIN);
		ReadFile( hCompiledFile, buf, 4, &readsize, NULL ) ;
		i=((buf[3]*256+buf[2])*256+buf[1])*256+buf[0];
		SetFilePointer(hCompiledFile,i,NULL,FILE_BEGIN);
		ReadFile( hCompiledFile, buf, 6, &readsize, NULL ) ;
		px=buf[1]*256+buf[0];
		py=buf[3]*256+buf[2];
		picture_transparent_flag[n]=buf[4];
		picture_transparent[n]=buf[5];
		ReadFile( hCompiledFile, buf, 4, &readsize, NULL ) ;
		filesize=((buf[3]*256+buf[2])*256+buf[1])*256+buf[0]; 
		hFile=hCompiledFile;
	} else {
		px=picture_x[n];
		py=picture_y[n];
		wsprintf(fn,TEXT("%s%s"),path,picture_name[n]);
		hFile = CreateFile( fn, GENERIC_READ, FILE_SHARE_READ,
					0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
		if ( hFile == INVALID_HANDLE_VALUE ) {
			MessageBox( NULL, fn, TEXT("File not Open"), MB_OK ) ;
			return;
		}
		filesize=GetFileSize(hFile,NULL);
	}

	ReadFile( hFile, &BmpFH, sizeof BmpFH, &readsize, NULL ) ;
	if (BmpFH.bfType!=0x4D42) {
		MessageBox( NULL, fn, TEXT("File Irregal"), MB_OK ) ;
		return;
	}

	ReadFile( hFile, &bmi, BmpFH.bfOffBits-sizeof(BmpFH), &readsize, NULL ) ;

	wmax=(winWidth>bmi.bmih.biWidth) ? winWidth : bmi.bmih.biWidth; 
	wmin=(winWidth<bmi.bmih.biWidth) ? winWidth : bmi.bmih.biWidth;
	hmax=(winHeight>bmi.bmih.biHeight) ? winHeight : bmi.bmih.biHeight;
	hmin=(winHeight<bmi.bmih.biHeight) ? winHeight : bmi.bmih.biHeight;

	bmp_width=bmi.bmih.biWidth; 
	bmp_height=bmi.bmih.biHeight;
	bmp_pixel=bmi.bmih.biBitCount;

	if ((filesize-BmpFH.bfOffBits)-(bmp_width*bmp_height*bmi.bmih.biBitCount/8)==0x3F0)
		x_file_offset=bmi.bmih.biWidth*bmi.bmih.biBitCount/8;
	else
		x_file_offset=(filesize-BmpFH.bfOffBits)/bmi.bmih.biHeight;

	if (hmax>winHeight) 
		for (j=hmax-1;j>=hmin;j--) 
			ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ; 
	
	if (bmi.bmih.biBitCount==1) {
		BYTE	c;
		static int shift[8]={7,6,5,4,3,2,1,0};
		if (bitspixel==2) {
			BYTE pal[2];
			for (j=0;j<2;j++)
				pal[j]=mono_color(bmi.rgq[j].rgbRed,bmi.rgq[j].rgbGreen,bmi.rgq[j].rgbBlue);
			if (picture_transparent_flag[n]) {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/8]>>shift[i%8])&1;
						if (c==picture_transparent[n]) continue;
						set_pixel(picBuf,px+i,py+j,pal[c]);
					}	
				}
			} else {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/8]>>shift[i%8])&1;
						set_pixel(picBuf,px+i,py+j,pal[c]);
					}	
				}
			}
		} else {
			if (picture_transparent_flag[n]) {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/8]>>shift[i%8])&1;
						if (c==picture_transparent[n]) continue;
						set_pixel24(picBuf,px+i,py+j,bmi.rgq[c].rgbRed,bmi.rgq[c].rgbGreen,bmi.rgq[c].rgbBlue);
					}	
				}
			} else {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/8]>>shift[i%8])&1;
						set_pixel24(picBuf,px+i,py+j,bmi.rgq[c].rgbRed,bmi.rgq[c].rgbGreen,bmi.rgq[c].rgbBlue);
					}	
				}
			}
		}
	}

	if (bmi.bmih.biBitCount==2) {
		int	c;
		static int shift[4]={6,4,2,0};
		if (bitspixel==2) {
			BYTE pal[4];
			for (j=0;j<4;j++)
				pal[j]=mono_color(bmi.rgq[j].rgbRed,bmi.rgq[j].rgbGreen,bmi.rgq[j].rgbBlue);
			if (picture_transparent_flag[n]) {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/4]>>shift[i%4])&3;
						if (c==picture_transparent[n]) continue;
						set_pixel(picBuf,px+i,py+j,pal[4]);
					}
				}
			} else {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/4]>>shift[i%4])&3;
						set_pixel(picBuf,px+i,py+j,pal[4]);
					}
				}
			}
		} else {
			if (picture_transparent_flag[n]) {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/4]>>shift[i%4])&3;
						if (c==picture_transparent[n]) continue;
						set_pixel24(picBuf,px+i,py+j,bmi.rgq[c].rgbRed,bmi.rgq[c].rgbGreen,bmi.rgq[c].rgbBlue);
					}
				}
			} else {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/4]>>shift[i%4])&3;
						set_pixel24(picBuf,px+i,py+j,bmi.rgq[c].rgbRed,bmi.rgq[c].rgbGreen,bmi.rgq[c].rgbBlue);
					}
				}
			}
		}
	}

	if (bmi.bmih.biBitCount==4) {
		int	c;
		static int shift[4]={4,0};
		if (bitspixel==2) {
			BYTE pal[16];
			for (j=0;j<16;j++)
				pal[j]=mono_color(bmi.rgq[j].rgbRed,bmi.rgq[j].rgbGreen,bmi.rgq[j].rgbBlue);
			if (picture_transparent_flag[n]) {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/2]>>shift[i%2])&15;
						if (c==picture_transparent[n]) continue;
						set_pixel(picBuf,px+i,py+j,pal[c]);
					}
				}
			} else {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/2]>>shift[i%2])&15;
						set_pixel(picBuf,px+i,py+j,pal[c]);
					}
				}
			}
		} else {
			if (picture_transparent_flag[n]) {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/2]>>shift[i%2])&15;
						if (c==picture_transparent[n]) continue;
						set_pixel24(picBuf,px+i,py+j,bmi.rgq[c].rgbRed,bmi.rgq[c].rgbGreen,bmi.rgq[c].rgbBlue);
					}
				}
			} else {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=(buf[i/2]>>shift[i%2])&15;
						set_pixel24(picBuf,px+i,py+j,bmi.rgq[c].rgbRed,bmi.rgq[c].rgbGreen,bmi.rgq[c].rgbBlue);
					}
				}
			}
		}
	}
	if (bmi.bmih.biBitCount==8) {
		int	c;
		BYTE pal[256];
		for (j=0;j<256;j++)
			pal[j]=mono_color(bmi.rgq[j].rgbRed,bmi.rgq[j].rgbGreen,bmi.rgq[j].rgbBlue);
		if (bitspixel==2) {
			if (picture_transparent_flag[n]) {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=buf[i];
						if (c==picture_transparent[n]) continue;
						set_pixel(picBuf,px+i,py+j,pal[c]);
					}
				}
			} else {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=buf[i];
						set_pixel(picBuf,px+i,py+j,pal[c]);
					}
				}
			}
		} else {
			if (picture_transparent_flag[n]) {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=buf[i];
						if (c==picture_transparent[n]) continue;
						set_pixel24(picBuf,px+i,py+j,bmi.rgq[c].rgbRed,bmi.rgq[c].rgbGreen,bmi.rgq[c].rgbBlue);
					}
				}
			} else {
				for (j=hmin-1;j>=0;j--) {
					ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
					for (i=0;i<wmin;i++) {
						c=buf[i];
						set_pixel24(picBuf,px+i,py+j,bmi.rgq[c].rgbRed,bmi.rgq[c].rgbGreen,bmi.rgq[c].rgbBlue);
					}
				}
			}
		}
	}
	if (bmi.bmih.biBitCount==24) {
		BYTE *bb;
		if (bitspixel==2) {
			for (j=hmin-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
				bb=buf;
				for (i=0;i<wmin;i++) {
					set_pixel(picBuf,px+i,py+j,mono_color(*(bb+2),*(bb+1),*(bb+0)));
					bb+=3;
				}
			}
		} else {
			for (j=hmin-1;j>=0;j--) {
				ReadFile( hFile, buf, x_file_offset, &readsize, NULL ) ;
				bb=buf;
				for (i=0;i<wmin;i++) {
					set_pixel24(picBuf,px+i,py+j,*(bb+2),*(bb+1),*(bb+0));
					bb+=3;
				}
			}
		}
	}
	if (compiled_scenario_flag==FALSE) {
		CloseHandle( hFile ) ;
	}
}

BOOL select_scenario(HWND hWnd)
{
	int		i,j;
	TCHAR	fn2[MAX_PATH];
	WIN32_FIND_DATA data;
	HANDLE	hFile; 

	GetModuleFileName(NULL,path,MAX_PATH);
	j=0;
	for (i=0;;i++) {
		if (path[i]=='\\') j=i;
		if (path[i]=='\0') {
			path[j]='\\';
			path[j+1]='\0';
			break;
		} 
	}
	
	for (i=0;i<256;i++)
		sc_fn[i]=(TCHAR *)malloc(sizeof(TCHAR)*MAX_PATH);
	
	wsprintf(fn2,TEXT("%s*"),path);
	hFile=FindFirstFile(fn2,&data);
	if (hFile == INVALID_HANDLE_VALUE) return FALSE;
	for (i=0,j=0;;i++) {
		TCHAR	str[MAX_PATH],*s=str;
		wsprintf(str,TEXT("%s"),data.cFileName);
		for (;;s++) {
			if (*s=='\0') break;
			if ('A'<=(*s) && (*s)<='Z') *s+='a'-'A';
		}
		if (*(s-4)=='.' && *(s-3)=='j' && *(s-2)=='z' && *(s-1)=='t') {
			wsprintf(sc_fn[j],TEXT("%s"),data.cFileName); 
			j++;
		}
		if (*(s-4)=='.' && *(s-3)=='j' && *(s-2)=='z' && *(s-1)=='n') {
			wsprintf(sc_fn[j],TEXT("%s"),data.cFileName);
			j++;
		}
		if (FindNextFile(hFile,&data)==NULL) break;  
	}
	FindClose(hFile);
	sc_fn[i+1][0]='\0';
	scenario_filename[0]='\0';  

	if (j==0) { 
		MessageBox( NULL, TEXT("No scenario file exists"), TEXT("Erro"), MB_OK ) ; 
		for (i=0;i<256;i++)
			free(sc_fn[i]);
		return false;
	}

	if (j==1) {
		wsprintf(scenario_filename,TEXT("%s"),sc_fn[0]);
	} else {
		DialogBox(ghInst, MAKEINTRESOURCE(IDD_SELECT_SCENARIO), hWnd, select_scenarioDlgProc);
	}

	for (i=0;i<256;i++)
		free(sc_fn[i]);

	if (scenario_filename[0]=='\0') {
		MessageBox( NULL, TEXT("No scenario file exists"), TEXT("Error"), MB_OK ) ;
		return FALSE;
	}

	{
		TCHAR	str[MAX_PATH],*s=str;
		wsprintf(s,TEXT("%s"),scenario_filename);
		for (;;s++) {
			if (*s=='\0') break;
			if ('A'<=(*s) && (*s)<='Z') *s+='a'-'A';
		}
		if (*(s-4)=='.' && *(s-3)=='j' && *(s-2)=='z' && *(s-1)=='t') 
			compiled_scenario_flag=FALSE;
		else
			compiled_scenario_flag=TRUE;
	} 

	return TRUE;
}

BOOL CALLBACK select_scenarioDlgProc(HWND hDlg, UINT uMessage,
						   WPARAM wParam, LPARAM lParam)
{
	int		i,j;

	switch (uMessage) {
		case WM_INITDIALOG:
			for (i=0;;i++) {
				TCHAR str[MAX_PATH];
				if (sc_fn[i][0]=='\0') break;
				for (j=0;;j++) {
					str[j]=sc_fn[i][j];
					if (str[j]=='\0') {
//						str[j-4]='\0';
						break;
					}
				}
				ListBox_AddString(GetDlgItem(hDlg,IDC_SELECT_SCENARIO), str);
			}
			ListBox_SetCurSel(GetDlgItem(hDlg,IDC_SELECT_SCENARIO),0);
			SetFocus(GetDlgItem(hDlg, IDC_SELECT_SCENARIO));
			return FALSE;
		case WM_COMMAND:		
			switch (LOWORD(wParam)) {
				case IDC_SELECT_SCENARIO:
					if (HIWORD(wParam)==LBN_DBLCLK) {
						wsprintf(scenario_filename,TEXT("%s"),sc_fn[ListBox_GetCurSel(GetDlgItem(hDlg,IDC_SELECT_SCENARIO))]);
						EndDialog(hDlg, TRUE);
						InvalidateRect(GetParent(hDlg),NULL,FALSE);
						return TRUE;
					}
					return FALSE;
				case IDOK:
					wsprintf(scenario_filename,TEXT("%s"),sc_fn[ListBox_GetCurSel(GetDlgItem(hDlg,IDC_SELECT_SCENARIO))]);
				case IDCANCEL:
					EndDialog(hDlg, TRUE);
					InvalidateRect(GetParent(hDlg),NULL,FALSE);
					return TRUE;
			}
			break;
	}
	return FALSE;	// メッセージを処理しなかった場合はFALSEを返す
}



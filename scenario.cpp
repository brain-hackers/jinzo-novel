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
#include <Winnls.h>
#include "jznovel.h"

BOOL	CALLBACK selDlgProc(HWND, UINT, WPARAM, LPARAM);
void	select_answer(HWND,int);

BYTE	*scenario,*scenario_tmp;
int		scenario_size;
int		sp=0,sp_backup=0;

#define	MAX_SELECT	4
TCHAR	sel_str[MAX_SELECT][256];
TCHAR	sel_jump_tag[MAX_SELECT][4];
int		sel_num;

int		select_active;
int		select_py[MAX_SELECT+1];

#define MAX_VAL	10
TCHAR	val_jump_tag[MAX_VAL][4];

#define	KAISOU_MAX	2000
int		kaisou_array[KAISOU_MAX];
BYTE	kaisou_font_array[KAISOU_MAX];
int		kaisou_max,kaisou;
int		add_kaisou_flag;
BOOL	kaisou_flag;

int		text_sx,text_sy,text_dx,text_dy;

int		xpos,ypos;
BYTE	letter_height=12;
BYTE	letter_height_backup=12;

int		current_graphic=0;

BOOL	clear_flag;

LOGFONT		lf;

PAT_PIC	pattern_pic;
int	pattern_mode=0;


int hex_char(TCHAR c)
{
	int res=0;
	if ('0'<=c && c<='9')	res=c-'0';
	if ('A'<=c && c<='F')	res= c-'A'+10;
	if ('a'<=c && c<='f')	res= c-'a'+10;
	res*=0x11;
	return res;
}

void get_line(TCHAR *buf)
{
	BYTE sjis_buf[1000];
	int i;

	for (i=0;;i++) {
		if (scenario[sp]=='\\' && scenario[sp+1]=='n') {
			sjis_buf[i]='\n';
			sp+=2;
			continue;
		}
		if (scenario[sp]==0x0d && scenario[sp+1]==0x0a) {
			sjis_buf[i]='\0';
			sp+=2;
			MultiByteToWideChar(CP_ACP, 0, (const char *)sjis_buf, -1, buf, 1000);
			return;
		}
		sjis_buf[i]=scenario[sp];
		sp++;
	} 
}

void text_draw(HWND hWnd,TCHAR *str)
{
	if (str==NULL) {
		fill(textBuf,3);
		ypos=0;
		conv_pic();
//		InvalidateRect(hWnd,NULL,FALSE);
	} else {
		RECT	rc;
		int		h; 
		int		i,j,k=0;
		rc.top=text_sy+ypos; 
		rc.bottom=text_sy+text_dy-1;
		rc.left=text_sx;
		rc.right=text_sx+text_dx-1;
		h=DrawText(textDC, str,-1,&rc, DT_LEFT | DT_TOP | DT_WORDBREAK );
		overlap_text(hWnd,text_sx,text_sy+ypos,text_sx+text_dx-1,text_sy+ypos+h);
		ypos+=h;

		k=text_sx;
		for (i=text_sx+text_dx-1;i>=text_sx;i--) {
			for (j=0;j<letter_height;j++) {
				if (get_pixel(textBuf,i,text_sy+ypos-letter_height+j)==0) {
					k=i;
					break;
				}
			}
			if (k!=text_sx) break;
		}
		if (k>=text_sx+text_dx-1-8 || k==text_sx) {
			int	iy; 
			iy=text_sy+ypos+letter_height-10;
			if (text_sy+ypos-10>=winHeight-8) iy=winHeight-1-8;
			locate_tap(text_sx,iy);
		} else {
			int	iy;
			iy=text_sy+ypos-10;
			if (text_sy+ypos-10>=winHeight-8) iy=winHeight-1-8;
			locate_tap(k+4,iy);
		}
/*
		{
			TCHAR	Message[256];
			wsprintf(Message,TEXT("%d %d : %d %d"),text_sx,text_sy+ypos+letter_height-10,k+(letter_height-8)/2,text_sy+ypos-10);
			MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		}
*/
	}
}

void jump(TCHAR	*jt)
{
	int i;
	for (i=0;;i++) {
		if (scenario[i  ]=='%' && 
			scenario[i+1]=='t' &&
			scenario[i+2]==jt[0] && 
			scenario[i+3]==jt[1] && 
			scenario[i+4]==jt[2]) {
			sp=i+7;
			return;
		}
	}
}

void set_font(HWND hWnd,BYTE fontsize)
{
	HFONT	hFont;

	lf.lfHeight=fontsize;;
	hFont = CreateFontIndirect( &lf );
	SelectObject(textDC, hFont);
	SelectObject(tmpDC, hFont);
    DeleteObject(hFont);
	letter_height=fontsize;
}

void put_graphic(HWND hWnd,int n) 
{
	int i,j;

	if (bitspixel==2)
		fill(picBuf,bw_bg_color[0]);
	else  
		fill24(picBuf,bg_color[0][0],bg_color[0][1],bg_color[0][2]);

	if (n!=0) {
		for (i=3;i>=0;i--) {
			j=picture[n][i];
			if (j>=0)
				load_picture(hWnd,j);
		}
	}
}

void add_kaisou()
{
	kaisou_max++;
	kaisou_array[kaisou_max]=sp;
	kaisou_font_array[kaisou_max]=letter_height;
	kaisou=kaisou_max;
	letter_height_backup=letter_height;
	add_kaisou_flag=FALSE;
}

SC_EVENT	read_scenario(HWND hWnd)
{
	TCHAR	str[1000],*s,*ss;
	TCHAR	jump_tag[3];
	int		i,n;

	if (clear_flag==TRUE) {
		text_draw(hWnd,NULL);
		clear_flag=FALSE;
	}
	if (kaisou_flag==FALSE) {
		if (add_kaisou_flag==TRUE) {
			add_kaisou();
		}
	}

	end_tap_pattern(hWnd);
	for (;;) {
		get_line(str);
		s=str;
		if (*s=='%') {
			s++;
			switch (*s) {
				case 'j':
					switch (*(s+1)) {
						case 's':
							if (!kaisou_flag) {
								InvalidateRect(hWnd,NULL,FALSE);
								sel_num=*(s+2)-'1'+2;
								select_active=0;
//								select_answer(hWnd);
//								InvalidateRect(hWnd,NULL,FALSE);
								return SC_SELECT;
								text_draw(hWnd,NULL);
								add_kaisou();
								continue;
							} else {
								clear_flag=TRUE;
								return SC_NEXT;
							}
							continue;;
						case 't':
							jump_tag[0]=*(s+2);							
							jump_tag[1]=*(s+3);							
							jump_tag[2]=*(s+4);
							jump(jump_tag);
							continue;;
						case 'v':
							jump(val_jump_tag[*(s+2)-'0']);
							continue;
						default:
							break;
					}
					break;
				case 's':
					if (!kaisou_flag) {
						n=*(s+1)-'0';
						ss=sel_str[n];
						for (i=3;;i++) {	
							if (*(s+i)==',') {
								*ss++='\0';
								*ss++='\0';
								sel_jump_tag[n][0]=*(s+i+1);
								sel_jump_tag[n][1]=*(s+i+2);
								sel_jump_tag[n][2]=*(s+i+3);
								break;
							} else {
								*ss++=*(s+i);
							}
						}
					}
					continue;
				case 'v':
					if (!kaisou_flag) {
						n=*(s+1)-'0';
						ss=sel_str[n];
						*ss++='\0';
						*ss++='\0';
						val_jump_tag[n][0]=*(s+3);
						val_jump_tag[n][1]=*(s+4);
						val_jump_tag[n][2]=*(s+5);
					}
					continue;
				case 't': 
					continue;
				case 'w':
					if (kaisou_flag!=TRUE) 
						start_tap_pattern(hWnd,PAT_WAIT);
					return	SC_WAIT;
					break;
				case 'n':
					clear_flag=TRUE;
					start_tap_pattern(hWnd,kaisou_flag==TRUE ? PAT_KAISOU : PAT_NEXT);
					if (kaisou_flag!=TRUE)
						add_kaisou_flag=TRUE;
					return	SC_NEXT;
					break;
				case 'g':
					if (!kaisou_flag) { 
						if (*(s+1)=='c') {
							int r,g,b;
							r=hex_char(*(s+3));
							g=hex_char(*(s+4)); 
							b=hex_char(*(s+5));
							pic_color[0][0]=r;
							pic_color[0][1]=g;
							pic_color[0][2]=b;
							r=hex_char(*(s+8));
							g=hex_char(*(s+9));
							b=hex_char(*(s+10));
							pic_color[1][0]=r;
							pic_color[1][1]=g;
							pic_color[1][2]=b;
							bw_pic_color[0]=*(s+13)-'0';
							bw_pic_color[1]=*(s+14)-'0';
							bw_pic_color[2]=*(s+15)-'0';
							bw_pic_color[3]=*(s+16)-'0';
						} else {
							n=(*(s+1)-'0')*10+(*(s+2)-'0');
							put_graphic(hWnd,n);
							current_graphic=n;
							put_pic();
							if (n!=0) {
								if (action_mode==0) return SC_PICTURE;
								if (action_mode==2) conv_pic();
							}
							overlap_text(hWnd,0,0,winWidth-1,winHeight-1);						
						}
					}
					continue;
				case 'a':
					if (*(s+1)=='t') {
						if (!kaisou_flag) {
							wsprintf(about_title_str,TEXT("%s%c"),s+3,0);
						}
					}
					if (*(s+1)=='a') {
						if (!kaisou_flag) {
							wsprintf(about_author_str,TEXT("%s%c"),s+3,0);
						}
					}
					if (*(s+1)=='v') {
						if (!kaisou_flag) {
							wsprintf(about_version_str,TEXT("%s%c"),s+3,0);
						}
					}
					continue;
				case 'z':
					init_scenario_file();
					return	SC_END;
					break;
				case 'f':
					if (*(s+1)=='c') {
						if (!kaisou_flag) {
							int r,g,b;
							r=hex_char(*(s+3));
							g=hex_char(*(s+4));
							b=hex_char(*(s+5));
							text_color[0][0]=r;
							text_color[0][1]=g;
							text_color[0][2]=b;
							r=hex_char(*(s+8));
							g=hex_char(*(s+9));
							b=hex_char(*(s+10));
							text_color[1][0]=r;
							text_color[1][1]=g;
							text_color[1][2]=b;
							bw_text_color[0]=*(s+13)-'0';
							bw_text_color[1]=*(s+16)-'0';
						}
					} else {
						n=(*(s+1)-'0')*10+(*(s+2)-'0');
						set_font(hWnd,(BYTE)n);
					}
					continue;
				case 'b':
					if (*(s+1)=='c') {
						if (!kaisou_flag) {
							int r,g,b;
							r=hex_char(*(s+3));
							g=hex_char(*(s+4));
							b=hex_char(*(s+5));
							bg_color[0][0]=r;
							bg_color[0][1]=g;
							bg_color[0][2]=b;
							r=hex_char(*(s+8));
							g=hex_char(*(s+9));
							b=hex_char(*(s+10));
							bg_color[1][0]=r;
							bg_color[1][1]=g;
							bg_color[1][2]=b;
							bw_bg_color[0]=*(s+13)-'0';
							bw_bg_color[1]=*(s+16)-'0';
						}
					}
					continue;
				case 'd':
					if (!kaisou_flag) {
						text_sx=(*(s+ 1)-'0')*100+(*(s+ 2)-'0')*10+(*(s+ 3)-'0');
						text_sy=(*(s+ 5)-'0')*100+(*(s+ 6)-'0')*10+(*(s+ 7)-'0');
						text_dx=(*(s+ 9)-'0')*100+(*(s+10)-'0')*10+(*(s+11)-'0');
						text_dy=(*(s+13)-'0')*100+(*(s+14)-'0')*10+(*(s+15)-'0');
					}
					continue;
				case 'm':
					if (*(s+1)=='g') {
						if (!kaisou_flag) {
							action_mode=*(s+2)-'0';
						}
					}
					continue;
				default: 
					continue;
			}
		}
		if (str[0]=='\0') {
			ypos+=letter_height+1;
		} else {
			text_draw(hWnd,str);
		}
	}
}
void init_scenario_file()
{
	HFONT	hFontSystem;
	int		i,j;

	sp=0;

	for (i=0;i<MAX_SELECT;i++) {
		wsprintf(sel_str[i],TEXT(""));
		for (j=0;j<3;j++)
			sel_jump_tag[i][j]=' ';
	}
	for (i=0;i<MAX_VAL;i++) {
		for (j=0;j<3;j++)
			val_jump_tag[i][j]=' ';
	}

	text_sx=0;
	text_sy=0;
	text_dx=winWidth;
	text_dy=winHeight;

	clear_flag=TRUE;
	add_kaisou_flag=TRUE;
	kaisou=0;
	kaisou_max=0;

	hFontSystem = (HFONT)GetStockObject(SYSTEM_FONT);
    GetObject( hFontSystem, sizeof(LOGFONT), &lf );
	DeleteObject(hFontSystem);
 
	locate_tap(0,250);
}

void select_answer(HWND hWnd,int n)
{	
	int		i,h;
	select_py[0]=text_sy+ypos+15-26;
	for (i=0;i<sel_num;i++) {
		h=put_button(sel_str[i],text_sx,select_py[i],text_dx, i==n ? TRUE:FALSE);
		select_py[i+1]=select_py[i]+h;
	}
}

void end_answer(HWND hWnd,int n)
{
/*
	{
		TCHAR Message[256];
		wsprintf(Message,TEXT("%d %d\n%c%c%c\n%c%c%c\n"),n,sel_num,
			sel_jump_tag[0][0],
			sel_jump_tag[0][1],
			sel_jump_tag[0][2],
			sel_jump_tag[1][0],
			sel_jump_tag[1][1],
			sel_jump_tag[1][2]);
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
	}
*/
	jump(sel_jump_tag[n]);
	text_draw(hWnd,NULL);	
	add_kaisou();
	InvalidateRect(hWnd,NULL,FALSE);
}
/*
BOOL CALLBACK selDlgProc(HWND hDlg, UINT uMessage,
						   WPARAM wParam, LPARAM lParam)
{
	RECT	rc;
	int j;
	static int str_id[]={ID_STR0,ID_STR1,ID_STR2,ID_STR3};
	HFONT	hf;
	int     py;

	switch (uMessage) {
	  case WM_INITDIALOG:
		py=0;
		select_active=0;
		if (!rotate_flag) {
			hf = CreateFontIndirect( &lf );
			SelectObject(GetDC(hDlg), hf);
			for (j=0;j<sel_num+2;j++) {
				int h;
				SendMessage(GetDlgItem(hDlg,str_id[j]),WM_SETFONT,(WPARAM)hf,MAKELPARAM(TRUE, 0));
				SetDlgItemText(hDlg, str_id[j], sel_str[j])	;
				rc.left=4;
				rc.right=text_dx-4;
				rc.top=0;
				rc.bottom=999;
				h=DrawText(textDC, sel_str[j],-1,&rc, DT_CENTER | DT_TOP | DT_WORDBREAK );
				MoveWindow(GetDlgItem(hDlg,str_id[j]),0,py,text_dx,h+8,FALSE);
				py+=h+8;
			}
		} else {
			for (j=sel_num+2-1;j>=0;j--) {
				int h;
				rc.left=0;
				rc.right=999;
				rc.top=4;
				rc.bottom=text_dx-4;
				h=DrawText(textDC, sel_str[j],-1,&rc, DT_CENTER | DT_TOP | DT_WORDBREAK );
				MoveWindow(GetDlgItem(hDlg,str_id[j]),py,0,h+8,text_dx,FALSE);
				py+=h+8;
			}
		}
		DeleteObject(hf);
		if (!rotate_flag) {
			GetWindowRect(hDlg, &rc);
			rc.left=text_sx;  
			rc.right=text_sx+text_dx-1;
			MoveWindow(hDlg,rc.left,text_sy+ypos+15-26+cbh,rc.right-rc.left,py,FALSE);
		} else {
			GetWindowRect(hDlg, &rc);
			rc.top=text_sx;  
			rc.bottom=text_sx+text_dx-1;
			MoveWindow(hDlg,rotate_offset-(text_sy+ypos+15-26)-py,text_sx+cbh,py,rc.bottom-rc.top,FALSE);
		}
	return FALSE;

	  case WM_COMMAND:		
		switch (wParam) {
		  case ID_STR0:
			EndDialog(hDlg, TRUE);
			InvalidateRect(GetParent(hDlg),NULL,FALSE);
			jump(sel_jump_tag[0]);
			return TRUE;
		  case ID_STR1:
			EndDialog(hDlg, TRUE);
			InvalidateRect(GetParent(hDlg),NULL,FALSE);
			jump(sel_jump_tag[1]);
			return TRUE;
		  case ID_STR2:
			EndDialog(hDlg, TRUE);
			InvalidateRect(GetParent(hDlg),NULL,FALSE);
			jump(sel_jump_tag[2]);
			return TRUE;
		  case ID_STR3:
			EndDialog(hDlg, TRUE);
			InvalidateRect(GetParent(hDlg),NULL,FALSE);
			jump(sel_jump_tag[3]);
			return TRUE;
		}
		break;

	  case WM_DRAWITEM:
		  {
			  RECT rc;
			  int	i,j,k;
			  tagDRAWITEMSTRUCT	*t=(tagDRAWITEMSTRUCT *)(lParam);
			if (t->CtlID==ID_STR0) k=0;
			if (t->CtlID==ID_STR1) k=1;
			if (t->CtlID==ID_STR2) k=2;
			if (t->CtlID==ID_STR3) k=3;
			fill(textBuf,2);
			fill(tmpBuf,2);
			for (j=0;j<t->rcItem.bottom-t->rcItem.top;j++) {
				set_pixel(tmpBuf,0,j,0);
				set_pixel(tmpBuf,t->rcItem.right-t->rcItem.left-1,j,0);
			}
			for (i=0;i<t->rcItem.right-t->rcItem.left;i++) {
				set_pixel(tmpBuf,i,0,0);
				set_pixel(tmpBuf,i,t->rcItem.bottom-t->rcItem.top-1,0);
			}
			if (select_active==k) {
				for (j=2;j<t->rcItem.bottom-t->rcItem.top-2;j++) {
					if (j%2==1) continue;
					set_pixel(tmpBuf,2,j,0);
					set_pixel(tmpBuf,t->rcItem.right-t->rcItem.left-3,j,0);
				}
				for (i=2;i<t->rcItem.right-t->rcItem.left-2;i++) {
					if (i%2==1) continue;
					set_pixel(tmpBuf,i,2,0);
					set_pixel(tmpBuf,i,t->rcItem.bottom-t->rcItem.top-3,0);
				}
			}
			rc.left=0;
			rc.right=text_dx-4;
			rc.top=0;
			rc.bottom=999;
			DrawText(textDC, sel_str[k],-1,&rc, DT_CENTER | DT_TOP | DT_WORDBREAK );
			set_pixel(textBuf,160,10,0);
			for (j=0;j<t->rcItem.bottom-t->rcItem.top;j++)
				for (i=0;i<t->rcItem.right-t->rcItem.left;i++) 
					if (get_pixel(textBuf,j,t->rcItem.right-t->rcItem.left-1-i)==0)
						set_pixel(tmpBuf,i-4,j,0);
			BitBlt(t->hDC,t->rcItem.left,t->rcItem.top,winHeight,winWidth,
					tmpDC,0,0,SRCCOPY);
		  }
		  return TRUE;
		case WM_KEYDOWN:
			switch (LOWORD(wParam)) {
				case VK_UP:
					select_active--;
					if (select_active==0) sel_num=sel_num-1;
					return TRUE;;
				case VK_DOWN:
					select_active++;
					if (select_active==sel_num) sel_num=0;
					return TRUE;;
			}
		
	}
	return FALSE;	// メッセージを処理しなかった場合はFALSEを返す
}
*/
void put_kaisou(HWND hWnd,KAISOU_MODE mode)
{
	if (mode==K_END) {
		kaisou_flag=FALSE;
		kaisou=kaisou_max; 
		sp=kaisou_array[kaisou];
		letter_height=kaisou_font_array[kaisou];
		set_font(hWnd,letter_height);
		put_graphic(hWnd,current_graphic);
		put_pic();
		text_draw(hWnd,NULL);
		for (;;) {
			read_scenario(hWnd);
			if (sp==sp_backup) break;
		}
		conv_pic();
		overlap_text(hWnd,0,0,winWidth-1,winHeight-1);
		return;
	}
	if (mode==K_START) {
		kaisou_flag=TRUE;
		sp_backup=sp;
		kaisou=kaisou_max;
		return;
	}
	if (mode==K_PREV) {
		kaisou--;
		if (kaisou<=1)
			kaisou=1;
	}
	if (mode==K_NEXT) {
		kaisou++;
		if (kaisou>kaisou_max)
			kaisou=kaisou_max; 
	}
	sp=kaisou_array[kaisou];
	letter_height=kaisou_font_array[kaisou];
	set_font(hWnd,letter_height);
	if (bitspixel==2)	fill_r(baseBuf,bw_bg_color[1]);	
	else				fill24_r(baseBuf,bg_color[1][0],bg_color[1][1],bg_color[1][2]);		
	clear_flag=TRUE;
	for (;;) {
		SC_EVENT	ret;
		ret=read_scenario(hWnd); 
		if (ret==SC_NEXT) return;
		if (kaisou==kaisou_max && sp>=sp_backup) {
			start_tap_pattern(hWnd,PAT_KAISOU);
			return;
		}
	}
}

void save_data(HWND hWnd)
{
	HANDLE	hFile;
	int		i;
	TCHAR	str[1024];
	DWORD	writesize;

	hFile = CreateFile( save_data_fn, GENERIC_WRITE, 0, 0, 
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) {
		TCHAR		Message[ 256 ] ;
		wsprintf( Message, TEXT("File Save error\n%s"),save_data_fn ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ; 
		return;
	}
	
	wsprintf(str,TEXT("p:%08d%%"),sp);
	WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
 
	wsprintf(str,TEXT("g:%02d,%d,%d%%"),current_graphic,action_mode,action_flag);
	WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
 
	for (i=0;i<MAX_SELECT;i++) {
		wsprintf(str,TEXT("s:%02d,%c%c%c,%s%%"),i,sel_jump_tag[i][0],sel_jump_tag[i][1],sel_jump_tag[i][2],sel_str[i]);
		WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
	}

	for (i=0;i<MAX_VAL;i++) {
		wsprintf(str,TEXT("v:%02d,%c%c%c%%"),i,val_jump_tag[i][0],val_jump_tag[i][1],val_jump_tag[i][2]);
		WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
	}
 
	wsprintf(str,TEXT("t:%02d%%"),(int)sc_value);
	WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
 
	wsprintf(str,TEXT("m:%04d%%"),kaisou_max); 
	WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
 
	wsprintf(str,TEXT("a:%1d%%"),(int)add_kaisou_flag);
	WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
 
	wsprintf(str,TEXT("d:%03d,%03d,%03d,%03d%%"),text_sx,text_sy,text_dx,text_dy);
	WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
 
	wsprintf(str,TEXT("c:%02x%02x%02x,%02x%02x%02x,%02x%02x%02x,%02x%02x%02x,%02x%02x%02x,%02x%02x%02x,%d%d%d%d,%d,%d,%d,%d%%"),
		pic_color[0][0],pic_color[0][1],pic_color[0][2],
		pic_color[1][0],pic_color[1][1],pic_color[1][2],
		bg_color[0][0],bg_color[0][1],bg_color[0][2],
		bg_color[1][0],bg_color[1][1],bg_color[1][2],
		text_color[0][0],text_color[0][1],text_color[0][2],
		text_color[1][0],text_color[1][1],text_color[1][2],
		bw_pic_color[0],bw_pic_color[1],bw_pic_color[2],bw_pic_color[3],
		bw_bg_color[0],bw_bg_color[1],
		bw_text_color[0],bw_text_color[1]);
	WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
 
	for (i=0;i<=kaisou_max;i++) {
		wsprintf(str,TEXT("k:%04d,%08d,%02d%%"),i,kaisou_array[i],kaisou_font_array[i]);
		WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
	}
	wsprintf(str,TEXT("*%%"));
	WriteFile( hFile, str, sizeof(TCHAR)*wcslen(str), &writesize, NULL ) ;
	
	CloseHandle( hFile ) ;
}

void load_data(HWND hWnd)
{
	HANDLE	hFile;
	DWORD	readsize;
	TCHAR	str[1024],*s;
	int		n;
	hFile = CreateFile( save_data_fn, GENERIC_READ, FILE_SHARE_READ,
				0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
	if ( hFile == INVALID_HANDLE_VALUE ) {
		MessageBox( NULL, save_data_fn, TEXT("No Save Data."), MB_OK ) ;
		return;
	}

	for (;;) {
		s=str;
		for (;;) {
			ReadFile( hFile, s,sizeof(TCHAR), &readsize, NULL ) ; 
			if (*s=='%') {  
				*s++='\0';
				break;
			}
			s++;
		}
		if (str[0]=='*') break;

		if (str[0]=='p') {
			sp_backup=0;
			sp_backup+=(str[2]-'0')*10000000;
			sp_backup+=(str[3]-'0')*1000000;
			sp_backup+=(str[4]-'0')*100000;
			sp_backup+=(str[5]-'0')*10000;
			sp_backup+=(str[6]-'0')*1000;
			sp_backup+=(str[7]-'0')*100;
			sp_backup+=(str[8]-'0')*10;
			sp_backup+=(str[9]-'0')*1;
		}

		if (str[0]=='g') {
			current_graphic=(str[2]-'0')*10+(str[3]-'0');
			action_mode=str[5]-'0';
			action_flag=str[7]-'0';
		}

		if (str[0]=='s') {
			n=(str[2]-'0')*10+(str[3]-'0');
			sel_jump_tag[n][0]=str[5];
			sel_jump_tag[n][1]=str[6];
			sel_jump_tag[n][2]=str[7];
			_stscanf(&str[9],TEXT("%s"),sel_str[n]);
		}
		if (str[0]=='v') {
			n=(str[2]-'0')*10+(str[3]-'0');
			val_jump_tag[n][0]=str[5];
			val_jump_tag[n][1]=str[6];
			val_jump_tag[n][2]=str[7];
		}

		if (str[0]=='t') {
			sc_value=(SC_EVENT)((str[2]-'0')*10+(str[3]-'0'));
		}

		if (str[0]=='a') {
			add_kaisou_flag=(BOOL)(str[2]-'0');
		}

		if (str[0]=='m') {
			kaisou_max=(str[2]-'0')*1000+(str[3]-'0')*100+(str[4]-'0')*10+(str[5]-'0');
		}

		if (str[0]=='d') {
			text_sx=(str[ 2]-'0')*100+(str[ 3]-'0')*10+(str[ 4]-'0');
			text_sy=(str[ 6]-'0')*100+(str[ 7]-'0')*10+(str[ 8]-'0');
			text_dx=(str[10]-'0')*100+(str[11]-'0')*10+(str[12]-'0');
			text_dy=(str[14]-'0')*100+(str[15]-'0')*10+(str[16]-'0');
		}

		if (str[0]=='c') {
			pic_color[0][0]=hex_char(str[ 2]);
			pic_color[0][1]=hex_char(str[ 4]);
			pic_color[0][2]=hex_char(str[ 6]);
			pic_color[1][0]=hex_char(str[ 9]);
			pic_color[1][1]=hex_char(str[11]);
			pic_color[1][2]=hex_char(str[13]);
			bg_color[0][0]=hex_char(str[16]);
			bg_color[0][1]=hex_char(str[18]);
			bg_color[0][2]=hex_char(str[20]);
			bg_color[1][0]=hex_char(str[23]);
			bg_color[1][1]=hex_char(str[25]);
			bg_color[1][2]=hex_char(str[27]);
			text_color[0][0]=hex_char(str[30]); 
			text_color[0][1]=hex_char(str[32]);
			text_color[0][2]=hex_char(str[34]);
			text_color[1][0]=hex_char(str[37]);
			text_color[1][1]=hex_char(str[39]);
			text_color[1][2]=hex_char(str[41]);
			bw_pic_color[0]=str[44]-'0';
			bw_pic_color[1]=str[45]-'0';
			bw_pic_color[2]=str[46]-'0';
			bw_pic_color[3]=str[47]-'0';
			bw_bg_color[0]=str[49]-'0';
			bw_bg_color[1]=str[51]-'0';
			bw_text_color[0]=str[53]-'0';
			bw_text_color[1]=str[55]-'0';
		}

		if (str[0]=='k') {
			n=(str[2]-'0')*1000+(str[3]-'0')*100+(str[4]-'0')*10+(str[5]-'0');
			kaisou_array[n]=0;
			kaisou_array[n]+=(str[ 7]-'0')*10000000;
			kaisou_array[n]+=(str[ 8]-'0')*1000000;
			kaisou_array[n]+=(str[ 9]-'0')*100000;
			kaisou_array[n]+=(str[10]-'0')*10000;
			kaisou_array[n]+=(str[11]-'0')*1000;
			kaisou_array[n]+=(str[12]-'0')*100;
			kaisou_array[n]+=(str[13]-'0')*10;
			kaisou_array[n]+=(str[14]-'0')*1;
			kaisou_font_array[n]=(str[16]-'0')*10+(str[17]-'0');
		}
	}
	CloseHandle( hFile ) ; 
	clear_flag=TRUE;
	put_kaisou(hWnd,K_END);
	InvalidateRect(hWnd,NULL,FALSE);
}

void start_tap_pattern(HWND hWnd,PAT_PIC n)
{
	if (n!=PAT_PREVIOUS) {
		pattern_pic=n;
	}
	pattern_mode=0;
	put_tap_pattern(hWnd,pattern_pic,pattern_mode);	
	SetTimer(hWnd, 1000,500,NULL); 
}

void end_tap_pattern(HWND hWnd)
{
	KillTimer(hWnd, 1000); 
	put_tap_pattern(hWnd,PAT_NONE,0);	
	put_tap_pattern(hWnd,PAT_NONE,pattern_mode);	
}


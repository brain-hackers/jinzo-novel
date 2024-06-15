#ifndef	UNICODE
#define	UNICODE
#endif	/* UNICODE */
#ifndef	_UNICODE
#define	_UNICODE
#endif	/* _UNICODE */
#define	STRICT
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include	<commdlg.h>
#include <wingdi.h> 
#include <Winuser.h>
#include "resource.h" 
#include <Winbase.h>
#include "jznovel.h"
DWORD	gle=0;

/* =====================================================================
	�֐��̃v���g�^�C�v�錾
===================================================================== */
// WinMain����Ă΂��֐�
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);

// �R�[���o�b�N�֐�
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

// �_�C�A���O�֐�
BOOL CALLBACK SelectScenarioDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PropertyDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK TreeDlgProc(HWND, UINT, WPARAM, LPARAM);

// ���b�Z�[�W�����֐�
void OnCreate(HWND, WPARAM, LPARAM);
void OnDestroy(HWND, WPARAM, LPARAM);
void OnPaint(HWND, WPARAM, LPARAM);
void OnHelp(HWND, WPARAM, LPARAM);
void SelectScenario(HWND);
void mouse_down(HWND,WPARAM, LPARAM);
void mouse_move(HWND,WPARAM, LPARAM);
void mouse_up(HWND,WPARAM, LPARAM);
void make_command_bar(HWND);

void start_tap_pattern(HWND,PAT_PIC);
void end_tap_pattern(HWND);

// �t�@�C���E���j���[�֐�
void OnFileExit(HWND);			// �I��
// �w���v�E���j���[�֐�
void OnHelpAbout(HWND);			// �o�[�W�������

/* =====================================================================
	�O���[�o���ϐ�
===================================================================== */
TCHAR gszAppName[]	= TEXT("JZNovel");	// �N���X�̖��O
TCHAR gszAppTitle[]	= TEXT("JZNovel");	// �E�B���h�E�E�^�C�g��

HINSTANCE ghInst	= NULL;		// ���݂̃C���X�^���X
HWND      ghWnd		= NULL;		// ���C���E�B���h�E�̃n���h��
HWND      ghWndCB	= NULL;		// �R�}���h�o�[�̃n���h��
HACCEL	  hAccel    = NULL;

int	cbh; 

HDC		baseDC	= NULL;		// �I�t�X�N���[��
HBITMAP	baseBmp	= NULL;
BYTE	*baseBuf;

HDC		picDC	= NULL;		// �I�t�X�N���[��
HBITMAP	picBmp	= NULL;
BYTE	*picBuf;
 
HDC		textDC	= NULL;		// �I�t�X�N���[��
HBITMAP	textBmp	= NULL;
BYTE	*textBuf;

HDC		tmpDC	= NULL;		// �I�t�X�N���[��
HBITMAP	tmpBmp	= NULL;
BYTE	*tmpBuf;

int		bitspixel;

int		winWidth,winHeight;
int		winWidth_dev,winHeight_dev;
BOOL	pspc_flag;

BOOL	action_flag;
int		action_mode;

SC_EVENT	sc_value;
BOOL		key_kaisou_flag;

static TBBUTTON tb[] = {
	{ 0, 0,            TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{ 0, IDM_KAISOU_BUTTON,TBSTATE_ENABLED, TBSTYLE_CHECK | TBSTYLE_AUTOSIZE, 0, NULL} 
//	{ 0, 0,            TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
};

TCHAR	about_title_str[128];
TCHAR	about_author_str[128];
TCHAR	about_version_str[128];

BOOL	esc_flag=TRUE;
//BOOL	rotate_flag=TRUE;
BOOL	rotate_flag=FALSE;
BOOL	select_flag=FALSE;
BOOL	select2_flag=FALSE;

/* =====================================================================
	Windows�v���O�����̃��C���֐�
===================================================================== */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					 LPTSTR lpszCmdParm, int nCmdShow)
{
	MSG    msg;

    // �E�B���h�E�̌���
    HWND hWnd = FindWindow(gszAppName, gszAppTitle);
    if (hWnd) {
        // ���ɋN�����Ă���E�B���h�E���őO�ʂɈړ����ďI��
        SetForegroundWindow(hWnd);
        return 0;
    }
 
	ghInst = hInstance;

	if (!hPrevInstance) {					// �ق��̃C���X�^���X���s�� ?
		if (!InitApplication(hInstance))	// ���ʂ̏���������
			return FALSE;					// �������Ɏ��s�����ꍇ�͏I��
	}
	if (!InitInstance(hInstance, nCmdShow))	// �C���X�^���X���̏���������
		return FALSE;						// ���s�����ꍇ�͏I��

	hAccel=LoadAccelerators(ghInst,MAKEINTRESOURCE(IDR_ACCEL));
	while (GetMessage(&msg, NULL, 0, 0)) {	// ���b�Z�[�W�̎擾�ƃf�B�X�p�b�`
		if (!TranslateAccelerator(msg.hwnd,hAccel,&msg)) {
			TranslateMessage(&msg);				// ���z�L�[�R�[�h�̕ϊ�
			DispatchMessage(&msg);				// ���b�Z�[�W�̃f�B�X�p�b�`
		}
	}
	return msg.wParam;		// PostQuitMessage()�֐��̖߂�l��Ԃ�
}

/* =====================================================================
	�E�B���h�E�E�N���X�̓o�^
===================================================================== */
BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style		 = 0;					// �N���X�E�X�^�C��
	wc.lpfnWndProc	 = MainWndProc;			// �E�B���h�E�E�v���V�[�W��
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;			// �C���X�^���X�E�n���h��
	wc.hIcon		 = NULL;
	wc.hCursor		 = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName	 = NULL;				// ���j���[�̖��O
	wc.lpszClassName = gszAppName;			// �E�B���h�E�E�N���X�̖��O

	return RegisterClass(&wc);	// �E�B���h�E�E�N���X�̓o�^
}

/* =====================================================================
	�E�B���h�E�̍쐬�ƕ\��
===================================================================== */
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ghWnd = CreateWindow( 
 		gszAppName,				// �o�^���ꂽ�E�B���h�E�E�N���X�̖��O
		gszAppTitle,			// �^�C�g���E�o�[�ɕ\������e�L�X�g
		WS_VISIBLE,				// �E�B���h�E�E�X�^�C��
		CW_USEDEFAULT,			// �E�B���h�E�̕\���ʒu (����)
		CW_USEDEFAULT,			//                      (����)
		CW_USEDEFAULT,			// �E�B���h�E�̑傫��   (��)
		CW_USEDEFAULT,			//                      (����)
		NULL,					// �e�E�B���h�E�̃n���h��
		NULL,					// �E�B���h�E�E�N���X�̃��j���[���g�p
		hInstance,				// �A�v���P�[�V�����E�C���X�^���X�̃n���h��
		NULL					// �E�B���h�E�쐬�f�[�^�̃A�h���X
	);
	if (!ghWnd)
		return FALSE;			// �E�B���h�E�̍쐬�Ɏ��s

	ShowWindow(ghWnd, nCmdShow);	// �E�B���h�E�\����Ԃ̐ݒ�
	UpdateWindow(ghWnd);			// �N���C�A���g�̈�̍X�V

	return TRUE;
}

/* =====================================================================
	�E�B���h�E�E�v���V�[�W��
===================================================================== */
void jzn_timer(HWND hWnd)
{
	if (pattern_mode==0)	pattern_mode=1;
	else					pattern_mode=0;
	put_tap_pattern(hWnd,pattern_pic,pattern_mode);
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMessage,
						 WPARAM wParam, LPARAM lParam)
{
	switch (uMessage) {
		case WM_TIMER:		jzn_timer(hWnd);		break;
		case WM_CREATE:		OnCreate(hWnd, wParam, lParam);		break;
		case WM_DESTROY:	OnDestroy(hWnd, wParam, lParam);	break;
		case WM_PAINT:		OnPaint(hWnd, wParam, lParam);		break; 
		case WM_HELP:		OnHelp(hWnd, wParam, lParam);		break;

		case WM_COMMAND: 
			switch (LOWORD(wParam)) {
			// �t�@�C���E���j���[
				case IDM_EXIT:		OnFileExit(hWnd);		break; 
 
			// �w���v�E���j���[
				case IDM_ABOUT:		OnHelp(hWnd, wParam, lParam);		break;

				case IDM_SAVE:		
					save_data(hWnd);
//					action_flag=FALSE;
					break;
				case IDM_LOAD:		
					load_data(hWnd);
					key_kaisou_flag=FALSE;
					action_flag=FALSE;
					select_active=0;
					put_kaisou(hWnd,K_END);
					make_command_bar(hWnd);
					break;
				case IDM_COMPILE_SCENARIO:
					compile_scenario();
					break;
				case IDM_KAISOU_BUTTON:
					if (action_flag!=TRUE) {	
						if (key_kaisou_flag==FALSE) {
							if (!select_flag) {
								key_kaisou_flag=TRUE;
								put_kaisou(hWnd,K_START);
								put_kaisou(hWnd,K_PREV);
							}
						} else {
							key_kaisou_flag=FALSE;
							put_kaisou(hWnd,K_END);
							action_flag=FALSE;
						}
					}
					make_command_bar(hWnd);
					InvalidateRect(hWnd,NULL,FALSE);
					break;
				default:
					return DefWindowProc(hWnd, uMessage, wParam, lParam);
			}

		case WM_KEYDOWN:
			switch (LOWORD(wParam)) {
				case VK_LEFT:
					break;
				case VK_UP:
					if (select_flag) {
						select_active--;
						if (select_active==-1) select_active=sel_num-1;
						select_answer(hWnd,select_active);
						InvalidateRect(hWnd,NULL,FALSE);
						break;
					}
					if (action_flag!=TRUE) {	
						if (key_kaisou_flag==FALSE) {
							key_kaisou_flag=TRUE;
							put_kaisou(hWnd,K_START);
						}
						put_kaisou(hWnd,K_PREV);
						make_command_bar(hWnd);
						InvalidateRect(hWnd,NULL,FALSE);
					}
					break;
				case VK_RIGHT:
					break;
				case VK_DOWN:
					if (select_flag) {
						select_active++;
						if (select_active==sel_num) select_active=0;
						select_answer(hWnd,select_active);
						InvalidateRect(hWnd,NULL,FALSE);
						break;
					}
					if (key_kaisou_flag==FALSE) {
						if (action_flag==TRUE) {
							conv_pic();
							overlap_text(hWnd,0,0,winWidth-1,winHeight-1);
							action_flag=FALSE;
						}
						for (;;) {
							sc_value=read_scenario(hWnd); 
							if (action_mode!=0 && sc_value==SC_PICTURE) continue;
							break;
						}
						if (sc_value==SC_PICTURE) {
							action_flag=TRUE;
							end_tap_pattern(hWnd);
							put_pic();
						}
						if (sc_value==SC_SELECT) {
							select_flag=TRUE;
							select_answer(hWnd,select_active);
							InvalidateRect(hWnd,NULL,FALSE);
						}
					} else {
						put_kaisou(hWnd,K_NEXT);
					}
					InvalidateRect(hWnd,NULL,FALSE);
					break;
				case VK_SPACE:
//				case VK_RETURN:
				case VK_F23:
					if (select_flag) {
						select_flag=FALSE;
						end_answer(hWnd,select_active);
						SendMessage(hWnd,WM_KEYDOWN,0x28,0x1);
						SendMessage(hWnd,WM_KEYUP,0x28,0xC0000001); 
						break;
					}
					if (action_flag==TRUE) { 
						conv_pic();
						overlap_text(hWnd,0,0,winWidth,winHeight);
						InvalidateRect(hWnd,NULL,FALSE);
						start_tap_pattern(hWnd,PAT_PREVIOUS);
						action_flag=FALSE;  
						break;
					} else {
						if (key_kaisou_flag==FALSE) {
							action_flag=TRUE;
							end_tap_pattern(hWnd); 
							put_pic();
//							put_kaisou(hWnd,K_END);
						} else {
							key_kaisou_flag=FALSE;
							put_kaisou(hWnd,K_END);
							make_command_bar(hWnd);
						}
						InvalidateRect(hWnd,NULL,FALSE);
					}
					break;
				case VK_ESCAPE:
					if (esc_flag==TRUE) {
						cbh=0;
						esc_flag=FALSE;
					} else {
						cbh=CommandBar_Height(ghWndCB);
						esc_flag=TRUE;
					}
//					ShowWindow(FindWindow(TEXT("HHTaskBar"),TEXT("")),esc_flag ? SW_SHOW : SW_HIDE),
					CommandBar_Show(ghWndCB,esc_flag);
					InvalidateRect(hWnd,NULL,FALSE);
					break;
				default:
					return DefWindowProc(hWnd, uMessage, wParam, lParam);
			}
			break;
		case WM_LBUTTONDOWN:	mouse_down(hWnd,wParam, lParam);	break;
		case WM_MOUSEMOVE:		mouse_move(hWnd,wParam, lParam);	break;
		case WM_LBUTTONUP:		mouse_up(hWnd,wParam, lParam);		break;

		case WM_INITMENUPOPUP:
			EnableMenuItem(CommandBar_GetMenu( GetDlgItem(hWnd,1), 0),IDM_COMPILE_SCENARIO,
				MF_BYCOMMAND | (compiled_scenario_flag ? MF_GRAYED : MF_ENABLED));
			EnableMenuItem(CommandBar_GetMenu( GetDlgItem(hWnd,1), 0),IDM_SAVE,
				MF_BYCOMMAND | (select_flag | key_kaisou_flag | action_flag ? MF_GRAYED : MF_ENABLED));
			EnableMenuItem(CommandBar_GetMenu( GetDlgItem(hWnd,1), 0),IDM_LOAD,
				MF_BYCOMMAND | (select_flag | key_kaisou_flag | action_flag ? MF_GRAYED : MF_ENABLED));
			break;
		default: 
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
	} 

	return 0;
}

/* =====================================================================
	�E�B���h�E�쐬���̏���
===================================================================== */
void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDC	hDC; 
	RECT rc;
    TCHAR szBuf[8];
	TCHAR		Message[ 256 ] ;
	// �R�}���h�o�[�̐���
//	InitCommonControls();
	ghWndCB = CommandBar_Create(ghInst, hWnd, 1);

	// ��ʂ̃T�C�Y���擾
	cbh=CommandBar_Height(ghWndCB);
	GetWindowRect(hWnd, &rc);
//	winWidth  = rc.right - rc.left;
//	winHeight = rc.bottom - rc.top - cbh; 
	winWidth_dev = GetSystemMetrics( SM_CXSCREEN ) ;
	winHeight_dev = GetSystemMetrics( SM_CYSCREEN ) ;
	winWidth=winWidth_dev;
	winHeight=winHeight_dev;
	if (winWidth<=240)	pspc_flag=TRUE;
	else				pspc_flag=FALSE;
/*	
	if (pspc_flag==FALSE) {
		return;
	}
*/	
	CommandBar_InsertMenubar(ghWndCB, ghInst, IDR_MAIN_MENU, 0);

	CommandBar_AddBitmap( ghWndCB, ghInst, IDR_TOOLBAR,	1,	16, 16 ) ;
    LoadString(ghInst, IDS_KAISOU_BUTTON, (LPTSTR)&szBuf, sizeof(szBuf) - 1);
    tb[1].iString = SendMessage(ghWndCB, TB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuf);
	CommandBar_AddButtons( ghWndCB, sizeof(tb) / sizeof(TBBUTTON), tb) ;
//    SendMessage(ghWndCB, TB_SETIMAGELIST, 0, NULL);

	CommandBar_AddAdornments(ghWndCB, CMDBAR_HELP, 0);

	// �A�C�R���̐ݒ�
	HICON hIcon = (HICON)LoadImage(ghInst, MAKEINTRESOURCE(IDI_MAIN_ICON),
		IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	SendMessage(hWnd, WM_SETICON, FALSE, (WPARAM)hIcon);

	bitspixel=GetDeviceCaps(GetDC(NULL),BITSPIXEL);
//bitspixel=2;
	if (!select_scenario(hWnd)) {
		SendMessage(hWnd, WM_CLOSE, 0, 0L);
		return;
	};
	load_cnf_file();

	rotate_flag=FALSE;
	if (cnf_height>winHeight) 
		if (cnf_height<=winWidth_dev)
			if (cnf_width<=winHeight_dev) {
				rotate_flag=TRUE;
				rotate_offset=cnf_width+(winWidth_dev-cnf_width)/2;
			}


	// �I�t�X�N���[���̍쐬
	bmi.bmih.biWidth = winWidth;
	bmi.bmih.biHeight = winHeight;
	bmi.bmih.biSize = sizeof(bmi.bmih) ;
	bmi.bmih.biPlanes = 1 ;
	bmi.bmih.biBitCount = (bitspixel==2) ? 2 : 24; 
	bmi.bmih.biCompression = BI_RGB ;
	bmi.bmih.biSizeImage = 0 ;
	bmi.bmih.biXPelsPerMeter = 0 ; 
	bmi.bmih.biYPelsPerMeter = 0 ;
	bmi.bmih.biClrUsed = 0 ; 
	bmi.bmih.biClrImportant = 0 ;
	bmi.rgq[0].rgbBlue = bmi.rgq[0].rgbGreen = bmi.rgq[0].rgbRed = 0 ;
	bmi.rgq[1].rgbBlue = bmi.rgq[1].rgbGreen = bmi.rgq[1].rgbRed = 132 ;
	bmi.rgq[2].rgbBlue = bmi.rgq[2].rgbGreen = bmi.rgq[2].rgbRed = 198 ;
	bmi.rgq[3].rgbBlue = bmi.rgq[3].rgbGreen = bmi.rgq[3].rgbRed = 255 ;
	
	hDC = GetDC(NULL);
	baseBmp = CreateDIBSection( hDC, (PBITMAPINFO)&bmi.bmih, DIB_RGB_COLORS,
							 (void **)&baseBuf, NULL, 0 ) ;
	if ( !baseBmp ) {
		wsprintf( Message, TEXT("baseBmp error (%d) %d %d"),GetLastError() ,winWidth_dev,winHeight_dev) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}
	baseDC = CreateCompatibleDC( hDC ) ; 
	if ( !baseDC ) {
		wsprintf( Message, TEXT("baseDC error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}
	SelectObject( baseDC, baseBmp ) ; 
/*
	picBmp = CreateDIBSection( hDC, (PBITMAPINFO)&bmi.bmih, DIB_RGB_COLORS,
							 (void **)&picBuf, NULL, 0 ) ;
	if ( !baseBmp ) {
		wsprintf( Message, TEXT("picBmp error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}
	picDC = CreateCompatibleDC( hDC ) ;
	if ( !picDC ) {
		wsprintf( Message, TEXT("picDC error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	} 
	SelectObject( picDC, picBmp ) ; 
*/
	if (rotate_flag) {
		winWidth=cnf_width;
		winHeight=cnf_height;
		bmi.bmih.biWidth = winWidth;
		bmi.bmih.biHeight = winHeight;
	}
	
	if (bitspixel==2)
		picBuf=(BYTE *)malloc(winWidth*winHeight/4);
	else
		picBuf=(BYTE *)malloc(winWidth*winHeight*3);
	if (picBuf==NULL) {
		wsprintf( Message, TEXT("picBuf error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}

	bmi.bmih.biBitCount = 2; 
	textBmp = CreateDIBSection( hDC, (PBITMAPINFO)&bmi.bmih, DIB_RGB_COLORS,
							 (void **)&textBuf, NULL, 0 ) ;
	if ( !textBmp ) {
		wsprintf( Message, TEXT("textBmp error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}
	textDC = CreateCompatibleDC( hDC ) ;
	if ( !textDC ) {
		wsprintf( Message, TEXT("textDC error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}
	SelectObject( textDC, textBmp ) ; 

	tmpBmp = CreateDIBSection( hDC, (PBITMAPINFO)&bmi.bmih, DIB_RGB_COLORS,
							 (void **)&tmpBuf, NULL, 0 ) ;
	if ( !tmpBmp ) {
		wsprintf( Message, TEXT("tmpBmp error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}
	tmpDC = CreateCompatibleDC( hDC ) ;
	if ( !tmpDC ) {
		wsprintf( Message, TEXT("tmpDC error (%d)"),GetLastError() ) ;
		MessageBox( NULL, Message, TEXT("Error"), MB_OK ) ;
		MessageBeep( MB_ICONASTERISK ) ;
		return  ;
	}
	SelectObject( tmpDC, tmpBmp ) ; 

	action_flag=FALSE;
	action_mode=0;
	sc_value=SC_NEXT;
	key_kaisou_flag=FALSE;

	put_graphic(hWnd,0);
	put_pic();
	fill(textBuf,3);

    SetForegroundWindow(hWnd);

	SendMessage(hWnd,WM_KEYDOWN,0x28,0x1);
	SendMessage(hWnd,WM_KEYUP,0x28,0xC0000001);

//	SetTimer(hWnd, 1000,100,NULL); 
//	start_tap_pattern(hWnd,PAT_NEXT);
}

void make_command_bar(HWND hWnd)
{
	SendMessage(ghWndCB, TB_SETSTATE, IDM_KAISOU_BUTTON, MAKELPARAM((key_kaisou_flag==TRUE ? TBSTATE_PRESSED : 0)|TBSTATE_ENABLED, 0));
}

/* =====================================================================
	��ʂ̕`��
===================================================================== */ 
void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC         hDC;
	RECT		rc;

//	SetFocus(hWnd);
	hDC = BeginPaint(hWnd, &ps);
	GetWindowRect(hWnd, &rc);

	BitBlt(hDC, 0,cbh,winWidth_dev, winHeight_dev, baseDC, 0,0, SRCCOPY);

	EndPaint(hWnd, &ps);
} 
void mouse_down(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int	x,y;
	x = LOWORD(lParam); 
	y = HIWORD(lParam);
	
	if (rotate_flag) {
		int xx,yy;
		xx=x;
		yy=y;
		x=yy;
		y=rotate_offset-xx;
		x-=cbh;
	} else {
		y-=cbh;
	}

	if (select_flag) {
		int i;
		if (text_sx<=x && x<text_sx+text_dx) {
			for (i=0;i<sel_num;i++) {
				if (select_py[i]<=y && y<select_py[i+1]) {
					select_active=i;
					select_answer(hWnd,select_active);
					InvalidateRect(hWnd,NULL,FALSE);
				}
			}
		}
		return;
	}

	if (y<winHeight/2) {
		SendMessage(hWnd,WM_KEYDOWN,0x26,0x1);
		SendMessage(hWnd,WM_KEYUP,0x26,0xC0000001);
	} else {
		SendMessage(hWnd,WM_KEYDOWN,0x28,0x1);
		SendMessage(hWnd,WM_KEYUP,0x28,0xC0000001);
	}
}

void mouse_move(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int	x,y;
	x = LOWORD(lParam);
	y = HIWORD(lParam);

	if (rotate_flag) {
		int xx,yy;
		xx=x;
		yy=y;
		x=yy;
		y=rotate_offset-xx;
		x-=cbh;
	} else {
		y-=cbh;
	}

	if (select_flag) {
		int i;
		if (text_sx<=x && x<text_sx+text_dx) {
			for (i=0;i<sel_num;i++) { 
				if (select_py[i]<=y && y<select_py[i+1]) {
					select_active=i;
					select_answer(hWnd,select_active);
					InvalidateRect(hWnd,NULL,FALSE); 
				} 
			}
		}
	}
}

void mouse_up(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int	x,y;
	x = LOWORD(lParam);
	y = HIWORD(lParam);

	if (rotate_flag) {
		int xx,yy;
		xx=x;
		yy=y;
		x=yy;
		y=rotate_offset-xx;
		x-=cbh;
	} else {
		y-=cbh;
	}

	if (select_flag) {
		int i;
		if (text_sx<=x && x<text_sx+text_dx) {
			for (i=0;i<sel_num;i++) {
				if (select_py[i]<=y && y<select_py[i+1]) {
					select_active=i;
					select_answer(hWnd,select_active);
					InvalidateRect(hWnd,NULL,FALSE);
					SendMessage(hWnd,WM_KEYDOWN,VK_SPACE,0x1);
					SendMessage(hWnd,WM_KEYUP,VK_SPACE,0xC0000001); 
				}
			}
		}
	}
}

/* =====================================================================
	�E�B���h�E�j�����̏���
===================================================================== */

void OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// �R�}���h�o�[�̔j��
	CommandBar_Destroy(ghWndCB);
	PostQuitMessage(0); 
} 

/* =====================================================================
	�w���v�{�^���������ꂽ
===================================================================== */
void OnHelp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	OnHelpAbout(hWnd); 
}

/* =====================================================================
	�t�@�C���E���j���[ - �I��
===================================================================== */
void OnFileExit(HWND hWnd) 
{
	SendMessage(hWnd, WM_CLOSE, 0, 0L);
}

/* =====================================================================
	�w���v�E���j���[ - �o�[�W�������̕\��
===================================================================== */
void OnHelpAbout(HWND hWnd)
{
	DialogBox(ghInst, MAKEINTRESOURCE(IDD_ABOUT_DIALOG),
		hWnd, AboutDlgProc);

}

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT uMessage,
						   WPARAM wParam, LPARAM lParam)
{
	switch (uMessage) {
	  case WM_INITDIALOG:			// �_�C�A���O�E�{�b�N�X�̏�����
		SetDlgItemText (hDlg, IDC_ABOUT_TITLE, about_title_str);
		SetDlgItemText (hDlg, IDC_ABOUT_AUTHOR, about_author_str);
		SetDlgItemText (hDlg, IDC_ABOUT_VERSION, about_version_str);

		SetFocus(GetDlgItem(hDlg, IDOK));	// OK�{�^���Ƀt�H�[�J�X��ݒ�
		return FALSE;				// �t�H�[�J�X��ݒ肵������FALSE��Ԃ�

	  case WM_COMMAND:				// �R�}���h���󂯎����
		switch (wParam) {
		  case IDOK:				// [OK]�{�^���������ꂽ
		  case IDCANCEL:			// [����]���I�����ꂽ
			EndDialog(hDlg, TRUE);	// �^�C�A���O�E�{�b�N�X�����
			return TRUE;
		}
		break;
	}
	return FALSE;	// ���b�Z�[�W���������Ȃ������ꍇ��FALSE��Ԃ�
}
/*
void SelectScenario(HWND hWnd)
{
	DialogBox(ghInst, MAKEINTRESOURCE(IDD_FILE),
		hWnd, SelectScenarioDlgProc);
}

BOOL CALLBACK SelectScenarioDlgProc(HWND hDlg, UINT uMessage,
						   WPARAM wParam, LPARAM lParam)
{
	switch (uMessage) {
		case WM_INITDIALOG:			// �_�C�A���O�E�{�b�N�X�̏�����
			



			SetFocus(GetDlgItem(hDlg, IDOK));	// OK�{�^���Ƀt�H�[�J�X��ݒ�
			return FALSE;				// �t�H�[�J�X��ݒ肵������FALSE��Ԃ�

	  case WM_COMMAND:				// �R�}���h���󂯎����
		switch (wParam) {
		  case IDOK:				// [OK]�{�^���������ꂽ
		  case IDCANCEL:			// [����]���I�����ꂽ
			EndDialog(hDlg, TRUE);	// �^�C�A���O�E�{�b�N�X�����
			return TRUE;
		}
		break;
	}
	return FALSE;	// ���b�Z�[�W���������Ȃ������ꍇ��FALSE��Ԃ�
}
*/

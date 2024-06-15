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
	関数のプロトタイプ宣言
===================================================================== */
// WinMainから呼ばれる関数
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);

// コールバック関数
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

// ダイアログ関数
BOOL CALLBACK SelectScenarioDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PropertyDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK TreeDlgProc(HWND, UINT, WPARAM, LPARAM);

// メッセージ処理関数
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

// ファイル・メニュー関数
void OnFileExit(HWND);			// 終了
// ヘルプ・メニュー関数
void OnHelpAbout(HWND);			// バージョン情報

/* =====================================================================
	グローバル変数
===================================================================== */
TCHAR gszAppName[]	= TEXT("JZNovel");	// クラスの名前
TCHAR gszAppTitle[]	= TEXT("JZNovel");	// ウィンドウ・タイトル

HINSTANCE ghInst	= NULL;		// 現在のインスタンス
HWND      ghWnd		= NULL;		// メインウィンドウのハンドル
HWND      ghWndCB	= NULL;		// コマンドバーのハンドル
HACCEL	  hAccel    = NULL;

int	cbh; 

HDC		baseDC	= NULL;		// オフスクリーン
HBITMAP	baseBmp	= NULL;
BYTE	*baseBuf;

HDC		picDC	= NULL;		// オフスクリーン
HBITMAP	picBmp	= NULL;
BYTE	*picBuf;
 
HDC		textDC	= NULL;		// オフスクリーン
HBITMAP	textBmp	= NULL;
BYTE	*textBuf;

HDC		tmpDC	= NULL;		// オフスクリーン
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
	Windowsプログラムのメイン関数
===================================================================== */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					 LPTSTR lpszCmdParm, int nCmdShow)
{
	MSG    msg;

    // ウィンドウの検索
    HWND hWnd = FindWindow(gszAppName, gszAppTitle);
    if (hWnd) {
        // 既に起動しているウィンドウを最前面に移動して終了
        SetForegroundWindow(hWnd);
        return 0;
    }
 
	ghInst = hInstance;

	if (!hPrevInstance) {					// ほかのインスタンス実行中 ?
		if (!InitApplication(hInstance))	// 共通の初期化処理
			return FALSE;					// 初期化に失敗した場合は終了
	}
	if (!InitInstance(hInstance, nCmdShow))	// インスタンス毎の初期化処理
		return FALSE;						// 失敗した場合は終了

	hAccel=LoadAccelerators(ghInst,MAKEINTRESOURCE(IDR_ACCEL));
	while (GetMessage(&msg, NULL, 0, 0)) {	// メッセージの取得とディスパッチ
		if (!TranslateAccelerator(msg.hwnd,hAccel,&msg)) {
			TranslateMessage(&msg);				// 仮想キーコードの変換
			DispatchMessage(&msg);				// メッセージのディスパッチ
		}
	}
	return msg.wParam;		// PostQuitMessage()関数の戻り値を返す
}

/* =====================================================================
	ウィンドウ・クラスの登録
===================================================================== */
BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style		 = 0;					// クラス・スタイル
	wc.lpfnWndProc	 = MainWndProc;			// ウィンドウ・プロシージャ
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;			// インスタンス・ハンドル
	wc.hIcon		 = NULL;
	wc.hCursor		 = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName	 = NULL;				// メニューの名前
	wc.lpszClassName = gszAppName;			// ウィンドウ・クラスの名前

	return RegisterClass(&wc);	// ウィンドウ・クラスの登録
}

/* =====================================================================
	ウィンドウの作成と表示
===================================================================== */
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ghWnd = CreateWindow( 
 		gszAppName,				// 登録されたウィンドウ・クラスの名前
		gszAppTitle,			// タイトル・バーに表示するテキスト
		WS_VISIBLE,				// ウィンドウ・スタイル
		CW_USEDEFAULT,			// ウィンドウの表示位置 (水平)
		CW_USEDEFAULT,			//                      (垂直)
		CW_USEDEFAULT,			// ウィンドウの大きさ   (幅)
		CW_USEDEFAULT,			//                      (高さ)
		NULL,					// 親ウィンドウのハンドル
		NULL,					// ウィンドウ・クラスのメニューを使用
		hInstance,				// アプリケーション・インスタンスのハンドル
		NULL					// ウィンドウ作成データのアドレス
	);
	if (!ghWnd)
		return FALSE;			// ウィンドウの作成に失敗

	ShowWindow(ghWnd, nCmdShow);	// ウィンドウ表示状態の設定
	UpdateWindow(ghWnd);			// クライアント領域の更新

	return TRUE;
}

/* =====================================================================
	ウィンドウ・プロシージャ
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
			// ファイル・メニュー
				case IDM_EXIT:		OnFileExit(hWnd);		break; 
 
			// ヘルプ・メニュー
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
	ウィンドウ作成時の処理
===================================================================== */
void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDC	hDC; 
	RECT rc;
    TCHAR szBuf[8];
	TCHAR		Message[ 256 ] ;
	// コマンドバーの生成
//	InitCommonControls();
	ghWndCB = CommandBar_Create(ghInst, hWnd, 1);

	// 画面のサイズを取得
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

	// アイコンの設定
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


	// オフスクリーンの作成
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
	画面の描画
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
	ウィンドウ破棄時の処理
===================================================================== */

void OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// コマンドバーの破棄
	CommandBar_Destroy(ghWndCB);
	PostQuitMessage(0); 
} 

/* =====================================================================
	ヘルプボタンが押された
===================================================================== */
void OnHelp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	OnHelpAbout(hWnd); 
}

/* =====================================================================
	ファイル・メニュー - 終了
===================================================================== */
void OnFileExit(HWND hWnd) 
{
	SendMessage(hWnd, WM_CLOSE, 0, 0L);
}

/* =====================================================================
	ヘルプ・メニュー - バージョン情報の表示
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
	  case WM_INITDIALOG:			// ダイアログ・ボックスの初期化
		SetDlgItemText (hDlg, IDC_ABOUT_TITLE, about_title_str);
		SetDlgItemText (hDlg, IDC_ABOUT_AUTHOR, about_author_str);
		SetDlgItemText (hDlg, IDC_ABOUT_VERSION, about_version_str);

		SetFocus(GetDlgItem(hDlg, IDOK));	// OKボタンにフォーカスを設定
		return FALSE;				// フォーカスを設定した時はFALSEを返す

	  case WM_COMMAND:				// コマンドを受け取った
		switch (wParam) {
		  case IDOK:				// [OK]ボタンが押された
		  case IDCANCEL:			// [閉じる]が選択された
			EndDialog(hDlg, TRUE);	// タイアログ・ボックスを閉じる
			return TRUE;
		}
		break;
	}
	return FALSE;	// メッセージを処理しなかった場合はFALSEを返す
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
		case WM_INITDIALOG:			// ダイアログ・ボックスの初期化
			



			SetFocus(GetDlgItem(hDlg, IDOK));	// OKボタンにフォーカスを設定
			return FALSE;				// フォーカスを設定した時はFALSEを返す

	  case WM_COMMAND:				// コマンドを受け取った
		switch (wParam) {
		  case IDOK:				// [OK]ボタンが押された
		  case IDCANCEL:			// [閉じる]が選択された
			EndDialog(hDlg, TRUE);	// タイアログ・ボックスを閉じる
			return TRUE;
		}
		break;
	}
	return FALSE;	// メッセージを処理しなかった場合はFALSEを返す
}
*/

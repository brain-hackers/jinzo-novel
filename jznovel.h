extern HINSTANCE ghInst;
extern HWND      ghWnd;
extern HWND      ghWndCB;

typedef struct {
	BITMAPINFOHEADER	bmih ;
	RGBQUAD	rgq[ 256 ] ;
} BMI ;
extern BMI	bmi;
extern BITMAPFILEHEADER	BmpFH ;
extern BITMAPINFOHEADER	BmpIH ;

extern int	winWidth,winHeight;
extern int	winWidth_dev,winHeight_dev;
extern BYTE *baseBuf,*picBuf,*textBuf,*tmpBuf;
extern HDC	baseDC,picDC,textDC,tmpDC;
extern int		bitspixel; 

#define CNF_FILE_NAME	TEXT("JZNovel.txt")
extern void load_cnf_file(void);
extern int	cnf_width,cnf_height;

extern int	cbh;
extern int	rotate_offset;

extern int	text_sx,text_dx,text_sy,text_dy;
extern int	select_py[];

typedef enum {
	SC_END,SC_WAIT,SC_NEXT,SC_PICTURE,SC_SELECT
} SC_EVENT;
extern SC_EVENT	sc_value;

typedef enum {
	K_PREV,K_NEXT,K_START,K_END
} KAISOU_MODE;

extern BOOL key_kaisou_flag; 
extern BOOL	rotate_flag;

extern void load_picture(HWND,int);
extern void	set_pixel(BYTE *,int,int,BYTE);
extern void	set_pixel_r(BYTE *,int,int,BYTE);
extern BYTE	get_pixel(BYTE *,int,int); 
extern void	set_pixel24(BYTE *,int,int,int,int,int);
extern void	set_pixel24_r(BYTE *,int,int,int,int,int);
extern int	get_pixel24(BYTE *,int,int *,int *,int *);
extern BYTE  mono_color(int,int,int); 
extern BYTE  mono_color255(int,int,int); 
extern void	conv_pic(void);
extern void fill(BYTE *,BYTE);
extern void fill24(BYTE *,int,int,int);
extern void fill_r(BYTE *,BYTE);
extern void fill24_r(BYTE *,int,int,int);
extern void put_pic(void);
extern void overlap_text(HWND,int,int,int,int);
extern void put_graphic(HWND,int);

void init_scenario_file();
SC_EVENT	read_scenario(HWND);
extern void put_kaisou(HWND,KAISOU_MODE);
extern TCHAR	scenario_filename[MAX_PATH];

#define MAX_PICTURE	100
extern TCHAR	picture_name[MAX_PICTURE][MAX_PATH];
extern int		picture_x[MAX_PICTURE],picture_y[MAX_PICTURE];
extern int		picture[MAX_PICTURE][4];

extern void		save_data(HWND);
extern void		load_data(HWND);
extern TCHAR	save_data_fn[MAX_PATH];

extern BYTE tap_pattern[4][2][8][8];
typedef enum {
	PAT_NONE,PAT_WAIT,PAT_NEXT,PAT_KAISOU,PAT_PREVIOUS
} PAT_PIC;
void locate_tap(int,int);
extern void put_tap_pattern(HWND,PAT_PIC,int);
extern void start_tap_pattern(HWND,PAT_PIC);
extern void end_tap_pattern(HWND);
extern PAT_PIC	pattern_pic;
extern int	pattern_mode;

extern int	pic_color[2][3];
extern int bg_color[2][3];
extern int	text_color[2][3];
extern int	action_mode;
extern BOOL action_flag;

extern TCHAR	about_title_str[128];
extern TCHAR	about_author_str[128];
extern TCHAR	about_version_str[128];

extern BOOL		select_scenario(HWND);
extern int hex_char(TCHAR);

extern void	compile_scenario(void);
extern BYTE	*scenario;
extern int	scenario_size;
extern BOOL	compiled_scenario_flag;

extern BYTE	bw_pic_color[4];
extern BYTE	bw_text_color[2];
extern BYTE	bw_bg_color[2];

extern int	put_button(TCHAR *,int,int,int,BOOL);
extern int		sel_num,select_active;
extern void	select_answer(HWND,int);
extern void	end_answer(HWND,int);
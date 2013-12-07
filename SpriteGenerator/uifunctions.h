#ifndef _UIFUNCTIONS_H_
#define _UIFUNCTIONS_H_
#undef UNICODE

#include "wysaid.h"

#define BGFILENAME "bg.png"

extern const char* g_menuItems[];
int GetFileNameDlg(HWND hwnd, char filenames[]);
bool SaveFileNameDlg(HWND hwnd, LPSTR filename);

class JigsawView
{
public:
    JigsawView():m_uiHwnd(NULL), m_pBGImg(NULL), m_width(0), m_height(0), m_hScroll(0), m_vScroll(0), m_toolBarScroll(0), m_bIsRDown(false), m_bSmoothMode(false), m_bShowBorder(true), m_bShowBoundary(true),m_bShowTips(true) {}
    ~JigsawView(){ closegraph(); }

    bool init(int w, int h);
    void resize(int w, int h);
    void keyFunc();
    void cursorFunc();
    void update();

    void renderToolBar();
    void render();
    void toolBarScroll(int);
    void toolBarClick(int);
    void moveImage(int, int);
    void addImages(const char*, int);
    bool getSmoothMode(){return m_bSmoothMode;}

private:

    void runSpecialCode1();

    HWND m_uiHwnd;
    PIMAGE m_pBGImg;
    ChunkManager m_chunks;
	int m_bgImgWidth, m_bgImgHeight;
    int m_width, m_height;
    int m_hScroll, m_vScroll;
    int m_toolBarScroll;
	int m_x, m_y;
    bool m_bIsRDown, m_bIsLDown;
    bool m_bSmoothMode, m_bShowBorder;
    bool m_bShowBoundary, m_bShowTips;
};

LRESULT CALLBACK ShowConfDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

bool ExtractResource(LPCSTR strDstFile, LPCSTR strResType, LPCSTR strResName);

#endif
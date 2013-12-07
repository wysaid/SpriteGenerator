#include "uifunctions.h"

const char* g_menuItems[] = 
{
    "Add Image",
    "Delete Image",
    "Clear Images",    
    "Show Conf",
    "Smooth Mode",
    "Hide/Show Border",
    "Hide/Show Bound",
    "Auto Align",
    "Save only image",
    "Save with css",
    "Save with xml",
    "Save with text",
    "Maxmize window",
    "Resize window",
    "Hide/Show tips",
};

extern int g_relativeX;
extern int g_relativeY;

const ChunkManager* g_chunkManager;

bool JigsawView::init(int w, int h)
{
    initgraph(w, h);
    char buffer[BUFFER_SIZE];
    GetTempPathA(BUFFER_SIZE, buffer);
    strcat(buffer, "background.png");
	if(access(buffer, 0))
		ExtractResource(buffer, "png", MAKEINTRESOURCEA(IDB_BGPNG));
	m_pBGImg = newimage();
    if(getimage_pngfile(m_pBGImg, buffer))
    {
        return false;
    }
    m_bgImgWidth = getwidth(m_pBGImg);
    m_bgImgHeight = getheight(m_pBGImg);
    m_width = w;
    m_height = h;
    m_uiHwnd = getHWnd();
    g_chunkManager = &m_chunks;
    return true;
}

void JigsawView::resize(int w, int h)
{
    initgraph(w, h);
    m_uiHwnd = getHWnd();
}

void JigsawView::keyFunc()
{
    int c;
    if(kbhit()) c = getch();
    else return;
    switch(c)
    {
    case ' ':
        ;
    case '\r':
        g_relativeX = TOOLBARWIDTH;
        g_relativeY = 0;
        render();
        break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        m_chunks.widenChunkWhoseLength(c-'0');
        render();
        break;
    case 's':case 'S':
        m_chunks.specialAlign();
        render();
        break;
/*    case 'r':case 'R':
        setcaption("Running Spacial Code!");
        runSpecialCode1();
        break;*/
    case 27:
        exit(0);
        break;
    default:;
    }
}

void JigsawView::cursorFunc()
{
    MOUSEMSG m;
    static int preX, preY;
    static bool isTracking = false;
    if(mousemsg())
    {
        do 
        {
            m = GetMouseMsg();
        } while (!m_bSmoothMode && mousemsg() && m.uMsg == WM_MOUSEMOVE);
    }
    else return;
    if(m.x < TOOLBARWIDTH && !isTracking)
    {
        if(m.uMsg == WM_LBUTTONUP)
        {
            m_bIsLDown = false;
            toolBarClick(m.y);
        }
        else if(m.uMsg == WM_LBUTTONDOWN)
            m_bIsLDown = true;
        else if(m.uMsg == WM_MOUSEWHEEL)
            toolBarScroll(m.wheel);
        else if(m.uMsg == WM_RBUTTONUP)
            m_bIsRDown = false;
    }
    else
    {
        if(m.uMsg == WM_LBUTTONUP)
        {
            if(isTracking) ;
            isTracking = false;
        }
        else if(m.uMsg == WM_LBUTTONDOWN)
        {
            preX = m.x;
            preY = m.y;
            if(m_chunks.picTest(m.x - g_relativeX, m.y - g_relativeY) != NULL)
                isTracking = true;
        }
        else if(m.uMsg == WM_RBUTTONUP)
        {
            m_bIsRDown = false;
        }
        else if(m.uMsg == WM_RBUTTONDOWN)
        {
            m_bIsRDown = true;
            preX = m.x;
            preY = m.y;
        }
        if(m.uMsg == WM_MOUSEMOVE && m_bIsRDown)
        {
            moveImage(m.x - preX, m.y - preY);
            preX = m.x;
            preY = m.y;            
        }
        else if(isTracking)
        {
            m_chunks.moveActiveImage(m.x - preX, m.y - preY);
            preX = m.x;
            preY = m.y;
            render();
        }
    }
}

void JigsawView::update()
{
    cursorFunc();
    keyFunc();
}

void JigsawView::renderToolBar()
{
    bar(0, 0, TOOLBARWIDTH, m_height);
    int i = m_toolBarScroll % TOOLITEMHEIGHT;
    int index = 0;
    char buffer[BUFFER_SIZE];
    for(; i < m_height && index < sizeof(g_menuItems) / sizeof(*g_menuItems); i += TOOLITEMHEIGHT)
    {
        sprintf(buffer, "%s", g_menuItems[index++]);
        outtextxy(5, i + 10, buffer);
        line(0, i + TOOLITEMHEIGHT, TOOLBARWIDTH, i + TOOLITEMHEIGHT);
    }
}

void JigsawView::render()
{
    for(int j=m_y<0?m_y%m_bgImgHeight:m_y%m_bgImgHeight-m_bgImgHeight; j < m_height; j += m_bgImgHeight)
    {
        for(int i=m_x<0?m_x%m_bgImgWidth+TOOLBARWIDTH:m_x%m_bgImgWidth+TOOLBARWIDTH-m_bgImgWidth; i < m_width; i += m_bgImgWidth)
        {
            putimage(i, j, m_pBGImg);
        }
    }
    setcolor(0);
    if(m_bShowTips) outtextrect(g_relativeX + 300, g_relativeY + 250, 500, 500, "ʹ��˵��: \r\nʹ��������������˵�ѡ����\r\nʹ������Ҽ������϶��������λ��\r\nʹ�����������������϶�ͼƬԪ�ص�λ��\r\n�������UI�������Ϊ����ʹ��C++�������ʵ�ֶ��ǵ��������ṩ��ʹ�ù����г��������뷴�������Ա㼰ʱ����!\r\n�������:\r\n�������ṩ���СͼƬ��ƴ�ӣ������������ļ���\r\n��ݼ�:�������ּ�1~9���Խ���/��Ϊ1��ͼƬ�ĳ�/������1��ֻ��һ�����ص�ͼƬ�򳤿�һ�����ӡ�");
    setcolor(0xff0000);
    m_chunks.render(m_bShowBorder);
    setcolor(GREEN);
    line(TOOLBARWIDTH, g_relativeY, m_width, g_relativeY);
    line(g_relativeX, 0, g_relativeX, m_height);    
    outtextxy(g_relativeX-10, g_relativeY-5, "(0,0)");
    setcolor(BLUE);
    if(m_bShowBoundary)
        m_chunks.renderBound();
    setcolor(WHITE);
    renderToolBar();
}

void JigsawView::toolBarScroll(int n)
{

}

void JigsawView::toolBarClick(int index)
{
    char buffer[BUFFER_SIZE];
    int n;
    index += abs(m_toolBarScroll);
    index /= TOOLITEMHEIGHT;
    switch(index)
    {
    case 0:
        if((n = GetFileNameDlg(m_uiHwnd, buffer)) > 0)
        {
            addImages(buffer, n);
        }
        break;
    case 1:
        m_chunks.delTopImage();
        break;
    case 2:
        m_chunks.clearImage();
        break;
    case 3:
        {
            DialogBoxA(getHInstance(), (LPCSTR)IDD_SHOWCONF, m_uiHwnd, (DLGPROC)ShowConfDlg);
        }
        break;
    case 4:
        if(m_bSmoothMode = !m_bSmoothMode)
            setcaption("Smooth mode is enabled!");
        else setcaption("Smooth mode is disabled!");
        break;
    case 5:
        if(m_bShowBorder = !m_bShowBorder)
            setcaption("Border is enabled!");
        else setcaption("Border is disabled!");
        break;
    case 6:
        if(m_bShowBoundary = !m_bShowBoundary)
            setcaption("Boundary is enabled!");
        else setcaption("Boundary is disabled!");
        break;
    case 7:
        m_chunks.autoAlign();
        break;
    case 8:
        if(!SaveFileNameDlg(m_uiHwnd, buffer))
            break;
        if(m_chunks.saveOnlyImage(buffer))
            MessageBoxA(m_uiHwnd, "Successfully saved!", "Congratulations", MB_OK);
        else MessageBoxA(m_uiHwnd, "Failed to save! \r\nCheck if there is any pictures above the canvas!", "Attention", MB_OK);
        break;
    case 9:
        if(!SaveFileNameDlg(m_uiHwnd, buffer))
            break;
        if(m_chunks.saveConfiguration(buffer, ChunkManager::CSS))
            MessageBoxA(m_uiHwnd, "Successfully saved!", "Congratulations", MB_OK);
        else MessageBoxA(m_uiHwnd, "Failed to save! \r\nCheck if there is any pictures above the canvas!", "Attention", MB_OK);
        break;
    case 10:
        if(!SaveFileNameDlg(m_uiHwnd, buffer))
            break;
        if(m_chunks.saveConfiguration(buffer, ChunkManager::XML))
            MessageBoxA(m_uiHwnd, "Successfully saved!", "Congratulations", MB_OK);
        else MessageBoxA(m_uiHwnd, "Failed to save! \r\nCheck if there is any pictures above the canvas!", "Attention", MB_OK);
        break;
    case 11:
        if(!SaveFileNameDlg(m_uiHwnd, buffer))
            break;
        if(m_chunks.saveConfiguration(buffer, ChunkManager::TEXT))
            MessageBoxA(m_uiHwnd, "Successfully saved!", "Congratulations", MB_OK);
        else MessageBoxA(m_uiHwnd, "Failed to save! \r\nCheck if there is any pictures above the canvas!", "Attention", MB_OK);
        break;
    case 12:
        {
            int width = GetSystemMetrics(SM_CXSCREEN);
            int height = GetSystemMetrics(SM_CYSCREEN);
            width -= 100;
            height -= 100;
            resize(width, height);
            m_width = width;
            m_height = height;
        }
        break;
    case 13:
        resize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
        m_width = DEFAULT_WINDOW_WIDTH;
        m_height = DEFAULT_WINDOW_HEIGHT;
        break;
    case 14:
        m_bShowTips = !m_bShowTips;
        break;
    default:;
    }
    render();
}

void JigsawView::moveImage(int x, int y)
{
    m_x += x;
    m_y += y;
    g_relativeX += x;
    g_relativeY += y;
    render();
}

void JigsawView::addImages(const char* openFilenames, int offset)
{
    char buffer[BUFFER_SIZE];
    vector<string> vecBuffer;
    strcpy(buffer, openFilenames);
    buffer[offset] = 0;
    if(buffer[offset-1] != '\\' && buffer[offset-1] != '/')
        buffer[offset-1] = '\\';
    buffer[offset] = 0;
    openFilenames += offset;
    while(*openFilenames)
    {
        string tmp = buffer;
        tmp += openFilenames;
        while(*openFilenames++);
        vecBuffer.push_back(tmp);
    }
    m_chunks.addChunks(vecBuffer);
}

void JigsawView::runSpecialCode1()
{
	m_chunks.clearImage();
    char readFolderPath[BUFFER_SIZE];
    char saveFolderPath[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    getcwd(readFolderPath, BUFFER_SIZE);
    strcpy(saveFolderPath, readFolderPath);
    sprintf(buffer, "��ѡ��������ļ��У���ǰ:%s", readFolderPath);
    if(GetFolderPathDlg(m_uiHwnd, readFolderPath, buffer))
        chdir(readFolderPath);
    if(!SaveFolderPathDlg(m_uiHwnd, saveFolderPath, "��ѡ������ļ��У���ѡ��ȡ�������������������ͬĿ¼��"))
    {
        setcaption("δ��������ļ��У�Ĭ�������������ͬĿ¼��!");
        strcpy(saveFolderPath, readFolderPath);
    }    

    ChunkManager::SaveDataKind kind = ChunkManager::TEXT;
    
    for(int i=0; i != 10000; ++i)
    {
        sprintf(buffer, "%dM_0.png", i);
        if(access(buffer, 0))
            continue;
        for(int j=0; j != 8; ++j)
        {
            sprintf(buffer, "%dM_%d.png", i, j);
            if(!m_chunks.addChunk(buffer))
            {
                strcat(buffer, " Not exist!");
                m_chunks.clearImage();
                cleardevice();
                outtextxy(100, 100, buffer);
                getch();
                break;
            }
        }
        render();
        Sleep(100);
        m_chunks.specialAlign();
        render();
        Sleep(100);
        sprintf(buffer, "%s\\%dM.png",saveFolderPath , i);
        if(m_chunks.saveConfiguration(buffer, kind))
        {
            strcat(buffer, " Success!");
            setcaption(buffer);
        }
        else 
        {
            strcat(buffer, " Failed!");
            setcaption(buffer);
        }
        m_chunks.clearImage();
        Sleep(100);
    }
}

LRESULT CALLBACK ShowConfDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_RADIOCSS:
            if(g_chunkManager == NULL) break;
            {
                string s;
                g_chunkManager->getDataCSS(s);
                SetDlgItemTextA(hDlg, IDC_RESULTVIEW, s.c_str());
            }
            break;
        case IDC_RADIOXML:
            if(g_chunkManager == NULL) break;
            {
                string s;
                g_chunkManager->getDataXML(s);
                SetDlgItemTextA(hDlg, IDC_RESULTVIEW, s.c_str());
            }
            break;
        case IDC_RADIOCUSTOM:
            if(g_chunkManager == NULL) break;
            {
                string s;
                g_chunkManager->getDataText(s);
                SetDlgItemTextA(hDlg, IDC_RESULTVIEW, s.c_str());
            }
            break;
        case IDCANCEL:
        case IDOK:
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        default:;
        }
    }
    return FALSE;
}

bool ExtractResource(LPCSTR strDstFile, LPCSTR strResType, LPCSTR strResName)
{
    HANDLE hFile = ::CreateFile(strDstFile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;
    HRSRC	hRes	= FindResource(NULL, strResName, strResType);
    HGLOBAL	hMem	= LoadResource(NULL, hRes);
    DWORD	dwSize	= SizeofResource(NULL, hRes);
    DWORD dwWrite = 0;
    ::WriteFile(hFile, hMem, dwSize, &dwWrite, NULL);
    ::CloseHandle(hFile);
    return true;
}
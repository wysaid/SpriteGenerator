#include "wysaid.h"

int g_relativeX = TOOLBARWIDTH;
int g_relativeY = 0;

string Chunk::trimName(LPCSTR str)
{
    string s(str);
    string::size_type t1 = s.find_last_of('\\');
    string::size_type t2 = s.find_last_of('.');
    if(t1 != string::npos && t2 != string::npos && t1 < t2)
        return s.substr(t1+1, t2 - t1-1);
    else if(t2 != string::npos)
        return s.substr(0, t2);
    return string("image");
}

bool Chunk::init(LPCSTR filename, int left, int top)
{
    int index = strlen(filename);
    const char* tmp = filename + index - 4;
    if(index <= 4)
        return false;
    int status = 0;
    delimage(m_pimg);
    m_pimg = newimage();
    if(strcmp(tmp, ".png") == 0)
        status = getimage_pngfile(m_pimg, filename);
    else 
        status = getimage(m_pimg, filename);
    if(status != 0) return false;
    setAlpha(m_pimg);
    m_data.name = trimName(filename);
    m_data.left = left;
    m_data.top = top;
    m_data.width = getwidth(m_pimg);
    m_data.height = getheight(m_pimg);
    return true;
}

void Chunk::setAlpha(PIMAGE pimg)
{
    int size = getwidth(pimg) * getheight(pimg);
	int i;
    color_t* buffer = (color_t*)getbuffer(pimg);
    for(i=0; i != size; ++i)
    {
        if(buffer[i] >> 24 != 0)
            return;
    }
    for(i=0; i != size; ++i)
    {
        buffer[i] |= 0xff000000;
    }
}

void Chunk::move(int x, int y)
{
    m_data.left += x;
    m_data.top += y;
}

void Chunk::resize(float fdx, float fdy)
{
    int width = m_data.width * fdx;
    int height = m_data.height * fdy;
    PIMAGE pimg;
    pimg = newimage(width, height);
    wy_putimage_scale_alpha(pimg, m_pimg, 0, 0);
    delimage(m_pimg);
    m_pimg = pimg;
    m_data.width = width;
    m_data.height = height;
}

void Chunk::render(bool showBorder)
{
    putimage_withalpha(NULL, m_pimg, g_relativeX + m_data.left,g_relativeY + m_data.top);
    if(showBorder)
    {
        outtextxy(g_relativeX + m_data.left, g_relativeY+m_data.top, m_data.name.c_str());
        moveto(g_relativeX + m_data.left, g_relativeY + m_data.top);
        linerel(m_data.width, 0);
        linerel(0, m_data.height);
        linerel(-m_data.width, 0);
        linerel(0, -m_data.height);
    }
}

ChunkManager::~ChunkManager()
{
    for(std::vector<Chunk*>::iterator iter = m_vecChunks.begin(); iter != m_vecChunks.end(); ++iter)
    {
        delete *iter;
    }
}

bool ChunkManager::addChunks(std::vector<string> filenames, int dX, int dY)
{
    int x = 0, y = 0;
    for(std::vector<string>::iterator iter = filenames.begin(); iter != filenames.end(); ++iter)
    {
        if(!addChunk(iter->c_str(), (x+=dX), y+=dY))
            return false;
    }
    return true;
}

bool ChunkManager::addChunk(LPCSTR filename, int x, int y)
{
    Chunk* p = new Chunk();
    bool b = p->init(filename, x, y);
    if(b) m_vecChunks.push_back(p);
    return b;
}

void ChunkManager::move(int x, int y)
{

}

void ChunkManager::moveActiveImage(int x, int y)
{
    if(m_activeChunk == NULL) return;
    ImageData* pData = m_activeChunk->getData();
    pData->left += x;
    pData->top += y;
}

void ChunkManager::delTopImage()
{
    m_activeChunk = NULL;
    if(!m_vecChunks.empty())
    {
		std::vector<Chunk*>::iterator iter = m_vecChunks.end() - 1;
        delete *iter;
        m_vecChunks.erase(iter);
    }
}

void ChunkManager::clearImage()
{

    m_activeChunk = NULL;
    for(std::vector<Chunk*>::iterator iter = m_vecChunks.begin(); iter != m_vecChunks.end(); ++iter)
    {
        delete *iter;
    }
    m_vecChunks.clear();
}

void ChunkManager::render(bool showBorder)
{
    for(std::vector<Chunk*>::iterator iter = m_vecChunks.begin(); iter != m_vecChunks.end(); ++iter)
    {
        (*iter)->render(showBorder);
    }
    if(m_activeChunk == NULL) return;
    ImageData* pData = m_activeChunk->getData();
    line(g_relativeX + pData->left, g_relativeY + pData->top, g_relativeX + pData->left + pData->width, g_relativeY + pData->top + pData->height);
    line(g_relativeX + pData->left, g_relativeY + pData->top + pData->height, g_relativeX + pData->left + pData->width, g_relativeY + pData->top);
}

void ChunkManager::renderBound()
{
    RECT rt = getChunkBoundary();
    rt.left += g_relativeX;
    rt.right += g_relativeX;
    rt.top += g_relativeY;
    rt.bottom += g_relativeY;
    line(rt.left, rt.top, rt.right, rt.top);
    line(rt.left, rt.bottom, rt.right, rt.bottom);
    line(rt.left, rt.top, rt.left, rt.bottom);
    line(rt.right, rt.top, rt.right, rt.bottom);
}

RECT ChunkManager::getChunkBoundary()const
{
    RECT rt = {0x7fffffff,0x7fffffff,0xffffffff,0xffffffff};

    for(std::vector<Chunk*>::const_iterator iter = m_vecChunks.begin(); iter != m_vecChunks.end(); ++iter)

    {
        ImageData* pData = (*iter)->getData();

        if(pData->left < rt.left)
            rt.left = pData->left;
        if(pData->top < rt.top)
            rt.top = pData->top;
        if(pData->left + pData->width > rt.right)
            rt.right = pData->left + pData->width;
        if(pData->top + pData->height > rt.bottom)
            rt.bottom = pData->top + pData->height;
    }
    return rt;
}

bool ChunkManager::saveConfiguration(LPCSTR filename, SaveDataKind kind)
{
    switch(kind)
    {
    case TEXT:
        if(!saveDataAsText(filename))
            return false;
        break;
    case XML:
        if(!saveDataAsXML(filename))
            return false;
        break;
    case CSS:
        if(!saveDataAsCSS(filename))
            return false;
        break;
    default:
        return false;
    }
    return saveOnlyImage(filename);
}

Chunk* ChunkManager::picTest(int x, int y)
{
    for(std::vector<Chunk*>::reverse_iterator riter = m_vecChunks.rbegin(); riter != m_vecChunks.rend(); ++riter)
    {
        ImageData* pData = (*riter)->getData();
        if(x >= pData->left - 1 && x <= pData->left+pData->width +1 && y >= pData->top - 1 && y <= pData->top + pData->height + 1)
        {
            m_activeChunk = *riter;
            if(*riter != *m_vecChunks.rbegin())
            {
                m_vecChunks.erase(m_vecChunks.begin() + (m_vecChunks.rend()-1-riter));
                m_vecChunks.push_back(m_activeChunk);
            }            
            return m_activeChunk;
        }
    }
    return m_activeChunk = NULL;
}

void ChunkManager::autoAlign()
{
    int cnt = m_vecChunks.size();
    if(cnt == 0) return;
    cnt = ceil(sqrtf(cnt))-1;
    int i=0, j=0, n=0;
    int maxHeight = 0;
    for(std::vector<Chunk*>::iterator iter = m_vecChunks.begin(); iter != m_vecChunks.end(); ++iter)
    {
        ImageData* pData = (*iter)->getData();
        pData->left = i;
        pData->top = j;
        i += pData->width + SPACING;
        if(maxHeight < pData->height)
            maxHeight = pData->height;
        if(++n > cnt)
        {
            n = 0;
            j += maxHeight + SPACING;
            maxHeight = 0;
            i = 0;
        }
    }
}

void ChunkManager::specialAlign()
{
    if(m_vecChunks.size() != 8)
        return;
    ImageData* p[8];
    for(std::vector<Chunk*>::size_type t = 0; t != m_vecChunks.size(); ++t)
    {
        p[t] = m_vecChunks[t]->getData();
    }
    p[6]->left = p[7]->left = p[2]->top = p[1]->top = p[0]->top = p[0]->left = 0;
    p[5]->left = p[1]->left = p[0]->width + SPACING;
    p[4]->left = p[3]->left = p[2]->left = p[0]->width + p[1]->width + 2*SPACING;
    p[7]->top = p[3]->top = p[0]->height + SPACING;
    p[6]->top = p[5]->top = p[4]->top = p[0]->height + p[7]->height + 2*SPACING;

}

bool ChunkManager::saveOnlyImage(LPCSTR filename)
{
    if(m_vecChunks.empty()) return false;
    RECT rt = getChunkBoundary();
    PIMAGE pimg = newimage(rt.right - rt.left, rt.bottom - rt.top);
    memset(getbuffer(pimg), 0, getwidth(pimg)*getheight(pimg)*sizeof(color_t));
    for(std::vector<Chunk*>::iterator iter = m_vecChunks.begin(); iter != m_vecChunks.end(); ++iter)
    {
        ImageData* pData = (*iter)->getData();
        wy_putimage_alpha(pimg, (*iter)->getChunkImage(), pData->left - rt.left, pData->top - rt.top);
    }
    string s(filename);
    int index = s.find_last_of('.');
    if(index == string::npos) 
        savepng(pimg, (s + ".png").c_str(), 1);
    else if(s.substr(index+1) == "png")
        savepng(pimg, filename, 1);
    else saveimage(pimg, filename);
    delimage(pimg);
    return true;
}

void ChunkManager::getDataText(string& s)const
{
    vector<ImageData*> vecData;
    char buffer[BUFFER_SIZE];
    RECT rt = getChunkBoundary();
    getConfigData(vecData);
    s = "";
    //s += "///* Attention: the order is 'left', 'top', 'width', 'height' */\r\n";
    sprintf(buffer, "%d %d ", rt.right - rt.left, rt.bottom - rt.top);
    s += buffer;
    for(std::vector<ImageData*>::iterator iter = vecData.begin(); iter != vecData.end(); ++iter)
    {
//         ss << "@ " << (*iter)->name << ' '
//             << (*iter)->left << ' ' << (*iter)->top << ' '
//             << (*iter)->width << ' ' << (*iter)->height << ' ';
        sprintf(buffer, "@ %s %d %d %d %d ", 
            (*iter)->name.c_str(), (*iter)->left - rt.left, (*iter)->top - rt.top, (*iter)->width, (*iter)->height);
        s += buffer;
    }
}

void ChunkManager::getDataCSS(string& s)const
{
    vector<ImageData*> vecData;
    getConfigData(vecData);
    char buffer[BUFFER_SIZE];
    RECT rt = getChunkBoundary();
    s = "";
    for(std::vector<ImageData*>::iterator iter = vecData.begin(); iter != vecData.end(); ++iter)
    {
//         ss << "." << (*iter)->name << "{ background-position: "
//             << (*iter)->left << "px " << (*iter)->top << "px; width: "
//             << (*iter)->width << "px; height: " << (*iter)->height << "px;}\n";
        sprintf(buffer, ".%s { background-position: %dpx %dpx; width: %dpx; height: %d px;}\r\n", 
            (*iter)->name.c_str(), (*iter)->left - rt.left, (*iter)->top - rt.top, (*iter)->width, (*iter)->height);
        s += buffer;
    }
    
}

void ChunkManager::getDataXML(string& s)const
{
    vector<ImageData*> vecData;
    char buffer[BUFFER_SIZE];
    RECT rt = getChunkBoundary();
    getConfigData(vecData);
    s = "";
    sprintf(buffer, "<spriteSet imagenum=\"%d\" width=\"%d\" height=\"%d\">\r\n", vecData.size(), rt.right-rt.left, rt.bottom-rt.top);
    s += buffer;
    for(std::vector<ImageData*>::iterator iter = vecData.begin(); iter != vecData.end(); ++iter)
    {
//         ss << "<sprite name=\"" << (*iter)->name << "\" left=\""
//             << (*iter)->left << "\" top=\"" << (*iter)->top << "\" width=\""
//             << (*iter)->width << "\" height=\"" << (*iter)->height << "\"/>" << endl;
        sprintf(buffer, "<sprite name=\"%s\" left=\"%d\" top=\"%d\" width=\"%d\" height=\"%d\"/>\r\n", 
            (*iter)->name.c_str(), (*iter)->left - rt.left, (*iter)->top - rt.top, (*iter)->width, (*iter)->height);
        s += buffer;
    }
    s += "</spriteSet>\n";
}

bool ChunkManager::saveDataAsText(LPCSTR ptrFileName)
{
    using std::fstream;
    vector<ImageData*> vecData;
    getConfigData(vecData);
    string filename(ptrFileName);
    int index = filename.find_last_of('.');
    if(index != string::npos)
        filename.erase(filename.begin()+index, filename.end());
    filename += ".txt";
    fstream fout(filename.c_str(), fstream::out);
    if(!fout) return false;
    string data;
    getDataText(data);
    fout << data;
    fout.close();
    return true;
}

bool ChunkManager::saveDataAsXML(LPCSTR ptrFileName)
{
    using std::fstream;
    vector<ImageData*> vecData;
    getConfigData(vecData);
    string filename(ptrFileName);
    int index = filename.find_last_of('.');
    if(index != string::npos)
        filename.erase(filename.begin()+index, filename.end());
    filename += ".xml";
    fstream fout(filename.c_str(), fstream::out);
    if(!fout) return false;
    string data;
    getDataXML(data);
    fout << data;
    fout.close();
    return true;
}

bool ChunkManager::saveDataAsCSS(LPCSTR ptrFileName)
{
    using std::fstream;
    vector<ImageData*> vecData;
    getConfigData(vecData);
    string filename(ptrFileName);
    int index = filename.find_last_of('.');
    if(index != string::npos)
        filename.erase(filename.begin()+index, filename.end());
    filename += ".css";
    fstream fout(filename.c_str(), fstream::out);
    if(!fout) return false;
    string data;
    getDataCSS(data);
    fout << data;
    fout.close();
    return true;
}

void ChunkManager::getConfigData(std::vector<ImageData*>& vecData) const
{
    vecData.clear();
    for(std::vector<Chunk*>::const_iterator iter = m_vecChunks.begin(); iter != m_vecChunks.end(); ++iter)
    {
        vecData.push_back((*iter)->getData());
    }
}

void ChunkManager::widenChunkWhoseLength(int n)
{
    for(std::vector<Chunk*>::iterator iter = m_vecChunks.begin(); iter != m_vecChunks.end(); ++iter)
    {
        int dx = 1, dy = 1;
        ImageData* pData = (*iter)->getData();
        if(pData->width >= n && pData->height >= n)
            continue;
        if(pData->width < n)
            dx = n;
        if(pData->height < n)
            dy = n;
        (*iter)->resize(dx, dy);
    }
}

bool SaveFileNameDlg(HWND hwnd, LPSTR filename)
{
    assert(filename != NULL);
    *filename = 0;
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.hwndOwner = hwnd;
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filename;
    ofn.lpstrTitle = "请设置要保存的图片名,设置为非png格式可能损失alpha通道!";
    ofn.nMaxFile = BUFFER_SIZE;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrFilter = "PNG File(*.png)\0*.png\0Image Files(*.png;*.jpg;*.bmp)\0*.jpg;*.jpeg;*.bmp;*.png\0All Files(*.*)\0*.*\0\0";
    return GetSaveFileNameA(&ofn);
}

void wy_putimage_alpha(PIMAGE dst, PIMAGE src, int x, int y)
{
    int srcWidth = getwidth(src), srcHeight = getheight(src);
    int dstWidth = getwidth(dst), dstHeight = getheight(dst);
    int xStart = x, xEnd = x + min(srcWidth, dstWidth-x);
    int yStart = y, yEnd = y + min(srcHeight, dstHeight-y);
    color_t *srcBuffer = (color_t*)getbuffer(src), *dstBuffer = (color_t*)getbuffer(dst);
    int n=0;
    for(int j = yStart; j < yEnd; ++j)
    {
        for(int i = xStart; i < xEnd; ++i)
        {
            dstBuffer[i + j * dstWidth] = srcBuffer[n++];
        }
    }
}

void wy_putimage_scale_alpha(PIMAGE dst, PIMAGE src, int dstX, int dstY, int srcWidth, int srcHeight, int dstWidth, int dstHeight)
{
    if(srcWidth == 0) srcWidth = getwidth(src);
    if(srcHeight == 0) srcHeight = getheight(src);
    if(dstWidth == 0) dstWidth = getwidth(dst);
    if(dstHeight == 0) dstHeight = getheight(dst);
    int xStart = dstX, xEnd = xStart + min(dstWidth, getwidth(dst) - dstX);
    int yStart = dstY, yEnd = yStart + min(dstHeight, getheight(dst) - dstY);
    color_t *srcBuffer = (color_t*)getbuffer(src), *dstBuffer = (color_t*)getbuffer(dst);
    double xScale = srcWidth / (double)(xEnd-xStart), yScale = srcHeight / (double)(yEnd-yStart);
    for(int j=yStart, j1=0; j < yEnd; ++j, ++j1)
    {
        for(int i=xStart, i1=0; i < xEnd; ++i, ++i1)
        {
            const int indexDst = i+j*dstWidth;
            const int indexSrc = int((i1*xScale + int(j1*yScale)*srcWidth));
            dstBuffer[indexDst] = srcBuffer[indexSrc];
        }
    }
}

int GetFileNameDlg(HWND hwnd, char filenames[])
{
    assert(filenames != NULL);
    *filenames = 0;
    OPENFILENAMEA ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.hwndOwner = hwnd;
    ofn.lStructSize = sizeof(ofn);
    ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;
    ofn.lpstrFile = filenames;
    ofn.nMaxFile = BUFFER_SIZE;
    ofn.lpstrFilter = "Image Files(*.jpg;*.png;*.bmp等)\0*.jpg;*.jpeg;*.png;*.bmp\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrTitle = "请选择需要合成的图片文件，按住ctrl可多选!";
    if(!GetOpenFileNameA(&ofn)) return -1;
    return ofn.nFileOffset;
}

bool GetFolderPathDlg(HWND hwnd, LPSTR folderPath, LPCSTR title)
{
    BROWSEINFOA bfo;
    memset(&bfo, 0, sizeof(bfo));
    bfo.hwndOwner = hwnd;
    bfo.lpszTitle = title;
#ifndef BIF_USENEWUI
#define BIF_USENEWUI 0x0050
#endif
#ifndef BIF_UAHINT
#define BIF_UAHINT 0x0100
#endif

    bfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_UAHINT;
    
    LPITEMIDLIST lpDlist = SHBrowseForFolderA(&bfo);
    if(lpDlist == NULL)
        return false;
    SHGetPathFromIDListA(lpDlist, folderPath);
    return true;
}

bool SaveFolderPathDlg(HWND hwnd, LPSTR folderPath, LPCSTR title)
{
    BROWSEINFOA bfo;
    memset(&bfo, 0, sizeof(bfo));
    bfo.hwndOwner = hwnd;
    bfo.lpszTitle = title;
    bfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_UAHINT;

    LPITEMIDLIST lpDlist = SHBrowseForFolderA(&bfo);
    if(lpDlist == NULL)
        return false;
    SHGetPathFromIDListA(lpDlist, folderPath);
    return true;
}
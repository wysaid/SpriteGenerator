#ifndef _WYSAID_H_
#define _WYSAID_H_

#include <Windows.h>
#include <ShlObj.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <direct.h>
#include <io.h>
#include "graphics.h"
#include "resource.h"

#define TOOLBARWIDTH 170
#define TOOLITEMHEIGHT 40
#define BUFFER_SIZE 1024
#define SPACING 0
#define DEFAULT_WINDOW_WIDTH 1024
#define DEFAULT_WINDOW_HEIGHT 650

using std::vector;
using std::string;
using std::endl;

struct ImageData
{
    string name;
    int left, top;
    int width, height;
};

class Chunk
{
public:
    Chunk() : m_pimg(NULL){}
    ~Chunk(){ delimage(m_pimg); }
    
    bool init(LPCSTR filename, int left = 0, int top = 0);
    void move(int x, int y);
    void render(bool showBorder = false);
    void resize(float fdx, float fdy);
    ImageData* getData() { return &m_data; }
    PIMAGE getChunkImage() { return m_pimg; }
private:
    void setAlpha(PIMAGE);
    string trimName(LPCSTR);
    PIMAGE m_pimg;
    ImageData m_data;
};

class ChunkManager
{
public:
    ChunkManager():m_activeChunk(0){}
    ~ChunkManager();
    enum SaveDataKind{TEXT, XML, CSS};
    bool addChunks(vector<string> filenames, int dX = 20, int dY = 20);
    bool addChunk(LPCSTR filename, int x = 0, int y = 0);
    void move(int x, int y);
    void moveActiveImage(int x, int y);
    void delTopImage();
    void clearImage();
    void render(bool showBorder = false);
    void renderBound();
    RECT getChunkBoundary()const;
    bool saveConfiguration(LPCSTR filename, SaveDataKind kind);
    bool saveOnlyImage(LPCSTR);
    void getDataText(string&)const;
    void getDataCSS(string&)const;
    void getDataXML(string&)const;
    bool saveDataAsText(LPCSTR);
    bool saveDataAsXML(LPCSTR);
    bool saveDataAsCSS(LPCSTR);
    void getConfigData(vector<ImageData*>&)const;
    Chunk* picTest(int x, int y);
    void autoAlign();
    void specialAlign();
    void widenChunkWhoseLength(int n);
private:
    vector<Chunk*> m_vecChunks;
	Chunk* m_activeChunk;
};

void wy_putimage_alpha(PIMAGE dst, PIMAGE src, int x, int y);
void wy_putimage_scale_alpha(PIMAGE dst, PIMAGE src, int dstX, int dstY, int srcWidth = 0, int srcHeight = 0, int dstWidth = 0, int dstHeight = 0);
bool GetFolderPathDlg(HWND hwnd, LPSTR folderPath, LPCSTR title);
bool SaveFolderPathDlg(HWND hwnd, LPSTR folderPath, LPCSTR title);
#endif
//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib
#include <strsafe.h>
#include "windows.h"


// ������ ������ ����  

float zoom = 1.0f;

typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;//����� � ������� ������ 
} sprite;

sprite racket;//������� ������
sprite enemy;//������� ����������
sprite ball;//�����

struct {
    int score, balls;//���������� ��������� ����� � ���������� "������"
    bool action = false;//��������� - �������� (����� ������ ������ ������) ��� ����
} game;

struct {
    HWND hWnd;//����� ����
    HDC device_context, context;// ��� ��������� ���������� (��� �����������)
    int width, height;//���� �������� ������� ���� ������� ������� ���������
} window;

HBITMAP hBack;// ����� ��� �������� �����������
float topLimitWidth = 1400;    // ������ ������
float bottomLimitWidth = 2080; // ������ �����
//c����� ����
char popupText[64] = { 0 };
int popupTimer = 0;

void ZoomLIm() {
    zoom -= 0.004f;
    if (zoom < 0.8f) { 
        zoom = 0.8f;
    }
   
}
void ZoomOut() {
    zoom += 0.005f; // ����������� ������ (���������)
    if (zoom > 1.0f) {
        zoom = 1.0f;
    } // ���������� ������������ �������
    
}

void InitGame()
{
    //� ���� ������ ��������� ������� � ������� ������� gdi
    //���� ������������� - ����� ������ ������ ����� � .exe 
    //��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "26802.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "Komnata.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------
    
    racket.width = 150;
    racket.height =300;
    racket.speed = 12;//�������� ����������� �������
    racket.x = window.width / 2.;//������� ���������� ����
    racket.y = window.height - racket.height;//���� ���� ���� ������ - �� ������ �������

    enemy.x = racket.x;//� ���������� �������� ������ � �� �� ����� ��� � ������

    ball.dy = (rand() % 65 + 35) / 100.;//��������� ������ ������ ������
    ball.dx = -(1 - ball.dy);//��������� ������ ������ ������
    ball.speed = 11;
    ball.rad = 20;
    ball.x = racket.x;//x ���������� ������ - �� ������� �������
    ball.y = racket.y - ball.rad;//����� ����� ������ �������

    game.score = 0;
    game.balls = 9;

   
}

void ProcessSound(const char* name)//������������ ���������� � ������� .wav, ���� ������ ������ � ��� �� ����� ��� � ���������
{
    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//���������� name �������� ��� �����. ���� ASYNC ��������� ����������� ���� ���������� � ����������� ���������
}

void ShowScore()
{
    //�������� �������� � �������
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//����� ��� ������
    _itoa_s(game.score, txt, 10);//�������������� �������� ���������� � �����. ����� �������� � ���������� txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

void ShowPopup(const char* text, int duration_ms)
{
    if (text)
    {
        size_t len = strlen(text);
        if (len >= sizeof(popupText))
            len = sizeof(popupText) - 1;
        memcpy(popupText, text, len);
        popupText[len] = '\0'; // ��������� ������
    }
    else
    {
        popupText[0] = '\0';
    }
    popupTimer = duration_ms;
}

// ������� ��� ����������� ������
void Texts()
{
    if (popupTimer > 0 && strlen(popupText) > 0) {
            // ��������� ������ �� ������ ��� � ������ �������
            int x = (window.width - 200) / 2; // ��������, �� ������
            int y = window.height / 2;
            SetTextColor(window.context, RGB(255, 0, 0)); // ������� ���� ��� ����������
            SetBkMode(window.context, TRANSPARENT);
            // �������� �����, ���� �����
            // CreateFont � SelectObject, ��� � ShowScore()

            TextOutA(window.context, x, y, popupText, (int)strlen(popupText));

            popupTimer -= 16;
            if (popupTimer <= 0)
            {
                popupText[0] = '\0'; // ������� �����, ����� �� �����
            }
     }
}

void ProcessInput()
{
    if (GetAsyncKeyState('D') && GetAsyncKeyState('S') || GetAsyncKeyState('A') && GetAsyncKeyState('S') ) {
        racket.speed = 12;
    }
    if (GetAsyncKeyState('W') && GetAsyncKeyState('D') || GetAsyncKeyState('W') && GetAsyncKeyState('A')) {
        racket.speed = 10;
    }
    if (GetAsyncKeyState('W')) {
        racket.y -= racket.speed;
        racket.speed = 10;
    }
    if (GetAsyncKeyState('A')) {
        racket.x -= racket.speed;
    }
    if (GetAsyncKeyState('S')) {
        racket.y += racket.speed;
        racket.speed = 12;
    }
    if (GetAsyncKeyState('D')) {
        racket.x += racket.speed;
    }
    if (racket.y < 0) {
        racket.y = 0;
    }
    if (racket.y > window.height - racket.height) {
        racket.y = window.height - racket.height;
    }
    if (GetAsyncKeyState('W')) {
        ZoomLIm();
    }
    if (GetAsyncKeyState('S')) {
        ZoomOut();
    }
    if (GetAsyncKeyState('I') & 0x8000) {
        ShowPopup("Aim Venom !", 2000); // ���������� 2 �������
    }


    //if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    //if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;

    //if (!game.action && GetAsyncKeyState(VK_SPACE))
    //{
       // game.action = true;
//ProcessSound("bounce.wav");
   // }
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1,  HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;
    

    hMemDC = CreateCompatibleDC(hDC); // ������� �������� ������, ����������� � ���������� �����������
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// �������� ����������� bitmap � �������� ������

    if (hOldbm) // ���� �� ���� ������, ���������� ������
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // ���������� ������� �����������

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//��� ������� ������� ����� ����� ��������������� ��� ����������
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // ������ ����������� bitmap
        }

        SelectObject(hMemDC, hOldbm);// ��������������� �������� ������
    }

    DeleteDC(hMemDC); // ������� �������� ������
}

void ShowRacketAndBall()
{
    int drawWidth = (int)(racket.width * zoom);
    int drawHeight = (int)(racket.height * zoom);
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//������ ���
    ShowBitmap(window.context, racket.x - drawWidth / 2, racket.y, drawWidth, drawHeight, racket.hBitmap);
    if (ball.dy < 0 && (enemy.x - racket.width / 4 > ball.x || ball.x > enemy.x + racket.width / 4))
    {
        //��������� ���������� ���������. �� ����� ����, ��������� ������� �� �����������, � �� �� ������� �������� �� ��� ������� �� ������
        //������ �����, �� ������ ������ ������ �� �������, � ������� ���������� ������� - ����������� ��� �����
        //�������� ����� ������ ���� ����� ����� �����, � ������ ���� ����� �� ��� X ������� �� ������� �������� ����� �������
        //� ���� ������, �� ��������� ���������� ������� � ������ � ��������� 9 � 1
        enemy.x = ball.x * .1 + enemy.x * .9;
    }

    ShowBitmap(window.context, enemy.x - racket.width / 2, 0, racket.width, racket.height, enemy.hBitmap);//������� ���������
}

void LimitRacket()
{
    
    // ������ ������ ������ � ������� Y �������
    float currentLimitWidth = topLimitWidth + (bottomLimitWidth - topLimitWidth) * (racket.y / (float)window.height);
    float minX = (window.width - currentLimitWidth) / 2 + racket.width / 2;
    float maxX = (window.width + currentLimitWidth) / 2 - racket.width / 2;
    if (racket.x < minX) {
        racket.x = minX;
    }
    if (racket.x > maxX) {
        racket.x = maxX;
    }

    // ���� ����������� �� y
    float minY = 400;
    if (zoom <= 0.70f) { 
        minY = 100;
    }
    if (racket.y < minY) {
        racket.y = minY;
    }
    if (racket.y > window.height - racket.height) {
        racket.y = window.height - racket.height;

    }
    
}


void CheckWalls()
{
    if (ball.x < ball.rad || ball.x > window.width - ball.rad)
    {
        ball.dx *= -1;
        ProcessSound("bounce.wav");
    }
}

void CheckRoof()
{
    if (ball.y < ball.rad + racket.height)
    {
        ball.dy *= -1;
        ProcessSound("bounce.wav");
    }
}

bool tail = false;

void CheckFloor()
{
    if (ball.y > window.height - ball.rad - racket.height)//����� ������� ����� ������� - ����������� �������
    {
        if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//����� �����, � �� �� � ������ ��������� ������
        {
            game.score++;//�� ������ ������� ���� ���� ����
            ball.speed += 5. / game.score;//�� ����������� ��������� - ���������� �������� ������
            ball.dy *= -1;//������
            racket.width -= 10. / game.score;//������������� ��������� ������ ������� - ��� ���������
            ProcessSound("bounce.wav");//������ ���� �������
        }
        
    }
}

void ProcessRoom()
{
    //������������ �����, ������� � ���. ������� - ���� ������� ����� ���� ���������, � ������, ��� ������� �� ����� ������ ������������� ����� ������� �������� ������
    CheckWalls();
    CheckRoof();
    CheckFloor();
}


void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//�� ������ ���� ������� ����� ��������� ���������� ��� ���������
    window.width = r.right - r.left;//���������� ������� � ���������
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//������ �����
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//����������� ���� � ���������
    GetClientRect(window.hWnd, &r);
    // ����� ��������� �����
   
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//����� �������������� ��� ��� ����� ��� ��������� � ����
    InitGame();//����� �������������� ���������� ����

    mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//������ ���, ������� � �����
        ShowScore();//������ ���� � �����
        Texts();
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
        Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)
        ProcessInput();//����� ����������
        LimitRacket();//���������, ����� ������� �� ������� �� �����        ProcessRoom();//������������ ������� �� ���� � �������, ��������� ������ � ��������
    }
 
}




//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <mmsystem.h>


// секция данных игры  

float zoom = 1.0f;

typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    HBITMAP hBitmap;//хэндл к спрайту шарика 
} sprite;

sprite racket;//ракетка игрока
sprite enemy;//ракетка противника
sprite ball;//шарик

struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения

float topLimitWidth = 1400;    // ширина сверху
float bottomLimitWidth = 2080; // ширина снизу


void ZoomLIm() {
    zoom -= 0.004f;
    if (zoom < 0.8f) { 
        zoom = 0.8f;
    }
   
}
void ZoomOut() {
    zoom += 0.005f; // увеличивать размер (отдаление)
    if (zoom > 1.0f) {
        zoom = 1.0f;
    } // ограничить максимальный масштаб
    
}

void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "26802.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "komnata.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------
    
    racket.width = 150;
    racket.height =300;
    racket.speed = 12;//скорость перемещения ракетки
    racket.x = window.width / 2.;//ракетка посередине окна
    racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки

    enemy.x = racket.x;//х координату оппонета ставим в ту же точку что и игрока

    game.score = 0;
    game.balls = 9;

   
}



void ShowScore()
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//буфер для текста
    _itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
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

    if (GetAsyncKeyState('W')) {
        ZoomLIm();
    }
    if (GetAsyncKeyState('S')) {
        ZoomOut();
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
    

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}

void ShowRacketAndBall()
{
    int drawWidth = (int)(racket.width * zoom);
    int drawHeight = (int)(racket.height * zoom);
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
    ShowBitmap(window.context, racket.x - drawWidth / 2, racket.y, drawWidth, drawHeight, racket.hBitmap);
    if (ball.dy < 0 && (enemy.x - racket.width / 4 > ball.x || ball.x > enemy.x + racket.width / 4))
    {
        //имитируем разумность оппонента. на самом деле, компьютер никогда не проигрывает, и мы не считаем попадает ли его ракетка по шарику
        //вместо этого, мы всегда делаем отскок от потолка, а раектку противника двигаем - подставляем под шарик
        //движение будет только если шарик летит вверх, и только если шарик по оси X выходит за пределы половины длины ракетки
        //в этом случае, мы смешиваем координаты ракетки и шарика в пропорции 9 к 1
        enemy.x = ball.x * .1 + enemy.x * .9;
    }

    ShowBitmap(window.context, enemy.x - racket.width / 2, 0, racket.width, racket.height, enemy.hBitmap);//ракетка оппонента
}

void LimitRacket()
{
    
    // Расчет ширины границ в текущей Y позиции
    float currentLimitWidth = topLimitWidth + (bottomLimitWidth - topLimitWidth) * (racket.y / (float)window.height);
    float minX = (window.width - currentLimitWidth) / 2 + racket.width / 2;
    float maxX = (window.width + currentLimitWidth) / 2 - racket.width / 2;
    if (racket.x < minX) {
        racket.x = minX;
    }
    if (racket.x > maxX) {
        racket.x = maxX;
    }

    // ваши ограничения по y
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



void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);
    // Левая наклонная линия
   
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowScore();//рисуем очик и жизни
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)
        ProcessInput();//опрос клавиатуры
        LimitRacket();//проверяем, чтобы ракетка не убежала за экран        ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
    }
     
}


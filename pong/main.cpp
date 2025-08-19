//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib
#include <strsafe.h>
#include "windows.h"


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
//cекция кода
char popupText[64] = { 0 };
int popupTimer = 0;

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
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "26802.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "Komnata.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------
    
    racket.width = 150;
    racket.height =300;
    racket.speed = 12;//скорость перемещения ракетки
    racket.x = window.width / 2.;//ракетка посередине окна
    racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки

    enemy.x = racket.x;//х координату оппонета ставим в ту же точку что и игрока

    ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
    ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
    ball.speed = 11;
    ball.rad = 20;
    ball.x = racket.x;//x координата шарика - на середие ракетки
    ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки

    game.score = 0;
    game.balls = 9;

   
}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
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

void ShowPopup(const char* text, int duration_ms)
{
    if (text)
    {
        size_t len = strlen(text);
        if (len >= sizeof(popupText))
            len = sizeof(popupText) - 1;
        memcpy(popupText, text, len);
        popupText[len] = '\0'; // завершаем строку
    }
    else
    {
        popupText[0] = '\0';
    }
    popupTimer = duration_ms;
}

// функция для отображения текста
void Texts()
{
    if (popupTimer > 0 && strlen(popupText) > 0) {
            // Отрисовка текста по центру или в нужной позиции
            int x = (window.width - 200) / 2; // например, по центру
            int y = window.height / 2;
            SetTextColor(window.context, RGB(255, 0, 0)); // красный цвет для заметности
            SetBkMode(window.context, TRANSPARENT);
            // Выберите шрифт, если нужно
            // CreateFont и SelectObject, как в ShowScore()

            TextOutA(window.context, x, y, popupText, (int)strlen(popupText));

            popupTimer -= 16;
            if (popupTimer <= 0)
            {
                popupText[0] = '\0'; // очищаем текст, чтобы не мешал
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
        ShowPopup("Aim Venom !", 2000); // показывать 2 секунды
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
    if (ball.y > window.height - ball.rad - racket.height)//шарик пересек линию отскока - горизонталь ракетки
    {
        if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//шарик отбит, и мы не в режиме обработки хвоста
        {
            game.score++;//за каждое отбитие даем одно очко
            ball.speed += 5. / game.score;//но увеличиваем сложность - прибавляем скорости шарику
            ball.dy *= -1;//отскок
            racket.width -= 10. / game.score;//дополнительно уменьшаем ширину ракетки - для сложности
            ProcessSound("bounce.wav");//играем звук отскока
        }
        
    }
}

void ProcessRoom()
{
    //обрабатываем стены, потолок и пол. принцип - угол падения равен углу отражения, а значит, для отскока мы можем просто инвертировать часть вектора движения шарика
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
        Texts();
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)
        ProcessInput();//опрос клавиатуры
        LimitRacket();//проверяем, чтобы ракетка не убежала за экран        ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
    }
 
}




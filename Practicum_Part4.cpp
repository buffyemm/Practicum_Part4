// Practicum_Part4.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Practicum_Part4.h"

#define MAX_LOADSTRING 100



typedef struct {
	float x, y, width, height, rad, dx, dy, speed;
	HBITMAP hBitmap;//хэндл к спрайту шарика 
	bool isActive;

} sprite;
const int line = 15, column = 7;
sprite racket;//ракетка игрока

sprite blocks[line][column];
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

//cекция кода

void InitWindow() { // инициализация структуры window

	RECT r;
	GetClientRect(window.hWnd, &r);
	window.width = r.right - r.left;
	window.height = r.bottom - r.top;


}

void InitGame()
{
	//в этой секции загружаем спрайты с помощью функций gdi
	//пути относительные - файлы должны лежать рядом с .exe 
	//результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
	ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	auto blockBMP = (HBITMAP)LoadImageA(NULL, "bill.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	//------------------------------------------------------

	for (int i = 0; i < line;i++) {
		for (int j = 0; j < column; j++) {
			blocks[i][j].width = window.width / line;
			blocks[i][j].height = window.height / 3 / column;
			blocks[i][j].x = blocks[i][j].width * i;
			blocks[i][j].y = blocks[i][j].height * j + window.height / 3;
			blocks[i][j].isActive = true;
			blocks[i][j].hBitmap = blockBMP;
		}
	}
	racket.width = 300;
	racket.height = 50;
	racket.speed = 30;//скорость перемещения ракетки
	racket.x = window.width / 2.;//ракетка посередине окна
	racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки


	ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
	ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
	ball.speed = 20;
	ball.rad = 20;
	ball.x = racket.x;//x координата шарика - на середие ракетки
	ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки

	game.score = 0;
	game.balls = 9;



}


bool HelpCollise(sprite first, sprite second) {


	if (first.x <= second.x + second.width && +
		first.x + first.width >= second.x &&
		first.y <= second.y + second.height &&
		first.y + first.height >= second.y) {

		return true;
	}

	return false;

}


void ProcessInput()
{
	if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
	if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;

	if (!game.action && GetAsyncKeyState(VK_SPACE))
	{
		game.action = true;
	}
}

void LimitRacket()
{
	racket.x = max(racket.x, racket.width / 2.);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
	racket.x = min(racket.x, window.width - racket.width / 2.);//аналогично для правого угла
}

void CheckWalls()
{
	if (ball.x < ball.rad || ball.x > window.width - ball.rad)
	{
		ball.dx *= -1;
	}
}

void CheckRoof()
{

	if (ball.y < ball.rad)
	{
		ball.dy *= -1;
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
			// ball.speed += 5. / game.score;//но увеличиваем сложность - прибавляем скорости шарику
			ball.dy *= -1;//отскок
			racket.width -= 10. / game.score;//дополнительно уменьшаем ширину ракетки - для сложности
		}
		else
		{//шарик не отбит

			tail = true;//дадим шарику упасть ниже ракетки

			if (ball.y - ball.rad > window.height)//если шарик ушел за пределы окна
			{
				game.balls--;//уменьшаем количество "жизней"


				if (game.balls < 0) { //проверка условия окончания "жизней"

					MessageBoxA(window.hWnd, "game over", "", MB_OK);//выводим сообщение о проигрыше
					InitGame();//переинициализируем игру
				}

				ball.dy = (rand() % 65 + 35) / 100.;//задаем новый случайный вектор для шарика
				ball.dx = -(1 - ball.dy);
				ball.x = racket.x;//инициализируем координаты шарика - ставим его на ракетку
				ball.y = racket.y - ball.rad;
				game.action = false;//приостанавливаем игру, пока игрок не нажмет пробел
				tail = false;
			}
		}
	}
}

void Collision_blocks(HDC hDC) {

	bool collisionHandled = false; // Флаг для отслеживания, было ли обработано столкновение

	float length = sqrt(pow((ball.dx * ball.speed), 2) + pow((ball.dy * ball.speed), 2));

	float bx = ball.x;
	float by = ball.y;


	for (int k = 0; k < length; k++) {

		float s = k / (float)length;
		float new_x = bx + (ball.dx * ball.speed) * s;
		float new_y = by + (ball.dy * ball.speed) * s;

		SetPixel(hDC, new_x, new_y, RGB(255, 20, 147));

		for (int i = 0; i < line; i++) {

			for (int j = 0; j < column; j++) {

				//SetPixel(hDC,)

				if (blocks[i][j].isActive && !collisionHandled) { // Проверяем только если столкновение ещё не обработано

					if (HelpCollise(ball, blocks[i][j])) {

						// Определяем, с какой стороны произошло столкновение
						float overlapLeft = (ball.x + ball.rad) - blocks[i][j].x; // расстояние до левой стороны блока
						float overlapRight = (blocks[i][j].x + blocks[i][j].width) - (ball.x - ball.rad); // расстояние до правой стороны блока
						float overlapUP = (ball.y + ball.rad) - blocks[i][j].y; // расстояние до верхней стороны блока
						float overlapDOWN = (blocks[i][j].y + blocks[i][j].height) - (ball.y - ball.rad); // расстояние до нижней стороны блока

						// Находим минимальное перекрытие вручную
						float minOverlapX = min(overlapLeft, overlapRight);
						float minOverlapY = min(overlapUP, overlapDOWN);


						// Изменяем направление мяча в зависимости от стороны столкновения
						if (minOverlapX < minOverlapY) {
							ball.dx = -ball.dx; // Отскок по горизонтали
						}
						else {
							ball.dy = -ball.dy; // Отскок по вертикали
						}

						collisionHandled = true; // Столкновение обработано, больше не проверяем другие блоки
						blocks[i][j].isActive = false; // Деактивируем блок
						return;
					}
				}
			}
		}
	}

}

void ProcessRoom()
{
	CheckWalls();
	CheckRoof();
	CheckFloor();
	//обрабатываем стены, потолок и пол. принцип - угол падения равен углу отражения, а значит, для отскока мы можем просто инвертировать часть вектора движения шарика
}

void ProcessBall()
{
	if (game.action)
	{
		//если игра в активном режиме - перемещаем шарик
		ball.x += ball.dx * ball.speed;
		ball.y += ball.dy * ball.speed;
	}
	else
	{
		//иначе - шарик "приклеен" к ракетке
		ball.x = racket.x;
	}
}

void DrawBitmap(HDC hdcDest, int x, int y, int w, int h, HBITMAP hBmp, bool transparent) {
	if (!hBmp) return;
	HDC hMemDC = CreateCompatibleDC(hdcDest);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hBmp);
	BITMAP bmp;
	GetObject(hBmp, sizeof(BITMAP), &bmp);

	if (transparent) {

		TransparentBlt(hdcDest, x, y, w, h, hMemDC, 0, 0, w, h, RGB(0, 0, 0));// прозрачность

	}
	else {

		StretchBlt(hdcDest, x, y, w, h, hMemDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY); // растягивание на фул экран
	}
	SelectObject(hMemDC, hOldBmp);
	DeleteDC(hMemDC);
};


void ShowObject(HDC hMemDC) {

	// задник
	DrawBitmap(hMemDC, 0, 0, window.width, window.height, hBack, false);

	DrawBitmap(hMemDC, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap, false); // ракетка игрока
	DrawBitmap(hMemDC, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true); // шарик


	// Отрисовка блоков
	for (int i = 0; i < line; i++) {
		for (int j = 0; j < column; j++) {
			if (blocks[i][j].isActive) {
				DrawBitmap(hMemDC, blocks[i][j].x, blocks[i][j].y, blocks[i][j].width, blocks[i][j].height, blocks[i][j].hBitmap, false);
			}
		}
	}

}

void ProcessGame() {

	LimitRacket();
	ProcessInput();
	ProcessBall();
	//Collision_blocks();
	ProcessRoom();

}


void Case_KEYdown(WPARAM wParam, HWND hwnd) {

	if (wParam == VK_ESCAPE) {

		DestroyWindow(hwnd);

	}


}

void Case_Destroy(HWND hwnd) {

	PostQuitMessage(0);
	KillTimer(hwnd, 1);
	

}

void Case_Timer(WPARAM wParam, HWND hwnd) {

	if (wParam == 1) { // у каждого таймера есть свой айди, и если таймер под айдишником 1 закончился, то мы запускаем то что ниже

		InvalidateRect(hwnd, NULL, FALSE); // перерисовка всего окна
		ProcessGame();


	}

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hI, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {


	const wchar_t CLASS_NAME[] = L"KAMEN";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hI;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);


	window.hWnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"JOB IS DONE",
		WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), // функция узнает размер окна 
		NULL,
		NULL,
		hI,
		NULL
	);


	if (window.hWnd == NULL) return 0;


	InitWindow();
	InitGame();

	ShowWindow(window.hWnd, nCmdShow);

	SetTimer(window.hWnd, 1, 16, NULL);// ставим таймер на 16 милесикунд~60фпс



	MSG msg = { };

	while (GetMessage(&msg, NULL, 0, 0) > 0) {

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {


	switch (uMsg) {


	case WM_KEYDOWN:

		Case_KEYdown(wParam, hwnd);
		break;

	

	case WM_DESTROY: // когда уничтожается
		Case_Destroy(hwnd);
		return 0;


	case WM_CREATE: { // кейс когда создается окно 

		//Case_Create(hwnd);
		break;

	}

	case WM_TIMER:

		Case_Timer(wParam, hwnd);

		break;

	case WM_PAINT: { // вывод на экран картинки 

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// 1. Создаём буфер в памяти
		HDC hMemDC = CreateCompatibleDC(hdc);
		HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, window.width, window.height);
		HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hMemBmp);


		// --- Платформа и герой ---

		ShowObject(hMemDC);


		// 3. Копируем готовый буфер на экран
		BitBlt(hdc, 0, 0, window.width, window.height, hMemDC, 0, 0, SRCCOPY);

		Collision_blocks(hdc);
		// 4. Очистка
		SelectObject(hMemDC, hOldBmp);
		DeleteObject(hMemBmp);
		DeleteDC(hMemDC);
		EndPaint(hwnd, &ps);
	}

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

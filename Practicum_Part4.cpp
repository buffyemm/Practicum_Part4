// Practicum_Part4.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Practicum_Part4.h"

#define MAX_LOADSTRING 100
#define M_PI 3.14159265358979323846f

typedef struct {
	float x, y, width, height, rad, dx, dy, speed;
	HBITMAP hBitmap;//хэндл к спрайту шарика 
	bool isActive;


} sprite;

sprite set_setting(int width, int height, int speed, int x, int startY) {

	sprite a;
	a.width = width;
	a.height = height;
	a.speed = speed;
	a.x = x;
	a.y = startY - height;

	return a;
}
const int line = 15, column = 7;
sprite racket;//ракетка игрока
sprite trace;
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

void LoadPicture() {

	ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	auto blockBMP = (HBITMAP)LoadImageA(NULL, "bill.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

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
}

void InitGame()
{
	racket = set_setting(300, 50, 30, window.width / 2., window.height);
	//в этой секции загружаем спрайты с помощью функций gdi
	//пути относительные - файлы должны лежать рядом с .exe 
	//------------------------------------------------------
	LoadPicture();

	ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
	ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
	ball.speed = 20;
	ball.rad = 20;
	ball.x = racket.x;//x координата шарика - на середие ракетки
	ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки

	game.score = 0;
	game.balls = 9;



}


bool HelpCollise(sprite first, sprite second) { // изменил теперь тут вместо x, dx

	if (first.dx <= second.x + second.width &&
		first.dx >= second.x &&
		first.dy <= second.y + second.height &&
		first.dy >= second.y) {
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

void DistanceCalculate(sprite first, sprite second) {

	float overlapLeft = first.dx - second.x;
	float overlapRight = (second.x + second.width) - first.dx;
	float overlapTop = first.dy - second.y;
	float overlapBottom = (second.y + second.height) - first.dy;

	float minOverlapX = min(overlapLeft, overlapRight);
	float minOverlapY = min(overlapTop, overlapBottom);

	if (minOverlapX < minOverlapY) {
		ball.dx = -ball.dx;
	}
	else {
		ball.dy = -ball.dy;
	}

}


void Collision_blocks(HDC hDC) {

	bool collisionHandled = false;
	float length = ball.speed; // так как вектор полета шара нормализованн, не нужно расчитывать по формуле длинну вектора полета шара, потому что он является скоростью шара

	for (int k = 0; k < length && !collisionHandled; k++) {

		float s = k / length;

		// расчет координат точки трассировки, запускаем в цикл, чтобы ставить луч
		trace.dx = ball.x + (ball.dx * ball.speed) * s;  
		trace.dy = ball.y + (ball.dy * ball.speed) * s;

		SetPixel(hDC, trace.dx, trace.dy, RGB(255, 20, 147)); //отрисовка пикселя 

		for (int i = 0; i < line && !collisionHandled; i++) {

			for (int j = 0; j < column && !collisionHandled; j++) {

				if (blocks[i][j].isActive && HelpCollise(trace, blocks[i][j])) { // чутка изменил функию HelpCollise 

					
					DistanceCalculate(trace, blocks[i][j]);

					// обновляем позицию мяча
					ball.x = trace.dx;
					ball.y = trace.dy;

					collisionHandled = true;
					blocks[i][j].isActive = false;
					return;
				}
			}
		}
	}
}

void COL(HDC hDC) {

	float bx = ball.x;
	float by = ball.y;

	// Предварительно вычисляем угол направления
	float move_angle = atan2(ball.dy, ball.dx);
	const int COLLISION_POINTS = 8;

	// Временные переменные для обработки множественных столкновений
	float current_dx = ball.dx;
	float current_dy = ball.dy;
	float current_x = bx;
	float current_y = by;

	// Оставшееся расстояние, которое нужно пройти в этом кадре
	float remaining_length = sqrt((ball.dx * ball.speed) * (ball.dx * ball.speed) +
		(ball.dy * ball.speed) * (ball.dy * ball.speed));

	for (int collisionCount = 0; remaining_length > 0; collisionCount++) {
		bool collisionHandled = false;
		float length = remaining_length; // Текущая длина для проверки

		for (int k = 0; k < length && !collisionHandled; k++) {
			float s = k / length;
			float center_x = current_x + current_dx * k;
			float center_y = current_y + current_dy * k;

			for (int point_idx = 0; point_idx < COLLISION_POINTS && !collisionHandled; point_idx++) {
				float angle = (M_PI * point_idx) / (COLLISION_POINTS - 1) - M_PI / 2;

				// Вычисляем смещение точки относительно центра
				float offset_x = cos(move_angle + angle) * ball.rad;
				float offset_y = sin(move_angle + angle) * ball.rad;

				// АБСОЛЮТНЫЕ координаты точки на окружности
				float check_x = center_x + offset_x;
				float check_y = center_y + offset_y;

				// Отрисовываем абсолютные координаты (можно включить для дебага)
				SetPixel(window.context, (int)check_x, (int)check_y, RGB(255, 20, 147));

				// Проверяем столкновение с АБСОЛЮТНЫМИ координатами
				for (int i = 0; i < line && !collisionHandled; i++) {
					for (int j = 0; j < column && !collisionHandled; j++) {
						if (blocks[i][j].isActive) {
							if (check_x >= blocks[i][j].x && check_x <= blocks[i][j].x + blocks[i][j].width &&
								check_y >= blocks[i][j].y && check_y <= blocks[i][j].y + blocks[i][j].height) {

								// Определяем сторону столкновения (используем центр шара)
								float block_center_x = blocks[i][j].x + blocks[i][j].width / 2;
								float block_center_y = blocks[i][j].y + blocks[i][j].height / 2;

								// Вектор от центра блока к центру шара
								float dx_to_block = center_x - block_center_x;
								float dy_to_block = center_y - block_center_y;

								// Определяем, с какой стороны произошло столкновение
								float overlapLeft = (center_x + ball.rad) - blocks[i][j].x;
								float overlapRight = (blocks[i][j].x + blocks[i][j].width) - (center_x - ball.rad);
								float overlapTop = (center_y + ball.rad) - blocks[i][j].y;
								float overlapBottom = (blocks[i][j].y + blocks[i][j].height) - (center_y - ball.rad);

								// Находим минимальное перекрытие
								float minOverlapX = min(overlapLeft, overlapRight);
								float minOverlapY = min(overlapTop, overlapBottom);

								// Определяем направление отскока
								if (minOverlapX < minOverlapY) {
									// Горизонтальное столкновение
									current_dx = -current_dx;
								}
								else {
									// Вертикальное столкновение
									current_dy = -current_dy;
								}

								// Обновляем угол направления для следующей итерации
								move_angle = atan2(current_dy, current_dx);

								// Перемещаем шар в точку столкновения (на расстояние k)
								current_x = center_x;
								current_y = center_y;

								// Уменьшаем оставшееся расстояние на k
								remaining_length = remaining_length - k;

								collisionHandled = true;
								blocks[i][j].isActive = false;

							}
						}
					}
				}
			}
		}

		// Если не было столкновений, выходим из цикла по столкновениям
		if (!collisionHandled) {
			// Перемещаем шар на оставшееся расстояние
			current_x += current_dx * remaining_length;
			current_y += current_dy * remaining_length;
			remaining_length = 0;
		}
	}

	// Обновляем позицию и направление шара
	ball.x = current_x;
	ball.y = current_y;
	ball.dx = current_dx;
	ball.dy = current_dy;


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

	HDC hdc = GetDC(window.hWnd); // нужно для отриссовки трассировки, отладачный луч, он может мерцать потому что он не находиться в буффере 

	LimitRacket();
	ProcessInput();
	ProcessBall();
	Collision_blocks(hdc);
	//COL(hdc);
	ProcessRoom();
	
	ReleaseDC(window.hWnd, hdc);
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

void Case_Paint(HDC hdc) {

	// 1. Создаём буфер в памяти
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, window.width, window.height);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hMemBmp);


	// --- Платформа и герой ---

	ShowObject(hMemDC);

	// 3. Копируем готовый буфер на экран
	BitBlt(hdc, 0, 0, window.width, window.height, hMemDC, 0, 0, SRCCOPY);

	// 4. Очистка
	SelectObject(hMemDC, hOldBmp);
	DeleteObject(hMemBmp);
	DeleteDC(hMemDC);

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

	case WM_KEYDOWN: {

		Case_KEYdown(wParam, hwnd);
		break;
	}
	case WM_DESTROY: {// когда уничтожается
		Case_Destroy(hwnd);
		return 0;
	}
	case WM_CREATE: { // кейс когда создается окно 

		//Case_Create(hwnd);
		break;

	}
	case WM_TIMER: {

		Case_Timer(wParam, hwnd);
		break;
	}
	case WM_PAINT: { // вывод на экран картинки 

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		Case_Paint(hdc);
		
		EndPaint(hwnd, &ps);
	}

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

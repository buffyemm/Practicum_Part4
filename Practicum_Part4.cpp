// Practicum3.cpp : Defines the entry point for the application.
//
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "winmm.lib")
#include "framework.h"
#include "vector"
#include <string>

using namespace std;

#define SCALE  0.104f

bool GameActiv = true; // если игра активна
BOOL time_at = false; // время прошло клика стрелы

// по хорошему, куда то закинуть эту переменную, но я оставил ее так, для заднкиа меню
HBITMAP menu = (HBITMAP)LoadImageW(NULL, L"menu.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
// создадим для удобства структуру window чтобы не носиться со всеми переменными
struct {

	HWND hWnd;
	int width, height;

}window;

// структура для спрайтов
struct sprite {
	float x, y, width, height, speed;
	bool inJump = false;
};

enum class item_ {

	Sword,
	Bow,
	Axe,
	block

};

// класс по стуи тоже самое что и структура но со своими плюшками
class object_{
public:
	sprite model;
	HBITMAP picture;
	item_ ID;

	void set_setting(int width, int height, int x, int y, LPWSTR name) {

		model.width = width;
		model.height = height;
		model.x = x;
		model.y;
		picture = ((HBITMAP)LoadImageW(NULL, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));

	}

};


class Arrow_ : public object_ {

public:

	float directionX, directionY;
	bool activ;

	// упрощенная инициализация, где то написан метод, где то конструктор
	Arrow_(float x, float y) {

		model.x = x;
		model.y = y;
		model.speed = 10;
		model.width = 40;
		model.height = 40;
		picture = (HBITMAP)LoadImageW(NULL, L"ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		activ = false;
	}

	// вышел ли за экран, проверка для шара
	bool isOutOfScreen() {

		return (model.x + model.width > window.width ||
			model.x < 0 ||
			model.y < 0 ||
			model.y + model.height > window.height);
	}


};



// сделал класс производный клас, который наследует object_
class portal_ : public object_ {
public:
	int target;
	vector<HBITMAP> anim;
	int currentFrame = 0;

	//упрощонная инициализация через конструктор
	portal_(float x, float y, float width, float height, LPCWSTR name, int temp) { //конструктор класса

		//set_setting(width, height, x, y, name); не фурычит почему то РАЗБЕРИСЬ!)
		model.x = x;
		model.y = y;
		model.width = width;
		model.height = height;
		picture = (HBITMAP)LoadImageW(NULL, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		target = temp;
		anim.push_back((HBITMAP)LoadImageW(NULL, L"portal0.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		anim.push_back((HBITMAP)LoadImageW(NULL, L"portal1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		anim.push_back((HBITMAP)LoadImageW(NULL, L"portal2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		anim.push_back((HBITMAP)LoadImageW(NULL, L"portal3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		anim.push_back((HBITMAP)LoadImageW(NULL, L"portal4.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		anim.push_back((HBITMAP)LoadImageW(NULL, L"portal5.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		anim.push_back((HBITMAP)LoadImageW(NULL, L"portal6.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		anim.push_back((HBITMAP)LoadImageW(NULL, L"portal7.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));

	}

	void HelpAnim() {

	static bool end = false;

		/*if (currentFrame < anim.size() && end) {
		}
			currentFrame++;
		else {

			currentFrame--;

			if (currentFrame >= anim.size()) currentFrame = 0;

		}*/

		if (currentFrame == anim.size() - 1) {
			end = true;
		}
		else if (currentFrame == 0) {
			end = false;
		}
		if (end) {
			currentFrame--;
		}
		else {
			currentFrame++;
			
		}

	}

};

// создаем структуру 
struct Character {

	sprite model;
	HBITMAP picture;
	int current_loc = 0;
	vector<object_> item;
	vector<object_> rig;
	HBITMAP anim[4];
	int HP;

	//метод для упрощения инициализации игрока
	void set_parameters(float x, float y, float width, float height, float speed, int hp) {

		model.x = x;
		model.y = y;
		model.width = width;
		model.height = height;
		model.speed = speed;
		HP = hp;

	}

	//вспомогательная функция, которая отслеживает ID предметов в снаряжении и изходя из этого передает число, которое будет вставляться в массив картинок, для отображения игрока
	int get_rig_index() {

		if (rig.empty()) return 0;

		else return (int)rig[0].ID + 1;

	}

	void set_picture(int i, LPCWSTR name) {

		anim[i] = ((HBITMAP)LoadImageW(NULL, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));

	}

};


struct location_ {

	vector<object_> item;
	HBITMAP hBack; // создадим переменную для нашей картинки 
	vector<portal_> portal;
	vector<object_> platform;
};

Character hero;
Character enemy;
std::vector<Arrow_>arrow;

location_ room[2];


struct {

	POINT p;
	//коллизия мышки
	bool collise_mouse(sprite first) {

		if (p.x >= first.x &&
			p.x <= first.x + first.width &&
			p.y <= first.y + first.height &&
			p.y >= first.y)

			return true;

		else
			return false;

	}


}mouse;

struct menuSettings {

	int menuWidth, menuHeight;
	int centerX, centerY;
	int avatarWidth, avatarHeight;
	int avatarX, avatarY;
	int squaereSize, spacing;
	int itemStartY;

};

object_ Hand;

void InitWindow() { // инициализация структуры window

	RECT r;
	GetClientRect(window.hWnd, &r);
	window.width = r.right - r.left;
	window.height = r.bottom - r.top;


}

//расставляем по локации предметы, противника
void SetINlocation() {

	room[0].item.push_back({ {window.width * SCALE, window.height - hero.model.height, window.width * SCALE * (0.377f), window.height * SCALE * (0.377f), 20},
	(HBITMAP)LoadImageW(NULL, L"sword.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE), item_::Sword });

	room[1].item.push_back({ {window.width * SCALE, window.height - hero.model.height, window.width * SCALE * (0.377f), window.height * SCALE * (0.377f), 30},
	(HBITMAP)LoadImageW(NULL, L"axe.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE), item_::Axe });

	room[1].item.push_back({ {window.width - (window.width * SCALE), window.height - hero.model.height, window.width * SCALE * (0.5f), window.height * SCALE * (0.5f), 10},
	(HBITMAP)LoadImageW(NULL, L"bow.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE), item_::Bow });

	// закидываем порталы
	room[0].portal.emplace_back(window.width - hero.model.width, window.height - hero.model.height * 2, hero.model.width, hero.model.height, L"portal6.bmp", 1);

	room[1].portal.emplace_back(window.width * (0.5f), window.height - hero.model.height * 3, hero.model.width, hero.model.height, L"portal.bmp", 0);

	enemy.item.push_back({ { enemy.model.x, enemy.model.y, window.width * SCALE * (0.377f), window.width * SCALE * (0.377f), 0 },
		(HBITMAP)LoadImageW(NULL, L"key.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE), item_::block });
	
	// платформы
	room[0].platform.push_back({ { window.width * SCALE, window.height - hero.model.height * 1.8f, window.width * SCALE , window.height * SCALE * (0.377f), 20 },
		(HBITMAP)LoadImageW(NULL, L"racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE), item_::block });

	room[0].platform.push_back({ { window.width / 3.f, window.height - hero.model.height * 3, window.width * SCALE , window.height * SCALE * (0.377f), 20 },
		(HBITMAP)LoadImageW(NULL, L"racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE), item_::block });

}

//ставим картинки
void Set_HBITMAP() {

	enemy.picture = (HBITMAP)LoadImageW(NULL, L"E0.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	Hand.picture = (HBITMAP)LoadImageW(NULL, L"hand.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	hero.set_picture(0, L"A0.bmp");
	hero.set_picture(1, L"S.bmp");
	hero.set_picture(2, L"B.bmp");
	hero.set_picture(3, L"A.bmp");

}


void InitGame() {

	// инициализирую переменные в hero, дальше будем делать метод.

	hero.set_parameters(window.width * SCALE, window.height - (window.height * SCALE), window.width * SCALE * (0.377), window.height * SCALE, 20, 100); // метод

	enemy.set_parameters(window.width / 2, hero.model.y, hero.model.width, hero.model.height, 10, 150);

	Set_HBITMAP();

	SetINlocation();


}

//void AnimPortal() {
//
//	if(room[hero.current_loc].portal[0].)
//
//
//
//}

//вспомогательная функиця которая проверяет находиться ли first внутри second, проверка коллизии  
bool СheckCollise(sprite first, sprite second) {


	if (first.x <= second.x + second.width && +
		first.x + first.width >= second.x &&
		first.y <= second.y + second.height &&
		first.y + first.height >= second.y) {

		return true;
	}

	return false;

}

//коллизия шара
void Collise_ball() {


	if (!hero.rig.empty() && hero.rig[0].ID == item_::Bow) {

		for (auto ball : arrow) {

			if (СheckCollise(enemy.model, ball.model)) {


				if (enemy.HP > 0) {

					enemy.HP -= hero.rig[0].model.speed;
					enemy.model.y -= 500;

				}
				else {

					for (int i = 0; i < enemy.item.size(); i++) {

						room[hero.current_loc].item.emplace_back(enemy.item[i]);
						enemy.item.erase(enemy.item.cbegin() + i);
						enemy.item.clear();
					}

					enemy.picture = NULL;

					enemy.set_parameters(rand() % window.width, window.height / 2, 100, 150, 10, 100);

				}

			}

		}

	}

}

void ColliseBlock() {

	for (auto p : room[hero.current_loc].platform) {

		if (СheckCollise(hero.model, p.model)) {

			float UP = abs(p.model.y - (hero.model.y + hero.model.height));
			float DOWN = abs((p.model.y + p.model.height) - hero.model.y);
			int over_Y = min(UP, DOWN);

			float LEFT = abs(p.model.x - (hero.model.x + hero.model.width));
			float RIGHT = abs(p.model.x + p.model.width - hero.model.x);
			float over_X = min(LEFT, RIGHT);

			if (over_X < over_Y)
			{
				if (LEFT < RIGHT)
				{
					hero.model.x = p.model.x - hero.model.width;
				}
				else
				{
					hero.model.x = p.model.x + p.model.width;
				}
			}
			else {
				if (UP < DOWN)
				{
					hero.model.y = p.model.y - hero.model.height;
					hero.model.inJump = false;
				}
				else
				{
					hero.model.y = p.model.y + p.model.height;
				}
			}
		}
	}
}



//боевка с противником
void Enemy_Fight() {

	if (СheckCollise(enemy.model, hero.model)) {


		if (!hero.rig.empty() && (hero.rig[0].ID == item_::Axe || hero.rig[0].ID == item_::Sword)) {


			if (enemy.HP > 0) {

				enemy.HP -= hero.rig[0].model.speed;
				enemy.model.y -= 500;

			}
			else {

				for (int i = 0; i < enemy.item.size(); i++) {

					room[hero.current_loc].item.emplace_back(enemy.item[i]);
					enemy.item.erase(enemy.item.cbegin() + i);
					enemy.item.clear();
				}

				enemy.picture = NULL;

				enemy.set_parameters(rand() % window.width, window.height / 2, 100, 150, 10, 100);

			}

		}

	}

}

// логика движения противника
void EnemyMove() {

	float dist = 500;

	float distToHero = abs(enemy.model.x - hero.model.x);


	if (distToHero < dist && enemy.model.y - hero.model.y < 200) {

		if (enemy.model.x < hero.model.x) { // right enemy

			enemy.HP > 20 ? enemy.model.x += 10 : enemy.model.x -= 10;


		}
		else {

			enemy.HP > 20 ? enemy.model.x -= 10 : enemy.model.x += 10;

		}

		Enemy_Fight();


	}

	enemy.model.y += 30;
	enemy.model.y = min((enemy.model.y), (window.height - enemy.model.height));

}


// отрисовка
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



// подсчет размера меню
menuSettings MenuCalculate() {

	menuSettings layot;

	// размеры меню, делаем в процентах от экрана, чтобы внезависимо от экрана меню ровно вставало
	layot.menuWidth = (int)(window.width * 0.20f);
	layot.menuHeight = (int)(window.height * 0.35f);
	layot.spacing = (int)(layot.menuHeight * 0.02f); // отступ в 2% ширины экрана

	// позиция меню относительно экрана
	layot.centerX = (window.width - layot.menuWidth) / 2;
	layot.centerY = (window.height - layot.menuHeight) / 2;

	// расчитываем размеры игрока относительно меню как делали с меню, только вместо ширины/высоты экрана, у нас меню
	layot.avatarWidth = (int)(layot.menuWidth * 0.3f); // 30% от ширины меню
	layot.avatarHeight = (int)(layot.menuHeight * 0.45f);// 45% от высоты меню

	// ставим аватар героя в меню 
	layot.avatarX = layot.centerX + (layot.menuWidth - layot.avatarWidth) / 2;
	layot.avatarY = layot.centerY + layot.spacing; // небольшой оступ от y меню

	// расчитаем размер квадрата предмета который будет помещен в инвентарь
	layot.squaereSize = (int)(layot.menuWidth * 0.1f); //10% от ширины экрана
	layot.itemStartY = layot.avatarY + layot.avatarHeight + (int)(layot.menuHeight * 0.05f); // стартовая позиция отрисовки предметов 

	return layot;

}

// отрисовка заднего фона
void DrawMenuBackground(HDC hMemDC, const menuSettings& layot) {

	DrawBitmap(hMemDC, layot.centerX, layot.centerY, layot.menuWidth, layot.menuHeight, menu, false);

}

//отрисовка аватара в меню
void DrawAvatarinMenu(HDC hMemDC, const menuSettings& layot) {


	DrawBitmap(hMemDC, layot.avatarX, layot.avatarY, layot.avatarWidth, layot.avatarHeight, hero.anim[hero.get_rig_index()], false);

}

// отрисовка предметов в меню
void DrawMenuItems(HDC hMemDC, const menuSettings& layout) {

	if (hero.item.empty()) return;

	for (int i = 0; i < hero.item.size(); i++) {

		hero.item[i].model.x = layout.centerX + i * (layout.menuWidth - layout.squaereSize) / 7;
		hero.item[i].model.y = layout.itemStartY + (layout.squaereSize + layout.spacing);
		hero.item[i].model.width = layout.squaereSize;
		hero.item[i].model.height = layout.squaereSize;

		DrawBitmap(hMemDC, hero.item[i].model.x, hero.item[i].model.y, hero.item[i].model.width, hero.item[i].model.height, hero.item[i].picture, false); // предметы в слотах

	}

}

//отрисовка снаряжения в меню
void DrawMenuHand(HDC hMemDC, const menuSettings& layout) {

	Hand.model.x = layout.avatarX + layout.squaereSize * 4;
	Hand.model.y = layout.avatarY + layout.squaereSize;
	Hand.model.width = layout.squaereSize;
	Hand.model.height = layout.squaereSize;

	DrawBitmap(hMemDC, Hand.model.x, Hand.model.y, Hand.model.width, Hand.model.height, hero.rig.empty() ? Hand.picture : hero.rig[0].picture, false); // слот рук


}

//главная функция отрисовки меню
void Menu(HDC hMemDC) {

	if (!GameActiv) {

		//Back_menu(hMemDC);

		menuSettings layout = MenuCalculate();

		DrawMenuBackground(hMemDC, layout);
		DrawAvatarinMenu(hMemDC, layout);
		DrawMenuItems(hMemDC, layout);
		DrawMenuHand(hMemDC, layout);



	}

}

// отрисовка всех объектов
void ShowObject(HDC hMemDC) {

	// задник
	DrawBitmap(hMemDC, 0, 0, window.width, window.height, room[hero.current_loc].hBack, false);

	// отрисовка предметов
	for (auto i : room[hero.current_loc].item) {

		DrawBitmap(hMemDC, i.model.x, i.model.y, i.model.width, i.model.height, i.picture, false);

	}
	// порталы
	for (auto p : room[hero.current_loc].portal) {

		DrawBitmap(hMemDC, p.model.x, p.model.y, p.model.width, p.model.height, p.anim[p.currentFrame], true);

	}

	// герой
	DrawBitmap(hMemDC, hero.model.x, hero.model.y, hero.model.width, hero.model.height, hero.anim[hero.get_rig_index()], true);

	//отрисовка стрел
	for (auto a : arrow) {

		if (a.activ)

			DrawBitmap(hMemDC, a.model.x, a.model.y, a.model.width, a.model.height, a.picture, true);

	}
	//отрисовка противника
	DrawBitmap(hMemDC, enemy.model.x, enemy.model.y, enemy.model.width, enemy.model.height, enemy.picture, true);

	for (auto p : room[hero.current_loc].platform) {


			DrawBitmap(hMemDC, p.model.x, p.model.y, p.model.width, p.model.height, p.picture, false);

	}

	Menu(hMemDC);



}

// логика поралов
void Portal_Logic() {

	for (auto p : room[hero.current_loc].portal) {

		if (СheckCollise(hero.model, p.model)) { // если произошла коллизия с порталом, то пепермещаем игрока

			hero.current_loc = p.target;
			hero.model.x = hero.model.width;

		}
	}

}



// опрос клавиатуры
void ProcesImput() {

	static bool drop = false;
	static float jump = 0;

	float gravity = 30;
	if (GetAsyncKeyState('A')) hero.model.x -= hero.model.speed;
	if (GetAsyncKeyState('D')) hero.model.x += hero.model.speed;

	if (GetAsyncKeyState(VK_SPACE) && !hero.model.inJump) {

		drop = true;
		jump = 90;
		hero.model.inJump = true;

	}

	hero.model.y += gravity - jump;
	jump *= .9;
	hero.model.y = min((hero.model.y), (window.height - hero.model.height));

	if (hero.model.y + hero.model.height >= window.height)
		hero.model.inJump = false;
}

//логика полета шара
void ProcesBall() {

	for (int i = 0; i < arrow.size(); i++) {

		if (arrow[i].activ) {
			arrow[i].model.x += arrow[i].directionX * arrow[i].model.speed;
			arrow[i].model.y += arrow[i].directionY * arrow[i].model.speed;
		}
	}
}

//логика подбора
void Pick() {

	for (int i = 0; i < room[hero.current_loc].item.size(); i++) {
		if (mouse.collise_mouse(room[hero.current_loc].item[i].model)) {

			hero.item.emplace_back(room[hero.current_loc].item[i]);
			room[hero.current_loc].item.erase(room[hero.current_loc].item.cbegin() + i);

		}

	}

	if (!GameActiv) {

		for (int i = 0; i < hero.item.size(); i++) {
			if (mouse.collise_mouse(hero.item[i].model)) {

				hero.item[i].model.x = hero.model.x + hero.model.width * 2;
				hero.item[i].model.y = hero.model.y;

				room[hero.current_loc].item.emplace_back(hero.item[i]);
				hero.item.erase(hero.item.cbegin() + i);

			}

		}


	}


}

//логика правой кнопки мыши
void Rbutton() {


	for (int i = 0; i < hero.item.size(); i++) {

		if (mouse.collise_mouse(hero.item[i].model)) {

			if (hero.rig.size() < 1) {

				hero.rig.emplace_back(hero.item[i]);
				hero.item.erase(hero.item.cbegin() + i);

			}

		}

	}

	for (int i = 0; i < hero.rig.size(); i++) {

		if (mouse.collise_mouse(Hand.model)) {

			hero.item.emplace_back(hero.rig[i]);
			hero.rig.erase(hero.rig.cbegin() + i);


		}

	}


}

// границы комнаты, чтобы чел не ушел
void Proces_room() {


	if (hero.model.x <= window.width - window.width)
		hero.model.x = 0;

	if (hero.model.x >= window.width - hero.model.width)

		hero.model.x = window.width - hero.model.width;

}

//расчет направления шара
void Mouse_Action() {


	arrow.push_back(Arrow_(hero.model.x + hero.model.width, hero.model.y + hero.model.height / 2));

	// Вычисляем вектор направления
	int targetX = mouse.p.x;
	int targetY = mouse.p.y;

	for (int i = 0; i < arrow.size(); i++) {

		if (!arrow[i].activ) {

			// Вычисляем разницы
			int diffX = targetX - arrow[i].model.x;
			int diffY = targetY - arrow[i].model.y;

			// Вычисляем длину вектора (расстояние)
			float distance = sqrt(diffX * diffX + diffY * diffY);

			// Нормализуем вектор (делаем длину = 1)
			if (distance > 0) {
				arrow[i].directionX = diffX / distance;
				arrow[i].directionY = diffY / distance;
			}
			else {
				arrow[i].directionX = 0;
				arrow[i].directionY = 0;
			}

			arrow[i].activ = true;
		}
	}


}

//очистка шаров
void Clean_arrows() {

	for (int i = arrow.size() - 1; i >= 0; i--) {

		if (arrow[i].isOutOfScreen()) {

			if (arrow[i].picture != NULL) {
				DeleteObject(arrow[i].picture);
				arrow[i].picture = NULL;
			}

			arrow.erase(arrow.begin() + i);
		}

	}

}

void Process_game() {

	GetCursorPos(&mouse.p);

	if (GameActiv) {

		ProcesImput();
		Proces_room();
		Portal_Logic();
		EnemyMove();
		ProcesBall();
		Collise_ball();
		ColliseBlock();
	}
}

// кесы для WindoProc, чтобы код был более читаемый
void Case_KEYdown(WPARAM wParam, HWND hwnd) {

	if (wParam == VK_ESCAPE) {

		DestroyWindow(hwnd);

	}

	if (wParam == 'I') {

		GameActiv ? GameActiv = false : GameActiv = true;

	}

	if (wParam == 'E')
		Pick();

}

void Case_Destroy(HWND hwnd) {

	PostQuitMessage(0);
	KillTimer(hwnd, 1);
	KillTimer(hwnd, 2);

}

void Case_Create(HWND hwnd) {

	room[0].hBack = (HBITMAP)LoadImageW(NULL, L"fon2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); // загружаем картинку в переменную HBITMAP 
	room[1].hBack = (HBITMAP)LoadImageW(NULL, L"test.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); // загружаем картинку в переменную HBITMAP 


	if (!room[0].hBack || !room[0].hBack) MessageBoxW(hwnd, L"Не удалось!", L"ОШИБКА", MB_ICONERROR);

}

void Case_Timer(WPARAM wParam, HWND hwnd) {

	if (wParam == 1) { // у каждого таймера есть свой айди, и если таймер под айдишником 1 закончился, то мы запускаем то что ниже

		InvalidateRect(hwnd, NULL, FALSE); // перерисовка всего окна
		Process_game();

	}

	if (wParam == 2 && time_at) {

		KillTimer(hwnd, 2);
		Mouse_Action();
		Clean_arrows();
		time_at = false;
	}

	if (wParam == 3) {

		room[0].portal[0].HelpAnim();

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
	SetTimer(window.hWnd, 3, 160, NULL);// таймер для анимаций


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

	case WM_LBUTTONDOWN:

		if (!time_at && !hero.rig.empty() && hero.rig[0].ID == item_::Bow) {
			SetTimer(hwnd, 2, 600, NULL);
			time_at = true;
		}
		break;

	case WM_RBUTTONDOWN:

		Rbutton();
		break;

	case WM_DESTROY: // когда уничтожается
		Case_Destroy(hwnd);
		return 0;


	case WM_CREATE: { // кейс когда создается окно 

		Case_Create(hwnd);
		break;

	}
	case WM_TIMER:

		Case_Timer(wParam, hwnd);

		break;

	case WM_PAINT: { // вывод на экран картинки 

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		Case_Paint(hdc);
		EndPaint(hwnd, &ps);
	}

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

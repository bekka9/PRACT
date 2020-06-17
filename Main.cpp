
#include <iostream>
#include <opencv2/opencv.hpp>
#include <ViZDoom.h>
#include <chrono>
#include <thread>

using namespace vizdoom;

DoomGame* game = new DoomGame();
const std::vector<double> actions[3] = {
	{ 1, 0, 0 }, // left
	{ 0, 1, 0 }, // right
	{ 0, 0, 1 }, // shoot
};

void sleep(unsigned int time) {
	std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

void game_init() {
	game->setViZDoomPath("../vizdoom/vizdoom");	//задаем путь к исполняемому движку
	game->setDoomGamePath("../vizdoom/freedoom2.wad");	//устанавливаем путь к файлу
	game->loadConfig("../vizdoom/scenarios/basic.cfg");	//загружаем конфигурацию
	game->setScreenResolution(RES_640X480); 	//устанавливаем разрешение экрана
	game->setLabelsBufferEnabled(true); 	// включаем буфер lable
	game->setWindowVisible(true); 	//делаем окно видимым
	game->setRenderWeapon(true);  //добавляем оружие игрока
	game->setRenderHud(true); //добавляем индикатор
	game->init(); 	//инициализируем игру
}

void find_demon_and_kill(GameStatePtr state) {
	double eps = 10; // monster's width (ширина монстра)
	if (state->labels[0].objectPositionY - eps > state->labels[1].objectPositionY) {
		game->makeAction(actions[0]); //left
	}	//проверяем положение человека относительно демона: если находимся правее, то передвигаемся в левую сторону
	else if (state->labels[0].objectPositionY + eps < state->labels[1].objectPositionY) {
		game->makeAction(actions[1]); //right
	}	//проверяем положение человека относительно демона: если находимся левее, то передвигаемся в правую сторону
	else {
		game->makeAction(actions[2]); // shoot (стреляем в демона)
	}
}

int main() {
	game_init();

	int episodes = 10;
	unsigned int sleepTime = 2000 / DEFAULT_TICRATE;
	for (int i = 0; i < episodes; ++i) {
		std::cout << "Episode #" << i + 1 << "\n";
		game->newEpisode();
		while (!game->isEpisodeFinished()) {
			auto state = game->getState();
			auto screenBuf = state->screenBuffer;

			find_demon_and_kill(state);
			sleep(sleepTime);
		}
		sleep(sleepTime * 10);
		std::cout << "Total reward is: " << game->getTotalReward() << std::endl;
	}
	game->close();
	delete game;
}

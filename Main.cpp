#include <iostream>
#include <opencv2/opencv.hpp>
#include <ViZDoom.h>
#include <chrono>
#include <thread>
#include <opencv2/core/mat.hpp>



std::string path = "C:\\practice\\vizdoom";
auto game = std::make_unique<vizdoom::DoomGame>();
const unsigned int sleepTime = 1000 / vizdoom::DEFAULT_TICRATE;
auto screenBuff = cv::Mat(480, 640, CV_8UC3);
double s;
void RunTask1(int episodes)
{
	try
	{
		game->loadConfig(path + "\\scenarios\\task1.cfg");
		game->init(); 	
	}
	catch(std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	auto greyscale = cv::Mat(480, 640, CV_8UC1);
	std::vector<double> action;
	cv::Mat labels;
	for (auto i = 0; i < episodes; i++)
	{
		game->newEpisode();
		std::cout << "Episode #" << i + 1 << std::endl;
		
		while (!game->isEpisodeFinished())
		{
			//версия с кластерами: 
			
			const auto& gamestate = game->getState();

			std::memcpy(screenBuff.data, gamestate->screenBuffer->data(), gamestate->screenBuffer->size());

			cv::extractChannel(screenBuff, greyscale, 2);

			cv::threshold(greyscale, greyscale, 200, 255, cv::THRESH_BINARY);

			std::vector<cv::Point2f> data(0);
			for (int l = 0; l < 640; l++)
			{
				for (int j = 0; j < 480; j++)
				{
					if ((int)greyscale.at<unsigned char>(j,l) == 255) {
						data.push_back(cv::Point2f(l, j));
						
					}
				}
				
			}
		
			
			//cv::Mat   bestLabels, centers;

			//std::vector<cv::Point> centers;
			//cv::kmeans(greyscale, 2, bestLabels, cv::TermCriteria(cv::TermCriteria::EPS, max, eps), 10, cv::KMEANS_PP_CENTERS, centers);
			
			/*
			cv::Mat data = greyscale;
			data = data.reshape(1, data.cols / 2); // n rows a 2 cols
			data.convertTo(data, CV_32F);
			*/
			
			
			std::vector<cv::Point2f> centers;
			greyscale.convertTo(greyscale, CV_32F);

			cv::kmeans(data, 2, labels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0), 3, cv::KMEANS_RANDOM_CENTERS, centers);
			greyscale.convertTo(greyscale, CV_8UC1);

			//centers = centers.reshape(2, centers.rows);
			//data = data.reshape(2, data.rows);

			//cv::circle(greyscale, cv::Point(centers.at<float>(0, 0), centers.at<float>(0, 1)), 4, cv::Scalar(200), 5);
			cv::Point center = centers[0];
			cv::circle(greyscale,center, 4, cv::Scalar(200,0,0), 5);
			//centers[0].
			
			
			float y1 = centers[0].y;
			float y2 = centers[1].y;
			double eps = 21;
			if (y2 < y1) { // тогда y[0] демон 

				if (centers[0].x + eps < centers[1].x) {
					game->makeAction({ 0, 1, 0 }); //
				}
				else if (centers[0].x - eps > centers[1].x) {
					game->makeAction({ 1, 0, 0 }); //
				}
				else {
					game->makeAction({ 0, 0, 1 }); // shoot
				}
			}
			else {
				 
				if (centers[0].x - eps > centers[1].x) {
					game->makeAction({ 0, 1, 0 }); //
				}
				else if (centers[0].x + eps < centers[1].x) {
					game->makeAction({ 1, 0, 0 }); //
				}
				else {
					game->makeAction({ 0, 0, 1 }); // shoot
				}
			}
			
			
			cv::imshow("Output Window", greyscale);
			cv::waitKey(sleepTime);
			
			
		}

		s = s + game->getTotalReward();
		std::cout << std::endl << game->getTotalReward() << std::endl;
	}
	
	std::cout << std::endl << "Arithmetic mean of 10 ep: " << s / 10 << std::endl;
}



int main()
{
	game->setViZDoomPath(path + "\\vizdoom.exe");	
	game->setDoomGamePath(path + "\\freedoom2.wad");	

	cv::namedWindow("Output Window", cv::WINDOW_AUTOSIZE);
	auto episides = 10;

	//=======================
	RunTask1(episides);
	//=======================

	game->close();
}




/*
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
	game->loadConfig("../vizdoom/scenarios/basic.cfg");	//загружаем конфигурацию, сценарий игры
	game->setScreenResolution(RES_640X480); 	//устанавливаем разрешение экрана
	game->setLabelsBufferEnabled(true); 	// включаем буфер lable
	game->setWindowVisible(true); 	//делаем окно видимым
	game->setRenderWeapon(true);  //добавляем оружие игрока
	game->setRenderHud(true); //добавляем меню 
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
	unsigned int sleepTime = 2000 / DEFAULT_TICRATE; //пауза после действия, замедляет игру
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
		std::cout << "Total reward is: " << game->getTotalReward() << std::endl; //очки
	}
	game->close();
	delete game;
}
*/

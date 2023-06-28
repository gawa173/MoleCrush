# include <Siv3D.hpp>


struct GameData {
	int32 score = 0;
	int32 mole_x[9] = { 300, 400, 500, 300, 400, 500, 300, 400, 500 };
	int32 mole_y[9] = { 200, 200, 200, 300, 300, 300, 400, 400, 400 };
	int timeLimitSeconds = 30;
	int remainingTime = timeLimitSeconds;
	double timeAccumulator = 0.0;
	double SpawnTime = 1.0;
	Circle circle{ 200, 200, 20 };
};

using App = SceneManager<String, GameData>;

class Title : public App::Scene {
public:
	Title(const InitData& init)
		: IScene{ init }
	{
		
	}

	const Audio start_audio{ Resource(U"sound/start.mp3") };
	const Texture mole{ U"image/mole.png" };
	const Texture hole{ U"image/hole.png" };

	void update() override
	{
		if (SimpleGUI::Button(U"NORMAL", Vec2{ 300,350 }, 200)) {
			start_audio.play();
			changeScene(U"Game");
		}
		if (SimpleGUI::Button(U"HARD", Vec2{ 300,450 }, 200)) {
			getData().SpawnTime = 0.5;
			start_audio.play();
			changeScene(U"Game");
		}
	}

	void draw() const override
	{
		Scene::SetBackground(Palette::Olivedrab);

		FontAsset(U"TitleFont")(U"MoleCrush").drawAt(400, 200);

		hole.scaled(0.2).drawAt(150, 300);
		mole.scaled(0.3).drawAt(150, 300);

		hole.scaled(0.2).drawAt(650, 400);
		mole.scaled(0.3).drawAt(650, 400);
	}
};

class Game : public App::Scene
{
public:

	Game(const InitData& init)
		: IScene{ init }
	{

	}

	Stopwatch stopwatch{ StartImmediately::Yes };
	double last_mole_time = 0;
	int32 mole_pos = Random(8);
	const Texture hit{ U"💥"_emoji };
	const Texture mole{ U"image/mole.png" };
	const Texture hole{ U"image/hole.png" };
	const Texture cursor{ U"image/cursor.png" };
	const Audio hit_audio{ Resource(U"sound/hit.mp3") };
	

	void update() override
	{
		//タイマー経過時間
		const int64 elapsedTime = stopwatch.ms() / 1000;
		getData().remainingTime = getData().timeLimitSeconds - elapsedTime;

		if (getData().remainingTime > 0) {
			FontAsset(U"ScoreFont")(Format(getData().score)).drawAt(40, 500);
			Rect{ 250,150,300,300 }.drawFrame(0, 3);
			Line{ 250, 250, 550, 250 }.draw(3, Palette::White);
			Line{ 250, 350, 550, 350 }.draw(3, Palette::White);
			Line{ 350, 150, 350, 450 }.draw(3, Palette::White);
			Line{ 450, 150, 450, 450 }.draw(3, Palette::White);
			for (auto i : step(9))
				hole.scaled(0.1).drawAt(getData().mole_x[i], getData().mole_y[i]);

			//残り時間を表示
			FontAsset(U"TimerFont")(Format(getData().remainingTime)).drawAt(40, 40);
			
			//スポーン経過時間
			double spawn_elapsedTime = static_cast<double>(elapsedTime) - last_mole_time;


			if (spawn_elapsedTime <= getData().SpawnTime) {
				getData().circle = { getData().mole_x[mole_pos], getData().mole_y[mole_pos], 35 };//当たり判定用の円
				mole.scaled(0.2).drawAt(getData().mole_x[mole_pos], getData().mole_y[mole_pos]);//モグラを表示
			}
			else {
				last_mole_time = static_cast<double>(elapsedTime);
				mole_pos = Random(8);
			}

			//もぐらをクリックした場合
			if (getData().circle.leftClicked()) {
				hit.scaled(1.3).drawAt(getData().mole_x[mole_pos], getData().mole_y[mole_pos]);
				hit_audio.play();
				getData().score += 1;
				mole_pos = Random(8);
				last_mole_time = static_cast<double>(elapsedTime);
			}
		}

		//終了
		else {
			if (getData().score <= 40) {
				FontAsset(U"EndFont")(U"Good!!").drawAt(400, 200);
			}
			else if (getData().score < 60) {
				FontAsset(U"EndFont")(U"Great!!!").drawAt(400, 200);
			}
			else {
				FontAsset(U"EndFont")(U"Excellent!!!!").drawAt(400, 200);
			}
			
			FontAsset(U"ScoreFont")(U"Score:"+ Format(getData().score)).drawAt(400, 300);

			if (SimpleGUI::Button(U"RETRY", Vec2{ 300,400 },200)) {
				getData().score = 0;
				changeScene(U"Game");
			}

			if (SimpleGUI::Button(U"TITLE", Vec2{ 300, 480 },200)) {
				getData().score = 0;
				changeScene(U"Title");
			}
		}
	}

	void draw() const override
	{
		Scene::SetBackground(Palette::Olivedrab);
		cursor.scaled(0.3).drawAt(Cursor::Pos());
		Cursor::RequestStyle(CursorStyle::Hidden);
	}
};

void Main() {
	FontAsset::Register(U"TitleFont", 60, Typeface::Heavy);
	FontAsset::Register(U"TimerFont", 48);
	FontAsset::Register(U"ScoreFont", 48);
	FontAsset::Register(U"EndFont", 60, Typeface::Black);

	const Audio BGM{ Resource(U"sound/BGM.mp3"), Loop::Yes };
	BGM.setVolume(0.5).play();

	// シーンマネージャーを作成
	App manager;

	manager.add<Title>(U"Title");
	manager.add<Game>(U"Game");

	while (System::Update())
	{
		// 現在のシーンを実行
		// シーンに実装した .update() と .draw() が実行される
		if (not manager.update())
		{
			break;
		}
	}
}

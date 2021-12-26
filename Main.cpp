#include <Siv3D.hpp> // OpenSiv3D v0.6.3
template <typename T>
bool chmin(T& a, const T& b) {
	if (a > b) {
		a = b;
		return true;
	}
	return false;
}
template <typename T>
bool chmax(T& a, const T& b) {
	if (a < b) {
		a = b;
		return true;
	}
	return false;
}
void Main() {
	int h = 2, w = 3;
	int mgHpx = 100, mgWpx = 50;
	int cellh = 50;
	int headh = 25;
	int mgHead = 10;
	std::vector<int> cellws(w, 100);

	Font fontSub(30);
	Font font(35);

	TextEditState tbH;
	tbH.text = U"2";
	TextEditState tbW;
	tbW.text = U"3";
	TextEditState tbInput;
	int focCelli = 0, focCellj = 0;

	std::vector<std::vector<String>> data(h, std::vector<String>(w));

	int ladjIdx = -1;

	double preClickTime = Scene::Time();

	while (System::Update()) {
		std::vector<int> posW(w + 1);
		for (int i = 0; i < w; i++) posW[i + 1] = posW[i] + cellws[i];
		
		auto idxToPos = [&](int i, int j) {
			return Vec2(mgWpx + posW[j], mgHpx + cellh * i);
		};

		Point mp = Cursor::Pos();
		if (MouseL.down()) {
			// 列幅調整
			bool isAdj = false;
			if (mgHpx - headh - mgHead <= mp.y && mp.y <= mgHpx - mgHead) {
				for (int i = 1; i <= w; i++) {
					if (abs(mgWpx + posW[i] - mp.x) < 10) {
						ladjIdx = i - 1;
						isAdj = true;
					}
				}
			}
			// 列幅調整ダブルクリック
			if (isAdj && Scene::Time() - preClickTime < 0.5) {
				int fitWidth = 50;
				for (int i = 0; i < h; i++) {
					chmax(fitWidth, (int)font(data[i][ladjIdx]).region().w + 10 * 2 + 2);
				}
				cellws[ladjIdx] = fitWidth;
				ladjIdx = -1;
			}
			preClickTime = Scene::Time();
			// セル選択
			if (!isAdj && mp.y > mgHpx - mgHead - headh) {
				int seli = -1, selj = -1;
				for (int i = 0; i < h; i++) {
					for (int j = 0; j < w; j++) {
						if (mgHpx + cellh * i <= mp.y && mp.y <= mgHpx + cellh * (i + 1)
							&& mgWpx + posW[j] <= mp.x && mp.x <= mgWpx + posW[j + 1]) {
							seli = i;
							selj = j;
							tbInput.text = data[i][j];
						}
					}
				}
				focCelli = seli;
				focCellj = selj;
			}
		}
		if (MouseL.pressed()) {
			if (ladjIdx != -1) {
				Cursor::RequestStyle(CursorStyle::ResizeLeftRight);
				cellws[ladjIdx] += mp.x - posW[ladjIdx + 1] - mgWpx;
				chmax(cellws[ladjIdx], 50);
			}
		}

		if (MouseL.up()) {
			ladjIdx = -1;
		}
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				RectF(idxToPos(i, j), cellws[j], cellh).draw(Palette::White).drawFrame(1, Palette::Black);
			}
		}

		// 列タブ描画
		for (int j = 0; j < w; j++) {
			Rect(mgWpx + posW[j], mgHpx - headh - mgHead, cellws[j], headh).draw(Palette::Lightgray).drawFrame(1, Palette::Black);
		}
		// 行タブ描画
		for (int i = 0; i < h; i++) {
			Rect(mgWpx -headh-mgHead, mgHpx +cellh*i, headh, cellh).draw(Palette::Lightgray).drawFrame(1, Palette::Black);
		}

		// フォーカスしているセル
		if (focCelli != -1 && focCellj != -1) {
			RectF(idxToPos(focCelli, focCellj), cellws[focCellj], cellh).drawFrame(4, Palette::Green);
			data[focCelli][focCellj] = tbInput.text;
		}
		else {
			tbInput.text = U"";
		}

		// テキストdataの描画
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				if (font(data[i][j]).region().w + 10 * 2 <= cellws[j]) {
					font(data[i][j]).draw(10 + mgWpx + posW[j], mgHpx + cellh * i, Palette::Black);
				}
				else {
					String drData = data[i][j];
					while (drData.size() > 0) {
						drData.pop_back();
						if (font(drData + U"...").region().w + 10 * 2 <= cellws[j]) {
							font(drData + U"...").draw(10 + mgWpx + posW[j], mgHpx + cellh * i, Palette::Black);
							break;
						}
					}
				}
			}
		}

		SimpleGUI::TextBox(tbH, Vec2(mgWpx, 10), 60, 4);
		SimpleGUI::TextBox(tbW, Vec2(mgWpx + 100, 10), 60, 4);
		int th = ParseOr<int>(tbH.text, -1);
		int tw = ParseOr<int>(tbW.text, -1);
		if ((tbH.text == U"" || th <= 0) || (tbW.text == U"" || tw <= 0)) {
			// 求 自然数
			th = 1;
			tw = 1;
		}
		if (th != h || tw != w) {
			// 表サイズの変更
			w = tw;
			h = th;
			cellws.resize(w, 100);
			data.resize(h);
			for (int i = 0; i < h; i++) data[i].resize(w, U"");
			focCelli = 0;
			focCellj = 0;
			tbInput.text = U"";
		}
		fontSub(U"列").draw(mgWpx + 65, 10);
		fontSub(U"行").draw(mgWpx + 100 + 65, 10);
		if (focCelli != -1 && focCellj != -1) {
			SimpleGUI::TextBox(tbInput, idxToPos(focCelli, focCellj) + Vec2(10, 10), cellws[focCellj]);
		}
	}
}

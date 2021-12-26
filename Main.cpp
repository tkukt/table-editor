#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include <Siv3D/Windows/Windows.hpp>
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



struct messageBox {
	String text;
	enum style {
		mb_YESNO,
		mb_OK
	};
	const Font font;
	int type;
	bool enable;
	int res;
	enum resType {
		NONE,
		YES,
		NO,
		OK
	};
	messageBox(String text, const Font& font, int type) : text(text), font(font), type(type), enable(false), res(resType::NONE) {}
	void draw() {
		assert(enable);
		Rect(Arg::center(400, 300), 300, 200).drawShadow(Vec2(4, 4), 16, 2, Palette::Lightgray).draw(Palette::White).drawFrame(2, Palette::Gray);
		font(text).draw(Rect(Arg::center(400, 290), 240, 120), Palette::Black);
		if (SimpleGUI::Button(U"はい", Vec2(280, 350), 100)) {
			res = resType::YES;
		}
		if (SimpleGUI::Button(U"いいえ", Vec2(420, 350), 100)) {
			res = resType::NO;
		}
	}
};

// ウィンドウ位置定数
const int mgHpx = 100, mgWpx = 50;
const int cellh = 50;
const int headh = 25;
const int mgHead = 10;

struct CellData {
	int h, w;
	std::vector<int> cellws;
	std::vector<int> sumW;
	std::vector<std::vector<String>> str;
	CellData() : h(2), w(3), cellws(w, 100), sumW(w + 1), str(h, std::vector<String>(w)) {}
	void reCalcWSum() {
		for (int i = 0; i < w; i++) sumW[i + 1] = sumW[i] + cellws[i];
	}
	void resize(int th, int tw) {
		w = tw;
		h = th;
		cellws.resize(w, 100);
		sumW.resize(w + 1);
		str.resize(h);
		for (int i = 0; i < h; i++) str[i].resize(w, U"");
	}
};

void mouseEventProc(CellData& cellData, const Font& font, double& preClickTime, int& ladjIdx, int& focCelli, int& focCellj, TextEditState& tbInput) {
	Point mp = Cursor::Pos();
	int hoverLadjIdx = -1;
	if (mgHpx - headh - mgHead <= mp.y && mp.y <= mgHpx - mgHead) {
		for (int i = 1; i <= cellData.w; i++) {
			if (abs(mgWpx + cellData.sumW[i] - mp.x) < 10) {
				hoverLadjIdx = i - 1;
			}
		}
	}
	bool isAdj = hoverLadjIdx != -1;
	if (isAdj) {
		Cursor::RequestStyle(CursorStyle::ResizeLeftRight);
	}
	if (MouseL.down()) {
		// 列幅調整
		ladjIdx = hoverLadjIdx;
		// 列幅調整ダブルクリック
		if (isAdj && Scene::Time() - preClickTime < 0.5) {
			int fitWidth = 50;
			for (int i = 0; i < cellData.h; i++) {
				chmax(fitWidth, (int)font(cellData.str[i][ladjIdx]).region().w + 10 * 2 + 2);
			}
			cellData.cellws[ladjIdx] = fitWidth;
			ladjIdx = -1;
		}
		preClickTime = Scene::Time();
		// セル選択
		if (!isAdj) {
			int seli = -1, selj = -1;
			for (int i = 0; i < cellData.h; i++) {
				for (int j = 0; j < cellData.w; j++) {
					if (mgHpx + cellh * i <= mp.y && mp.y <= mgHpx + cellh * (i + 1)
						&& mgWpx + cellData.sumW[j] <= mp.x && mp.x <= mgWpx + cellData.sumW[j + 1]) {
						seli = i;
						selj = j;
						tbInput.text = cellData.str[i][j];
					}
				}
			}
			focCelli = seli;
			focCellj = selj;
		}
	}
	if (MouseL.pressed()) {
		if (ladjIdx != -1) {
			cellData.cellws[ladjIdx] += mp.x - cellData.sumW[ladjIdx + 1] - mgWpx;
			chmax(cellData.cellws[ladjIdx], 50);
		}
	}

	if (MouseL.up()) {
		ladjIdx = -1;
	}
}
auto idxToPos = [&](const CellData& cellData, int i, int j) {
	return Vec2(mgWpx + cellData.sumW[j], mgHpx + cellh * i);
};
void drawCellData(CellData & cellData,const Font&font, const int&focCelli,const int&focCellj) {


	// セルの描画
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			RectF(idxToPos(cellData, i, j), cellData.cellws[j], cellh).draw(Palette::White).drawFrame(1, Palette::Black);
		}
	}

	// 列タブ描画
	for (int j = 0; j < cellData.w; j++) {
		Rect(mgWpx + cellData.sumW[j], mgHpx - headh - mgHead, cellData.cellws[j], headh).draw(Palette::Lightgray).drawFrame(1, Palette::Black);
	}
	// 行タブ描画
	for (int i = 0; i < cellData.h; i++) {
		Rect(mgWpx - headh - mgHead, mgHpx + cellh * i, headh, cellh).draw(Palette::Lightgray).drawFrame(1, Palette::Black);
	}

	// フォーカスしているセル
	if (focCelli != -1 && focCellj != -1) {
		RectF(idxToPos(cellData,focCelli, focCellj), cellData.cellws[focCellj], cellh).drawFrame(4, Palette::Green);
	}

	// テキストdataの描画
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			if (font(cellData.str[i][j]).region().w + 10 * 2 <= cellData.cellws[j]) {
				font(cellData.str[i][j]).draw(10 + mgWpx + cellData.sumW[j], mgHpx + cellh * i, Palette::Black);
			}
			else {
				String drData = cellData.str[i][j];
				while (drData.size() > 0) {
					drData.pop_back();
					if (font(drData + U"...").region().w + 10 * 2 <= cellData.cellws[j]) {
						font(drData + U"...").draw(10 + mgWpx + cellData.sumW[j], mgHpx + cellh * i, Palette::Black);
						break;
					}
				}
			}
		}
	}
}

void tbInputUpdate(CellData &cellData, const int&focCelli, const int &focCellj, TextEditState& tbInput) {
	if (focCelli != -1 && focCellj != -1) {
		cellData.str[focCelli][focCellj] = tbInput.text;
	}	else {
		tbInput.text = U"";
	}
}

void Main() {
	Window::SetTitle(U"table editor");
	const Font fontMsg(20);
	const Font fontSub(30);
	const Font font(35);

	CellData cellData;

	TextEditState tbH;
	tbH.text = U"2";
	TextEditState tbW;
	tbW.text = U"3";
	TextEditState tbInput;
	int focCelli = 0, focCellj = 0;


	int ladjIdx = -1;

	double preClickTime = Scene::Time();

	messageBox mbCreateNewTable(U"新しい表を作成します。この結果は削除されますがよろしいですか。", fontMsg, messageBox::style::mb_YESNO);

	auto enableMb = [&]() {
		return mbCreateNewTable.enable;
	};

	while (System::Update()) {


		if (mbCreateNewTable.res != messageBox::resType::NONE) {
			if (mbCreateNewTable.res == messageBox::resType::YES) {
				int th = ParseOr<int>(tbH.text, -1);
				int tw = ParseOr<int>(tbW.text, -1);
				if ((tbH.text == U"" || th <= 0) || (tbW.text == U"" || tw <= 0)) {
					// 求 自然数
					tbH.text = Format(cellData.h);
					tbW.text = Format(cellData.w);
				} else {
					// 表サイズの変更
					cellData.resize(th, tw);
					focCelli = 0;
					focCellj = 0;
					tbInput.text = U"";
				}
			}
			else {
				tbH.text = Format(cellData.h);
				tbW.text = Format(cellData.w);
			}
			mbCreateNewTable.res = messageBox::resType::NONE;
			mbCreateNewTable.enable = false;
		}
		if (enableMb()) {
			mbCreateNewTable.draw();
		}
		else {
			cellData.reCalcWSum();

			mouseEventProc(cellData, font, preClickTime, ladjIdx, focCelli, focCellj, tbInput);


			tbInputUpdate(cellData, focCelli, focCellj, tbInput);
		}

		drawCellData(cellData, font, focCelli, focCellj);


		SimpleGUI::TextBox(tbH, Vec2(mgWpx, 10), 60, 4, !enableMb());
		SimpleGUI::TextBox(tbW, Vec2(mgWpx + 100, 10), 60, 4, !enableMb());
		if (SimpleGUI::Button(U"create new table", Vec2(mgWpx + 200, 10), unspecified, !enableMb())) {
			mbCreateNewTable.enable = true;
		}
		fontSub(U"列").draw(mgWpx + 65, 10);
		fontSub(U"行").draw(mgWpx + 100 + 65, 10);
		if (focCelli != -1 && focCellj != -1) {
			SimpleGUI::TextBox(tbInput, idxToPos(cellData,focCelli, focCellj) + Vec2(10, 10), cellData.cellws[focCellj]);
		}
	}
}

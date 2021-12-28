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
		//mb_OK
	};
	const Font font;
	int type;
	bool enable;
	int res;
	enum resType {
		NONE,
		YES,
		NO,
		//OK
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
const int mgHpx = 160, mgWpx = 50;
const int cellh = 50;
const int headh = 25;
const int mgHead = 10;

struct borderType {
	enum class line {
		solid,
		dash,
		doubled
	};
	enum class thick {
		normal,
		bold
	};
	line l;
	thick t;
	borderType(line l, thick t) : l(l), t(t) {}
	borderType() : l(line::solid), t(thick::normal) {}
};
enum class tapos {
	left,
	center,
	right,
	SIZE
};

struct CellData {
	int h, w;
	String title;
	std::vector<int> cellws;
	std::vector<int> sumW;
	std::vector<std::vector<String>> str;
	std::vector<std::vector<HSV>> color;
	std::vector<std::vector<borderType>> btHol, btVer;
	std::vector<std::vector<tapos>> tas;
	CellData(int h, int w) : h(h), w(w), title(U"タイトル"),
		cellws(w, 100), sumW(w + 1),
		str(h, std::vector<String>(w)),
		color(h, std::vector<HSV>(w, HSV(0, 0, 1.0))),
		btHol(h + 1, std::vector<borderType>(w)),
		btVer(h, std::vector<borderType>(w + 1)),
		tas(h, std::vector<tapos>(w)) {
		reCalcWSum();
	}
	void reCalcWSum() {
		for (int i = 0; i < w; i++) sumW[i + 1] = sumW[i] + cellws[i];
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
void drawCellData(CellData& cellData, const Font& font, const int& focCelli, const int& focCellj) {
	// セルの描画
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			RectF(idxToPos(cellData, i, j), cellData.cellws[j], cellh).draw(cellData.color[i][j]).drawFrame(1, Palette::Black);
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
		RectF(idxToPos(cellData, focCelli, focCellj), cellData.cellws[focCellj], cellh).drawFrame(4, Palette::Green);
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

void tbInputUpdate(CellData& cellData, const int& focCelli, const int& focCellj, TextEditState& tbInput) {
	if (focCelli != -1 && focCellj != -1) {
		cellData.str[focCelli][focCellj] = tbInput.text;
	}
	else {
		tbInput.text = U"";
	}
}

String convertData(const CellData& cellData) {
	enum class bpos {
		right,
		bottom,
		top,
		left
	};
	
	auto taToClassS = [](tapos pos) {
		std::map<tapos, String> m = {
			{ tapos::left, U"L" },
			{ tapos::center, U"C" },
			{ tapos::right, U"R" }
		};
		return U"cTA" + m[pos];
	};
	auto bToClassS = [](bpos pos, borderType bt) {
		std::map<bpos, String> mpos = {
			{ bpos::bottom, U"B" },
			{ bpos::right, U"R" },
			{ bpos::top, U"T" },
			{ bpos::left, U"L" },
		};
		std::map<borderType::line, String> mline = {
			{ borderType::line::solid, U"S" },
			{ borderType::line::dash, U"D" },
			{ borderType::line::doubled, U"W" }
		};
		std::map<borderType::thick, String> mthick = {
			{ borderType::thick::normal, U"" },
			{ borderType::thick::bold, U"B" }
		};
		return U"t" + mpos[pos] + mline[bt.l] + mthick[bt.t];
	};
	
	String res = U"]";		// 表開始記号
	res += cellData.title;	// タイトル
	res += U"]";			// クラス開始記号
	// 表枠上/左側指定
	res += bToClassS(bpos::top, cellData.btHol[0][0]);
	res += U" " + bToClassS(bpos::left, cellData.btVer[0][0]);
	// TA多数決
	tapos maxTaType;
	{
		std::vector<int> taSs((int)tapos::SIZE, 0);
		for (int i = 0; i < cellData.h; i++) {
			for (int j = 0; j < cellData.w; j++) {
				taSs[(int)cellData.tas[i][j]]++;
			}
		}
		int maxCnt = 0;
		for (int i = 0; i < (int)tapos::SIZE; i++) {
			if (chmax(maxCnt, taSs[i])) {
				maxTaType = (tapos)i;
			}
		}
	}
	res += U" " + taToClassS(maxTaType);
	// 個々のセル指定
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			res += U";";
			res += cellData.str[i][j];
			res += bToClassS(bpos::right, cellData.btVer[i][j + 1]);
			res += U" " + bToClassS(bpos::bottom, cellData.btHol[i + 1][j]);
			res += U",#" + cellData.color[i][j].toColor().toHex();
		}
		res += U"\n";
	}
	//s3d::Print << res;
	return res;
}

void Main() {
	Window::SetTitle(U"table editor");
	Scene::SetBackground(Palette::White);
	const Font fontMsg(20);
	const Font fontSub(30);
	const Font font(35);

	CellData cellData(2, 3);

	TextEditState tbTitle;
	tbTitle.text = U"タイトル";

	TextEditState tbH;
	tbH.text = U"2";
	TextEditState tbW;
	tbW.text = U"3";
	TextEditState tbInput;
	int focCelli = 0, focCellj = 0;

	//String warnMsg = U"warnig";

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
				}
				else {
					// 表サイズの変更
					cellData = CellData(th, tw);
					focCelli = 0;
					focCellj = 0;
					tbInput.text = U"";
					tbTitle.text = U"タイトル";
				}
			}
			else {
				tbH.text = Format(cellData.h);
				tbW.text = Format(cellData.w);
			}
			mbCreateNewTable.res = messageBox::resType::NONE;
			mbCreateNewTable.enable = false;
		}
		drawCellData(cellData, font, focCelli, focCellj);
		if (enableMb()) {
			mbCreateNewTable.draw();
		}
		else {
			cellData.reCalcWSum();

			mouseEventProc(cellData, font, preClickTime, ladjIdx, focCelli, focCellj, tbInput);


			tbInputUpdate(cellData, focCelli, focCellj, tbInput);
		}

		SimpleGUI::TextBox(tbH, Vec2(mgWpx, 10), 60, 3, !enableMb());
		SimpleGUI::TextBox(tbW, Vec2(mgWpx + 100, 10), 60, 3, !enableMb());
		if (SimpleGUI::Button(U"新規作成", Vec2(mgWpx + 200, 10), unspecified, !enableMb())) {
			mbCreateNewTable.enable = true;
		}
		fontSub(U"列").draw(mgWpx + 65, 10, Palette::Black);
		fontSub(U"行").draw(mgWpx + 100 + 65, 10, Palette::Black);
		SimpleGUI::TextBox(tbTitle, Vec2(mgWpx, 60), 400, unspecified, !enableMb());
		cellData.title = tbTitle.text;
		if (SimpleGUI::Button(U"コンバート", Vec2(mgWpx + 410, 60), unspecified, !enableMb())) {
			String res = convertData(cellData);
			Clipboard::SetText(res);
		}
		//fontSub(warnMsg).region(mgWpx, 550).stretched(5).drawFrame(2, Palette::Black).draw(Palette::Yellow);
		//fontSub(warnMsg).draw(mgWpx, 550, Palette::Black);
		if (focCelli != -1 && focCellj != -1) {
			SimpleGUI::TextBox(tbInput, idxToPos(cellData, focCelli, focCellj) + Vec2(10, 10), cellData.cellws[focCellj]);
		}
	}
}

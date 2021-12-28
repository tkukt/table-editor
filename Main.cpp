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
	enum class styleType {
		mb_YESNO,
		mb_OK
	};
	const Font font;
	styleType style;
	bool enable;
	enum class resType {
		NONE,
		YES,
		NO,
		OK
	};
	resType res;
	messageBox(String text, const Font& font, styleType style) : text(text), font(font), style(style), enable(false), res(resType::NONE) {}
	void draw() {
		assert(enable);
		Rect(Arg::center(400, 300), 300, 200).drawShadow(Vec2(4, 4), 16, 2, Palette::Lightgray).draw(Palette::White).drawFrame(2, Palette::Gray);
		font(text).draw(Rect(Arg::center(400, 290), 240, 120), Palette::Black);
		if (style == styleType::mb_YESNO) {
			if (SimpleGUI::Button(U"はい", Vec2(280, 350), 100)) {
				res = resType::YES;
			}
			if (SimpleGUI::Button(U"いいえ", Vec2(420, 350), 100)) {
				res = resType::NO;
			}
		}
		else {
			if (SimpleGUI::Button(U"OK", Vec2(350, 350), 100)) {
				res = resType::OK;
			}
		}
	}
	bool isAnsed() {
		return res != resType::NONE;
	}
	void reset() {
		res = resType::NONE;
		enable = false;
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


namespace rightClick {
	bool isColorSel;
};

struct FocArea {
	int i1, i2, j1, j2;
	bool isAct;
	FocArea(bool isAct = false) : i1(0), i2(0), j1(0), j2(0), isAct(isAct) { assert(!isAct); }
	FocArea(int i, int j) : i1(i), i2(i), j1(j), j2(j), isAct(true) {}
	FocArea(int i1, int i2, int j1, int j2) :i1(i1), i2(i2), j1(j1), j2(j2), isAct(true) {}
	int w() {
		return j2 - j1 + 1;
	}
	int h() {
		return i2 - i1 + 1;
	}
};

struct CellData {
	int h, w;
	String title;
	std::vector<int> cellws;
	std::vector<int> sumW;
	std::vector<std::vector<String>> str;
	FocArea focArea;
	std::vector<std::vector<HSV>> color;
	std::vector<std::vector<borderType>> btHol, btVer;
	std::vector<std::vector<tapos>> tas;
	CellData(int h, int w) : h(h), w(w), title(U"タイトル"),
		cellws(w, 100), sumW(w + 1),
		focArea(),
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

void mouseEventProc(CellData& cellData, const Font& font, double& preClickTime, int& ladjIdx, TextEditState& tbInput, messageBox& mbColorSel) {
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
			cellData.focArea = (seli == -1 || selj == -1) ? FocArea(false) : FocArea(seli, selj);
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

	if (MouseR.down()) {
		// 右クリック
		// セル色変更
		for (int i = 0; i < cellData.h; i++) {
			for (int j = 0; j < cellData.w; j++) {
				if (mgWpx + cellData.sumW[j] < mp.x && mp.x < mgWpx + cellData.sumW[j + 1]
					&& mgHpx + cellh * i < mp.y && mp.y < mgHpx + cellh * (i + 1)) {
					/*rightClick::isColorSel = true;
					rightClick::pos = mp;*/
					mbColorSel.enable = true;
				}
			}
		}
	}
}
auto idxToPos = [&](const CellData& cellData, int i, int j) {
	return Vec2(mgWpx + cellData.sumW[j], mgHpx + cellh * i);
};
void drawCellData(CellData& cellData, const Font& font) {
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
	if (cellData.focArea.isAct) {
		int width = 0;
		for (int j = cellData.focArea.j1; j <= cellData.focArea.j2; j++) {
			width += cellData.cellws[j];
		}
		RectF(idxToPos(cellData, cellData.focArea.i1, cellData.focArea.j1), width, cellh * cellData.focArea.h()).drawFrame(4, Palette::Green);
	}

	// 右クリック時カラーピッカーの描画
	/*if (rightClick::isColorSel) {
		SimpleGUI::ColorPicker(cellData.color[cellData.focArea.i1][cellData.focArea.j1], rightClick::pos);
	}*/

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

void tbInputUpdate(CellData& cellData, TextEditState& tbInput) {
	if (cellData.focArea.isAct) {
		cellData.str[cellData.focArea.i1][cellData.focArea.j1] = tbInput.text;
	}
	else {
		tbInput.text = U"";
	}
}

template<typename T>
T getMode(const std::vector<std::vector<T>>& s) {
	int h = s.size();
	int w = s[0].size();
	std::map<T, int> cnt;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			cnt[s[i][j]]++;
		}
	}
	int maxCnt = 0;
	T mode;
	for (const auto& p : cnt) {
		if (chmax(maxCnt, p.second)) {
			mode = p.first;
		}
	}
	return mode;
};

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
	res += U" " + taToClassS(getMode(cellData.tas));
	// 個々のセルの指定を調べる
	std::vector<std::vector<String>> bClassSr(cellData.h, std::vector<String>(cellData.w));
	std::vector<std::vector<String>> bClassSb(cellData.h, std::vector<String>(cellData.w));
	std::vector<std::vector<String>> colS(cellData.h, std::vector<String>(cellData.w));
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			bClassSr[i][j] = bToClassS(bpos::right, cellData.btVer[i][j + 1]);
			bClassSb[i][j] = bToClassS(bpos::bottom, cellData.btHol[i + 1][j]);
			colS[i][j] = U"#" + cellData.color[i][j].toColor().toHex();
		}
	}
	String modeBClassSr = getMode(bClassSr);
	String modeBClassSb = getMode(bClassSb);
	String modeColS = getMode(colS);
	res += U" " + modeBClassSr;
	res += U" " + modeBClassSb;
	res += U" " + modeColS;
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			res += U";";
			res += cellData.str[i][j];
			String segRes = U"";
			if (modeBClassSr != bClassSr[i][j]) {
				segRes += bClassSr[i][j];
			}
			if (modeBClassSb != bClassSb[i][j]) {
				if (segRes.size() != 0) segRes += U" ";
				segRes += bClassSb[i][j];
			}
			if (modeColS != colS[i][j]) {
				if (segRes.size() != 0) segRes += U",";
				segRes += colS[i][j];
			}
			res += segRes;
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

	//String warnMsg = U"warnig";

	int ladjIdx = -1;

	double preClickTime = Scene::Time();

	messageBox mbCreateNewTable(U"新しい表を作成します。この結果は削除されますがよろしいですか。", fontMsg, messageBox::styleType::mb_YESNO);
	messageBox mbReqN(U"列数および行数には自然数を入力してください。", fontMsg, messageBox::styleType::mb_OK);
	messageBox mbColorSel(U"", fontMsg, messageBox::styleType::mb_OK);
	messageBox mbConvertSuc(U"コンバート結果がクリップボードにコピーされました。", fontMsg, messageBox::styleType::mb_OK);

	auto enableMb = [&]() {
		return mbCreateNewTable.enable || mbReqN.enable || mbColorSel.enable || mbConvertSuc.enable;
	};

	while (System::Update()) {
		if (mbCreateNewTable.isAnsed()) {
			if (mbCreateNewTable.res == messageBox::resType::YES) {
				int th = ParseOr<int>(tbH.text, -1);
				int tw = ParseOr<int>(tbW.text, -1);
				if ((tbH.text == U"" || th <= 0) || (tbW.text == U"" || tw <= 0)) {
					mbReqN.enable = true;
					tbH.text = Format(cellData.h);
					tbW.text = Format(cellData.w);
				}
				else {
					// 表サイズの変更
					cellData = CellData(th, tw);
					tbInput.text = U"";
					tbTitle.text = U"タイトル";
				}
			}
			else {
				tbH.text = Format(cellData.h);
				tbW.text = Format(cellData.w);
			}
			mbCreateNewTable.reset();
		}
		if (mbReqN.isAnsed()) {
			mbReqN.reset();
		}
		if (mbColorSel.isAnsed()) {
			mbColorSel.reset();
		}
		if (mbConvertSuc.isAnsed()) {
			mbConvertSuc.reset();
		}
		drawCellData(cellData, font);
		if (enableMb()) {
			if (mbCreateNewTable.enable) mbCreateNewTable.draw();
			if (mbReqN.enable) mbReqN.draw();
			if (mbColorSel.enable) {
				mbColorSel.draw();
				SimpleGUI::ColorPicker(cellData.color[cellData.focArea.i1][cellData.focArea.j1], Vec2(320, 210));
			}
			if (mbConvertSuc.enable) mbConvertSuc.draw();
		}
		else {
			cellData.reCalcWSum();

			mouseEventProc(cellData, font, preClickTime, ladjIdx, tbInput, mbColorSel);

			tbInputUpdate(cellData, tbInput);
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
			mbConvertSuc.enable = true;
		}
		//fontSub(warnMsg).region(mgWpx, 550).stretched(5).drawFrame(2, Palette::Black).draw(Palette::Yellow);
		//fontSub(warnMsg).draw(mgWpx, 550, Palette::Black);
		if (cellData.focArea.isAct && !enableMb()) {
			SimpleGUI::TextBox(tbInput, idxToPos(cellData, cellData.focArea.i1, cellData.focArea.j1) + Vec2(10, 10), cellData.cellws[cellData.focArea.j1]);
		}
	}
}

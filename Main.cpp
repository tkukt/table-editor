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
	messageBox() {}
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
const int mgHpx = 200, mgWpx = 50;
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
	HSV color;
	borderType(line l, thick t, HSV(color)) : l(l), t(t), color(color) {}
	borderType() : l(line::solid), t(thick::normal), color(Palette::Black) {}
	bool operator==(borderType rhs) {
		return l == rhs.l && t == rhs.t && color == rhs.color;
	}
	bool operator!=(borderType rhs) {
		return !(*this == rhs);
	}
};
enum class tapos {
	left,
	center,
	right,
	SIZE
};

namespace inputMode {
	bool isBorderMode;
	borderType nowInBt;
};

namespace borderColorSample {
	Circle r(mgWpx + 410, 78, 12);
	HSV borderColor;
};
namespace cellColorSample {
	HSV color;
};

struct pos {
	int i, j;
	pos() : i(0), j(0) {}
	pos(int i, int j) : i(i), j(j) {}
	pos operator-() { return pos(-i, -j); }
	pos operator+=(pos rhs) {
		i += rhs.i;
		j += rhs.j;
		return *this;
	}
	pos operator-=(pos rhs) {
		i -= rhs.i;
		j -= rhs.j;
		return *this;
	}
	pos operator*=(int rhs) {
		i *= rhs;
		j *= rhs;
		return *this;
	}
	pos operator+(pos rhs) { return pos(*this) += rhs; }
	pos operator-(pos rhs) { return pos(*this) -= rhs; }
	pos operator*(int rhs) { return pos(*this) *= rhs; }
	bool operator==(pos rhs) {
		return i == rhs.i && j == rhs.j;
	}
	bool operator!=(pos rhs) {
		return !(*this == rhs);
	}
};

struct CellData {
	int h, w;
	String title;
	std::vector<int> cellws;
	std::vector<int> sumW;
	std::vector<std::vector<String>> str;
	std::vector<std::vector<HSV>> color;
	std::vector<std::vector<pos>> cmbPar;
	std::vector<std::vector<pos>> cmbSz;
	std::vector<std::vector<borderType>> btHol, btVer;
	std::vector<std::vector<tapos>> tas;
	CellData(int h, int w) : h(h), w(w), title(U"タイトル"),
		cellws(w, 100), sumW(w + 1),
		str(h, std::vector<String>(w)),
		color(h, std::vector<HSV>(w, HSV(0, 0, 1.0))),
		cmbPar(h, std::vector<pos>(w)),
		cmbSz(h, std::vector<pos>(w, pos(1, 1))),
		btHol(h + 1, std::vector<borderType>(w)),
		btVer(h, std::vector<borderType>(w + 1)),
		tas(h, std::vector<tapos>(w)) {
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				cmbPar[i][j] = pos(i, j);
			}
		}
		reCalcWSum();
	}
	void reCalcWSum() {
		for (int i = 0; i < w; i++) sumW[i + 1] = sumW[i] + cellws[i];
	}
};

struct FocArea {
	int i1, i2, j1, j2; // 1:先に選択したセル 2:2番目に選択したセル
	int u, d, r, l, h, w;
	bool isAct;
	FocArea(bool isAct = false) : i1(-1), i2(-1), j1(-1), j2(-1), isAct(isAct) { assert(!isAct); }
	FocArea(int i, int j, const CellData& cellData) : i1(i), i2(i), j1(j), j2(j), isAct(true) { calcSelArea(cellData); }
	FocArea(int i1, int i2, int j1, int j2, const CellData& cellData) :i1(i1), i2(i2), j1(j1), j2(j2), isAct(true) { calcSelArea(cellData); }
	void add(int i, int j, const CellData& cellData) {
		assert(i1 != -1 && j1 != -1);
		i2 = i, j2 = j;
		calcSelArea(cellData);
	}
	void calcSelArea(const CellData& cellData) { // 結合されたセルも考慮して選択領域を計算(udlr)
		u = INT_MAX, d = 0, l = INT_MAX, r = 0;
		for (int i = std::min(i1, i2); i <= std::max(i1, i2); i++) {
			for (int j = std::min(j1, j2); j <= std::max(j1, j2); j++) {
				const pos& par = cellData.cmbPar[i][j];
				chmin(u, par.i);
				chmax(d, par.i + cellData.cmbSz[par.i][par.j].i - 1);
				chmin(l, par.j);
				chmax(r, par.j + cellData.cmbSz[par.i][par.j].j - 1);
			}
		}
		h = d - u + 1;
		w = r - l + 1;
	}
};

// 返り値はcellDataを変更したかどうか
bool mouseEventProc(CellData& cellData, FocArea& focArea, const Font& font, double& preClickTime, int& ladjIdx, TextEditState& tbinput, messageBox& mbColorSel) {
	Point mp = Cursor::Pos();
	bool tIsChangeData = false;

	// 罫線作成モードのとき
	if (inputMode::isBorderMode) {
		if (MouseL.down()) {
			int minDist = INT_MAX;
			bool isHol = false;
			int mi = -1, mj = -1;
			// 横
			for (int i = 0; i < cellData.h + 1; i++) {
				for (int j = 0; j < cellData.w; j++) {
					if (mgWpx + cellData.sumW[j] <= mp.x && mp.x <= mgWpx + cellData.sumW[j + 1]) {
						if (chmin(minDist, abs((mgHpx + cellh * i) - mp.y))) {
							isHol = true;
							mi = i;
							mj = j;
						}
					}
				}
			}
			// 縦
			for (int i = 0; i < cellData.h; i++) {
				for (int j = 0; j < cellData.w + 1; j++) {
					if (mgHpx + cellh * i <= mp.y && mp.y <= mgHpx + cellh * (i + 1)) {
						if (chmin(minDist, abs((mgWpx + cellData.sumW[j]) - mp.x))) {
							isHol = false;
							mi = i;
							mj = j;
						}
					}
				}
			}
			if (minDist <= 10) {
				// 線上クリック判定
				auto& bt = (isHol ? cellData.btHol[mi][mj] : cellData.btVer[mi][mj]);
				if (bt != inputMode::nowInBt) {
					tIsChangeData = true;
					bt = inputMode::nowInBt;
				}
			}
		}
		return tIsChangeData; // early return
	}

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
			bool isEmpty = true;
			for (int i = 0; i < cellData.h; i++) {
				if (cellData.cmbSz[i][ladjIdx].j == 1 && cellData.str[i][ladjIdx].size() > 0) {
					isEmpty = false;
					chmax(fitWidth, (int)font(cellData.str[i][ladjIdx]).region().w + 10 * 2 + 2);
				}
			}
			if (!isEmpty) {
				cellData.cellws[ladjIdx] = fitWidth;
				tIsChangeData = true;
			}
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
					}
				}
			}
			// アクティブセルが変更されてかつ内容が編集された場合
			if ((focArea.i1 != seli || focArea.j1 != selj) && focArea.isAct && tbinput.text != cellData.str[focArea.u][focArea.l]) {
				tIsChangeData = true;
				cellData.str[focArea.u][focArea.l] = tbinput.text;
			}
			if (seli != -1 && selj != -1) { // セルをクリックした場合
				if (focArea.isAct && KeyShift.pressed()) { // Shiftを押しながらクリックした場合
					// アクティブセルを範囲選択
					focArea.add(seli, selj, cellData);
				}
				else {
					// 別セルクリック
					focArea = FocArea(seli, selj, cellData);
				}
				tbinput.text = cellData.str[focArea.u][focArea.l];
			}
			else { // 範囲外をクリックした場合
				focArea = FocArea(false);
			}
		}
	}
	if (MouseL.pressed()) {
		if (ladjIdx != -1) {
			cellData.cellws[ladjIdx] += mp.x - cellData.sumW[ladjIdx + 1] - mgWpx;
			chmax(cellData.cellws[ladjIdx], 50);
			tIsChangeData = true;
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
	return tIsChangeData;
}
auto idxToPos = [](const CellData& cellData, int i, int j) {
	return Vec2(mgWpx + cellData.sumW[j], mgHpx + cellh * i);
};

bool combCell(CellData& cellData, const FocArea& focArea, bool& isSuccess) {
	isSuccess = true;
	for (int i = focArea.u; i <= focArea.d; i++) {
		for (int j = focArea.l; j <= focArea.r; j++) {
			if (cellData.cmbPar[i][j] != pos(i, j) || cellData.cmbSz[i][j] != pos(1, 1)) {
				isSuccess = false;
			}
		}
	}
	if (!isSuccess) return false;
	cellData.cmbSz[focArea.u][focArea.l] = pos(focArea.h, focArea.w);
	for (int i = focArea.u; i <= focArea.d; i++) {
		for (int j = focArea.l; j <= focArea.r; j++) {
			cellData.cmbPar[i][j] = pos(focArea.u, focArea.l);
		}
	}
	String defStr = U""; // 結合後の文字列は左上のみ採用
	for (int i = focArea.d; i >= focArea.u; i--) {
		for (int j = focArea.r; j >= focArea.l; j--) {
			if (cellData.str[i][j].size() > 0) {
				defStr = cellData.str[i][j];
				cellData.str[i][j] = U"";
			}
		}
	}
	cellData.str[focArea.u][focArea.l] = defStr;
	return true;
}

bool reCombCell(CellData& cellData, const FocArea& focArea, bool& isSuccess) {
	isSuccess = true;
	for (int i = focArea.u; i <= focArea.d; i++) {
		for (int j = focArea.l; j <= focArea.r; j++) {
			const pos& p = cellData.cmbPar[i][j];
			const pos& sz = cellData.cmbSz[i][j];
			if (!(focArea.u <= p.i && p.i + sz.i - 1 <= focArea.d && focArea.l <= p.j && p.j + sz.j - 1 <= focArea.r)) {
				isSuccess = false;
			}
		}
	}
	if (!isSuccess) return false;
	bool isChange = false;
	for (int i = focArea.u; i <= focArea.d; i++) {
		for (int j = focArea.l; j <= focArea.r; j++) {
			if (cellData.cmbSz[i][j] != pos(1, 1)) isChange = true;
			cellData.cmbPar[i][j] = pos(i, j);
			cellData.cmbSz[i][j] = pos(1, 1);
		}
	}
	return isChange;
}

void keyboardInputProc(int& recordCur, const int& recordSz) {
	if ((KeyControl + KeyZ).down()) { // Ctrl+Z
		recordCur = std::max(0, recordCur - 1);
	}
	if ((KeyControl + KeyY).down()) { // Ctrl+Y
		recordCur = std::min(recordSz - 1, recordCur + 1);
	}
}

void drawCellData(const CellData& cellData, const FocArea& focArea, const Font& font) {
	// セルの描画
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			RectF(idxToPos(cellData, i, j), cellData.cellws[j], cellh).draw(cellData.color[i][j]);
		}
	}

	// 罫線の描画
	auto drawBorder = [&](bool isHol, int i, int j, const borderType& bt) {
		int thick = (bt.t == borderType::thick::bold) ? 3 : 1;
		int vi = isHol ? 0 : 1;
		int vj = isHol ? 1 : 0;
		Vec2 mg = isHol ? Vec2(0, 2) : Vec2(2, 0);
		switch (bt.l) {
		case borderType::line::solid:
			Line(idxToPos(cellData, i, j), idxToPos(cellData, i + vi, j + vj)).draw(thick, bt.color);
			break;
		case borderType::line::dash:
			Line(idxToPos(cellData, i, j), idxToPos(cellData, i + vi, j + vj)).draw(LineStyle::SquareDot, thick, bt.color);
			break;
		case borderType::line::doubled:
			Line(idxToPos(cellData, i, j) + mg, idxToPos(cellData, i + vi, j + vj) + mg).draw(thick, bt.color);
			Line(idxToPos(cellData, i, j) - mg, idxToPos(cellData, i + vi, j + vj) - mg).draw(thick, bt.color);
			break;
		default:
			assert(false && "illegal borderType");
			break;
		}
	};
	for (int i = 0; i < cellData.h + 1; i++) { // 横線
		for (int j = 0; j < cellData.w; j++) {
			bool isCmbed = false;
			if (i != 0) {
				const pos& p = cellData.cmbPar[i - 1][j];
				if (p.i + cellData.cmbSz[p.i][p.j].i > i) {
					isCmbed = true;
				}
			}
			if (!isCmbed) drawBorder(true, i, j, cellData.btHol[i][j]);
		}
	}
	for (int i = 0; i < cellData.h; i++) { // 縦線
		for (int j = 0; j < cellData.w + 1; j++) {
			bool isCmbed = false;
			if (j != 0) {
				const pos& p = cellData.cmbPar[i][j - 1];
				if (p.j + cellData.cmbSz[p.i][p.j].j > j) {
					isCmbed = true;
				}
			}
			if (!isCmbed) drawBorder(false, i, j, cellData.btVer[i][j]);
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
	if (focArea.isAct) {
		int width = 0;
		for (int j = focArea.l; j <= focArea.r; j++) {
			width += cellData.cellws[j];
		}
		RectF(idxToPos(cellData, focArea.u, focArea.l), width, cellh * (abs(focArea.u - focArea.d) + 1)).drawFrame(4, Palette::Green);
	}

	// 右クリック時カラーピッカーの描画
	/*if (rightClick::isColorSel) {
		SimpleGUI::ColorPicker(cellData.color[cellData.focArea.i1][cellData.focArea.j1], rightClick::pos);
	}*/

	// テキストdataの描画
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			if (cellData.cmbPar[i][j] == pos(i, j)) {
				if (font(cellData.str[i][j]).region().w + 10 * 2 <= cellData.sumW[j + cellData.cmbSz[i][j].j] - cellData.sumW[j]) {
					font(cellData.str[i][j]).draw(10 + mgWpx + cellData.sumW[j], mgHpx + cellh * (2 * i + cellData.cmbSz[i][j].i - 1) / 2, Palette::Black);
				}
				else {
					String drData = cellData.str[i][j];
					while (drData.size() > 0) {
						drData.pop_back();
						if (font(drData + U"...").region().w + 10 * 2 <= cellData.sumW[j + cellData.cmbSz[i][j].j] - cellData.sumW[j]) {
							font(drData + U"...").draw(10 + mgWpx + cellData.sumW[j], mgHpx + cellh * (2 * i + cellData.cmbSz[i][j].i - 1) / 2, Palette::Black);
							break;
						}
					}
				}
			}
		}
	}
}

void tbInputUpdate(CellData& cellData, const FocArea& focArea, TextEditState& tbInput) {
	if (focArea.isAct) {
		cellData.str[focArea.i1][focArea.j1] = tbInput.text;
	}
	else {
		tbInput.text = U"";
	}
}

namespace getModeSub {
	template<typename T>
	bool isEmpty(T x) {return false;}
	bool isEmpty(String x) { return x.size() == 0; }
};

template<typename T>
T getMode(const std::vector<std::vector<T>>& s) {
	int h = (int)s.size();
	int w = (int)s[0].size();
	assert(h * w != 0);
	std::map<T, int> cnt;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (!getModeSub::isEmpty(s[i][j])) {
				cnt[s[i][j]]++;
			}
		}
	}
	int maxCnt = 0;
	assert(cnt.size() != 0);
	T mode = cnt.begin()->first;
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

	auto taToClassS = [](tapos pos) { // text alignment
		std::map<tapos, String> m = {
			{ tapos::left, U"L" },		// 左揃え
			{ tapos::center, U"C" },	// 中央揃え
			{ tapos::right, U"R" }		// 右揃え
		};
		return U"tX" + m[pos];	// tX[LCR]
	};
	auto bToClassS = [](bpos pos, borderType bt) { // 罫線の種類
		std::map<bpos, String> mpos = { // 罫線の位置
			{ bpos::bottom, U"B" },		// 下
			{ bpos::right, U"R" },		// 右
			{ bpos::top, U"T" },		// 上(全体のみのexc)
			{ bpos::left, U"L" },		// 左(全体のみのexc)
		};
		std::map<borderType::line, String> mline = { // 線種
			{ borderType::line::solid, U"S" },	// 実線(solid)
			{ borderType::line::dash, U"D" },	// 破線(dash)
			{ borderType::line::doubled, U"W" }	// 二重線(W doubled)
		};
		std::map<borderType::thick, String> mthick = { // 太さ
			{ borderType::thick::normal, U"" },	// 中線（普通）
			{ borderType::thick::bold, U"B" }	// 太線
		};
		return U"t" + mpos[pos] + mline[bt.l] + mthick[bt.t];	// t[BRTL][SDW]B?
	};

	String res = U"]";		// 表開始記号
	res += cellData.title;	// タイトル
	res += U"]";			// クラス開始記号
	// 表枠上/左側の罫線種類指定
	res += bToClassS(bpos::top, cellData.btHol[0][0]);
	res += U" " + bToClassS(bpos::left, cellData.btVer[0][0]);
	res += U" " + taToClassS(getMode(cellData.tas));
	// 個々のセルの指定を調べる
	std::vector<std::vector<String>> bClassSr(cellData.h, std::vector<String>(cellData.w));
	std::vector<std::vector<String>> bClassSb(cellData.h, std::vector<String>(cellData.w));
	std::vector<std::vector<String>> colS(cellData.h, std::vector<String>(cellData.w));
	std::vector<std::vector<String>> bColSr(cellData.h, std::vector<String>(cellData.w));
	std::vector<std::vector<String>> bColSb(cellData.h, std::vector<String>(cellData.w));
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			if (cellData.cmbPar[i][j] == pos(i, j)) {
				const pos& sz = cellData.cmbSz[i][j];
				bClassSr[i][j] = bToClassS(bpos::right, cellData.btVer[i][(j + sz.j - 1) + 1]);
				bColSr[i][j] = U"#" + cellData.btVer[i][(j + sz.j - 1) + 1].color.toColor().toHex() + U"r";
				bClassSb[i][j] = bToClassS(bpos::bottom, cellData.btHol[(i + sz.i - 1) + 1][j]);
				bColSb[i][j] = U"#" + cellData.btHol[(i + sz.i - 1) + 1][j].color.toColor().toHex() + U"b";
				colS[i][j] = U"#" + cellData.color[i][j].toColor().toHex();
			}
		}
	}
	String modeBClassSr = getMode(bClassSr);
	String modeBColSr = getMode(bColSr);
	String modeBClassSb = getMode(bClassSb);
	String modeBColSb = getMode(bColSb);
	String modeColS = getMode(colS);
	res += U" " + modeBClassSr;
	res += U" " + modeBClassSb;
	res += U"," + modeColS;
	res += U" #" + cellData.btHol[0][0].color.toColor().toHex() + U"t"; // 上罫線色
	res += U" #" + cellData.btVer[0][0].color.toColor().toHex() + U"l"; // 左罫線色
	res += U" " + modeBColSr;
	res += U" " + modeBColSb;
	for (int i = 0; i < cellData.h; i++) {
		for (int j = 0; j < cellData.w; j++) {
			res += U";";
			if (cellData.cmbPar[i][j] == pos(i, j)) {
				res += cellData.str[i][j];
				String segRes = U"";
				if (modeBClassSr != bClassSr[i][j]) {
					segRes += bClassSr[i][j];
				}
				if (modeBClassSb != bClassSb[i][j]) {
					if (segRes.size() != 0) segRes += U" ";
					segRes += bClassSb[i][j];
				}
				bool isInsCom = false;
				if (modeColS != colS[i][j]) {
					if (segRes.size() != 0) segRes += U",", isInsCom = true;
					segRes += colS[i][j];
				}
				if (modeBColSr != bColSr[i][j]) {
					if (segRes.size() != 0) segRes += isInsCom ? U" " : U", ", isInsCom = true;
					segRes += bColSr[i][j];
				}
				if (modeBColSb != bColSb[i][j]) {
					if (segRes.size() != 0) segRes += isInsCom ? U" " : U", ";
					segRes += bColSb[i][j];
				}

				if (cellData.cmbSz[i][j] != pos(1, 1)) {
					if (segRes.size() != 0) segRes += U",";
					segRes += U"+" + Format(cellData.cmbSz[i][j].j) + U":" + Format(cellData.cmbSz[i][j].i);
				}

				if (segRes.size() != 0) res += U"]";
				res += segRes;
			}
			else {
				res += U"]*";
			}
		}
		res += U"\n";
	}
	//s3d::Print << res;
	return res;
}

namespace fontSize {
	const int nomal = 35;
	const int msg = 20;
	const int sub = 25;
};

void Main() {
	Window::SetTitle(U"table editor");
	Scene::SetBackground(Palette::White);

	const double zoomRatio = 1.1;
	const int zoomMaxLev = 6;

	const int fontSizeMin = round(fontSize::nomal * pow(zoomRatio, -zoomMaxLev)),
		fontSizeMax = round(fontSize::nomal * pow(zoomRatio, zoomMaxLev));
	std::vector<Font> fonts(fontSizeMax + 1);
	for (int i = fontSizeMin; i <= fontSizeMax; i++) { fonts[i] = Font(i); }
	int zoomLevel = 0;

	FocArea focArea;
	std::vector<CellData> record = { CellData(2, 3) };
	int recordCur = 0;

	TextEditState tbTitle;
	tbTitle.text = U"タイトル";

	TextEditState tbH;
	tbH.text = U"2";
	TextEditState tbW;
	tbW.text = U"3";
	TextEditState tbInput;

	int ladjIdx = -1;

	double preClickTime = Scene::Time();

	std::map<std::string, messageBox> mbs = {
		{"CreateNewTable", messageBox(U"新しい表を作成します。現在のデータは削除されますがよろしいですか。", fonts[fontSize::msg], messageBox::styleType::mb_YESNO)},
		{"ReqN", messageBox(U"列数および行数には自然数を入力してください。", fonts[fontSize::msg], messageBox::styleType::mb_OK)},
		{"ColorSel", messageBox(U"", fonts[fontSize::msg], messageBox::styleType::mb_OK)},
		{"BorderTypeSel", messageBox(U"", fonts[fontSize::msg], messageBox::styleType::mb_OK)},
		{"ColorLineSel", messageBox(U"", fonts[fontSize::msg], messageBox::styleType::mb_OK)},
		{"ConvertSuc", messageBox(U"コンバート結果がクリップボードにコピーされました。", fonts[fontSize::msg], messageBox::styleType::mb_OK)},
		{"ConvertFal", messageBox(U"表の上と左および結合されたセルの下と右の罫線は同じ種類にしてください。", fonts[fontSize::msg], messageBox::styleType::mb_OK)},
		{"CombFal", messageBox(U"その領域はすでに結合されたセルを含んでいるためセルを結合できません。", fonts[fontSize::msg], messageBox::styleType::mb_OK)},
		{"ReCombFal", messageBox(U"その領域は結合されたセルを完全に含んでいないためセルの結合を解除できません。", fonts[fontSize::msg], messageBox::styleType::mb_OK)},
	};
	auto enableMb = [&]() {
		for (const auto& [name, mb] : mbs) {
			if (mb.enable) return true;
		}
		return false;
	};

	const std::vector<String> btLineList = { U"実線", U"破線", U"二重線" };
	const std::vector<String> btThickList = { U"中線", U"太線" };
	inputMode::nowInBt;

	// TODO : ALL インポート未実装
	// TODO : ALL text alignment未実装
	// TODO : ALL 縦横スクロール未実装

	while (System::Update()) {
		CellData cellData = record[recordCur];
		bool isChangeData = false;

		bool isClickBotton = false;
		if (SimpleGUI::Button(U"セル結合", Vec2(mgWpx, 110), unspecified, focArea.isAct && !enableMb())) {
			bool isSuccess = true;
			isChangeData |= combCell(cellData, focArea, isSuccess);
			if (!isSuccess) {
				mbs["CombFal"].enable = true;
			}
			isClickBotton = true;
		}
		if (SimpleGUI::Button(U"セル結合解除", Vec2(mgWpx + 130, 110), unspecified, focArea.isAct && !enableMb())) {
			bool isSuccess = true;
			isChangeData |= reCombCell(cellData, focArea, isSuccess);
			if (!isSuccess) {
				mbs["ReCombFal"].enable = true;
			}
			isClickBotton = true;
		}
		fonts[20](U"文字揃え").draw(Vec2(mgWpx + 160 + 145, 113), Palette::Black);
		if (SimpleGUI::Button(U"左", Vec2(mgWpx + 160 + 72 + 155, 110), unspecified, focArea.isAct && !enableMb())) {
			isClickBotton = true;
		}
		if (SimpleGUI::Button(U"中", Vec2(mgWpx + 160 + 72 + 220, 110), unspecified, focArea.isAct && !enableMb())) {
			isClickBotton = true;
		}
		if (SimpleGUI::Button(U"右", Vec2(mgWpx + 160 + 72 + 285, 110), unspecified, focArea.isAct && !enableMb())) {
			isClickBotton = true;
		}

		if (mbs["CreateNewTable"].isAnsed()) {
			if (mbs["CreateNewTable"].res == messageBox::resType::YES) {
				int th = ParseOr<int>(tbH.text, -1);
				int tw = ParseOr<int>(tbW.text, -1);
				if ((tbH.text == U"" || th <= 0) || (tbW.text == U"" || tw <= 0)) {
					// 求 自然数
					mbs["ReqN"].enable = true;
					tbH.text = Format(cellData.h);
					tbW.text = Format(cellData.w);
				}
				else {
					// 表サイズの変更
					record = { CellData(th, tw) };
					recordCur = 0;
					tbInput.text = U"";
				}
			}
			else {
				tbH.text = Format(cellData.h);
				tbW.text = Format(cellData.w);
			}
		}
		if (mbs["ColorSel"].isAnsed()) {
			for (int i = focArea.u; i <= focArea.d; i++) {
				for (int j = focArea.l; j <= focArea.r; j++) {
					if (cellData.color[i][j] != cellColorSample::color) {
						isChangeData = true;
						cellData.color[i][j] = cellColorSample::color;
					}
				}
			}
		}
		for (auto& [name, mb] : mbs) {
			if (mb.isAnsed()) mb.reset();
		}
		drawCellData(cellData, focArea, fonts[fontSize::nomal]);
		if (enableMb()) {
			for (auto& [name, mb] : mbs) {
				if (mb.enable) mb.draw();
			}
			if (mbs["ColorSel"].enable) {
				SimpleGUI::ColorPicker(cellColorSample::color, Vec2(320, 210));
			}
			if (mbs["BorderTypeSel"].enable) {
				size_t nowBtLine = (size_t)inputMode::nowInBt.l,
					nowBtThick = (size_t)inputMode::nowInBt.t;
				SimpleGUI::RadioButtons(nowBtLine, btLineList, Vec2(300, 210), 100);
				SimpleGUI::RadioButtons(nowBtThick, btThickList, Vec2(420, 210), 100);
				inputMode::nowInBt.l = (borderType::line)nowBtLine;
				inputMode::nowInBt.t = (borderType::thick)nowBtThick;
			}
			if (mbs["ColorLineSel"].enable) {
				SimpleGUI::ColorPicker(borderColorSample::borderColor, Vec2(320, 210));
				inputMode::nowInBt.color = borderColorSample::borderColor;
			}
		}
		else {
			if (!isClickBotton) {
				isChangeData |= mouseEventProc(cellData, focArea, fonts[fontSize::nomal], preClickTime, ladjIdx, tbInput, mbs["ColorSel"]);
			}
			cellData.reCalcWSum();
			//tbInputUpdate(cellData, focArea, tbInput);
		}

		keyboardInputProc(recordCur, (int)record.size());

		SimpleGUI::TextBox(tbH, Vec2(mgWpx + 210, 10), 60, 2, !enableMb());
		SimpleGUI::TextBox(tbW, Vec2(mgWpx + 210 + 100, 10), 60, 2, !enableMb());
		if (SimpleGUI::Button(U"新規作成", Vec2(mgWpx + 410, 10), unspecified, !enableMb())) {
			mbs["CreateNewTable"].enable = true;
		}
		fonts[fontSize::sub](U"列").draw(mgWpx + 210 + 65, 10, Palette::Black);
		fonts[fontSize::sub](U"行").draw(mgWpx + 210 + 100 + 65, 10, Palette::Black);
		SimpleGUI::TextBox(tbTitle, Vec2(mgWpx, 10), 200, unspecified, !enableMb());
		cellData.title = tbTitle.text;
		if (SimpleGUI::Button(U"コンバート", Vec2(mgWpx + 550, 10), unspecified, !enableMb())) {
			// 表の上と左のborderTypeが一致しているかのチェック
			bool isCorBt = true;
			for (int j = 0; j < cellData.w - 1; j++) {
				if (cellData.btHol[0][j] != cellData.btHol[0][j + 1]) {
					isCorBt = false;
				}
			}
			for (int i = 0; i < cellData.h - 1; i++) {
				if (cellData.btVer[i][0] != cellData.btVer[i + 1][0]) {
					isCorBt = false;
				}
			}
			// 結合したセルの下と右のborderTypeが一致しているかのチェック
			for (int i = 0; i < cellData.h; i++) {
				for (int j = 0; j < cellData.w; j++) {
					if (cellData.cmbPar[i][j] == pos(i, j)) {
						const pos& sz = cellData.cmbSz[i][j];
						// 下
						for (int tj = 0; tj < sz.j - 1; tj++) {
							if (cellData.btHol[i + sz.i][j + tj] != cellData.btHol[i + sz.i][j + tj + 1]) {
								isCorBt = false;
							}
						}
						// 右
						for (int ti = 0; ti < sz.i - 1; ti++) {
							if (cellData.btVer[i + ti][j + sz.j] != cellData.btVer[i + ti + 1][j + sz.j]) {
								isCorBt = false;
							}
						}
					}
				}
			}

			if (isCorBt) {
				String res = convertData(cellData);
				Clipboard::SetText(res);
				mbs["ConvertSuc"].enable = true;
			}
			else {
				mbs["ConvertFal"].enable = true;
			}
		}
		SimpleGUI::CheckBox(inputMode::isBorderMode, U"罫線作成モード", Vec2(mgWpx, 60), unspecified, !enableMb());
		if (SimpleGUI::Button(btLineList[(int)inputMode::nowInBt.l]
			+ U" " + btThickList[(int)inputMode::nowInBt.t],
			Vec2(mgWpx + 200, 60), unspecified, inputMode::isBorderMode && !enableMb())) {
			mbs["BorderTypeSel"].enable = true;
		}
		if (SimpleGUI::Button(U"色　　", Vec2(mgWpx + 340, 60), unspecified, inputMode::isBorderMode && !enableMb())) {
			mbs["ColorLineSel"].enable = true;
		}
		borderColorSample::r.draw(borderColorSample::borderColor);

		if (focArea.isAct && focArea.i1 == focArea.i2 && focArea.j1 == focArea.j2 && !enableMb()) {
			SimpleGUI::TextBox(tbInput, (idxToPos(cellData, focArea.u, focArea.l) + idxToPos(cellData, focArea.d, focArea.l)) / 2 + Vec2(10, 10), cellData.sumW[focArea.r + 1] - cellData.sumW[focArea.l]);
		}

		if (isChangeData) {
			while (recordCur + 1 < (int)record.size()) {
				record.pop_back();
			}
			record.push_back(cellData);
			recordCur++;
			//Print << U"change";
		}
	}
}

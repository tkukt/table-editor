# table-editor
これは[駒場東邦物理部HP](https://github.com/ktpcmember/ktpcmember.github.io)の表の入力支援ツールです。

現在開発中のため，今後大幅に仕様が変更されることがあります。
## ディレクトリ
- Main.cpp：本体のソースコード
- App/tableEditor.exe：アプリケーション本体

ここから直接tableEditor.exeをダウンロードしてもいいですが，使いかたにあるような手順でダウンロードすることをお勧めします。
## 使いかた
まず，[リリース](https://github.com/tkukt/table-editor/releases/)にあるtableEditorをダウンロードして解凍します。解凍したら，以下のファイルが入っています。
- tableEditor.exe：これがアプリケーションの本体です。
- README.md：このREADMEと同じ内容が書いてあります。
- LICENCE：ライセンスです。MIT Licenseです。なお，Siv3DおよびSiv3Dが使用しているサードパーティ・ソフトウェアのライセンスは，F1キーを押すことで表示できます。

tableEditor.exeをダブルクリックして起動します。

起動できたら，まず左上に作成したい表の列数と行数を入力してください。そして，「create new table」をクリックすると，新しくその行数の表が作成されます。このとき，現在のセルのデータは消えてしまうので，注意のための確認画面が出ます。

セルをクリックすると，そのセルがアクティブセルとなり，緑の枠で囲まれます。なお，セルでない箇所をクリックすると，アクティブセルは解除されます。出てきたテキストボックスに記入したい文字列を入力すると，アクティブセルに文字列が書き込まれます。

基本機能に関しては以上です。
その他の機能については以下です。

### 列幅の調整
列タブの列の境界線をドラッグすると，列の幅が調整できます。また，その境界線をダブルクリックすると，文字列の幅にあわせて調整することができます。

### 省略表示
セルの幅に収まらないほど長い文字列がセルに書き込まれると，文字列の全ては表示されずに，収まる長さの文字列のプレフィックスに「...」を付した形で表示されます。
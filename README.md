# mono-to-stereo-gui

## 概要

MS2109系USBキャプチャを使っているユーザー向け[mono-to-stereo](https://github.com/ToadKing/mono-to-stereo)のGUI操作版。

![image](https://user-images.githubusercontent.com/92087784/193593505-111b864e-fe1a-4d71-8e2c-73be9d952ec4.png)


## インストール方法

Releasesから自分のPCにあったアーキテクチャ(x86/x64/arm/arm64)のZIPをダウンロードして解凍する。

解凍先は、書き込みに管理者権限が必要でないフォルダであればどこでも良い。

(例：`C:\Users\ndekopon\Documents\mono-to-strereo-gui` など)


## 使い方

起動するとタスクトレイに常駐する。

右クリックメニューから`capture`(入力)を選択する。

![image](https://user-images.githubusercontent.com/92087784/193987590-5a317cef-4605-4373-8363-1cf28b775e8b.png)

右クリックメニューから`render`(出力)を選択する。

![image](https://user-images.githubusercontent.com/92087784/193987872-15e0bd99-b6bc-488b-833d-d9bdacb0dd08.png)


選択時、両方が有効であればモノラル→ステレオの変換が開始される。

選択結果は同フォルダの`config.ini`に保存されるので、次回起動時は自動的にステレオ変換が開始される。

終了する際は、右クリックメニューから`exit`を選択する。

![image](https://user-images.githubusercontent.com/92087784/193988123-650e6a48-9240-4823-be2d-3c182714883a.png)


## 問題点

手元の環境では入力/出力の速度が一定でないのか、出力バッファが追いついたり離れすぎたりするので、
閾値を超えたら都度1フレームをスキップしたり、前の1フレームを重複させたりして違和感が減るような処理を入れた。

スキップ・重複を実施した回数はアイコンにカーソルを合わせた際に表示される。

![image](https://user-images.githubusercontent.com/92087784/193987174-92411b22-bf03-4eb9-b830-abafd9ba50cb.png)


ほかに問題点があったら同フォルダに生成されるログとともに [@ndekopon](https://twitter.com/ndekopon) へどうぞ。


## テスト環境

下記環境で検証し、音が途切れることなくステレオで聞こえることを確認。

- OS: Windows10 21H2(19044.2006)
- CPU: AMD Ryzen 5 5600X


## Q&A

- Q. `render`に出力したいデバイスが表示されない
    - A. 出力デバイスはサンプルレートが48000Hzになっていないと恐らく表示されないので、48000Hzに変更する
- Q. 複数MS2109のデバイスがあって同時に使いたい
    - A. デバイス毎にフォルダを作り、それぞれに`mono-to-stereo-gui.exe`を入れて使う


## ライセンス

[MIT license](https://choosealicense.com/licenses/mit/)


## 謝辞

音声処理部分は [mono-to-stereo](https://github.com/ToadKing/mono-to-stereo) を参考に改変した。感謝。


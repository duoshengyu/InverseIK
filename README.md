# If the text is garbled, please convert encoding to UTF-8.
# Character Inverse Kinematic
このプログラムはInverse Kinematicの実装です。

実行ファイル：Debug/MyPick.exe

*実行環境等を含めた実行方法:

動作確認：Windows 10　Visual Studio 2013

開発時間と人数:  二年前　グラフィックスのプログランム書くことがないから　四か月　一人

開発言語：C++  directx 9

外部ライブラリ：なし

実行方法："MyPick.sln"　Visual Studio 2013で開けたら、F5を押す。

操作方法：

右マウスクリック+ドラッグ　　　視点を回転

w,a,s,dボタン　　　　　　　カメラの移動　　　　　　　　　　

キャラクターの手と足で左マウスクリック+ドラッグ　　キャラクター姿勢を変更

*プログラムを作成する上で苦労した箇所は？

デバッグ、キャラクターモーションの勉強

*力をいれて作った部分で、「プログラム上」で特に注意してみてもらいたい箇所は？

Character.cpp中のCreateBoneBox関数。
キャラクターのメッシュは全体一つなので、選択場所をわかるように、手足にジオメトリーオブジェクトを
入れ込見ました。ジオメトリーオブジェクを手足と同じ変更するようにボーンから変換行列を抽出しています。

*参考にしたソースファイルがあるなら、どの様なところを参考にしましたか？またその部分のファイル名を書いてください

IKの実装は本「Character Animation with Direct3D」に参考。

*他人のコードと関数：　

メッシュのデータは本「Character Animation with Direct3D」のデータです。
 IKの実装も「Character Animation with Direct3D」に参考。
 BoneHierarchyLoader.h　BoneHierarchyLoader.cppはDirectXのsamples
 からです、少し修正。


# InverseIK
This is a simple program implement Inverse Kinematics.
Reference to the book Character Animation with Direct3D by Carl Cranberg,
and The Directx example program LocalDeformablePRT and Pick.

Platform:           Windows 10

Develop Tool:       Visual Studio 2013

Program language:   C++  directx 9

External libraries: none

usage:

1.Download zip or clone this repository.

2.Open "MyPick.sln" using Visual Studio 2013(Only test in version 2013).

3.Run.

prerequest:

Install directx SDK.
  
Screenshots:

![image](https://github.com/duoshengyu/InverseIK/blob/master/screenshots/1.PNG)

![image](https://github.com/duoshengyu/InverseIK/blob/master/screenshots/2.PNG)

![image](https://github.com/duoshengyu/InverseIK/blob/master/screenshots/3.PNG)

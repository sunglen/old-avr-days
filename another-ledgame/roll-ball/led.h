//--------------------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//   LED表示関数
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//--------------------------------------------------------------------------------
#define LEDWIDTH 16		// LEDドット幅
#define LEDXMIN  0
#define LEDXMAX  15
#define LEDYMIN  0
#define LEDYMAX  15

//--------------------------------------------------------------------------------
// LED初期化
// ・LED用ポートを初期化します。
// ・VRAMの内容を消します。
void led_init(void);

//--------------------------------------------------------------------------------
// LED消灯
// ・LEDを消します。スリープ前に使います。
// ・VRAMの内容は消えません。
void led_off(void);

//--------------------------------------------------------------------------------
// LED制御(32ビット送信)
// ・VRAM内容の1行分を表示します。
// ・16回呼び出すと1画面更新に相当します。
// ・メインループかタイマ割り込み関数内に置き、繰り返し呼び出してください。
void led_disp(void);

//--------------------------------------------------------------------------------
// VRAMアドレス取得
// ・VRAMを直接書き換えたいときに使用
// 戻り値：unsigned int 型ポインタ。VRAMの先頭アドレス。
// 　　　　VRAMは[1行目][2行目]...[16行目]の順で16bitずつ格納されています。
//使用例：unsigned int *p = led_getvram();   // VRAMアドレスを取得
unsigned int * led_getvram(void);

//--------------------------------------------------------------------------------
//ピクセル描画
//引数x：X座標。0〜15
//引数x：Y座標。0〜15
//引数color：カラーコード。0=消灯、1=点灯（赤色）、2=XOR1
//使用例：led_pset(5,8,1);   // 座標(5,8)に点を描きます。
void led_pset(char x,char y,char color);

//--------------------------------------------------------------------------------
//ライン描画
//引数x1：X1座標
//引数y1：Y1座標
//引数x2：X2座標
//引数y2：Y2座標
//引数color：カラーコード。0=消灯、1=点灯（赤色）、2=XOR1
void led_line(char x1 ,char y1 ,char x2 ,char y2 ,char color);

//--------------------------------------------------------------------------------
// 画面スクロール
// ・VRAMの中身を強制的にシフトします。
// 引数x1: X方向移動量。+だと左シフト(視点を右へ)、-だと右シフト(視点を左へ)
// 引数y1: Y方向移動量。+だと上シフト(視点を下へ)、-だと下シフト(視点を上へ)
void led_scroll(char x1,char y1);

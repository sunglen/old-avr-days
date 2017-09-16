//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//   サウンド再生関数
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------

//--------------------------------------------------------------------------
//サウンド機能初期化
// ・サウンド機能を初期化します。タイマ2を使用します。
//戻り値：int型。最大周波数の取得
unsigned int beep_init(void);


//---------------------------------------------------------------------------
//サウンドの強制オフ
//・サウンド出力を強制的に止めます。
void beep_off(void);


//---------------------------------------------------------------------------
//サウンド再生
// 引数hz：周波数[Hz]
// 引数time：再生時間[100ms単位]。たとえば10なら1秒間再生します。
void beep_set(unsigned int hz, int time);


//---------------------------------------------------------------------------
//サウンド出力カウンタの取得
//戻り値：int型。0以上だと再生中という意味です。
unsigned int beep_getcnt(void);


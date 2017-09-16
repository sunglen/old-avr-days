//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
// スイッチ用関数
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------


#define SW_B	 (1<<1)
#define SW_A	 (1<<2)
#define SW_UP	 (1<<4)
#define SW_LEFT	 (1<<5)
#define SW_RIGHT (1<<6)
#define SW_DOWN	 (1<<7)
#define SW_ALL   (SW_UP|SW_DOWN|SW_LEFT|SW_RIGHT|SW_A|SW_B)

//---------------------------------------------------------------
// スイッチ初期化
//   引数:初期化するポートのマスク
void sw_init(unsigned char bitmask);

//---------------------------------------------------------------
// スイッチ読み取り
//   引数:検出するポートのマスク
//   戻り値：0=スイッチオフ / 1=スイッチオン
char sw_get(unsigned char bitmask);


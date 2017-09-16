//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
// スイッチ用関数
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>

#define SW_PORT  PORTD		// プルアップ
#define SW_DDR   DDRD       // ポート入力設定
#define SW_PIN   PIND       // 入力ポート

//---------------------------------------------------------------
// スイッチ初期化
//   引数:初期化するポートのマスク
void sw_init(unsigned char bitmask)
{
 	SW_DDR  &= ~bitmask; 	//input
	SW_PORT |= bitmask; 	//pullup
}


//---------------------------------------------------------------
// スイッチ読み取り
//   引数:検出するポートのマスク
//   戻り値：0=スイッチオフ / 1=スイッチオン
char sw_get(unsigned char bitmask)
{
	if(SW_PIN & bitmask)
		return(0);  // スイッチ押されてない
	else
		return(1);  // スイッチ押されている
}


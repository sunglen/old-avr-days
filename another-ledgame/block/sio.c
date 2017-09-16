//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//  ISP用シリアル通信関数
//
// 商用利用は禁止。再配布は自由にできます。
// 改造して配布する場合にはわかりやすい形で明記してください。
// この文は修正禁止(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include "sio.h"

#define BAUDRATE 9600        //ボーレート

#ifndef F_CPU
#define   F_CPU   8000000    //CPUクロック周波数 8MHz
#endif

#define  PRESCALE   1        //プリスケーラ値
#define  PRESCALECR 1        //プリスケーラ設定値
#define  SIO_PORT PORTB      //ISP用ポート
#define  SIO_PIN  PINB       //ISP用ポート入力
#define  SIO_DDR  DDRB       //ISP用ポート入出力設定
#define  SIO_TX   4          //送信ビット
#define  SIO_RX   3          //受信ビット
#define  SIO_TIFR    TIFR1   //タイマ1フラグ
#define  SIO_MAXCNT  0x10000 //タイマ最大値

#define SIO_TCNT    ((F_CPU/PRESCALE)/BAUDRATE)

//-----------------------------------------------------------------------
// シリアルポート初期化
// ・タイマ1を使用します。他のプログラムでタイマ1を使用していると使えません。
// ・ISPコネクタのポートを設定します。
void sio_init(void)
{
    SIO_DDR |=  (1<<SIO_TX);
    SIO_DDR &= ~(1<<SIO_RX);
    SIO_PORT &= ~(1<<SIO_TX);
    SIO_PORT |= (1<<SIO_RX);

    TCCR1A= 0;                // タイマ1 モード 
    TCCR1B= PRESCALECR;       // タイマ1 プリスケーラ設定
}


//-----------------------------------------------------------------------
// 1bit分ウエイト
void sio_bitwait(void)
{
    while(!(SIO_TIFR & (1 << TOV1)));
    // TOV1ビットが1になるまで

    TCNT1 = SIO_MAXCNT-SIO_TCNT;
    SIO_TIFR |= (1 << TOV1);  // TIFRのビットをクリア
}

//-----------------------------------------------------------------------
// 1バイト送信
//引数 sio_txdat：送信データ0x00-0xff
int sio_tx(char sio_txdat)
{
    unsigned char mask = 0x01;

    //スタートビット
    SIO_PORT |= (1<<SIO_TX);
    TCNT1 = SIO_MAXCNT-SIO_TCNT;
    SIO_TIFR |= (1 << TOV1);  // TIFRのビットをクリア
    sio_bitwait();

    //データビット0-7
    while(mask != 0){    
        if(sio_txdat & mask)
            SIO_PORT &= ~(1<<SIO_TX);
        else
            SIO_PORT |= (1<<SIO_TX);

        mask = mask<<1;
        sio_bitwait();
    }
    //ストップビット
    SIO_PORT &= ~(1<<SIO_TX);
    sio_bitwait();
    return(0);
}

//-----------------------------------------------------------------------
// 1バイト受信
// 戻り値: 受信データ0x00～0xFF / 0xFFFF(-1)の場合はタイムアウトエラー
int sio_rx(void)
{
    #define TIMEOUT 30000
    int timeo;
    unsigned char sio_rxdat;
    unsigned char mask = 0x01;

    for(timeo=0; timeo<TIMEOUT; timeo++)
    {    
        if(SIO_PIN & (1<<SIO_RX)){  //スタートビット検出
               TCNT1 = SIO_MAXCNT-(SIO_TCNT/2);
            SIO_TIFR |= (1 << TOV1);  // TIFRのビットをクリア
            break;
        }
    }
    if(timeo>=TIMEOUT)return(-1);

    //スタートビット 読み飛ばし
    sio_bitwait();

    //データビット0-7
    sio_rxdat=0;
    while(mask != 0){    
        sio_bitwait();

        if((SIO_PIN & (1<<SIO_RX))==0)
            sio_rxdat |= mask;

        mask = mask<<1;
    }

    sio_bitwait();    //ストップビット読み飛ばし

    return((int)sio_rxdat);
}

//-----------------------------------------------------------------------
// 文字列送信
//引数 *str：文字列のポインタ。終端はNULL(0)
void sio_txstr(char *str)
{
    while(*str != 0)
    {
        sio_tx(*str++);
    }
}

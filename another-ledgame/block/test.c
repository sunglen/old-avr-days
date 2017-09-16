//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006-2007
// http://www.nico.to/avr/
//
//   NAZONO BLOCK
//�E�u���b�N������
//�E�ʃN���A����܂���
//�EX�ړ��ʂ��ω����܂���
//
// ���p���p�͋֎~�B�Ĕz�z�͎��R�ɂł��܂��B
// �������Ĕz�z����ꍇ�ɂ͂킩��₷���`�Ŗ��L���Ă��������B
// ���̕��͏C���֎~(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <avr/sleep.h>
#include "led.h"
#include "ledx.h"
#include "rand.h"
#include "sio.h"
#include "sw.h"
#include "beep.h"
#include "sleeping.h"

#define BLOCKMAX 5
unsigned int *pVram;
int shield[BLOCKMAX];


void led_printd(int x);
void led_print(char *p);



//---------------------------------------------------------------------
// 10�i��4���\��
void led_printd(int x)
{
	unsigned char temp;
	int ketaval=1000;

	while (ketaval != 0) {
		temp = x / ketaval;
		led_putch('0'+temp);
		x -= (ketaval*temp);
		ketaval /= 10;
	}
}

#define SHIELD_Y 2
//---------------------------------------------------------------------
// ������\��
void led_print(char *p)
{
	while(*p != 0){
		led_putch(*p++);
	}
}


//--------------------------------
// �\��
unsigned int shield_put(void)
{
	unsigned int total=0;
	int i;

	for(i=0;i<BLOCKMAX;i++){
		*(pVram+SHIELD_Y+i) = shield[i];
		total |= shield[i];
	}
	return(total);
}



//---------------------------------------------------------------------
// ���C������
int main(void)
{
	#define BALLSPEED 300   // �{�[���ړ����[�v��
	#define SWCHKCNT 200   // �X�C�b�`���o���[�v��
	#define BARWIDTH 3
	#define SFT 2
	int i;
	int keycnt;
	int ballcnt;
	char tx,ty;
	int score;	//Score
	char x,y;	//My Racket X,Y
	char bx,by;	//Ball X,y
	char bx1,by1;	//Ball X1,y1

	sw_init(SW_ALL);
	rand_init();
	sio_init();
	beep_init();
	led_init();				// LED������
	pVram = led_getvram();	// VRAM�A�h���X���擾
	sei();					//���荞�݋���

	while(1){
		x = 1;	//�v���[�����W
		y = 14;
		score = 0;

		led_init();				// LED������
		led_locate(0,0);
		led_print("NAZO");
		led_locate(0,8);
		led_print("BLK.");
		beep_set(500,10);

		for(i=0;i<10000;i++)
			led_disp();		// LED��ʍX�V

		led_init();				// LED������

		bx = 1 << SFT;	//���W
		by = (SHIELD_Y+BLOCKMAX)<<SFT;
		bx1 = 1;
		by1 = 2;
		shield[0]=((1<<13)|(1<<12)|(1<<11)|(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|0);
		shield[1]=(0);
		shield[2]=((1<<13)|(1<<12)|(1<<11)|(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|0);
		shield[3]=(0);
		shield[4]=((1<<13)|(1<<12)|(1<<11)|(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|0);
		shield_put();

		keycnt = 0;
		while(1){
			led_disp();		// LED��ʍX�V

			ballcnt++;
			if(ballcnt >= BALLSPEED){ //�{�[���ړ�
				ballcnt = 0;
				led_pset(bx>>SFT,by>>SFT,0);	//�{�[������

				tx = (bx+bx1)>>SFT;
				ty = (by+by1)>>SFT;
				if(ty <  0) by1 = -by1;
				if(tx <  0) bx1 = -bx1;
				if(tx > 15) bx1 = -bx1;

				if((ty==y)&&(tx>=x)&&(tx<(x+BARWIDTH))){
					beep_set(300,1);
					by1 = -by1;
				}
				if(ty>=15)break;

				tx=((bx+bx1)>>SFT);
				ty=(by>>SFT);
				i = ty - SHIELD_Y;
				if((i>=0)&&(i<BLOCKMAX)){
					if(shield[i] &(0x8000 >> tx)){
						shield[i] &= ~(0x8000 >> tx);
						bx1 = -bx1;
						score++;
						beep_set(600,1);
						shield_put();
					}
				}
				tx=(bx>>SFT);
				ty=((by+by1)>>SFT);
				i = ty - SHIELD_Y;
				if((i>=0)&&(i<BLOCKMAX)){
					if(shield[i] &(0x8000 >> tx)){
						shield[i] &= ~(0x8000 >> tx);
						by1 = -by1;
						score++;
						beep_set(600,1);
						shield_put();
					}
				}
				bx += bx1;
				by += by1;

				led_pset(bx>>SFT,by>>SFT,1);//�{�[���\��
			}

			keycnt++;
			if(keycnt >= SWCHKCNT) {
				keycnt = 0;

				led_line(x,y,x+(BARWIDTH-1),y,0);

				if((sw_get(SW_LEFT))&&(x>0)){				// �E�{�^��
					x--;
				}
				if((sw_get(SW_RIGHT))&&(x<=(LEDXMAX-BARWIDTH))) {	// ���{�^��
					x++;
				}
				if(sw_get(SW_A)){	// A�X�C�b�`
					sleeping();
				}
				led_line(x,y,x+(BARWIDTH-1),y,1);
			}
		}
		beep_set(170,15);
		led_locate(0,4);
		led_printd(score);

		for(i=0;i<20000;i++)
			led_disp();		// LED��ʍX�V

		sleeping();
	}
}
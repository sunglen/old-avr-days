//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006-2007
// http://www.nico.to/avr/
//
//   LED�\���֐�
//
// ���p���p�͋֎~�B�Ĕz�z�͎��R�ɂł��܂��B
// �������Ĕz�z����ꍇ�ɂ͂킩��₷���`�Ŗ��L���Ă��������B
// ���̕��͏C���֎~(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include "led.h"

#define CAPTURE_SW 0
#if CAPTURE_SW
	#include "sio.h"  // CAPTURE
#endif

unsigned int ledvram[LEDWIDTH];	 // LED Bitmap Buffer
char row=0;
char scanflag=0;

#define LED_PORT  PORTB	// LED�p�|�[�g
#define LED_DDR   DDRB	 // LED�p�|�[�g���o��
#define LED_CLK   0		// �N���b�N�r�b�g�ԍ�
#define LED_DATA  2		// �f�[�^�r�b�g�ԍ�
#define LED_LATCH 1		// ���b�`�r�b�g�ԍ�

void led_sendword(unsigned int dat);
void led_sendword2(unsigned int dat);
void led_sendwordv(unsigned int dat);
//void led_sendbyte2(unsigned int dat);

//---------------------------------------------------------------------
// LED������
// �ELED�p�|�[�g�����������܂��B
// �EVRAM�̓��e�������܂��B
void led_init(void)
{
	int i;

	LED_DDR |= ((1<<LED_LATCH)|(1<<LED_CLK)|(1<<LED_DATA));	//output
	LED_PORT &= ~(1<<LED_CLK);
	LED_PORT &= ~(1<<LED_LATCH);

	row=0;
	for(i=0;i<LEDWIDTH;i++)
		ledvram[i] = 0x0000;
}

//---------------------------------------------------------------------
// VRAM�A�h���X�擾
// �EVRAM�𒼐ڏ������������Ƃ��Ɏg�p
// �߂�l�Funsigned int �^�|�C���^�BVRAM�̐擪�A�h���X�B
// �@�@�@�@VRAM��[1�s��][2�s��]...[16�s��]�̏���16bit���i�[����Ă��܂��B
//�g�p��Funsigned int *p = led_getvram();   // VRAM�A�h���X���擾
unsigned int * led_getvram(void)
{
	return(ledvram);
}


//---------------------------------------------------------------------
// LED����
// �E���W�b�NIC���ׂĂ�Low�o�͂����āALED�������܂��B
// �E�X���[�v�O�Ɏg���܂��B
// �EVRAM�̓��e�͏����܂���B
void led_off(void)
{
	led_sendword(0x0000);
	led_sendword(0x0000);
	led_sendword(0x0000);

	LED_PORT |= (1<<LED_LATCH);
	LED_PORT &= ~(1<<LED_LATCH);
}

#if CAPTURE_SW
	unsigned char cap_cnt; //CAPTURE
#endif

//---------------------------------------------------------------------
// LED����(32�r�b�g���M)
// �EVRAM���e��1�s����\�����܂��B
// �E16��Ăяo����1��ʍX�V�ɑ������܂��B
// �E���C�����[�v���^�C�}���荞�݊֐����ɒu���A�J��Ԃ��Ăяo���Ă��������B
void led_disp(void)
{
	unsigned int temp;

	temp = 0x8000 >> row;
	if(scanflag){
		led_sendword(0xFFFF);	
		led_sendword(temp);		// �J�\�[�h
		//led_sendword2(ledvram[(int)row]);	//�A�m�[�h
		led_sendword(~ledvram[(int)row]);	//�A�m�[�h
	}else{
		led_sendword(0xFFFF);	
		led_sendwordv(temp);		// �J�\�[�hRed(�_��)
		//led_sendword2(temp);	//�A�m�[�h
		led_sendword(~temp);	//�A�m�[�h
	}

	LED_PORT |= (1<<LED_LATCH);
	LED_PORT &= ~(1<<LED_LATCH);

#if CAPTURE_SW
	if(cap_cnt < LEDWIDTH){		  //CAPTURE
		if(cap_cnt==0){
			sio_init();
			sio_tx('@'); //CAPTURE
		}
		temp = ledvram[(int)row];	//CAPTURE
		sio_tx(temp & 0xFF);		 //CAPTURE
		sio_tx(temp >> 8);		   //CAPTURE
	}								//CAPTURE
	cap_cnt++;					   //CAPTURE
#endif
	row++;
	if(row >= LEDWIDTH){
		row=0;
		scanflag ^=1;
	}
}

//---------------------------------------------------------------------
//�s�N�Z���`��
//����x�FX���W�B0�`15
//����x�FY���W�B0�`15
//����color�F�J���[�R�[�h�B0=�����A1=�_���i�ԐF�j�A2=XOR1
void led_pset(char x,char y,char color)
{
	unsigned int mask;

	if(x<0)return;
	if(y<0)return;
	if(x>15)return;
	if(y>15)return;

	mask=0x8000 >> x;

	if(color==1)
		ledvram[(int)y] |= mask;
	else if(color==0)
		ledvram[(int)y] &= ~mask;
	else if(color==2)
		ledvram[(int)y] ^= mask;

}

//--------------------------------------------------------------------------------
//���C���`��
//����x1�FX1���W
//����y1�FY1���W
//����x2�FX2���W
//����y2�FY2���W
//����color�F�J���[�R�[�h�B0=�����A1=�_���i�ԐF�j�A2=XOR1
void led_line(char x1 ,char y1 ,char x2 ,char y2 ,char color)
{
	char xd;	// X2-X1���W�̋���
	char yd;	// Y2-Y1���W�̋���
	char xs=1;  // X������1pixel�ړ���
	char ys=1;  // Y������1pixel�ړ���
	char e;

	xd = x2 - x1;	 // X2-X1���W�̋���
	if(xd < 0){
		xd = -xd;	 // X2-X1���W�̐�Βl
		xs = -1;	  // X������1pixel�ړ���
	}
	yd = y2 - y1;	 // Y2-Y1���W�̋���
	if(yd < 0){
		yd = -yd;	 // Y2-Y1���W�̐�Βl
		ys = -1;	  // Y������1pixel�ړ���
	}
	led_pset (x1, y1 ,color); //�h�b�g�`��
	e = 0;
	if( yd < xd ) {
		while( x1 != x2) {
			x1 += xs;
			e += (2 * yd);
			if(e >= xd) {
				y1 += ys;
				e -= (2 * xd);
			}
			led_pset (x1, y1 ,color); //�h�b�g�`��
		}
	}else{
		while( y1 != y2) {
			y1 += ys;
			e += (2 * xd);
			if(e >= yd) {
				x1 += xs;
				e -= (2 * yd);
			}
			led_pset (x1, y1 ,color); //�h�b�g�`��
		}
	}
}

//--------------------------------------------------------------------------------
// ��ʃX�N���[��
// �EVRAM�̒��g�������I�ɃV�t�g���܂��B
// ����x1: X�����ړ��ʁB+���ƍ��V�t�g(���_���E��)�A-���ƉE�V�t�g(���_������)
// ����y1: Y�����ړ��ʁB+���Ə�V�t�g(���_������)�A-���Ɖ��V�t�g(���_�����)
void led_scroll(char x1,char y1)
{
	int i;
	if(x1!=0){
		if(x1>0){
			for(i=0; i<LEDWIDTH; i++){
				ledvram[i] <<= x1;
			}
		}else{
			for(i=0; i<LEDWIDTH; i++){
				ledvram[i] >>= (-x1);
			}
		}
	}

	if(y1!=0){
		if(y1>0){
			for(i=0; i<LEDWIDTH; i++){
				if((i+y1)>15)
					ledvram[i] = 0;
				else
					ledvram[i] = ledvram[i+y1];
			}
		}else{
			for(i=LEDWIDTH-1; i>=0; i--){
				if((i+y1)<0)
					ledvram[i] = 0;
				else
					ledvram[i] = ledvram[i+y1];
			}
		}
	}
}



//---------------------------------------------------------------------
// ��ʃr�b�g����16bit���M
void led_sendword(unsigned int dat)
{
	char bitcnt=16;

	while(bitcnt-- > 0)
	{
		if(dat & 0x8000)
			LED_PORT |=  (1<<LED_DATA);
		else
			LED_PORT &= ~(1<<LED_DATA);

		dat <<= 1;
		LED_PORT |=  (1<<LED_CLK);
		LED_PORT &= ~(1<<LED_CLK);
	}
}

//---------------------------------------------------------------------
// ���ʃr�b�g����16bit���M
void led_sendword2(unsigned int dat)
{
	char bitcnt=16;

	while(bitcnt-- > 0)
	{
		if(dat & 1)
			LED_PORT |=  (1<<LED_DATA);
		else
			LED_PORT &= ~(1<<LED_DATA);

		dat >>= 1;
		LED_PORT |=  (1<<LED_CLK);
		LED_PORT &= ~(1<<LED_CLK);
	}
}

//---------------------------------------------------------------------
// ���ʃr�b�g����8bit���M
//void led_sendbyte2(unsigned int dat)
//{
//	char bitcnt=8;
//
//	while(bitcnt-- > 0)
//	{
//		if(dat & 0x1)
//			LED_PORT |=  (1<<LED_DATA);
//		else
//			LED_PORT &= ~(1<<LED_DATA);
//
//		dat >>= 1;
//		LED_PORT |=  (1<<LED_CLK);
//		LED_PORT &= ~(1<<LED_CLK);
//	}
//}

//---------------------------------------------------------------------
// LED�c1�񂸂�16�r�b�g���M���邽�߂̊֐�
void led_sendwordv(unsigned int bitmask)
{
	char bitcnt=16;
	int i=0;

	while(bitcnt-- > 0)
	{
		if(ledvram[i++] & bitmask)
			LED_PORT |=  (1<<LED_DATA); //LED�_��
		else
			LED_PORT &= ~(1<<LED_DATA); //LED����

		LED_PORT |=  (1<<LED_CLK);
		LED_PORT &= ~(1<<LED_CLK);
	}
}

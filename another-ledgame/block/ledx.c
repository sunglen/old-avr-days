//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//   LED�\���֐� �ǉ��@�\
// �E���̊֐����g�����߂ɂ�"sfont.h"���K�v�ł��B
//
// ���p���p�͋֎~�B�Ĕz�z�͎��R�ɂł��܂��B
// �������Ĕz�z����ꍇ�ɂ͂킩��₷���`�Ŗ��L���Ă��������B
// ���̕��͏C���֎~(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "led.h"
#include "ledx.h"
#include "sfont.h"	//4x6�X���[���t�H���g


char text_x=0;  // printf�p�J�[�\���ʒu
char text_y=0;


//---------------------------------------------------------------------
// �J�[�\���ʒu�ݒ�
// ����x�FX���W0�`15�B13�ȏゾ�Ǝ������s�B
// ����y�FY���W0�`15�B11�ȏゾ�Ɖ��։�ʃX�N���[�����܂��B
void led_locate(char tx,char ty)
{
	text_x = tx;
	text_y = ty;
}

//---------------------------------------------------------------------
// 1�L�����N�^��VRAM�]��
// �E4x6�X���[���t�H���g�p
// ����ch�F�L�����N�^�[�R�[�h�i0x00-0xff�j
// �߂�l�F0��Ԃ��܂��B
int led_putch(char ch)
{
	char tx;
	unsigned char i;
	unsigned char bitdata;
	unsigned int *pVram;
	PGM_P p;

	if((ch==10)||(text_x > (LEDWIDTH-4))){
		text_x = 0;
		text_y += 6;
	}
	if(text_y > (LEDWIDTH-6)){
		led_scroll(0,text_y-(LEDWIDTH-6));
		text_y=LEDWIDTH-6;
	}
	if((unsigned char)ch < 0x20)return 0;
	pVram = led_getvram();
	pVram += text_y;

	tx = (LEDWIDTH-4)-text_x;
	p = (PGM_P)smallfont;
	p += ((int)((unsigned char)ch - 0x20) * 3);

	for(i=0 ;i<6 ;i++) {
		bitdata = pgm_read_byte(p);
		if((i % 2)==0){
			bitdata >>= 4;
		}else{
			p++;
		}
		bitdata &= 0xf;
		*pVram &= ~(0xf << tx);
		*pVram |= ((int)bitdata << tx);
		pVram++;
	}
	text_x += 4;    // �J�[�\���ړ�
	if(((unsigned char)ch>=0xB0)&&((unsigned char)ch<=0xDC))
		text_x += 1;    // �J�[�\���ړ�(�J�^�J�i�p)

	return 0;
}



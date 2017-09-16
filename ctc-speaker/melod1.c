/* *******************************************************************************
   melod1.c  ATmega48�p�@��`�g�P��d�q�I���S�[���@050320�@im
�@���̃v���O������
�@�u04.12.13 tiny2313�� ��`�g�P��d�q�I���S�[��  koyama@cc.hirosaki-u.ac.jp�v
�@���ꕔ�ύX���č쐬�������̂ł��B
�@mega48���WMHz�Ŏg�p���܂��B
�@���̂܂܂ł̓R���p�C���ł��܂���B��̂R�Ȃ���ЂƂ�I���main�Ɉڂ��A
�@�s�v���͍폜���Ă���R���p�C�����Ă��������B
***********************************************************************************/
#define  Tmp  6                // �e���|...L16=Tmp*25.6ms �@���l������������Ƒ����Ȃ�
#include <avr/io.h>
#include "melotab1.c"
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "lcd.h"

#define demodelay 0xff

void mydelay(unsigned char v)
{
	unsigned char j;
	unsigned int i;
	for (j=0;j<v;j++) for (i=0;i<8000;i++) asm volatile ("nop"::);
}


uint8_t t,i;
void b(uint16_t x,uint8_t y){
  if(x>0){TCCR1A=0x40;  OCR1A=x;  }    // ��v�Ńg�O������@x�͹�K�Ō��܂�16�r�b�g�l
  else  TCCR1A=0x00;                  // �x���ł̓g�O�����Ȃ�
  for(t=0;t<y;t++) for(TCNT0=0;TCNT0<200;);  // .125us*1024*200*y=25.6*y ms�@����̒���
  TCCR1A=0x00;for(t=0;t<2;t++) for(TCNT0=0;TCNT0<10;); //������Ȃ���Ȃ��悤��
}
int main (void){
  
  lcd_init(LCD_DISP_ON);
  lcd_clrscr();
  
  lcd_puts("Hello, World!\n --mega48");
  mydelay(demodelay);
  lcd_clrscr();
  lcd_puts("CTC speaker!\n");
  mydelay(demodelay);
  lcd_clrscr();

  DDRB|=(1<<PB1);
  PORTB|=(1<<PB1);              // �S�o��
  TCCR1B=9;                  // Timer1: CK/1, CTC
  TCCR0B=5;                  // Timer0: CK/1024
  for(i=1;i<100;i++){         // 2��3�ɂ����2�J��Ԃ��@100��99�J��Ԃ�
/*�@--------------�@��������@���̓����}�[�N��T�ɹ�����R�s�[����---------*/
//�h���~�t�@�\���V�h+�x��
//    b(DO,L4);b(RE,L4);b(MI,L4);b(FA,L4);b(SO,L4);b(RA,L4);b(SI, L4);b(DOH,L4);b(Rest,L2);
// select the timebase=50 us (in the oszifox), the rect wave can be seen.
b(FA,L4);

/*�@--------------�@��������@��ɹ�����R�s�[����--------------------------*/
  }
  b(Rest,L2);
}                // �v���O�����̏I���@�R���p�C�����͎��̍s�Ⱥ���폜����




/*
//�h�V���\�t�@�~���h
    b(DOH,L2);b(SI,L2);b(RA,L2);b(SO,L2);b(FA,L2);b(MI,L2);b(RE,L2);b(DO,L2);
//�i����ɺ�́j�V���\�t�@�~���h
    b(SIL,L2);b(RAL,L2);(SOL,L2);b(FAL,L2);b(MIL,L2);b(REL,L2);b(DOL,L1);

//�������Ȃ���́@���̂�����
    b(DO,L4);b(DO,L8);b(RE,L8);b(MI,L8);b(MI,L8);b(SO,L4);//b(Rest,L2);
    b(MI,L8);b(MI,L8);b(RE,L8);b(RE,L8);b(DO,L4);b(Rest,L4);//b(Rest,L2);
//���Ȃ��Ƃ킽��
    b(MI,L4);b(MI,L8);b(FA,L8);b(SO, L4);b(DOH,L4); //b(Rest,L2);
    b(RA, L4);b(DOH,L4);b(SO,L4);b(Rest,L4);  //b(Rest,L8);
//�Ȃ��悭�@�����т܂���
    b(DOH,L4);b(DOH,L4);b(SI,L4);b(SO,L4); //b(Rest,L8);
    b(RA,L8);b(RA,L8);b(RA,L8);b(RA,L8);b(SO,L4);b(Rest,L4);
//�������Ȃ���́@���̂�����
    b(DO,L4);b(DO,L8); b(RE,L8); b(MI,L8); b(MI,L8); b(SO,L4);
    b(MI,L8);b(MI,L8); b(RE,L8); b(RE,L8);b(DO,L4);b(Rest,L4);


//�������Ȃ���́@���̂�����
    b(DO,L4);b(DO,L8);b(RE,L8);b(MI,L8);b(MI,L8);b(SO,L4);//b(Rest,L2);
    b(MI,L8);b(MI,L8);b(RE,L8);b(RE,L8);b(DO,L4);b(Rest,L4);//b(Rest,L2);
//���Ȃ��Ƃ킽��
    b(MI,L4);b(MI,L8);b(FA,L8);b(SO, L4);b(DOH,L4); //b(Rest,L2);
    b(RA, L4);b(DOH,L4);b(SO,L4);b(Rest,L4);  //b(Rest,L8);
//�Ȃ��悭�@�����т܂���
    b(DOH,L4);b(DOH,L4);b(SI,L4);b(SO,L4); //b(Rest,L8);
    b(RA,L8);b(RA,L8);b(RA,L8);b(RA,L8);b(SO,L4);b(Rest,L4);
//�������Ȃ���́@���̂�����
    b(DO,L4);b(DO,L8); b(RE,L8); b(MI,L8); b(MI,L8); b(SO,L4);
    b(MI,L8);b(MI,L8); b(RE,L8); b(RE,L8);b(DO,L4);b(Rest,L4);



//����Ђ���̂Ȃ�
    b(Rest,L8);b(SO,L8);b(FAu,L8);b(SO,L8);b(MI,L2);
    b(Rest,L8);b(MI,L8);b(REu,L8);b(MI,L8);b(DO,L2);
//���܂���ɂł�����
    b(Rest,L8);b(MI,L8);b(RE,L8);b(DO,L8);b(RE,L2);
    b(Rest,L8);b(SO,L8);b(RA,L8);b(SO,L8);b(MI,L2);
//�͂Ȃ�������݂̂�
    b(Rest,L8);b(SO,L8);b(RA,L8);b(SI,L8);b(DOH,L4);b(SO,L4);b(MI,L4);b(DO,L4);b(RA,L2);
//���܂���ɂł�����
    b(Rest,L8);b(RA,L8);b(SI,L8);b(RA,L8);b(SO,L4);b(FA,L4);b(MI,L4);b(RE,L4);b(DO,L4);
*/

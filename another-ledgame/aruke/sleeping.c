//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006-2007
// http://www.nico.to/avr/
//
//   Sleep
//
// ���p���p�͋֎~�B�Ĕz�z�͎��R�ɂł��܂��B
// �������Ĕz�z����ꍇ�ɂ͂킩��₷���`�Ŗ��L���Ă��������B
// ���̕��͏C���֎~(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <avr/sleep.h>
#include "sleeping.h"
#include "led.h"
#include "beep.h"


//---------------------------------------------------------------------
// �s���`�F���W���荞��
SIGNAL (SIG_PIN_CHANGE2)
{
}

//---------------------------------------------------------------------
// �X���[�v
//�E�X���[�v���܂��B�X�C�b�`�������ƕ��A���܂��B
void sleeping(void)
{
	unsigned int i = 50000;
	led_off();	// LED����
	beep_off();	// �T�E���h��~
	while(i-- > 0);	// �X�C�b�`����肪�����̂�҂�

	PCMSK2 |= ((1<<PCINT17)|(1<<PCINT18)|(1<<PCINT20)|(1<<PCINT21)|(1<<PCINT22)|(1<<PCINT23));
	PCICR |= (1<<PCIE2);	// �s���`�F���W���荞�݋���
	PCIFR |= (1<<PCIF2);	// �s���`�F���W���荞�݃t���O
	sei();					//���荞�݋���
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_mode();			//�X���[�v�J�n...
							//...�X���[�v���畜�A
	PCICR &= ~(1<<PCIE2);	// �s���`�F���W���荞�݋֎~
}


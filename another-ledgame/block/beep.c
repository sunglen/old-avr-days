//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//   BEEP
//
// ���p���p�͋֎~�B�Ĕz�z�͎��R�ɂł��܂��B
// �������Ĕz�z����ꍇ�ɂ͂킩��₷���`�Ŗ��L���Ă��������B
// ���̕��͏C���֎~(Please do not delete a document from here.)
//---------------------------------------------------------------------
// 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

#ifndef F_CPU
	#define F_CPU 8000000	// CPU�N���b�N���g��[Hz]
#endif

#define T2PRESCALE  256		// �^�C�}2�v���X�P�[���l
#define T2PRESCALER 6		// �^�C�}2�v���X�P�[���ݒ�
	// 1=div1 / 2=div8 / 3=div32 / 4=div64 / 5=div128 / 6=div256 / 7=div1K

#define HZMAX ((F_CPU/T2PRESCALE)/2)		// �T�E���h�o�͉\�ȍō����g��
#define HZMIN (((F_CPU/T2PRESCALE)/256)/2)	// �T�E���h�o�͉\�ȍŒ���g��

unsigned char beep_sta;
unsigned int beep_cnt;

#define BEEP_PORT PORTD
#define BEEP_DDR  DDRD
#define BEEP_BIT  0

//--------------------------------------------------------------------------
SIGNAL (SIG_OVERFLOW2)
{
    TCNT2 = beep_sta;        // �^�C�}2�̏����l�ݒ�   

	BEEP_PORT ^= (1<<BEEP_BIT);

	if((BEEP_PORT & (1<<BEEP_BIT))==0){ // 1�����̏o�͊���
    	beep_cnt--;
	 	if(beep_cnt == 0){	//�J�E���^��0�̏ꍇ�A�T�E���h�o�͏I��
    		TIMSK2 &= ~(1 << TOIE2);  // �^�C�}2�I�[�o�[�t���[���荞�݋֎~
		}
	}
}
//--------------------------------------------------------------------------
//�T�E���h�@�\������
// �E�T�E���h�@�\�����������܂��B�^�C�}2���g�p���܂��B
//�߂�l�Fint�^�B�Đ��\�ȍō����g����Ԃ��܂��B�Œ���g���͂��̒l��1/256�ł��B
unsigned int beep_init(void)
{
    TCCR2A= 0;       // TCCR2 �^�C�}���[�h�ݒ�
    TCCR2B= T2PRESCALER;       // TCCR2 �v���X�P�[���ݒ� 

	BEEP_DDR |= (1<<BEEP_BIT);	//�|�[�g���o�͂ɐݒ�
	BEEP_PORT &= ~(1<<BEEP_BIT);	//�|�[�g��Low�o��

	return(HZMAX);
}


//---------------------------------------------------------------------------
//�T�E���h�̋����I�t
//�E�T�E���h�o�͂������I�Ɏ~�߂܂��B
void beep_off(void)
{
	TIMSK2 &= ~(1 << TOIE2);  // �^�C�}2�I�[�o�[�t���[���荞�݋֎~
	BEEP_PORT &= ~(1<<BEEP_BIT);	//�|�[�g��Low�o��
}

//---------------------------------------------------------------------------
//�T�E���h�o�̓J�E���^�̎擾
//�߂�l�Fint�^�B0�ȏゾ�ƍĐ����Ƃ����Ӗ��ł��B
unsigned int beep_getcnt(void)
{
	return(beep_cnt);
}

//---------------------------------------------------------------------------
//�T�E���h�Đ�
// ����hz�F���g��[Hz]
// ����time�F�Đ�����[100ms�P��]�B���Ƃ���10�Ȃ�1�b�ԍĐ����܂��B
void beep_set(unsigned int hz,int time)
{
	if(hz < HZMIN) hz=HZMIN;
	if(hz > HZMAX) hz=HZMAX;

	beep_sta = (unsigned char)(0x100- (HZMAX/hz));
	beep_cnt = (hz/10)*time;

    TCNT2 = beep_sta;        // �^�C�}2�̏����l�ݒ�   
    TIFR2 |= (1 << TOV2);    // �I�[�o�[�t���[�t���O���N���A
    TIMSK2 |= (1 << TOIE2);  // �^�C�}2�I�[�o�[�t���[���荞�݋���
}


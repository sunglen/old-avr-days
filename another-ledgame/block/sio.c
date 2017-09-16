//---------------------------------------------------------------------
//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//  ISP�p�V���A���ʐM�֐�
//
// ���p���p�͋֎~�B�Ĕz�z�͎��R�ɂł��܂��B
// �������Ĕz�z����ꍇ�ɂ͂킩��₷���`�Ŗ��L���Ă��������B
// ���̕��͏C���֎~(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>
#include "sio.h"

#define BAUDRATE 9600        //�{�[���[�g

#ifndef F_CPU
#define   F_CPU   8000000    //CPU�N���b�N���g�� 8MHz
#endif

#define  PRESCALE   1        //�v���X�P�[���l
#define  PRESCALECR 1        //�v���X�P�[���ݒ�l
#define  SIO_PORT PORTB      //ISP�p�|�[�g
#define  SIO_PIN  PINB       //ISP�p�|�[�g����
#define  SIO_DDR  DDRB       //ISP�p�|�[�g���o�͐ݒ�
#define  SIO_TX   4          //���M�r�b�g
#define  SIO_RX   3          //��M�r�b�g
#define  SIO_TIFR    TIFR1   //�^�C�}1�t���O
#define  SIO_MAXCNT  0x10000 //�^�C�}�ő�l

#define SIO_TCNT    ((F_CPU/PRESCALE)/BAUDRATE)

//-----------------------------------------------------------------------
// �V���A���|�[�g������
// �E�^�C�}1���g�p���܂��B���̃v���O�����Ń^�C�}1���g�p���Ă���Ǝg���܂���B
// �EISP�R�l�N�^�̃|�[�g��ݒ肵�܂��B
void sio_init(void)
{
    SIO_DDR |=  (1<<SIO_TX);
    SIO_DDR &= ~(1<<SIO_RX);
    SIO_PORT &= ~(1<<SIO_TX);
    SIO_PORT |= (1<<SIO_RX);

    TCCR1A= 0;                // �^�C�}1 ���[�h 
    TCCR1B= PRESCALECR;       // �^�C�}1 �v���X�P�[���ݒ�
}


//-----------------------------------------------------------------------
// 1bit���E�G�C�g
void sio_bitwait(void)
{
    while(!(SIO_TIFR & (1 << TOV1)));
    // TOV1�r�b�g��1�ɂȂ�܂�

    TCNT1 = SIO_MAXCNT-SIO_TCNT;
    SIO_TIFR |= (1 << TOV1);  // TIFR�̃r�b�g���N���A
}

//-----------------------------------------------------------------------
// 1�o�C�g���M
//���� sio_txdat�F���M�f�[�^0x00-0xff
int sio_tx(char sio_txdat)
{
    unsigned char mask = 0x01;

    //�X�^�[�g�r�b�g
    SIO_PORT |= (1<<SIO_TX);
    TCNT1 = SIO_MAXCNT-SIO_TCNT;
    SIO_TIFR |= (1 << TOV1);  // TIFR�̃r�b�g���N���A
    sio_bitwait();

    //�f�[�^�r�b�g0-7
    while(mask != 0){    
        if(sio_txdat & mask)
            SIO_PORT &= ~(1<<SIO_TX);
        else
            SIO_PORT |= (1<<SIO_TX);

        mask = mask<<1;
        sio_bitwait();
    }
    //�X�g�b�v�r�b�g
    SIO_PORT &= ~(1<<SIO_TX);
    sio_bitwait();
    return(0);
}

//-----------------------------------------------------------------------
// 1�o�C�g��M
// �߂�l: ��M�f�[�^0x00�`0xFF / 0xFFFF(-1)�̏ꍇ�̓^�C���A�E�g�G���[
int sio_rx(void)
{
    #define TIMEOUT 30000
    int timeo;
    unsigned char sio_rxdat;
    unsigned char mask = 0x01;

    for(timeo=0; timeo<TIMEOUT; timeo++)
    {    
        if(SIO_PIN & (1<<SIO_RX)){  //�X�^�[�g�r�b�g���o
               TCNT1 = SIO_MAXCNT-(SIO_TCNT/2);
            SIO_TIFR |= (1 << TOV1);  // TIFR�̃r�b�g���N���A
            break;
        }
    }
    if(timeo>=TIMEOUT)return(-1);

    //�X�^�[�g�r�b�g �ǂݔ�΂�
    sio_bitwait();

    //�f�[�^�r�b�g0-7
    sio_rxdat=0;
    while(mask != 0){    
        sio_bitwait();

        if((SIO_PIN & (1<<SIO_RX))==0)
            sio_rxdat |= mask;

        mask = mask<<1;
    }

    sio_bitwait();    //�X�g�b�v�r�b�g�ǂݔ�΂�

    return((int)sio_rxdat);
}

//-----------------------------------------------------------------------
// �����񑗐M
//���� *str�F������̃|�C���^�B�I�[��NULL(0)
void sio_txstr(char *str)
{
    while(*str != 0)
    {
        sio_tx(*str++);
    }
}

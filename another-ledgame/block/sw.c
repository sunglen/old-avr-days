//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
// �X�C�b�`�p�֐�
//
// ���p���p�͋֎~�B�Ĕz�z�͎��R�ɂł��܂��B
// �������Ĕz�z����ꍇ�ɂ͂킩��₷���`�Ŗ��L���Ă��������B
// ���̕��͏C���֎~(Please do not delete a document from here.)
//---------------------------------------------------------------------
#include <avr/io.h>

#define SW_PORT  PORTD		// �v���A�b�v
#define SW_DDR   DDRD       // �|�[�g���͐ݒ�
#define SW_PIN   PIND       // ���̓|�[�g

//---------------------------------------------------------------
// �X�C�b�`������
//   ����:����������|�[�g�̃}�X�N
void sw_init(unsigned char bitmask)
{
 	SW_DDR  &= ~bitmask; 	//input
	SW_PORT |= bitmask; 	//pullup
}


//---------------------------------------------------------------
// �X�C�b�`�ǂݎ��
//   ����:���o����|�[�g�̃}�X�N
//   �߂�l�F0=�X�C�b�`�I�t / 1=�X�C�b�`�I��
char sw_get(unsigned char bitmask)
{
	if(SW_PIN & bitmask)
		return(0);  // �X�C�b�`������ĂȂ�
	else
		return(1);  // �X�C�b�`������Ă���
}


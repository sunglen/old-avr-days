//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
// �X�C�b�`�p�֐�
//
// ���p���p�͋֎~�B�Ĕz�z�͎��R�ɂł��܂��B
// �������Ĕz�z����ꍇ�ɂ͂킩��₷���`�Ŗ��L���Ă��������B
// ���̕��͏C���֎~(Please do not delete a document from here.)
//---------------------------------------------------------------------


#define SW_B	 (1<<1)
#define SW_A	 (1<<2)
#define SW_UP	 (1<<4)
#define SW_LEFT	 (1<<5)
#define SW_RIGHT (1<<6)
#define SW_DOWN	 (1<<7)
#define SW_ALL   (SW_UP|SW_DOWN|SW_LEFT|SW_RIGHT|SW_A|SW_B)

//---------------------------------------------------------------
// �X�C�b�`������
//   ����:����������|�[�g�̃}�X�N
void sw_init(unsigned char bitmask);

//---------------------------------------------------------------
// �X�C�b�`�ǂݎ��
//   ����:���o����|�[�g�̃}�X�N
//   �߂�l�F0=�X�C�b�`�I�t / 1=�X�C�b�`�I��
char sw_get(unsigned char bitmask);


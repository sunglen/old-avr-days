//(16x16Dot)LED Game for AVR  by Takuya Matsubara / NICO Corp. 2006
// http://www.nico.to/avr/
//
//   ����
//
// ���p���p�͋֎~�B�Ĕz�z�͎��R�ɂł��܂��B
// �������Ĕz�z����ꍇ�ɂ͂킩��₷���`�Ŗ��L���Ă��������B
// ���̕��͏C���֎~(Please do not delete a document from here.)
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// ����������
// �v���O�������ōŏ��Ɉ�񂾂��Ăяo���Ă��������B
// ���̊֐��̓^�C�}0���g���܂��B
// ���łɕʂ̏����Ń^�C�}0���g���Ă���ƌĂяo���܂���B
void rand_init(void);

//---------------------------------------------------------------------
// �����̎擾
// ���� max�F�����ő�͈�0�`255�B0����(max-1)�܂ł̗����𔭐������܂��B
// �߂�l�Funsigned char�^�B������Ԃ��܂��B0�`255
// �g�p��Fret = rand_get(10);   //0�`9�̗�����Ԃ��܂��B
unsigned char rand_get(unsigned char max);

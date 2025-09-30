/* DIY Keyboard1 for ATtiny2313 8MHz */
/*by takuya matsubara*/
/*https://nicotak.com*/

#include <avr/io.h>
#include <avr/pgmspace.h>

#define MASK_MODE  (1<<2)
#define MASK_TX2   (1<<3)
#define MASK_LED   (1<<4)
#define ID_DAT     (1<<5)
#define IC_CLK     (1<<6)

#define MODE_SIO 0
#define MODE_PS2 1

#define SWNUM_ALT      0    //�X�C�b�`�ԍ�
#define SWNUM_CTRL     1
#define SWNUM_SHIFT    2
#define SWNUM_CURSOR   60
#define SWNUM_NONE     0xff
                            //�L�����N�^�R�[�h
#define KB_BS        0x08     //BACK SPACE
#define KB_ENTER     0x0D     //ENTER
#define KB_ESC       0x1B     //ESC
#define KB_RUP       0x0E     //ROLLUP
#define KB_RDOWN     0x0F     //ROLLDOWN
#define KB_RIGHT     0x1C     //�J�[�\�����E�ֈړ�����     ��
#define KB_LEFT      0x1D     //�J�[�\�������ֈړ�����     ��
#define KB_UP        0x1E     //�J�[�\������ֈړ�����     ��
#define KB_DOWN      0x1F     //�J�[�\�������ֈړ�����     ��

/*charactor code*/
PROGMEM unsigned char siotable[] = {
        0,  0,   /*row=0*/
        0,  0,   /*CTRL*/
        0,  0,   /*SHIFT*/
      '1','!',
      'Q','q',
      'A','a',
      'Z','z',
        0,  0,
      '3','#',    /*row=1*/
      '2','\"',
      'E','e',
      'W','w',
      'D','d',
      'S','s',
      'C','c',
      'X','x',
      '5','%',    /*row=2*/
      '4','$',
      'T','t',
      'R','r',
      'G','g',
      'F','f',
      'B','b',
      'V','v',
      '7','\'',    /*row=3*/
      '6','&',
      'U','u',
      'Y','y',
      'J','j',
      'H','h',
      'M','m',
      'N','n',
      '9',')',    /*row=4*/
      '8','(',
      'O','o',
      'I','i',
      'L','l',
      'K','k',
      '.','>',
      ',','<',
      '-','=',    /*row=5*/
      '0','0',
      '@','`',
      'P','p',
      ':','*',
      ';','+',
      '_','_',
      '/','?',
      '^','~',    /*row=6*/
     '\\','|',
      '[','{',
    KB_BS,  0,
      ']','}',
 KB_ENTER,  0,
      ' ',  0,
        0,  0,
        0,  0,    /*row=7*/
        0,  0,
        0,  0,
        0,  0,
    KB_UP,  0,
  KB_DOWN,  0,
  KB_LEFT,  0,
 KB_RIGHT,  0
};


/*Make code*/
PROGMEM unsigned char ps2table[] = {
    0x11,    //SW00 ALT(append1)        /*row=0*/
    0x14,    //SW01 KB_CTRL
    0x12,    //SW02 KB_SHFT
    0x16,    //SW03     1
    0x15,    //SW04     Q
    0x1c,    //SW05     A
    0x1a,    //SW06     Z
    0x76,    //SW07     ESC(append2)
    0x26,    //SW08     3    /*row=1*/
    0x1e,    //SW09     2
    0x24,    //SW10     E
    0x1d,    //SW11     W
    0x23,    //SW12     D
    0x1b,    //SW13     S
    0x21,    //SW14     C
    0x22,    //SW15     X
    0x2e,    //SW16     5    /*row=2*/
    0x25,    //SW17     4
    0x2c,    //SW18     T
    0x2d,    //SW19     R
    0x34,    //SW20     G
    0x2b,    //SW21     F
    0x32,    //SW22     B
    0x2a,    //SW23     V
    0x3d,    //SW24     7    /*row=3*/
    0x36,    //SW25     6
    0x3c,    //SW26     U
    0x35,    //SW27     Y
    0x3b,    //SW28     J
    0x33,    //SW29     H
    0x3a,    //SW30     M
    0x31,    //SW31     N
    0x46,    //SW32     9    /*row=4*/
    0x3e,    //SW33     8
    0x44,    //SW34     O
    0x43,    //SW35     I
    0x4B,    //SW36     L
    0x42,    //SW37     K
    0x49,    //SW38     .
    0x41,    //SW39     ,
    0x4e,    //SW40     -    /*row=5*/
    0x45,    //SW41     0
    0x54,    //SW42     @
    0x4d,    //SW43     P
    0x52,    //SW44     :
    0x4c,    //SW45     ;
    0x51,    //SW46     _
    0x4a,    //SW47     /
    0x55,    //SW48     ^    /*row=6*/
    0x6A,    //SW49     yen |
    0x5b,    //SW50     [   {
    0x66,    //SW51    KB_BS
    0x5d,    //SW52     ]   }
    0x5a,    //SW53 KB_ENTER,
    0x29,    //SW54     space
    0x0E,    //SW55 zenkaku/hankaku (append3)
    0x0D,    //SW56     TAB(append4)    /*row=7*/
    0x83,    //SW57     F7(append5)
    0x0A,    //SW58     F8(append6)
    0x01,    //SW59     F9(append7)
    0x75,    //SW60 UP
    0x72,    //SW61 DOWN
    0x6b,    //SW62 LEFT
    0x74     //SW63 RIGHT
};

void ps2_sendbyte(unsigned char dat);
void ps2_recvbyte(void);
void timer_wait(unsigned int cnt);

char mode;

#define BAUDRATE 9600        //�{�[���[�g

#ifndef F_CPU
#define   F_CPU   8000000    //CPU�N���b�N���g�� 8MHz
#endif

#define  PRESCALE   1        //�v���X�P�[���l
#define  PRESCALECR 1        //�v���X�P�[���ݒ�l

#define  SIO_PORT PORTD      //ISP�p�|�[�g
#define  SIO_PIN  PIND       //ISP�p�|�[�g����
#define  SIO_DDR  DDRD       //ISP�p�|�[�g���o�͐ݒ�
#define  SIO_TX   1          //���M�r�b�g

//#define  SIO_TIFR    TIFR1   //�^�C�}1�t���O
#define  SIO_TIFR    TIFR   //�^�C�}1�t���O
#define  SIO_MAXCNT  0x10000 //�^�C�}�ő�l

#define TCNT_SIO    (SIO_MAXCNT-((F_CPU/PRESCALE)/BAUDRATE))
#define TCNT_1MS    (SIO_MAXCNT-((F_CPU/PRESCALE)/1000))
#define TCNT_50US   (SIO_MAXCNT-((F_CPU/PRESCALE)/(1000000/50)))
#define TCNT_10US   (SIO_MAXCNT-((F_CPU/PRESCALE)/(1000000/10)))
#define TCNT_5US    (SIO_MAXCNT-((F_CPU/PRESCALE)/(1000000/5)))
#define TCNT_600HZ  (SIO_MAXCNT-((F_CPU/PRESCALE)/600))  //BEEP 600Hz

//void sio_bitwait(unsigned char dat);
void sio_tx(unsigned char sio_txdat);


//----
// �E�G�C�g
void timer_wait(unsigned int cnt)
{
    TCNT1 = cnt;
    SIO_TIFR |= (1 << TOV1);  // TIFR�̃r�b�g���N���A

    while(!(SIO_TIFR & (1 << TOV1)));
    // TOV1�r�b�g��1�ɂȂ�܂�
}

//----
void timer_sio(void)
{
    TCNT1 = TCNT_SIO;
    SIO_TIFR |= (1 << TOV1);  // TIFR�̃r�b�g���N���A

    while(!(SIO_TIFR & (1 << TOV1)));    // TOV1�r�b�g��1�ɂȂ�܂�
}

//----
// 1�o�C�g���M
//���� sio_txdat�F���M�f�[�^0x00-0xff
void sio_tx(unsigned char sio_txdat)
{
    unsigned char bitmask = 1;

    SIO_PORT |= (1<<SIO_TX);    //0
    timer_sio();    //�X�^�[�g�r�b�g

    //�f�[�^�r�b�g0-7(���ʂ��瑗�M)
    while(bitmask){    
        if(sio_txdat & bitmask)
            SIO_PORT &= ~(1<<SIO_TX);    //1
        else
            SIO_PORT |= (1<<SIO_TX);    //0

        timer_sio();
        bitmask <<= 1;
    }
    //�X�g�b�v�r�b�g
    SIO_PORT &= ~(1<<SIO_TX);    //1
    timer_sio();
}

//----
void sio_tx2(unsigned char sio_txdat)
{
    unsigned char bitmask = 1;

    //�X�^�[�g�r�b�g
    PORTD &= ~MASK_TX2;    //0
    timer_sio();

    //�f�[�^�r�b�g0-7(���ʂ��瑗�M)
    while(bitmask){    
        if(sio_txdat & bitmask)
            PORTD |= MASK_TX2;    //1
        else
            PORTD &= ~MASK_TX2;    //0

        timer_sio();
        bitmask <<= 1;
    }
    PORTD |= MASK_TX2;    //1
    timer_sio();    //�X�g�b�v�r�b�g
}

//----
void wait_msec(int i)
{
    while(i--)
    {
        timer_wait(TCNT_1MS);
    }
}

//600Hz /2 = 300Hz
//void beep(void)
//{
//    int i=60;    // 0.1sec
//
//    while(i--)
//    {
//        PORTD ^= MASK_BEEP;
//        timer_wait(TCNT_600HZ);
//    }
//    PORTD &= ~MASK_BEEP;
//}

//   b11 b10 b09 b08 b07 b06 b05 b04 b03 b02  b01
// --+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+    +---
//   | | | | | | | | | | | | | | | | | | | | |    |
//   +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+ +----+
//
// -+   +---+---+---+---+---+---+---+---+---+------+--
//  |sta|d0 |d1 |d2 |d3 |d4 |d5 |d6 |d7 |po |stop
//  +---+---+---+---+---+---+---+---+---+---+
#define  PS2_PORT PORTD      //ISP�p�|�[�g
#define  PS2_PIN  PIND      //ISP�p�|�[�g
#define  PS2_DDR  DDRD       //ISP�p�|�[�g���o�͐ݒ�
#define  PS2_DAT   0          //�r�b�g
#define  PS2_CLK   1          //�r�b�g

//----
// PS2�R�l�N�^ 1bit���M
void ps2_sendclk(void)
{
    timer_wait(TCNT_10US);        //10uSec
    PS2_DDR |= (1<<PS2_CLK);    //output Low
    timer_wait(TCNT_50US);        //50uSec
    PS2_DDR &= ~(1<<PS2_CLK);    //output High(�n�C�C���s�[�_���X)
    timer_wait(TCNT_10US);        //10uSec
}

//----
char ps2_recvbit(void)
{
    ps2_sendclk();

    return(PS2_PIN & (1<<PS2_DAT));
}

//----
// PS2�R�l�N�^ 1bytes���M
void ps2_sendbyte(unsigned char dat)
{
    char pari;
    int i,wdat;

    if((PS2_PIN & (1<<PS2_CLK))==0)
        return;    //CLK��L�̏ꍇ�A����

    while((PS2_PIN & (1<<PS2_DAT))==0){        //DAT��L�̏ꍇ�A����(�f�[�^�Փ�)
        ps2_recvbyte();
//        return;    
    }
    wdat = (int)dat << 1;    //0�r�b�g�ڂ̓X�^�[�g�r�b�g

    pari=0;
    for(i=0; i<8; i++){
        if((1<<i) & dat){
            pari++;        //�r�b�g1�̐����J�E���g
        }
    }
    if((pari & 1)==0){
        //�r�b�g1�̐�������
        wdat |= (1<<9);    //�r�b�g9(�p���e�B)=1
    }
    wdat |= (1<<10);    //�r�b�g10(�X�g�b�v�r�b�g)=1

    for(i=0; i<11; i++){    //���ʂ���11�r�b�g���M
        if((1 << i) & wdat){
            PS2_DDR &= ~(1<<PS2_DAT);   //1(High�C���s�[�_���X)
        }else{
            PS2_DDR |= (1<<PS2_DAT);    //0
        }
        ps2_sendclk();    //send 1bit clock
        if((PS2_PIN & (1<<PS2_CLK))==0)
            break;    //CLK��L�̏ꍇ�A����
    }
    PS2_DDR &= ~(1<<PS2_DAT);    //1(High�C���s�[�_���X)
    wait_msec(2);    // 2msec wait
                    // *�@��ɂ���ă^�C�~���O���Ⴄ�\������
}

//----
// PS2�R�l�N�^ 1bytes��M
void ps2_recvbyte(void)
{
    if(mode != MODE_PS2)return;

    unsigned char mask=0x01;
    unsigned char bytedata=0x0;

    if((PS2_PIN & (1<<PS2_CLK))==0)
        return;        //CLK=Low�̏ꍇ�A����

    if((PS2_PIN & (1<<PS2_DAT))!=0)
        return;        //DAT��High�̏ꍇ�A����(�f�[�^�Ȃ�)

    ps2_recvbit();    //startbit

    while(mask != 0)
    {    //���ʂ���8�r�b�g��M
        if(ps2_recvbit())
            bytedata |= mask;

        mask <<= 1;
    }
    ps2_recvbit();    //paritybit

    if(ps2_recvbit()){    //Stop bit��High�̏ꍇ�A��M����
        PS2_DDR |= (1<<PS2_DAT);    //DAT = Low
        ps2_sendclk();    //send 1bit clock
        PS2_DDR &= ~(1<<PS2_DAT);    //1(High�C���s�[�_���X)
    }else{
        return;    //Stop bit��Low�̏ꍇ�A�t���[�~���O�G���[
    }
    wait_msec(2);    //2msec
                    // *�@��ɂ���ă^�C�~���O���Ⴄ�\������

    //��M�f�[�^��ǂƕԐM
    if(bytedata==0xff){        //Reset�R�}���h
        ps2_sendbyte(0xFA);        //Ack����
        //(300-500ms)
        wait_msec(600);       //600msec
        ps2_sendbyte(0xAA);        //BAT����
    }else if(bytedata==0xf2){      //ID�ǂݏo���R�}���h
        ps2_sendbyte(0xFA);        //Ack����
        ps2_sendbyte(0xab);        //�L�[�{�[�hID
        ps2_sendbyte(0x83);        //�L�[�{�[�hID
//    }else if(bytedata==0xED){    //���[�h�Z�b�g�R�}���h
    }else if(bytedata==0xEE){      //�G�R�[�R�}���h
        ps2_sendbyte(0xEE);        //����
    }else{    //�C�l�[�u���R�}���h
/*    }else if(bytedata==0xF4){*/    //�C�l�[�u���R�}���h
        ps2_sendbyte(0xfA);        //Ack����
    }
}


#define FLAG_MAKECODE 0
#define FLAG_BREAKCODE 1

//----
void ps2_sendcode(char swnum,char flag)
{
    if(mode != MODE_PS2)return;
    // PS/2 mode

    PGM_P p = (PGM_P)ps2table;

    if(swnum >= SWNUM_CURSOR)
        ps2_sendbyte(0xE0);        /*code(�J�[�\���L�[)*/

    if(flag == FLAG_BREAKCODE)
        ps2_sendbyte(0xF0);        /*break code*/

    ps2_sendbyte(pgm_read_byte(p+swnum));        /*code*/
}

//----
int main(void)
{
    #define TIMEOFLOOP 8      //����/���[�v[ms]
    #define REPEATMAX 1200    //Key Repeat start[ms]
    #define REPEATCYCLE 200   //Key Repeat cycle[ms]

    unsigned char swflag[64/8];   //�r�b�g��1���ƃX�C�b�`ON
    unsigned char swbak[64/8];    //�r�b�g��1���ƃX�C�b�`ON
    unsigned char bitmask;
    unsigned char swnow;
    unsigned char swxor;
    char rownum;//�s
    char swnum;
    int repeatcnt=0;

    PORTD |= MASK_MODE;    //Pullup
    DDRD &= ~MASK_MODE;
    PORTB = 0xff;          //Pullup
    DDRD |= (MASK_LED | ID_DAT | IC_CLK | MASK_TX2);
    PORTD |= (MASK_LED | ID_DAT | MASK_TX2);    //

    if(PIND & MASK_MODE){    //Mode sw=OFF
        // �V���A���|�[�g������
        SIO_DDR |=  (1<<SIO_TX);
        SIO_PORT &= ~(1<<SIO_TX);
        mode = MODE_SIO;
    }else{                //Mode sw=ON
        PS2_PORT &= ~((1<<PS2_DAT)|(1<<PS2_CLK));   //�v���A�b�v�Ȃ�/Low�o��
        PS2_DDR &= ~((1<<PS2_DAT)|(1<<PS2_CLK));    //1(High�C���s�[�_���X)
        mode = MODE_PS2;
    }

    TCCR1A= 0;                // �^�C�}1 ���[�h 
    TCCR1B= PRESCALECR;       // �^�C�}1 �v���X�P�[���ݒ�

    //���W�b�NIC������ / backup������
    for(rownum=0; rownum<8; rownum++){
        PORTD |=  IC_CLK;    //(74HC164)CLK
        PORTD &= ~IC_CLK;    //(74HC164)CLK

        swbak[(int)rownum] = 0xFF;
        swflag[(int)rownum] = 0;
    }
    wait_msec(500);    //��500msec
    PORTD &= ~MASK_LED;

    rownum = 0;
    swnum = 0;
    while(1)
    {
        ps2_recvbyte();

        //1�s�`8�s�̂���1��Low
        if(rownum==0){
            PORTD &= ~ID_DAT;   //(74HC164)DAT
        }else{
            PORTD |= ID_DAT;    //(74HC164)DAT
        }
        timer_wait(TCNT_50US);        //50uSec
        PORTD |=  IC_CLK;    //(74HC164)CLK
        PORTD &= ~IC_CLK;    //(74HC164)CLK
 //       timer_wait(TCNT_50US);        //50uSec
        timer_wait(TCNT_1MS);        //1mSec

        swnow = PINB;
        swxor = swbak[(int)rownum] ^ swnow;
        swbak[(int)rownum] = swnow;    //back up sw_data

        swnow &= ~swxor;
        //     ON-->OFF �܂��� OFF-->ON�ւ̕ω�����OFF�ɔ���

        bitmask = 0x80;
        while(bitmask){
            if(swflag[(int)rownum] & bitmask){        //sw ON
                if((swnow & bitmask)==0){    //ON-->ON  repeat
                    if(swnum > SWNUM_SHIFT){
                        repeatcnt+=TIMEOFLOOP;    //Alt/Shift/Ctrl�ȊO�̓��s�[�g
                        if(repeatcnt > REPEATMAX){
                            swflag[(int)rownum] &= ~bitmask;
                        }
                    }
                }else{    //ON-->OFF
                    PORTD &= ~MASK_LED;
                    ps2_sendcode(swnum,FLAG_BREAKCODE);    // Break Code
                    swflag[(int)rownum] &= ~bitmask;
                    repeatcnt=0;
                }
            }else{    //sw OFF
                if((swnow & bitmask)==0){    //OFF-->ON
                    unsigned char chrcode;
                    unsigned char offset;

                    PORTD |= MASK_LED;
                    offset = (swnum<<1);
                    if(swflag[0] &(0x80>>SWNUM_SHIFT)){
                        offset++;
                    }
                    chrcode = pgm_read_byte((PGM_P)siotable + offset);

                    if(mode==MODE_SIO){    //SIO(RS232C) mode
                        if(chrcode)
                            sio_tx(chrcode);
                    }else{                // PS/2 mode
                        ps2_sendcode(swnum,FLAG_MAKECODE);        // Make code
                    }
                    if(chrcode)
                        sio_tx2(chrcode);

                    swflag[(int)rownum] |= bitmask;
                    if(    repeatcnt > REPEATMAX)
                        repeatcnt = (REPEATMAX-REPEATCYCLE);
                    else    
                        repeatcnt = 0;
                }else{
                    //OFF-->OFF���ƂȂɂ�����
                }
            }
            bitmask >>= 1;
            swnum++;
        }

        rownum++;
        if(swnum >= 64){    //8x8=64sw
            rownum = 0;
            swnum = 0;
        }
    }
    return(0);
}

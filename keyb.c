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

#define SWNUM_ALT      0    //スイッチ番号
#define SWNUM_CTRL     1
#define SWNUM_SHIFT    2
#define SWNUM_CURSOR   60
#define SWNUM_NONE     0xff
                            //キャラクタコード
#define KB_BS        0x08     //BACK SPACE
#define KB_ENTER     0x0D     //ENTER
#define KB_ESC       0x1B     //ESC
#define KB_RUP       0x0E     //ROLLUP
#define KB_RDOWN     0x0F     //ROLLDOWN
#define KB_RIGHT     0x1C     //カーソルを右へ移動する     →
#define KB_LEFT      0x1D     //カーソルを左へ移動する     ←
#define KB_UP        0x1E     //カーソルを上へ移動する     ↑
#define KB_DOWN      0x1F     //カーソルを下へ移動する     ↓

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

#define BAUDRATE 9600        //ボーレート

#ifndef F_CPU
#define   F_CPU   8000000    //CPUクロック周波数 8MHz
#endif

#define  PRESCALE   1        //プリスケーラ値
#define  PRESCALECR 1        //プリスケーラ設定値

#define  SIO_PORT PORTD      //ISP用ポート
#define  SIO_PIN  PIND       //ISP用ポート入力
#define  SIO_DDR  DDRD       //ISP用ポート入出力設定
#define  SIO_TX   1          //送信ビット

//#define  SIO_TIFR    TIFR1   //タイマ1フラグ
#define  SIO_TIFR    TIFR   //タイマ1フラグ
#define  SIO_MAXCNT  0x10000 //タイマ最大値

#define TCNT_SIO    (SIO_MAXCNT-((F_CPU/PRESCALE)/BAUDRATE))
#define TCNT_1MS    (SIO_MAXCNT-((F_CPU/PRESCALE)/1000))
#define TCNT_50US   (SIO_MAXCNT-((F_CPU/PRESCALE)/(1000000/50)))
#define TCNT_10US   (SIO_MAXCNT-((F_CPU/PRESCALE)/(1000000/10)))
#define TCNT_5US    (SIO_MAXCNT-((F_CPU/PRESCALE)/(1000000/5)))
#define TCNT_600HZ  (SIO_MAXCNT-((F_CPU/PRESCALE)/600))  //BEEP 600Hz

//void sio_bitwait(unsigned char dat);
void sio_tx(unsigned char sio_txdat);


//----
// ウエイト
void timer_wait(unsigned int cnt)
{
    TCNT1 = cnt;
    SIO_TIFR |= (1 << TOV1);  // TIFRのビットをクリア

    while(!(SIO_TIFR & (1 << TOV1)));
    // TOV1ビットが1になるまで
}

//----
void timer_sio(void)
{
    TCNT1 = TCNT_SIO;
    SIO_TIFR |= (1 << TOV1);  // TIFRのビットをクリア

    while(!(SIO_TIFR & (1 << TOV1)));    // TOV1ビットが1になるまで
}

//----
// 1バイト送信
//引数 sio_txdat：送信データ0x00-0xff
void sio_tx(unsigned char sio_txdat)
{
    unsigned char bitmask = 1;

    SIO_PORT |= (1<<SIO_TX);    //0
    timer_sio();    //スタートビット

    //データビット0-7(下位から送信)
    while(bitmask){    
        if(sio_txdat & bitmask)
            SIO_PORT &= ~(1<<SIO_TX);    //1
        else
            SIO_PORT |= (1<<SIO_TX);    //0

        timer_sio();
        bitmask <<= 1;
    }
    //ストップビット
    SIO_PORT &= ~(1<<SIO_TX);    //1
    timer_sio();
}

//----
void sio_tx2(unsigned char sio_txdat)
{
    unsigned char bitmask = 1;

    //スタートビット
    PORTD &= ~MASK_TX2;    //0
    timer_sio();

    //データビット0-7(下位から送信)
    while(bitmask){    
        if(sio_txdat & bitmask)
            PORTD |= MASK_TX2;    //1
        else
            PORTD &= ~MASK_TX2;    //0

        timer_sio();
        bitmask <<= 1;
    }
    PORTD |= MASK_TX2;    //1
    timer_sio();    //ストップビット
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
#define  PS2_PORT PORTD      //ISP用ポート
#define  PS2_PIN  PIND      //ISP用ポート
#define  PS2_DDR  DDRD       //ISP用ポート入出力設定
#define  PS2_DAT   0          //ビット
#define  PS2_CLK   1          //ビット

//----
// PS2コネクタ 1bit送信
void ps2_sendclk(void)
{
    timer_wait(TCNT_10US);        //10uSec
    PS2_DDR |= (1<<PS2_CLK);    //output Low
    timer_wait(TCNT_50US);        //50uSec
    PS2_DDR &= ~(1<<PS2_CLK);    //output High(ハイインピーダンス)
    timer_wait(TCNT_10US);        //10uSec
}

//----
char ps2_recvbit(void)
{
    ps2_sendclk();

    return(PS2_PIN & (1<<PS2_DAT));
}

//----
// PS2コネクタ 1bytes送信
void ps2_sendbyte(unsigned char dat)
{
    char pari;
    int i,wdat;

    if((PS2_PIN & (1<<PS2_CLK))==0)
        return;    //CLKがLの場合、無効

    while((PS2_PIN & (1<<PS2_DAT))==0){        //DATがLの場合、無効(データ衝突)
        ps2_recvbyte();
//        return;    
    }
    wdat = (int)dat << 1;    //0ビット目はスタートビット

    pari=0;
    for(i=0; i<8; i++){
        if((1<<i) & dat){
            pari++;        //ビット1の数をカウント
        }
    }
    if((pari & 1)==0){
        //ビット1の数が偶数
        wdat |= (1<<9);    //ビット9(パリティ)=1
    }
    wdat |= (1<<10);    //ビット10(ストップビット)=1

    for(i=0; i<11; i++){    //下位から11ビット送信
        if((1 << i) & wdat){
            PS2_DDR &= ~(1<<PS2_DAT);   //1(Highインピーダンス)
        }else{
            PS2_DDR |= (1<<PS2_DAT);    //0
        }
        ps2_sendclk();    //send 1bit clock
        if((PS2_PIN & (1<<PS2_CLK))==0)
            break;    //CLKがLの場合、無効
    }
    PS2_DDR &= ~(1<<PS2_DAT);    //1(Highインピーダンス)
    wait_msec(2);    // 2msec wait
                    // *機種によってタイミングが違う可能性あり
}

//----
// PS2コネクタ 1bytes受信
void ps2_recvbyte(void)
{
    if(mode != MODE_PS2)return;

    unsigned char mask=0x01;
    unsigned char bytedata=0x0;

    if((PS2_PIN & (1<<PS2_CLK))==0)
        return;        //CLK=Lowの場合、無効

    if((PS2_PIN & (1<<PS2_DAT))!=0)
        return;        //DATがHighの場合、無効(データなし)

    ps2_recvbit();    //startbit

    while(mask != 0)
    {    //下位から8ビット受信
        if(ps2_recvbit())
            bytedata |= mask;

        mask <<= 1;
    }
    ps2_recvbit();    //paritybit

    if(ps2_recvbit()){    //Stop bitがHighの場合、受信成功
        PS2_DDR |= (1<<PS2_DAT);    //DAT = Low
        ps2_sendclk();    //send 1bit clock
        PS2_DDR &= ~(1<<PS2_DAT);    //1(Highインピーダンス)
    }else{
        return;    //Stop bitがLowの場合、フレーミングエラー
    }
    wait_msec(2);    //2msec
                    // *機種によってタイミングが違う可能性あり

    //受信データ解読と返信
    if(bytedata==0xff){        //Resetコマンド
        ps2_sendbyte(0xFA);        //Ack応答
        //(300-500ms)
        wait_msec(600);       //600msec
        ps2_sendbyte(0xAA);        //BAT完了
    }else if(bytedata==0xf2){      //ID読み出しコマンド
        ps2_sendbyte(0xFA);        //Ack応答
        ps2_sendbyte(0xab);        //キーボードID
        ps2_sendbyte(0x83);        //キーボードID
//    }else if(bytedata==0xED){    //モードセットコマンド
    }else if(bytedata==0xEE){      //エコーコマンド
        ps2_sendbyte(0xEE);        //応答
    }else{    //イネーブルコマンド
/*    }else if(bytedata==0xF4){*/    //イネーブルコマンド
        ps2_sendbyte(0xfA);        //Ack応答
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
        ps2_sendbyte(0xE0);        /*code(カーソルキー)*/

    if(flag == FLAG_BREAKCODE)
        ps2_sendbyte(0xF0);        /*break code*/

    ps2_sendbyte(pgm_read_byte(p+swnum));        /*code*/
}

//----
int main(void)
{
    #define TIMEOFLOOP 8      //時間/ループ[ms]
    #define REPEATMAX 1200    //Key Repeat start[ms]
    #define REPEATCYCLE 200   //Key Repeat cycle[ms]

    unsigned char swflag[64/8];   //ビットが1だとスイッチON
    unsigned char swbak[64/8];    //ビットが1だとスイッチON
    unsigned char bitmask;
    unsigned char swnow;
    unsigned char swxor;
    char rownum;//行
    char swnum;
    int repeatcnt=0;

    PORTD |= MASK_MODE;    //Pullup
    DDRD &= ~MASK_MODE;
    PORTB = 0xff;          //Pullup
    DDRD |= (MASK_LED | ID_DAT | IC_CLK | MASK_TX2);
    PORTD |= (MASK_LED | ID_DAT | MASK_TX2);    //

    if(PIND & MASK_MODE){    //Mode sw=OFF
        // シリアルポート初期化
        SIO_DDR |=  (1<<SIO_TX);
        SIO_PORT &= ~(1<<SIO_TX);
        mode = MODE_SIO;
    }else{                //Mode sw=ON
        PS2_PORT &= ~((1<<PS2_DAT)|(1<<PS2_CLK));   //プルアップなし/Low出力
        PS2_DDR &= ~((1<<PS2_DAT)|(1<<PS2_CLK));    //1(Highインピーダンス)
        mode = MODE_PS2;
    }

    TCCR1A= 0;                // タイマ1 モード 
    TCCR1B= PRESCALECR;       // タイマ1 プリスケーラ設定

    //ロジックIC初期化 / backup初期化
    for(rownum=0; rownum<8; rownum++){
        PORTD |=  IC_CLK;    //(74HC164)CLK
        PORTD &= ~IC_CLK;    //(74HC164)CLK

        swbak[(int)rownum] = 0xFF;
        swflag[(int)rownum] = 0;
    }
    wait_msec(500);    //約500msec
    PORTD &= ~MASK_LED;

    rownum = 0;
    swnum = 0;
    while(1)
    {
        ps2_recvbyte();

        //1行～8行のうち1つをLow
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
        //     ON-->OFF または OFF-->ONへの変化時はOFFに判定

        bitmask = 0x80;
        while(bitmask){
            if(swflag[(int)rownum] & bitmask){        //sw ON
                if((swnow & bitmask)==0){    //ON-->ON  repeat
                    if(swnum > SWNUM_SHIFT){
                        repeatcnt+=TIMEOFLOOP;    //Alt/Shift/Ctrl以外はリピート
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
                    //OFF-->OFFだとなにもせず
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

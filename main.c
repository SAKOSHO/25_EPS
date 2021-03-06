/* 
 * File:   main.c
 * Author: ²XØãÄ
 *
 * Created on 2021/06/29, 17:09
 * Discription@À±Ìd¹nÌvO
 * ÒWð
 * 2021/10/18F^C}[ÌÀ
 * 
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "MCP2515.h"
#include "EPS.h"

// CONFIG1
#pragma config FOSC  = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE  = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP    = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD   = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO  = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP   = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR21V   // Brown-out Reset Selection bit (Brown-out Reset set to 2.1V)
#pragma config WRT   = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ      20000000
#define _CAN_BAUDRATE   2


/*--Prtotype--*/
void CONFIG();                                                                  //úÝè
void timer_init();                                                             //^C}[ÌúÝè
void interrupt timer();                                                         //^C}[ÌèÝ
char BtoD(char data);                                                           //2ið10iÉÏ·

/*--Grobal Variables--*/
volatile char *rx_data;                                                         //óMf[^Ìó¯üêû
volatile char *fram_data;                                                       //frampÌ 
volatile char rx_status;                                                        //óMXe[^XpÌ 
volatile char rx_int;                                                           //óMtOmFpÌ 
volatile char data[8] = {2, 2, 2, 2, 2, 2, 2, 2};                               //HKf[^
volatile char mode = _ChargeMode;                                               //[htOiú[hF[d[hj
volatile char size;                                                             //f[^·
volatile int cnt = 0;                                                           //RyA}b`pÌJE^0
volatile int cnt1 = 0;                                                           //RyA}b`pÌJE^1


void main(void)
{ 
    CONFIG();                                                                   //úÝè
    __delay_ms(100);
    
    T1CONbits.TMR1ON = 1;                                                       //^C}[Jn
    
    while(1)
    {
        /* [hJÚ */
        rx_int = Read(_CANINTF);                                                //èÝtO
        
        if((rx_int & _Flagbit0) == 0b00000001)
        {   
            rx_data = Read_RX_ID(_F_RXB0SIDH, 13);
            Write(_CANINTF, 0b00000000);
            
           if(rx_data[3] == SIDH_MODE)                                         //[hîñ©ðmF
            {
                if((rx_data[5] & _ChargeMode) == 0b00000001)                    //[d[hÉÚs
                {
                    RB5 = 0;
                    RB6 = 0;
                    RB7 = 1;
                    mode = _ChargeMode;
                }
            
                if((rx_data[5] & _COMMMode) == 0b00000010)                      //_EN[hÉÚs
                {
                    RB5 = 0;
                    RB6 = 1;
                    RB7 = 0;
                    mode = _COMMMode;
                }
            
                if((rx_data[5] & _StanbyMode) == 0b00000011)                    //Ò@[hÉÚs
                {
                    RB5 = 1;
                    RB6 = 0; 
                    RB7 = 0;
                    mode = _StanbyMode;
                }
            
                if((rx_data[5] & _MissionMode) == 0b00000100)                   //~bV[hÉÚs
                {
                    RB5 = 0;
                    RB6 = 0; 
                    RB7 = 0;
                    mode = _MissionMode;
                }
            }
        }
        
        
        /* [d[h */
        if(mode == _ChargeMode)
        {
            if(cnt >= 10)
            {
                Write(_TXB0DLC , 0b00000001);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_MODE, SIDL_R, 0, 0);                   //[htOÇÝoµ
                Load_TX_Data(_F_TXB0D0, 1, 0);                                      //ÇÝoµ¾©çC1byteÌóf[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00000001);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_MODE, SIDL_W, EID8_MODE, EID0_MODE);   //[h«ÝIDÌÝè
                Load_TX_Data(_F_TXB0D0, 1, &mode);                                  //[h«Ý
                RTS0(_CAN_BAUDRATE);                                                //Mv


                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA1, SIDL_W, EID8_DATA1, EID0_DATA1);//obe[d³l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv


                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA2, SIDL_W, EID8_DATA2, EID0_DATA2);//obe[d¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA3, SIDL_W, EID8_DATA3, EID0_DATA3); //framÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA4, SIDL_W, EID8_DATA4, EID0_DATA4);//p¨èZTÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA5, SIDL_W, EID8_DATA5, EID0_DATA5);//p¨§äAN`G[^Ìd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA6, SIDL_W, EID8_DATA6, EID0_DATA6);//ÊM@íÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA7, SIDL_W, EID8_DATA7, EID0_DATA7);//ÊM@íÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv
                
                cnt = 0;
            }
        }
        
        /* ÊM[h */
        if(mode == _COMMMode)
        {
            if(cnt >= 10)
            {
                Write(_TXB0DLC , 0b00000001);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_MODE, SIDL_R, 0, 0);                   //[htOÇÝoµ
                Load_TX_Data(_F_TXB0D0, 1, 0);                                      //ÇÝoµ¾©çC1byteÌóf[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00000001);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_MODE, SIDL_W, EID8_MODE, EID0_MODE);   //[h«ÝIDÌÝè
                Load_TX_Data(_F_TXB0D0, 1, &mode);                                  //[h«Ý
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA1, SIDL_W, EID8_DATA1, EID0_DATA1);//obe[d³l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA2, SIDL_W, EID8_DATA2, EID0_DATA2);//obe[d¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA3, SIDL_W, EID8_DATA3, EID0_DATA3); //framÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA4, SIDL_W, EID8_DATA4, EID0_DATA4);//p¨èZTÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA5, SIDL_W, EID8_DATA5, EID0_DATA5);//p¨§äAN`G[^Ìd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA6, SIDL_W, EID8_DATA6, EID0_DATA6);//ÊM@íÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA7, SIDL_W, EID8_DATA7, EID0_DATA7);//ÊM@íÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv
                
                cnt = 0;
            }
        }
        
        /* Ò@[h */
        if(mode == _StanbyMode)
        {
            if(cnt >= 10)
            {
                Write(_TXB0DLC , 0b00000001);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_MODE, SIDL_R, 0, 0);                   //[htOÇÝoµ
                Load_TX_Data(_F_TXB0D0, 1, 0);                                      //ÇÝoµ¾©çC1byteÌóf[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00000001);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_MODE, SIDL_W, EID8_MODE, EID0_MODE);   //[h«ÝIDÌÝè
                Load_TX_Data(_F_TXB0D0, 1, &mode);                                  //[h«Ý
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA1, SIDL_W, EID8_DATA1, EID0_DATA1);//obe[d³l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv


                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA2, SIDL_W, EID8_DATA2, EID0_DATA2);//obe[d¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA3, SIDL_W, EID8_DATA3, EID0_DATA3); //framÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA4, SIDL_W, EID8_DATA4, EID0_DATA4);//p¨èZTÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA5, SIDL_W, EID8_DATA5, EID0_DATA5);//p¨§äAN`G[^Ìd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA6, SIDL_W, EID8_DATA6, EID0_DATA6);//ÊM@íÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA7, SIDL_W, EID8_DATA7, EID0_DATA7);//ÊM@íÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv
                
                cnt = 0;
            }
        }
        
        /* ~bV[h */
        if(mode == _MissionMode)
        {
            if(cnt >= 10)
            {
                Write(_TXB0DLC , 0b00000001);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_MODE, SIDL_R, 0, 0);                   //[htOÇÝoµ
                Load_TX_Data(_F_TXB0D0, 1, 0);                                      //ÇÝoµ¾©çC1byteÌóf[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00000001);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_MODE, SIDL_W, EID8_MODE, EID0_MODE);   //[h«ÝIDÌÝè
                Load_TX_Data(_F_TXB0D0, 1, &mode);                                  //[h«Ý
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA1, SIDL_W, EID8_DATA1, EID0_DATA1);//obe[d³l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA2, SIDL_W, EID8_DATA2, EID0_DATA2);//obe[d¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA3, SIDL_W, EID8_DATA3, EID0_DATA3); //framÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA4, SIDL_W, EID8_DATA4, EID0_DATA4);//p¨èZTÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA5, SIDL_W, EID8_DATA5, EID0_DATA5);//p¨§äAN`G[^Ìd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA6, SIDL_W, EID8_DATA6, EID0_DATA6);//ÊM@íÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv

                Write(_TXB0DLC , 0b00001000);                                       //bZ[WTCY1byte
                Load_TX_ID(_F_TXB0SIDH, SIDH_DATA7, SIDL_W, EID8_DATA7, EID0_DATA7);//ÊM@íÌd¬l«Ý
                Load_TX_Data(_F_TXB0D0, 8, data);                                   //f[^
                RTS0(_CAN_BAUDRATE);                                                //Mv
                
                cnt = 0;
            }
        }
    }
}

void CONFIG()
{
    OSCCON = 0b01101000;
    ANSEL  = 0b00000000;
    ANSELH = 0b00000000;
    TRISB  = 0b00000000;
    TRISC  = 0b00000000;
    PORTB  = 0b00000000;
    PORTC  = 0b00000000;
    
    INTCON = 0b11000000;                                                        //èÝÝè
    
    spi_init();
    timer_init();
    __delay_ms(100);
    
    MCP2515_init(_CAN_BAUDRATE);                                                //Æè ¦¸C®ìµÄ¢é2Éµ½DðÍÜ¾
    Write(_TXB0DLC , 0b00000001);                                               //bZ[WTCY8byte
    Write(_RXM0SIDH, 0b11111111);                                               //}XNÝèãÊSêv
    Write(_RXM0SIDL, 0b11111111);                                               //}XNÝèºÊSêv
    Write(_RXM0EID8, 0b11111111);                                               //}XNÝèg£ãÊSêv
    Write(_RXM0EID0, 0b11110000);                                               //}XNÝèg£ºÊãÊ4bitêv
    Write(_RXF0SIDH, 0b00000000);                                               //óMtB^ãÊrbgÌÝè
    Write(_RXF0SIDL, 0b00001000);                                               //óMtB^ºÊrbgÌÝè
    Write(_RXF0EID8, 0b00000000);                                               //óMtB^g£ãÊrbgÌÝè
    Write(_RXF0EID0, Sub_Filt);                                                 //óMtB^g£ºÊrbgÌÝè
    MCP2515_Open(0);                                                            //Æè ¦¸C0Éµ½DðÍÜ¾
}

void timer_init()
{
    T1CON = 0b00110000;
    TMR1H = 0b00000000;
    TMR1L = 0b00000000;
    PIE1bits.TMR1IE = 1;
    PIR1bits.TMR1IF = 0;
}

void interrupt timer()
{
    PIR1bits.TMR1IF = 0;
    TMR1H = 0b00000000;
    TMR1L = 0b00000000;
    
    cnt++;
}

char BtoD(char data)
{
    char  binary;
    char decimal = 0;
    char bas = 1;
            
    binary = data & 0b00001111;
    
    while(binary>0)
    {
        decimal = decimal + (binary % 10) * bas;
        binary = binary / 10;
        bas = bas * 2;
    }
    
    return decimal;
}

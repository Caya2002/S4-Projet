#include "IO.h"
#include <xc.h>
#include "config.h"

void init_IO(void)
{
    TRISBbits.TRISB4 = 1; // Configuration du micro en input
    ANSELBbits.ANSB4 = 1; // Configuration du micro en analog
    TRISBbits.TRISB14 = 0; // Configuration du speaker en output
    ANSELBbits.ANSB14 = 0; // Configuration du speaker en digital
    TRISBbits.TRISB0 = 1; // RB0 (BTNU) configurer en input
    ANSELBbits.ANSB0 = 0; // Disable le analog
    
    TRISBbits.TRISB1 = 1; // BTNL configurer en input
    ANSELBbits.ANSB1 = 0; // Disable le analog
    TRISBbits.TRISB8 = 1; // BTNR configurer en input
    ANSELBbits.ANSB8 = 0; // Disable le analog
    TRISFbits.TRISF0 = 1; // BTND configurer en input    
    TRISFbits.TRISF3 = 1; // SW0 configurer en input
    //TRISA &= 0xf00; // Mettre les leds en output
    TRISACLR = 0x00FF;
    LATA &= 0xf00; // Forcer les LEds(0@7) a 0
    //LATA |= 0x001;
    TRISFbits.TRISF3 = 1; //SW0 Input
    TRISBbits.TRISB9 = 1; //SW7 Input
    ANSELBbits.ANSB9 = 0;
    
    
    
//    TRISFbits.TRISF12 = 0;
//    TRISFbits.TRISF13 = 0;
//    TRISFbits.TRISF8 = 0;
//    TRISFbits.TRISF7 = 0;
//    TRISFbits.TRISF6 = 0;
    
    
}

void init_OC1(void)
{
    OC1RS = 0;
    OC1CONbits.ON = 0;
    tris_A_OUT = 0; // output
    ANSELBbits.ANSB14 = 0; // disable analog -> digital
    RPB14R = 0x0C; // connecte le OC1 au module audio
    OC1CONbits.OCTSEL = 0;
    OC1CONbits.OCM = 6;
    OC1CONbits.ON = 1;
}
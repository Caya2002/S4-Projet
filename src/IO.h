/* 
 * File:   IO.h
 * Author: mabea
 *
 * Created on February 23, 2025, 2:03 PM
 */


#ifndef IO_H
#define	IO_H

#ifdef __cplusplus
extern "C" {
#endif
    
#define BtnD PORTAbits.RA15    
#define BtnU PORTBbits.RB1
#define BtnR PORTBbits.RB8
#define BtnL PORTBbits.RB0
#define BtnC PORTFbits.RF0
#define SW0  PORTFbits.RF3
#define MODE10BITS  PORTBbits.RB9

void init_IO(void);
void init_OC1(void);

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* IO_H */


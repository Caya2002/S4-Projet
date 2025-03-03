/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    main.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "main.h"
#include "system_config.h"
#include "system/common/sys_module.h"   // SYS function prototypes
#include "driver/spi/src/dynamic/drv_spi_internal.h"
#include "UDP_app.h"
#include "led.h"
#include "ssd.h"
#include "accel.h"
#include "lcd.h"
#include "app_commands.h"






//Moyenne est faite direct sur la MX3 (GestionMoyenne dans accel.c)
//La switch qui fait afficher Moyenne sur le LCD dans accel.C



// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the TCPIP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
 */

MAIN_DATA machine;
CALIBRATION_DATA calibration;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

int Intense[3];
int Last_Intense[3];
unsigned short timer = 0;

float baseline_power;

/* Application's LED Task Function 
 Fonction qui fait clignoter une LED la LED1 à chaque 20000 execution du code
 */
static unsigned long int counter=0;
static void LedTask(void) 
{
    if(counter++ == 200){
        LED_ToggleValue(1);
        counter = 0;
    }  
}

void Interupt_ACL_Init(void)
{
    IFS0bits.INT4IF = 0;
    IEC0bits.INT4IE = 1;
    IPC4bits.INT4IP = 1;
    IPC4bits.INT4IS = 0;
    INTCONbits.INT4EP = 0;
    INT4Rbits.INT4R = 12;    //assigner le Interupt au boutton C en mettant 4, quand ca va être ok mettre 12
}

static bool sw0_old; 
void ManageSwitches()
{
    bool sw0_new = SWITCH0StateGet();
    if((sw0_new != sw0_old) && sw0_new)
    {
        //strcpy(UDP_Send_Buffer, "Bonjour S4\n\r");
        //UDP_bytes_to_send = strlen(UDP_Send_Buffer);
        UDP_Send_Packet = true;       
    }

    sw0_old = sw0_new; 
}

void RGB_Task()
{
    //if(timer_1m) {               // Interruption à chaque 1 ms
        //timer_1m = 0;            // Reset the compteur to capture the next event
        //Toute pour la Moyenne fait directement dans la MX3 avec la fonction GestionMoyenne dans accel.c
        Intense[0] = (MoyenneX*255)/2096;
        Intense[1] = (MoyenneY*255)/2096;
        Intense[2] = (MoyenneZ*255)/2096;

        if(Intense[0] <= 0)
        {
            Intense[0] = Last_Intense[0];
        }
        else
        {
          Last_Intense[0] = Intense[0];  
        }

        if(Intense[1] <= 0)
        {
            Intense[1] = Last_Intense[1];
        }
        else
        {
          Last_Intense[0] = Intense[0];  
        }

        if(Intense[2] <= 0)
        {
            Intense[2] = Last_Intense[2];
        }
        else
        {
          Last_Intense[0] = Intense[0];  
        }

        RGBLED_SetValue(Intense[0], Intense[1], Intense[2]); 
    //}
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void MAIN_Initialize ( void )

  Remarks:
    See prototype in main.h.
 */

void MAIN_Initialize ( void )
{
     
    /* Place the App state machine in its initial state. */
    machine.state = INIT;
    machine.previous_state = INIT;
    calibration.state = HORS_CALIBRATION;
    calibration.previous_state = HORS_CALIBRATION;
            

    machine.handleUSART0 = DRV_HANDLE_INVALID;

    UDP_Initialize(); // Initialisation de du serveur et client UDP
    LCD_Init(); // Initialisation de l'écran LCD
    ACL_Init(); // Initialisation de l'accéléromètre
    SSD_Init(); // Initialisation du Timer4 et de l'accéléromètre
    //Interupt_ACL_Init(); //Initialisation de l'interuption de l'accéléromètre
    RGBLED_Init();
    //Init_GestionDonnees();
    //initialize_timer_interrupt();
    //macro_enable_interrupts();

    
   
}


/******************************************************************************
  Function:
    void MAIN_Tasks ( void )
 * Fonction qui execute les tâches de l'application. Cette fonction est une
 * machine d'état :
 * 1. MAIN_STATE_INIT; Initialise les périphérique de communication USART et 
 *    passe à l'état 2 quand l'initialisation est terminée.
 * 2. MAIN_STATE_SERVICE_TASKS; Execute les tâches de l'application. Ne change 
 * jamais d'état.

  Remarks:
    See prototype in main.h.
 */

void MAIN_Tasks ( void )
{
    /* Check the application's current state. */
    switch (machine.state)
    {
            /* Application's initial state. */
        case INIT:
            baseline_power = 0;
            LCD_WriteStringAtPos("INITIALISATION  ", 0, 0);
            SSD_WriteDigitsGrouped(0x8888,0x01);
        {
            bool appInitialized = true;
            SYS_CONSOLE_MESSAGE("Init\r\n");
            
            if (machine.handleUSART0 == DRV_HANDLE_INVALID)
            {
                machine.handleUSART0 = DRV_USART_Open(MAIN_DRV_USART, DRV_IO_INTENT_READWRITE|DRV_IO_INTENT_NONBLOCKING);
                appInitialized &= (DRV_HANDLE_INVALID != machine.handleUSART0);
            }


            if (appInitialized)
            {
                machine.state = ATTENTE;
            }
            break;
        }

        case ATTENTE:
        {
            LCD_WriteStringAtPos("ATTENTE         ", 0, 0);
            LCD_WriteStringAtPos("C:Calib  L:Monit", 1, 0);
            SSD_Close();
            if(machine.state != machine.previous_state);
            //LCD_WriteStringAtPos("ATTENTE         ", 0, 0);
            //LCD_WriteStringAtPos("                ", 1, 0);
            if (ButtonCenterStateGet()) {
                machine.state = CALIBRATION;
                calibration.state = ATTENTE_YEUX_OUVERTS;
            }
            else if(ButtonLeftStateGet()){
                machine.state = MONITORING;
            }
           
            break;
        }
        
        case CALIBRATION:
            
            CALIBRATION_Tasks();
            
            if(machine.state != machine.previous_state){//si on vient d'arriver dans l'état de calibration
                LCD_WriteStringAtPos("CALIBRATION     ", 0, 0);
                calibration.state = ATTENTE_YEUX_OUVERTS;
            }
            if (calibration.state != HORS_CALIBRATION && ButtonCenterStateGet()) {//si on est en train de calibrer et qu'on pese sur BtnC
                machine.state = CALIBRATION;
                calibration.state = ATTENTE_YEUX_OUVERTS;
                SSD_WriteDigits(0,0,0,0,0,0,0,0);
            }
            // passage de calibration à monitoring se fait dans CALIBRATION_Tasks
            break;

        case MONITORING:
           LCD_WriteStringAtPos("MONITORING      ", 0, 0);
           LCD_WriteStringAtPos("C:Cal R:Attente ", 1, 0);
           SSD_WriteDigits(timer%10,(timer / 10) % 10, (timer / 100) % 10, timer/1000,0,0,0,0);
            if (ButtonRightStateGet()){
            machine.state = ATTENTE;
            }
            else if (ButtonCenterStateGet()){
                machine.state = CALIBRATION;
            }
            break;

        case TEST:
            LCD_WriteStringAtPos("TEST            ", 0, 0);
            LCD_WriteStringAtPos("                ", 1, 0);
            break;

            /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
    machine.previous_state = machine.state; 
}

void CALIBRATION_Tasks(void){
     switch (calibration.state) {
        case HORS_CALIBRATION:
            SSD_Close();
            LCD_WriteStringAtPos("HORS-CALIBRATION", 1, 0);
            timer = 0;
            break;

        case ATTENTE_YEUX_OUVERTS:
            SSD_Close();
            LCD_WriteStringAtPos("C_ATTENTE YEUX-O ", 0, 0);
            LCD_WriteStringAtPos("U: Cal 1/2     ", 1, 0);
            if (ButtonUpStateGet()) {
                calibration.state = CALIBRATION_YEUX_OUVERTS;
                timer = 0;
            }
            break;

        case CALIBRATION_YEUX_OUVERTS:
            LCD_WriteStringAtPos("C_CAL YEUX-O   ", 0, 0);
            LCD_WriteStringAtPos("Attend 1 sec... ", 1, 0);
            SSD_WriteDigits(timer%10,(timer / 10) % 10, (timer / 100) % 10, timer/1000,0,0,0,0);
            
            if (timer >= 1000) {
                calibration.state = ATTENTE_YEUX_FERMES;
                timer = 0;
                SSD_WriteDigits(timer%10,(timer / 10) % 10, (timer / 100) % 10, timer/1000,0,0,0,0);
            }
            break;

        case ATTENTE_YEUX_FERMES:
            LCD_WriteStringAtPos("ATTENTE YEUX-F  ", 0, 0);
            LCD_WriteStringAtPos("U: Cal 2/2    ", 1, 0);
            SSD_Close();
            if (ButtonUpStateGet()) {
                calibration.state = CALIBRATION_YEUX_FERMES;
                timer = 0;
                
            }
            break;

        case CALIBRATION_YEUX_FERMES:
            LCD_WriteStringAtPos("C_CALIB YEUX-F   ", 0, 0);
            LCD_WriteStringAtPos("Attend 1 sec... ", 1, 0);
            SSD_WriteDigits(timer%10,(timer / 10) % 10, (timer / 100) % 10, timer/1000,0,0,0,0);
            
            if (timer >= 1000) {
                calibration.state = CALCUL_NORMALISATION;
                timer = 0;
            }
            break;

        case CALCUL_NORMALISATION:
            SSD_Close();
            if (timer <= 1000) {
            LCD_WriteStringAtPos("C_CALCUL          ", 0, 0);
            LCD_WriteStringAtPos("En cours          ", 1, 0);
            }
            else{
                if (baseline_power) {//si on as finis le calcul du baseline power (si le baseline power est non-nul)
                    machine.state = MONITORING;
                    calibration.state = HORS_CALIBRATION;
                }
                else{
                    LCD_WriteStringAtPos("C_NON VALIDE  ", 0, 0);
                    LCD_WriteStringAtPos("C:Redo D:Bypass", 1, 0);
                    if(ButtonDownStateGet()){
                        baseline_power = 1;
                    }
                    
                }
            }
            break;

        default:
            calibration.state = HORS_CALIBRATION;
            break;
    }
    calibration.previous_state = calibration.state; // Mise à jour après le switch case
}

int main(void) {
    
    SYS_Initialize(NULL);
    MAIN_Initialize();
    SYS_INT_Enable();
    
    
    while (1) {
        SYS_Tasks();
        MAIN_Tasks();
        LedTask();
    };

    return 0;
}


/*******************************************************************************
 End of File
 */

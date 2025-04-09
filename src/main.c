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
//#include "accel.h"
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


//Moyenne mobile
#define N 5

typedef struct {
    uint32_t buffer[N];
    uint8_t index;
    uint8_t count;
    uint32_t sum;
} Context;

uint32_t moyenneMobile(Context *ctx, uint32_t new_sample) {
    ctx->sum -= ctx->buffer[ctx->index];
    ctx->buffer[ctx->index] = new_sample;
    ctx->sum += new_sample;

    ctx->index = (ctx->index + 1) % N;
    if (ctx->count < N) ctx->count++;

    return ctx->sum / ctx->count;
}

Context ctx_signalInput = {0};
Context ctx_threshold = {0};
Context ctx_feedBackTime = {0};


volatile uint32_t signalInput = 0;
volatile uint32_t threshold = 0;
volatile uint32_t feedBackTime = 0;

void __ISR(_ADC_VECTOR, IPL1AUTO) ADC_ISR(void)
{
    //signalInput = ADC1BUF0;
    //threshold = ADC1BUF1;
    //feedBackTime = ADC1BUF2;
    
    signalInput = moyenneMobile(&ctx_signalInput, ADC1BUF0);
    feedBackTime = moyenneMobile(&ctx_threshold, ADC1BUF1);
    threshold = moyenneMobile(&ctx_feedBackTime, ADC1BUF2);
    
    IFS0bits.AD1IF = 0;
}

void InitializeADC()
{
    TRISBbits.TRISB2 = 1; //AIC
    ANSELBbits.ANSB2 = 1;
    
    TRISBbits.TRISB3 = 1; //AIN1 //Potentiomètre
    ANSELBbits.ANSB3 = 1;
    
    TRISBbits.TRISB5 = 1; //BIN2 //Potentiomètre
    ANSELBbits.ANSB5 = 1;
    
    TRISBbits.TRISB6 = 1; // IR_RX // Signal input
    ANSELBbits.ANSB6 = 1;
    
    AD1CON1 = 0;
    AD1CON1bits.SSRC = 0b010; // Timer 3 period match ends sampling and starts conversion
    AD1CON1bits.ASAM = 1;
    
    //AD1CHSbits.CH0SA = 0b00010; //AN2
    
    AD1CON2bits.SMPI = 2;
    
    AD1CON2bits.CSCNA = 1;
    AD1CSSLbits.CSSL2 = 1;
    AD1CSSLbits.CSSL3 = 1;
    AD1CSSLbits.CSSL5 = 1;
    //AD1CSSLbits.CSSL6 = 1;
    
    IFS0bits.AD1IF = 0;
    IPC5bits.AD1IP = 1;
    IPC5bits.AD1IS = 0;
    IEC0bits.AD1IE = 1;
    AD1CON1bits.ON = 1;
}

void InitializeSpeaker()
{
    TRISBbits.TRISB14 = 0;
    ANSELBbits.ANSB14 = 0;
    RPB14R = 0x0C; // OC1
}

void InitializeOC1()
{
    OC1CON = 0;
    OC1RS = 0;
    OC1CONbits.OC32 = 0; // OC1RS est 16 bit
    OC1CONbits.OCTSEL = 1; // Utiliser Timer3 comme base
    OC1CONbits.OCM = 0b110; // PWM mode without fault
    OC1CONbits.ON = 1; // Enable OC1
}

void InitializeOC5()
{
    OC5CON = 0;
    OC5RS = 0;
    OC5CONbits.OC32 = 0; // OC1RS est 16 bit
    OC5CONbits.OCTSEL = 1; // Utiliser Timer3 comme base
    OC5CONbits.OCM = 0b110; // PWM mode without fault
    OC5CONbits.ON = 1; // Enable
}

void InitializeBargraph()
{
    TRISEbits.TRISE9 = 0; //BIN1
    RPE9R = 0b1011 ; //OC5
}

volatile uint32_t dataIndex = 0;
volatile uint8_t counter_8kHz = 0;
volatile uint8_t counter_640Hz = 0;
volatile uint16_t sendBufferIndex = 0;

void __ISR(_TIMER_3_VECTOR, IPL3AUTO) Timer3_ISR(void)
{    
    counter_8kHz++;
    counter_640Hz++;
    
    if(counter_8kHz >= 5) // Toutes les 6 interruptions
    {
        dataIndex++;
        counter_8kHz = 0;  // R?initialisation
    }
    
    if(counter_640Hz >= 75)
    {
        UDP_Send_Buffer[sendBufferIndex] = (signalInput >> 8) & 0xFF;
        UDP_Send_Buffer[sendBufferIndex + 1] = signalInput & 0xFF;
        sendBufferIndex += 2;
        counter_640Hz = 0;
    }
    IFS0bits.T3IF = 0;  //Clear flag
}

void InitializeTimer3()
{
    T3CONbits.ON = 0;
    T3CONbits.TCKPS = 0b000; //Timer Input Clock Prescale = 1:1 pour 48kHz
    T3CONbits.TCS = 0; //Timer Clock Source Select bit = Internal peripheral clock
    T3CONbits.TGATE = 0; //Gated time accumulation is disabled
    
    PR3 = 1022; //Valeur à atteindre (48kHz)
    TMR3 = 0; //Initialise compteur ? 0

    IPC3bits.T3IP = 3; //Set interrupt priority to 6
    IPC3bits.T3IS = 0; //Set interrupt sub-priority to 2
    IFS0bits.T3IF = 0; //Clear interrupt flag
    
    IEC0bits.T3IE = 1; // Activer l'interruption Timer3 (48kHz)
    T3CONbits.ON = 1;
}

#define SINUS_SIZE 20
uint16_t compteurCycle = 0;
uint16_t sinusTest[] = {512, 670, 812, 925, 998, 1023, 998, 925, 812,
                670, 512, 353, 211, 98, 25, 0, 25, 98, 211, 353};

int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void retroaction()
{
    static uint8_t timeOut = 10; //Temps [s] avant de recommencer à MONITORER
    static uint8_t done = 0;
    if(done == 0)
    {
        if(compteurCycle < map(feedBackTime, 0, 1023, (400*1), (400*4)))
        {
            if(dataIndex < SINUS_SIZE)
            {
                OC1RS = sinusTest[dataIndex];
            }
            else
            {
                dataIndex = 0;
                compteurCycle++;
            }
            LATAbits.LATA3 = 1;

        }
        else
        {
            done = 1;
            LATAbits.LATA3 = 0;
            dataIndex = 0;
            OC1RS = 0;
        }  
    }
    else if(done == 1) //Attendre 10 secondes apr?s fin du signal
    {
        LATAbits.LATA4 = 1;
        if(dataIndex > (400*20*timeOut)) //400Cycles/s * 20 points/cycle * 10s
        {
            done = 0;
            compteurCycle = 0;
            dataIndex = 0;
            sendBufferIndex = 0;
            machine.state = MONITORING;
            LATAbits.LATA4 = 0;
        }
    }
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
    SSD_Init(); // Initialisation du Timer4 et de l'accéléromètre
    
    InitializeTimer3();    
    InitializeSpeaker();
    InitializeOC1();
    InitializeADC();
    InitializeBargraph();
    InitializeOC5();
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

uint32_t niveauAttention = 1023; // valeur temporaire

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
                sendBufferIndex = 0;
                machine.state = MONITORING;
            }
            /*else if (ButtonRightStateGet())
            {
                LCD_WriteStringAtPos("IL FAUT SE      ", 0, 0);
                LCD_WriteStringAtPos("RECONCENTRER    ", 1, 0);                
                machine.state = RETROACTION;
            }*/
           
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
           uint32_t threshold_mapped = map(threshold, 20, 1005, 0, 100);
           SSD_WriteDigits(threshold_mapped%10,(threshold_mapped / 10) % 10, (threshold_mapped / 100) % 10, threshold_mapped/1000,0,0,0,0);
           
           

           if(sendBufferIndex >= 255)
           {
                UDP_Send_Packet = true;
                sendBufferIndex = 0;
           }
           
           
           OC5RS = map(niveauAttention, 0, 1023, 0, 680); //Bargraph
           //ManageSwitches();
           UDP_Tasks();
           
            if (ButtonRightStateGet()){
            machine.state = ATTENTE;
            }
            else if (ButtonCenterStateGet()){
                machine.state = CALIBRATION;
            }
            else if(niveauAttention < threshold)
            {
                LCD_WriteStringAtPos("IL FAUT SE      ", 0, 0);
                LCD_WriteStringAtPos("RECONCENTRER    ", 1, 0);                
                machine.state = RETROACTION;                
            }
            break;
            
        case RETROACTION:
            
            retroaction();
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

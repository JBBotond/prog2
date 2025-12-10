#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "fsm_functions/fsm.h"
#include "console_functions/keyboard.h"
#include "console_functions/display.h"
#include "console_functions/devConsole.h"

extern char * eventEnumToText[];
extern char * stateEnumToText[];

state_t state;
event_t event;
state_t previousState;
event_t previousEvent;

int priceCola = 125;
int insertedMoney = 0;
int exitCode = 0;             //signal graceful shutdown

// Local function prototypes State related
void S_InitialiseSubsystems_onEntry(void);
void S_InitialiseSubsystems_onExit(void); // not used NULL in state chart
void S_Configure_onEntry(void);
void S_Configure_onExit(void);
void S_WaitForCoins_onEntry(void);
void S_WaitForCoins_onExit(void); // not used NULL in state chart
void S_Process_20C_onEntry(void);
void S_Process_20C_onExit(void);
void S_Process_50C_onEntry(void);
void S_Process_50C_onExit(void);
void S_Process_100C_onEntry(void);
void S_Process_100C_onExit(void);

// Exception function, for gracefull shutdown
void S_ShutdownSystem(int exitCode);


// Subsystems
void dispenseCola(void);         ///cola dispensor

event_t getCoin(void);           ///coin acceptor

//helper functions
void delay_ms(uint32_t d);

// Main
int main(void)
{
   //           State                                     onEntry()                       onExit()
   FSM_AddState(S_INITIALISESUBSYSTEMS,&(state_funcs_t){  S_InitialiseSubsystems_onEntry, S_InitialiseSubsystems_onExit });
   FSM_AddState(S_CONFIGURE,           &(state_funcs_t){  S_Configure_onEntry,            S_Configure_onExit});
   FSM_AddState(S_WAITFORCOINS,        &(state_funcs_t){  S_WaitForCoins_onEntry,         S_WaitForCoins_onExit });
   FSM_AddState(S_PROCESS_20C,         &(state_funcs_t){  S_Process_20C_onEntry,          S_Process_20C_onExit });
   FSM_AddState(S_PROCESS_50C,         &(state_funcs_t){  S_Process_50C_onEntry,          S_Process_50C_onExit });
   FSM_AddState(S_PROCESS_100C,        &(state_funcs_t){  S_Process_100C_onEntry,          S_Process_50C_onExit });

   //                                 From                     Event                To
   FSM_AddTransition(&(transition_t){ S_START,                 E_START,        S_INITIALISESUBSYSTEMS    });
   FSM_AddTransition(&(transition_t){ S_INITIALISESUBSYSTEMS,  E_CONTINUE,     S_CONFIGURE });
   FSM_AddTransition(&(transition_t){ S_CONFIGURE,             E_CONFIG_READY, S_WAITFORCOINS });
   FSM_AddTransition(&(transition_t){ S_WAITFORCOINS,          E_20C,          S_PROCESS_20C });
   FSM_AddTransition(&(transition_t){ S_WAITFORCOINS,          E_50C,          S_PROCESS_50C });
   FSM_AddTransition(&(transition_t){ S_WAITFORCOINS,          E_100C,         S_PROCESS_100C });
   FSM_AddTransition(&(transition_t){ S_PROCESS_20C,           E_NOT_ENOUGH,   S_WAITFORCOINS });
   FSM_AddTransition(&(transition_t){ S_PROCESS_50C,           E_NOT_ENOUGH,   S_WAITFORCOINS });
   FSM_AddTransition(&(transition_t){ S_PROCESS_100C,          E_NOT_ENOUGH,   S_WAITFORCOINS });
   FSM_AddTransition(&(transition_t){ S_PROCESS_20C,           E_ENOUGH,       S_WAITFORCOINS });
   FSM_AddTransition(&(transition_t){ S_PROCESS_50C,           E_ENOUGH,       S_WAITFORCOINS });
   FSM_AddTransition(&(transition_t){ S_PROCESS_100C,          E_ENOUGH,       S_WAITFORCOINS });

   // Should unexpected events in a state be flushed or not?
   FSM_FlushEnexpectedEvents(true);

   // Set the finite state machine in the S_START state
   // and add a E_START event into the event buffer

   FSM_FlushEnexpectedEvents(true);

   /// Start the state machine
   FSM_RunStateMachine(S_START, E_START);

   /// Use this test function to test your model
   ///FSM_RevertModel();



   S_ShutdownSystem(exitCode);
   return 0;
}

void S_InitialiseSubsystems_onEntry(void)
{

   // Initalize the debug console
   DSPinitialise();                                /// Initialise User interface
   DSPshowDisplay();                               /// And debug console
   DCSinitialise();
   KYBinitialise();

   // Inform the CVM user
   insertedMoney = 0;                              /// 0 coins inserted
   DSPshow(2, "Program started");                  /// Update user interface
   DSPshow(3, "inserted Money = %d", insertedMoney);  ///

   FSM_AddEvent(E_CONTINUE);           /// Initialisation done, go to next state
}

void S_InitialiseSubsystems_onExit(void)
{
   /// nothing to do onExit
}

void S_Configure_onEntry(void)
{
   priceCola = 125;                   /// Configure the price
   FSM_AddEvent(E_CONFIG_READY);      /// Configuration complete Internal generated event
}

void S_Configure_onExit(void)
{
   /// nothing to do onExit
}

void S_WaitForCoins_onEntry(void)
{
   event_t coin = getCoin();          ///Get coin information from Coin Subsystem
   FSM_AddEvent(coin);                ///Inserted coin is a (simulated) external event
}

void S_WaitForCoins_onExit(void)
{
   /// nothing to do onExit
}

void S_Process_20C_onEntry(void)
{
   insertedMoney = insertedMoney + 20;   /// 20 c inserted, increase the inserted money
   DSPshow(4,"Inserted 20C");

   if (insertedMoney >= priceCola)       /// enough money inserted
   {
      FSM_AddEvent(E_ENOUGH);            /// yes, go on and dispense a Cola
   }
   else {
      FSM_AddEvent(E_NOT_ENOUGH);        /// no, ask for more coins
   }
   DSPshow(3,"Inserted money = %d", insertedMoney);
}

void S_Process_20C_onExit(void)
{
   /*
 * In this onExit function first the transition event is checked
 * This gives information about enoughe/no-enough money
 * to get a can of cola
 *
*/
   if (event == E_ENOUGH)
   {
      dispenseCola();             /// Let the dispense subsystem deliver a can of Cola
      insertedMoney = 0;          /// Reset the debet
   }
   else
   {
      DSPshow(6, "Not enough money inserted");     /// Not enough, ask for more :-)
      DSPshow(7, "Please insert coins");
   }
}

void S_Process_50C_onEntry(void)             /// See the 20c handling
{
   insertedMoney = insertedMoney + 50;
   DSPshow(4,"Inserted 50C");
   if (insertedMoney >= priceCola)   {
      FSM_AddEvent(E_ENOUGH);
   }
   else {
      FSM_AddEvent(E_NOT_ENOUGH);
   }
   DSPshow(3,"Inserted money = %d", insertedMoney);
}

void S_Process_50C_onExit(void)
{
   if (event == E_ENOUGH)
   {
      DSPshow(6, "Enough money inserted, can of Cola on his way!");
      dispenseCola();             /// Let the dispense subsystem deliver a can of Cola
      insertedMoney = 0;          /// Reset the debet
   }
   else
   {
      DSPshow(6, "Not enough money inserted");
   }
}

void S_Process_100C_onEntry(void)
{
   insertedMoney = insertedMoney + 100;   /// 20 c inserted, increase the inserted money
   DSPshow(4,"Inserted 100C");

   if (insertedMoney >= priceCola)       /// enough money inserted
   {
      FSM_AddEvent(E_ENOUGH);            /// yes, go on and dispense a Cola
   }
   else {
      FSM_AddEvent(E_NOT_ENOUGH);        /// no, ask for more coins
   }
   DSPshow(3,"Inserted money = %d", insertedMoney);
}


void S_ShutdownSystem(int status)
{
   if (status !=0)
   {
      DCSshowSystemError("System shutting down error code %d, state = %", status, stateEnumToText[state]);
   }
   else
   {
      DCSsimulationSystemInfo("System shutting down", status);
   }
   exit(status);
}

/// Subsystems (HAL -- simulations)

void dispenseCola(void)          /// coladispenser
{
   DSPshow(7, "Please take your can with Cola");
}

event_t getCoin(void)            /// coinacceptor
{
   char kb;
   event_t coin = E_EXIT_SYSTEM;
   DSPclearLine(4);
   kb = DCSsimulationSystemInputChar("Please insert a coin enter 1 for 20c enter 2 for 50c enter 3 for 100c", "123");
   switch (kb)
   {
      case '1':
         coin = E_20C;
         break;
      case '2':
         coin = E_50C;
         break;
      case '3':
         coin = E_100C;
         break;
      default:
         coin = E_EXIT_SYSTEM;
   }
   return coin;
}

/// Helper functions

/// simulate delay in miliseconds
void delay_ms(uint32_t d)
{
   DCSdebugSystemInfo("Delay waiting for %d micro-seconds", d);
   sleep(d/1000);
}

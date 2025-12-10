#ifndef STATES_H
#define STATES_H

typedef enum {
   S_NO,                         ///< Used for initialisation if state is not yet known
   S_START,                      ///< Initial state
   S_INITIALISESUBSYSTEMS,       ///< Initialised subsystems
   S_CONFIGURE,                  ///< Ask user for action
   S_PROCESS_20C,                ///< Handle the 20c insert
   S_PROCESS_50C,                ///< Handle the 50c insert
   S_PROCESS_100C,                ///< Handle the 50c insert
   S_WAITFORCOINS,               ///< Wait for the user to insert coins
   //end
} state_t;

#endif

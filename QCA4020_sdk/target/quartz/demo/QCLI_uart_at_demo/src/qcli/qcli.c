/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 */
// Copyright (c) 2018 Qualcomm Technologies, Inc.
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below) 
// provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived 
// from this software without specific prior written permission.
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include "stringl.h"

#include "qapi_ver.h"

#include "malloc.h"
#include "qurt_error.h"

#include "qurt_thread.h"
#include "qurt_types.h"
#include "string.h"

#include "pal.h"

#include "qcli.h"
#include "qcli_api.h"
#include "qcli_util.h"
#include "qosa_util.h"
#include "qc_util.h"


#define MAIN_PRINTF_HANDLE                                              ((QCLI_Group_Handle_t)&(QCLI_Context.Root_Group))

/**
*/
//#define THREAD_READY_EVENT_MASK                                         0x00000001

/**
*/
#define COMMAND_THREAD_PRIORITY                                         20

#define TAKE_LOCK(__lock__)                                             ((qurt_mutex_lock_timed(&(__lock__), QURT_TIME_WAIT_FOREVER)) == QURT_EOK)
#define RELEASE_LOCK(__lock__)                                          do { qurt_mutex_unlock(&(__lock__)); } while(0)

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

/**
  This structure reprents the result of a Find_Command() operation.
  */
typedef struct Find_Result_s
{
    qbool_t Is_Group;                           /**< A flag indicating if the result is a command or a group. */
    union
    {
        const QCLI_Command_t *Command;           /**< The entry that was found if it is a command. */
        Group_List_Entry_t   *Group_List_Entry;  /**< The entry that was found if it is a group. */
    } Data;
} Find_Result_t;

QCLI_Context_t QCLI_Context;

/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/

static QCLI_Command_Status_t Command_Ver(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_LogLevel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Reset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Up(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static QCLI_Command_Status_t Command_Root(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

static void Display_Group_Name(const Group_List_Entry_t *Group_List_Entry);
uint32_t Display_Help(Group_List_Entry_t *Command_Group, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static void Display_Usage(uint32_t Command_Index, const QCLI_Command_t *Command);
void Display_Command_List(const Group_List_Entry_t *Group_List_Entry);

static void Command_Thread(void *Thread_Parameter);

static void Execute_Command(uint32_t Command_Index, const QCLI_Command_t *Command, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
static qbool_t Find_Command(Group_List_Entry_t *Group_List_Entry, QCLI_Parameter_t *Command_Parameter, Find_Result_t *Find_Result);
static void Process_Command(void);
static qbool_t Unregister_Command_Group(Group_List_Entry_t *Group_List_Entry);

/* The following represents the list of global commands that are supported when
   not in a group. */
const QCLI_Command_t Root_Command_List[] =
{
    {Command_Ver,          false,  "ATVERSION",   "",                      "Display Build Info"},
    {Command_Help,         false,  "ATHELP",      "[Subgroup]",            "Display the help menu and all available commands"},
    {Command_LogLevel,     false,  "ATLOGLEVEL",  "[level]",               "Sets and Get the log level"},
    {Command_Reset,        false,  "ATRESET",     "",                      "Restart the application."}
};

#define ROOT_COMMAND_LIST_SIZE                        (sizeof(Root_Command_List) / sizeof(QCLI_Command_t))

/* The following represents the list of global commands that are supported when
   in a group. */
const QCLI_Command_t Common_Command_List[] =
{
    {Command_Up,         false, "Up",   "",                     "Exit command group (move to parent group)"},
    {Command_Root,       false, "Root", "",                     "Move to top-level group list"}
};

#define COMMON_COMMAND_LIST_SIZE                      (sizeof(Common_Command_List) / sizeof(QCLI_Command_t))

static uint32_t data_mode;
static uint32_t data_send_len;
QCLI_Command_Status_t qc_api_net_SendData(char *tx_data, uint32_t len);

/*-------------------------------------------------------------------------
 * Function Definitions
 *-----------------------------------------------------------------------*/
/**
  @brief This function is responsible for displaying build info.
  */
static QCLI_Command_Status_t Command_Ver(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    qapi_FW_Info_t  info;

    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        if( qapi_Get_FW_Info(&info) == QAPI_OK)
        {
            QCLI_Printf("QAPI Ver: %d.%d.%d\n",
                    (info.qapi_Version_Number&__QAPI_VERSION_MAJOR_MASK)>>__QAPI_VERSION_MAJOR_SHIFT,
                    (info.qapi_Version_Number&__QAPI_VERSION_MINOR_MASK)>>__QAPI_VERSION_MINOR_SHIFT,
                    (info.qapi_Version_Number&__QAPI_VERSION_NIT_MASK)>>__QAPI_VERSION_NIT_SHIFT  );
            QCLI_Printf("CRM  Num: %d\n", info.crm_Build_Number);

            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }
    else
    {
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief This function processes the "ATLOGLEVEL" command from the CLI.

  The parameters specified indicate the log level to prints logs accordingly.
  If no parameters are specified, current log level will be displayed.
  */
static QCLI_Command_Status_t Command_LogLevel(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    int32_t Level;

    if (Parameter_Count >= 1)
    {
        if (Verify_Integer_Parameter(&(Parameter_List[0]), 1, MAX_LOG_LEVEL))
        {
            Level = Parameter_List[0].Integer_Value;
            qc_api_SetLogLevel(Level);
            QCLI_Printf("\r\nOK\r\n");
            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            QCLI_Printf("Invalid Level!\r\n");
            QCLI_Printf("\r\nERROR\r\n");
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
    else
    {
        Level = qc_api_GetLogLevel();
        QCLI_Printf("The Log level is %d\r\n", Level);
        QCLI_Printf("\r\nOK\r\n");
        Ret_Val = QCLI_STATUS_SUCCESS_E;
    }

    return(Ret_Val);
}

/**
  @brief This function processes the "Help" command from the CLI.

  The parameters specified indicate the command or group to display the
  help message for.  If no parameters are specified, the list of commands
  for the current command group will be displayed. If the paramters
  specify a subgroup, the command list for that group will be displayed.
  If the paramters specify a command, the usage message for that command
  will be displayed.
  */
static QCLI_Command_Status_t Command_Help(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;
    uint32_t              Result;
    int32_t               Index;
    QCLI_Command_t        *Command;

    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        Result = Display_Help(QCLI_Context.Executing_Group, Parameter_Count, Parameter_List);

        /* if there was an error parsing the command list, print out an error
           message here (this is in addition to the usage message that will be
           printed out). */
        if(Result > 0)
        {
            Command = (QCLI_Command_t *)Root_Command_List;
            QCLI_Printf("ERROR\r\n");
            QCLI_Printf("Subgroup \"%s", Parameter_List[0].String_Value);

            for(Index = 1; Index < Result; Index ++)
            {
                QCLI_Printf(" %s", Parameter_List[Index].String_Value);
            }
            QCLI_Printf("\" not found.\r\n\r\n");
            QCLI_Printf("%s=%s\r\n", Command[1].Command_String, Command[1].Usage_String);
            QCLI_Printf("    %s\r\n", Command[1].Description);

            Ret_Val = QCLI_STATUS_ERROR_E;
        }
        else
        {
            QCLI_Printf("OK\r\n");
            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }
    else
    {
        QCLI_Printf("ERROR\r\n");
        Ret_Val = QCLI_STATUS_ERROR_E;
    }

    return(Ret_Val);
}

/**
  @brief This function processes the "Exit" command from the CLI.

  For embedded platforms, this is expected to reset the processor. A
  parameter of "1" will perform the reset.
  */
static QCLI_Command_Status_t Command_Reset(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Ret_Val;

#if 0
    if(Parameter_Count == 0)
    {
        if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
        {
            /* Exit the application. */
            PAL_Exit();

            RELEASE_LOCK(QCLI_Context.CLI_Mutex);

            Ret_Val = QCLI_STATUS_SUCCESS_E;
        }
        else
        {
            QCLI_Printf("could not exit, failed to take lock.\n");
            Ret_Val = QCLI_STATUS_ERROR_E;
        }
    }
    else
    {
        if((Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value == 1))
        {
#endif
            if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
            {
                /* Reset the application. */
                PAL_Reset();

                RELEASE_LOCK(QCLI_Context.CLI_Mutex);

                Ret_Val = QCLI_STATUS_SUCCESS_E;
            }
            else
            {
                QCLI_Printf("could not reset, failed to take lock.\n");
                Ret_Val = QCLI_STATUS_ERROR_E;
            }
#if 0
        }
        else
        {
            Ret_Val = QCLI_STATUS_USAGE_E;
        }
    }
#endif

    return(Ret_Val);
}

/**
  @brief This function processes the "Up" command from the CLI.

  This will change the current group to be the current parent. No
  parameters are expected for this command.
  */
static QCLI_Command_Status_t Command_Up(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        if(QCLI_Context.Current_Group != &(QCLI_Context.Root_Group))
        {
            QCLI_Context.Current_Group = QCLI_Context.Current_Group->Parent_Group;

            /* Display the command list again. */
            Display_Command_List(QCLI_Context.Current_Group);
        }

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }

    return(QCLI_STATUS_SUCCESS_E);
}

/**
  @brief This function processes the "Up" command from the CLI.

  This will change the current group to the root group. No parameters
  are expected for this command.
  */
static QCLI_Command_Status_t Command_Root(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        if(QCLI_Context.Current_Group != &(QCLI_Context.Root_Group))
        {
            QCLI_Context.Current_Group = &(QCLI_Context.Root_Group);

            /* Display the command list again. */
            Display_Command_List(QCLI_Context.Current_Group);
        }

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }

    return(QCLI_STATUS_SUCCESS_E);
}

/**
  @brief This function will display the group name, recursively displaying
  the name of the groups parents.

  @param Group_List_Entry is the group list whose name should be
  displayed.  If this isn't the root group, the parent group's
  name will be displayed first.
  */
static void Display_Group_Name(const Group_List_Entry_t *Group_List_Entry)
{
    /* If the group's parent isn't the root, display the parent first. */
    if(Group_List_Entry->Parent_Group != &(QCLI_Context.Root_Group))
    {
        Display_Group_Name(Group_List_Entry->Parent_Group);

        QCLI_Printf("\\");
    }

    /* Display this group's name. */
    QCLI_Printf("%s", Group_List_Entry->Command_Group->Group_String);
}

/**
  @brief This function will processes the help command, recursively
  decending groups if necessary.

  @param Group_List_Entry is the current command group for the help
  command.
  @param Paramter_Count is the number of parameters in the paramter list.
  @param Paramter_List is the paramter list provided to the help command.
  As the groups are recursively decended, the first paramter will
  be stripped off until the list is empty.

  @return
  - 0 if the help was displayed correctly.
  - A positive value indicating the depth of the error if a paramter was
  invalid.
  */
uint32_t Display_Help(Group_List_Entry_t *Group_List_Entry, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    uint32_t      Ret_Val;
    Find_Result_t Find_Result;

    /* If a parameter was specified, see if we can tie it to a command. */
    if(Parameter_Count >= 1)
    {
        if(Find_Command(Group_List_Entry, &(Parameter_List[0]), &Find_Result))
        {
            /* Command was found, assign it now. */
            if(Find_Result.Is_Group)
            {
                /* If this was a group, recurse into it. */
                Ret_Val = Display_Help(Find_Result.Data.Group_List_Entry, Parameter_Count - 1, &(Parameter_List[1]));

                /* If the recursive call returned an error, add one to it. */
                if(Ret_Val > 0)
                {
                    Ret_Val ++;
                }
            }
            else
            {
                /* If this was the last parameter specified, display the usage for
                   the command. If it wasn't, return an error. */
                if(Parameter_Count == 1)
                {
                    Display_Usage(Parameter_List[0].Integer_Value, Find_Result.Data.Command);

                    Ret_Val = 0;
                }
                else
                {
                    /* The error code indicates that the next parameter is invalid. */
                    Ret_Val = 2;
                }
            }
        }
        else
        {
            /* Command not found so return an error. */
            Ret_Val = 1;
        }
    }
    else
    {
        /* Display the command list for the current group. */
        Display_Command_List(Group_List_Entry);

        Ret_Val = 0;
    }

    return(Ret_Val);
}

/**
  @brief This function displays the usage string for a command.

  @param Command_Index is the index of the command in its associated
  commadn group.
  @param Command is the information structure for the command.
  */
static void Display_Usage(uint32_t Command_Index, const QCLI_Command_t *Command)
{
    QCLI_Printf("\n");
    if (0 == strncasecmp("AT", Command->Command_String, 2))
    {
        QCLI_Printf("%s=%s\n", Command->Command_String, Command->Usage_String);
    }
    else if (QCLI_Context.Current_Group == &QCLI_Context.Root_Group)
    {
        QCLI_Printf("%s%s=%s\n", QCLI_Context.Executing_Group->Command_Group->Group_String, Command->Command_String, Command->Usage_String);
    }
    else
    {
        QCLI_Printf("%s=%s\n", Command->Command_String, Command->Usage_String);
    }
    QCLI_Printf("    %s\n",    Command->Description);
    QCLI_Printf("\n");
}

/**
  @brief This function displays the list of commands and/or command groups
  for the specified gorup list.

  @param Group_List_Entry is the command group list entry to be displayed.
  */
void Display_Command_List(const Group_List_Entry_t *Group_List_Entry)
{
    uint32_t              Index;
    uint32_t              Command_Index;
    const QCLI_Command_t *Command_List;
    uint32_t              Command_List_Size;
    Group_List_Entry_t   *Subgroup_List_Entry;

    QCLI_Printf("\n");

    if(Group_List_Entry)
    {
        Command_Index = COMMAND_START_INDEX;

        QCLI_Printf("Command List");

        /* Display the common commands. */
        if(Group_List_Entry == &(QCLI_Context.Root_Group))
        {
            Command_List      = Root_Command_List;
            Command_List_Size = ROOT_COMMAND_LIST_SIZE;
        }
        else
        {
            QCLI_Printf(" (");

            if(Group_List_Entry != &(QCLI_Context.Root_Group))
            {
                Display_Group_Name(Group_List_Entry);
            }

            QCLI_Printf(")");

            Command_List      = Common_Command_List;
            Command_List_Size = COMMON_COMMAND_LIST_SIZE;
        }

        QCLI_Printf(":\n");

        QCLI_Printf("  Commands:\n");
        for(Index = 0; Index < Command_List_Size; Index ++)
        {
            QCLI_Printf("    %2d. %s\n", Command_Index, Command_List[Index].Command_String);
            Command_Index ++;
        }

        /* Display the command list. */
        if((Group_List_Entry->Command_Group != NULL) && (Group_List_Entry->Command_Group->Command_List != NULL))
        {
            QCLI_Printf("\n");

            for(Index = 0; Index < Group_List_Entry->Command_Group->Command_Count; Index ++)
            {
                if (QCLI_Context.Current_Group == &QCLI_Context.Root_Group)
                {
                    QCLI_Printf("    %2d. ", Command_Index);
                    Display_Group_Name(Group_List_Entry);
                    QCLI_Printf("%s\n", Group_List_Entry->Command_Group->Command_List[Index].Command_String);
                } else if (QCLI_Context.Executing_Group)
                {
                    QCLI_Printf("    %2d. %s\n", Command_Index, Group_List_Entry->Command_Group->Command_List[Index].Command_String);
                }
                Command_Index ++;
            }
        }

        /* Display the group list. */
        if(Group_List_Entry->Subgroup_List != NULL)
        {
            QCLI_Printf("\n");
            QCLI_Printf("  Subgroups:\n");

            Subgroup_List_Entry = Group_List_Entry->Subgroup_List;
            while(Subgroup_List_Entry != NULL)
            {
                QCLI_Printf("    %2d. %s\n", Command_Index, Subgroup_List_Entry->Command_Group->Group_String);

                Subgroup_List_Entry = Subgroup_List_Entry->Next_Group_List_Entry;
                Command_Index ++;
            }
        }

        QCLI_Printf("\n");
    }
}

/**
  @brief This function is a wrapper for commands which start in their own
  thread.

  @param Thread_Parameter is the parameter specified when the thread was
  started. It is expected to be a pointer to a Thread_Info_t
  structure.
  */
static void Command_Thread(void *Thread_Parameter)
{
    uint32_t              Index;
    Thread_Info_t        *Thread_Info;
    char                  Input_String[MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1];
    QCLI_Parameter_t      Parameter_List[MAXIMUM_NUMBER_OF_PARAMETERS];
    uint32_t              Parameter_Count;
    uint32_t              Command_Index;
    const QCLI_Command_t *Command;

    QCLI_Command_Status_t Result;

    if(Thread_Parameter)
    {
        memset(&Parameter_List, 0, sizeof(Parameter_List));

        /* Copy the thread info to local storage. */
        Thread_Info = (Thread_Info_t*)Thread_Parameter;
        memscpy(&Input_String, sizeof(Input_String), QCLI_Context.Input_String, sizeof(QCLI_Context.Input_String));
        memscpy(&Parameter_List, sizeof(Parameter_List), Thread_Info->Parameter_List, Thread_Info->Parameter_Count * sizeof(QCLI_Parameter_t));
        Parameter_Count = Thread_Info->Parameter_Count;
        Command         = Thread_Info->Command;
        Command_Index   = Thread_Info->Command_Index;

        /* Adjust the pointers in the paramter list for the local input string. */
        for(Index = 0; Index < Thread_Info->Parameter_Count; Index ++)
        {
            Parameter_List[Index].String_Value += (Input_String - QCLI_Context.Input_String);
        }

        /* Signal that the thread is ready.  The Thread parameter should be
           considered invalid after this point. */
        qurt_signal_set(&(Thread_Info->Thread_Ready_Event), THREAD_READY_EVENT_MASK);

        /* Execute the command. */
        Result = (*(Command->Command_Function))(Parameter_Count, Parameter_List);

        /* Take the mutex before modifying any global variables. */
        if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
        {
            if(Result == QCLI_STATUS_USAGE_E)
            {
                /* Print the usage message. */
                Display_Usage(Command_Index, Command);
                QCLI_Display_Prompt();
            }

            /* Decrement the number of active threads. */
            QCLI_Context.Thread_Count --;

            RELEASE_LOCK(QCLI_Context.CLI_Mutex);
        }
    }

    /* Terminate the thread. */
    qurt_thread_stop();
}

/**
  @brief This function executes a given command function.

  @param Command_Index is the index of the command to be executed in its
  associated command group.
  @param command is the information structure for the command to be
  executed.
  @param Parameter_Count is the parameter count that is passed to the
  command.
  @param Parameter_List is a pointer to the parameter list that is passed
  to the command.
  */
static void Execute_Command(uint32_t Command_Index, const QCLI_Command_t *Command, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t Result;
    qurt_thread_attr_t    Thread_Attribte;
    uint32                Signal_Waiting;
    qurt_thread_t         Thread_Handle;
    int                   Thread_Result;

    if(Command->Start_Thread)
    {
        /* Make sure we haven't maxed out the number of supported threads. */
        if(QCLI_Context.Thread_Count < MAXIMUM_THREAD_COUNT)
        {
            QCLI_Context.Thread_Count ++;

            /* Make sure the running event semaphore is taken. */
            qurt_signal_clear(&QCLI_Context.Thread_Info.Thread_Ready_Event, THREAD_READY_EVENT_MASK);

            /* Pass the function to the thread pool. */
            QCLI_Context.Thread_Info.Command         = Command;
            QCLI_Context.Thread_Info.Command_Index   = Command_Index;
            QCLI_Context.Thread_Info.Parameter_Count = Parameter_Count;
            QCLI_Context.Thread_Info.Parameter_List  = Parameter_List;

            /* Create a thread for the command. */
            qurt_thread_attr_init(&Thread_Attribte);
            qurt_thread_attr_set_name(&Thread_Attribte, "Commad Thread");
            qurt_thread_attr_set_priority(&Thread_Attribte, COMMAND_THREAD_PRIORITY);
            qurt_thread_attr_set_stack_size(&Thread_Attribte, THREAD_STACK_SIZE);
            Thread_Result = qurt_thread_create(&Thread_Handle, &Thread_Attribte, Command_Thread, (void *)&(QCLI_Context.Thread_Info));

            if(Thread_Result == QURT_EOK)
            {
                /* Wait for the thread to take over. */
                if(qurt_signal_wait_timed(&QCLI_Context.Thread_Info.Thread_Ready_Event, THREAD_READY_EVENT_MASK, QURT_SIGNAL_ATTR_WAIT_ANY, &Signal_Waiting, QURT_TIME_WAIT_FOREVER) != QURT_EOK)
                {
                    QCLI_Printf("Thread failed to start for command.\n");
                }
            }
            else
            {
                QCLI_Printf("Failed to create thread for command (%d).\n", Thread_Result);

                QCLI_Context.Thread_Count --;
            }
        }
        else
        {
            QCLI_Printf("Max threads reached.\n");
        }
    }
    else
    {
        /* Release the mutex while the function is being executed. */
        RELEASE_LOCK(QCLI_Context.CLI_Mutex);

        Result = (*(Command->Command_Function))(Parameter_Count, Parameter_List);

        if(!TAKE_LOCK(QCLI_Context.CLI_Mutex))
        {
            QCLI_Printf("Failed to re-take the mutex!\n");
        }

        if(Result == QCLI_STATUS_USAGE_E)
        {
            Display_Usage(Command_Index, Command);
        }
    }
}

/**
  @brief This function searches the command and/or group lists for a
  match to the provided parameter.

  @param Group_List_Entry is the group to search.
  @param Command_Parameter is the paramter to search for.
  @param Find_Result is a pointer to where the found entry will be stored
  if successful (i.e., true was returned).

  @return
  - true if a matching command or group was found in the list.
  - false if the command or group was not found.
  */
static qbool_t Find_Command(Group_List_Entry_t *Group_List_Entry, QCLI_Parameter_t *Command_Parameter, Find_Result_t *Find_Result)
{
    qbool_t               Ret_Val;
    uint32_t              Index;
    uint32_t              Command_Index;
    uint32_t              String_Length;
    const QCLI_Command_t *Command_List;
    uint32_t              Command_List_Length;
    Group_List_Entry_t   *Subgroup_List_Entry;

    /* Get the size of the string. Include the null byte so the comparison
       doesn't match substrings. */
    String_Length = strlen((const char *)(Command_Parameter->String_Value)) + 1;

    if(Group_List_Entry != NULL)
    {
        /* Determine which common command list is going to be used. */
        if(Group_List_Entry == &(QCLI_Context.Root_Group))
        {
            Command_List        = Root_Command_List;
            Command_List_Length = ROOT_COMMAND_LIST_SIZE;
        }
        else
        {
            Command_List        = Common_Command_List;
            Command_List_Length = COMMON_COMMAND_LIST_SIZE;
        }

        if(Command_Parameter->Integer_Is_Valid)
        {
            /* Command was specified as an integer. */
            if(Command_Parameter->Integer_Value >= COMMAND_START_INDEX)
            {
                Command_Index = Command_Parameter->Integer_Value - COMMAND_START_INDEX;

                /* If the integer is a valid value for the command group, use it. */
                if(Command_Index < Command_List_Length)
                {
                    /* Command is in the common command list. */
                    Ret_Val                   = true;
                    Find_Result->Is_Group     = false;
                    Find_Result->Data.Command = &(Command_List[Command_Index]);
                }
                else
                {
                    Command_Index -= Command_List_Length;

                    if((Group_List_Entry->Command_Group != NULL) && (Command_Index < Group_List_Entry->Command_Group->Command_Count))
                    {
                        /* Command is in the group's command list. */
                        Ret_Val                   = true;
                        Find_Result->Is_Group     = false;
                        Find_Result->Data.Command = &(Group_List_Entry->Command_Group->Command_List[Command_Index]);
                    }
                    else
                    {
                        if(Group_List_Entry->Command_Group != NULL)
                        {
                            Command_Index -= Group_List_Entry->Command_Group->Command_Count;
                        }

                        /* Search the group list. */
                        Group_List_Entry = Group_List_Entry->Subgroup_List;
                        while((Group_List_Entry != NULL) && (Command_Index != 0))
                        {
                            Group_List_Entry = Group_List_Entry->Next_Group_List_Entry;
                            Command_Index --;
                        }

                        if(Group_List_Entry != NULL)
                        {
                            /* Command is in the subgroup list. */
                            Ret_Val                            = true;
                            Find_Result->Is_Group              = true;
                            Find_Result->Data.Group_List_Entry = Group_List_Entry;
                        }
                        else
                        {
                            Ret_Val = false;
                        }
                    }
                }
            }
            else
            {
                Ret_Val = false;
            }
        }
        else
        {
            /* Command was specified as a string. */
            Command_Index = COMMAND_START_INDEX;
            Ret_Val       = false;

            /* Search the common command list. */
            for(Index = 0; (Index < Command_List_Length) && (!Ret_Val); Index ++)
            {
                if(Memcmpi(Command_Parameter->String_Value, Command_List[Index].Command_String, String_Length) == 0)
                {
                    /* Command found. */
                    Ret_Val                          = true;
                    Find_Result->Is_Group            = false;
                    Find_Result->Data.Command        = &(Command_List[Index]);
                    Command_Parameter->Integer_Value = Command_Index;
                }
                else
                {
                    Command_Index ++;
                }
            }

            /* Only search the command group if it isn't NULL. */
            if((!Ret_Val) && (Group_List_Entry->Command_Group != NULL))
            {
                /* If the comamnd wasn't found yet, search the group's command list. */
                for(Index = 0; (Index < Group_List_Entry->Command_Group->Command_Count) && (!Ret_Val); Index ++)
                {
                    if(Memcmpi(Command_Parameter->String_Value, Group_List_Entry->Command_Group->Command_List[Index].Command_String, String_Length) == 0)
                    {
                        /* Command found. */
                        Ret_Val                           = true;
                        Find_Result->Is_Group             = false;
                        Find_Result->Data.Command         = &(Group_List_Entry->Command_Group->Command_List[Index]);
                        Command_Parameter->Integer_Value  = Command_Index;
                    }
                    else
                    {
                        Command_Index ++;
                    }
                }
            }

            if(!Ret_Val)
            {
                /* If the comamnd wasn't found yet, search the group's subgroup
                   list. */
                Subgroup_List_Entry = Group_List_Entry->Subgroup_List;
                while((Subgroup_List_Entry != NULL) && (!Ret_Val))
                {
                    if(Memcmpi(Command_Parameter->String_Value, Subgroup_List_Entry->Command_Group->Group_String, String_Length) == 0)
                    {
                        /* Command found. */
                        Ret_Val                            = true;
                        Find_Result->Is_Group              = true;
                        Find_Result->Data.Group_List_Entry = Subgroup_List_Entry;
                        Command_Parameter->Integer_Value   = Command_Index;
                    }
                    else
                    {
                        Command_Index ++;
                        Subgroup_List_Entry = Subgroup_List_Entry->Next_Group_List_Entry;
                    }
                }
            }
        }
    }
    else
    {
        Ret_Val = false;
    }

    return(Ret_Val);
}

/**
  @brief This function processes a command received from the console.
  */
static void Process_Command(void)
{
    qbool_t       Result;
    Find_Result_t Find_Result;
    uint32_t      Parameter_Count;
    uint32_t      Index;
    uint32_t      Command_Index;
    qbool_t       Inside_Quotes;
    uint32_t      Input_Length;

    /* Store the input length locally so any re-displays of the prompt will not
       include the command. */
    Input_Length              = QCLI_Context.Input_Length;
    QCLI_Context.Input_Length = 0;

    /* Parse the command until its end is reached or the parameter list is full. */
    Parameter_Count = 0;
    Index           = 0;
    Inside_Quotes   = false;
    Result          = true;

    /* Check, given AT Command is belong to which group ? */
    if (strncasecmp("ATW", QCLI_Context.Input_String,  3) == 0) {
        memsmove(QCLI_Context.Input_String + 4,
                MAXIMUM_QCLI_COMMAND_STRING_LENGTH,
                QCLI_Context.Input_String + 3,
                Input_Length - 3);
        QCLI_Context.Input_String[3] = '=';
        /* is command have sub group ? */
        if (strncasecmp("P2P", QCLI_Context.Input_String + 4,  3) == 0) {
            memsmove(QCLI_Context.Input_String + 8,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH,
                    QCLI_Context.Input_String + 7,
                    Input_Length - 6);
            QCLI_Context.Input_String[7] = '=';
        }
    }
    else if (strncasecmp("ATN", QCLI_Context.Input_String,  3) == 0) {
        memsmove(QCLI_Context.Input_String + 4,
                MAXIMUM_QCLI_COMMAND_STRING_LENGTH,
                QCLI_Context.Input_String + 3,
                Input_Length - 3);
        QCLI_Context.Input_String[3] = '=';
    }
    else if (strncasecmp("ATM", QCLI_Context.Input_String,  3) == 0) {
        memsmove(QCLI_Context.Input_String + 7,
                MAXIMUM_QCLI_COMMAND_STRING_LENGTH,
                QCLI_Context.Input_String + 6,
                Input_Length - 6);
        QCLI_Context.Input_String[6] = '=';
    }
    else if (strncasecmp("ATZ", QCLI_Context.Input_String,  3) == 0) {
        memsmove(QCLI_Context.Input_String + 5,
                MAXIMUM_QCLI_COMMAND_STRING_LENGTH,
                QCLI_Context.Input_String + 4,
                Input_Length - 4);
        QCLI_Context.Input_String[4] = '=';
    }
    else if (strncasecmp("ATT", QCLI_Context.Input_String,  3) == 0) {
        memsmove(QCLI_Context.Input_String + 5,
                MAXIMUM_QCLI_COMMAND_STRING_LENGTH,
                QCLI_Context.Input_String + 4,
                Input_Length - 4);
        QCLI_Context.Input_String[4] = '=';
    }
    else
 if (strncasecmp("ATBLE", QCLI_Context.Input_String,  5) == 0) {
        memsmove(QCLI_Context.Input_String + 6,
                MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                QCLI_Context.Input_String + 5,
                Input_Length - 5);
        QCLI_Context.Input_String[5] = '=';
        /* is command have sub group ? */
        if (strncasecmp("AIOS", QCLI_Context.Input_String + 6,  4) == 0) {
            memsmove(QCLI_Context.Input_String + 11,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 10,
                    Input_Length - 9);
            QCLI_Context.Input_String[10] = '=';
        } else if (strncasecmp("BAS", QCLI_Context.Input_String + 6,  3) == 0) {
            memsmove(QCLI_Context.Input_String + 10,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 9,
                    Input_Length - 8);
            QCLI_Context.Input_String[9] = '=';
        } else if (strncasecmp("GAPS", QCLI_Context.Input_String + 6,  4) == 0) {
            memsmove(QCLI_Context.Input_String + 11,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 10,
                    Input_Length - 9);
            QCLI_Context.Input_String[10] = '=';
        } else if (strncasecmp("HIDS", QCLI_Context.Input_String + 6,  4) == 0) {
            memsmove(QCLI_Context.Input_String + 11,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 10,
                    Input_Length - 9);
            QCLI_Context.Input_String[10] = '=';
        } else if (strncasecmp("SCPS", QCLI_Context.Input_String + 6,  4) == 0) {
            memsmove(QCLI_Context.Input_String + 11,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 10,
                    Input_Length - 9);
            QCLI_Context.Input_String[10] = '=';
        } else if (strncasecmp("SPPLE", QCLI_Context.Input_String + 6,  5) == 0) {
            memsmove(QCLI_Context.Input_String + 12,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 11,
                    Input_Length - 10);
            QCLI_Context.Input_String[11] = '=';
        } else if (strncasecmp("DIS", QCLI_Context.Input_String + 6,  3) == 0) {
            memsmove(QCLI_Context.Input_String + 10,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 9,
                    Input_Length - 8);
            QCLI_Context.Input_String[9] = '=';
        } else if (strncasecmp("HRS", QCLI_Context.Input_String + 6,  3) == 0) {
            memsmove(QCLI_Context.Input_String + 10,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 9,
                    Input_Length - 8);
            QCLI_Context.Input_String[9] = '=';
        } else if (strncasecmp("TPS", QCLI_Context.Input_String + 6,  3) == 0) {
            memsmove(QCLI_Context.Input_String + 10,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 9,
                    Input_Length - 8);
            QCLI_Context.Input_String[9] = '=';
        } else if (strncasecmp("V5", QCLI_Context.Input_String + 6,  2) == 0) {
            memsmove(QCLI_Context.Input_String + 9,
                    MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1,
                    QCLI_Context.Input_String + 8,
                    Input_Length - 7);
            QCLI_Context.Input_String[8] = '=';
        }

    }

    while((Result) && (QCLI_Context.Input_String[Index] != '\0') && (Parameter_Count <= MAXIMUM_NUMBER_OF_PARAMETERS))
    {
        /* Consume any leading ' ', ',', '='. */
        while(QCLI_Context.Input_String[Index] == ' ' || QCLI_Context.Input_String[Index] == ',' || QCLI_Context.Input_String[Index] == '=')
        {
            Index ++;
        }

        /* If the first character is '"', consume it. */
        if(QCLI_Context.Input_String[Index] == '"')
        {
            Inside_Quotes = true;
            Index ++;
        }

        /* Assuming the end of the command hasn't been reached, assign the current
           string location as the current parameter's string. */
        if(QCLI_Context.Input_String[Index] != '\0')
        {
            QCLI_Context.Parameter_List[Parameter_Count].String_Value = &QCLI_Context.Input_String[Index];

            /* Find the end of the paramter.  The end of parameter is determined as
               either a null character (end of input), a double quote, and if not
               currenlty inside of quotes, a space. */
            while((Result) && (QCLI_Context.Input_String[Index] != '\0') &&
                    (QCLI_Context.Input_String[Index] != '"') &&
                    ((Inside_Quotes) ||
                     (QCLI_Context.Input_String[Index] != ' ')) &&
                    (QCLI_Context.Input_String[Index] != ',') &&
                    (QCLI_Context.Input_String[Index] != '='))
            {
                /* Handle escaped characters. */
                if(QCLI_Context.Input_String[Index] == '\\')
                {
                    if((Index + 1) < Input_Length)
                    {
                        /* Currently only '\' and '"' characters are escaped. */
                        if((QCLI_Context.Input_String[Index + 1] == '\\') || (QCLI_Context.Input_String[Index + 1] == '\"'))
                        {
                            /* Simply consume the escape character. */
                            memsmove(&(QCLI_Context.Input_String[Index]), Input_Length - Index, &(QCLI_Context.Input_String[Index + 1]), Input_Length - Index - 1);

                            Input_Length --;
                        }
                        else
                        {
                            QCLI_Printf("Invalid escape sequence \"\\%c\"\n", QCLI_Context.Input_String[Index + 1]);
                            Result = false;
                        }
                    }
                    else
                    {
                        QCLI_Printf("Invalid escape sequence\n");
                        Result = false;
                    }
                }

                Index ++;
            }

            if(QCLI_Context.Input_String[Index] == '"')
            {
                /* The parameter ended in a quote so invert the flag indicating we
                   are inside of quotes. */
                Inside_Quotes = !Inside_Quotes;
            }

            /* Make sure the parameter string is NULL terminated. */
            if(QCLI_Context.Input_String[Index] != '\0')
            {
                QCLI_Context.Input_String[Index] = '\0';
                Index++;
            }

            /* Try to convert the command to an integer. */
            QCLI_Context.Parameter_List[Parameter_Count].Integer_Is_Valid = String_To_Integer(QCLI_Context.Parameter_List[Parameter_Count].String_Value, &(QCLI_Context.Parameter_List[Parameter_Count].Integer_Value));

            Parameter_Count++;
        }
    }

    /* Make sure any quotes were properly terminated. */
    if(Inside_Quotes)
    {
        QCLI_Printf("\" not terminated\n");
        Result = false;
    }

    if((Result) && (Parameter_Count > 0))
    {
        /* Initialize the find results to the current group state so that it can
           be used to recursively search the groups. */
        Find_Result.Data.Group_List_Entry = QCLI_Context.Current_Group;
        Find_Result.Is_Group              = true;
        Index                             = 0;
        QCLI_Context.Executing_Group      = QCLI_Context.Current_Group;

        /* Search for the command that was entered. Note that if the command or
           group is found, the index will actually indicate the first parameter
           for the command. */
        while((Result) && (Find_Result.Is_Group) && (Index < Parameter_Count))
        {
            Result = Find_Command(Find_Result.Data.Group_List_Entry, &(QCLI_Context.Parameter_List[Index]), &Find_Result);

            /* If navigating into a subgroup, update the executing group. */
            if(Find_Result.Is_Group)
            {
                QCLI_Context.Executing_Group = Find_Result.Data.Group_List_Entry;
            }

            Index ++;
        }

        if(Result)
        {
            if(Find_Result.Is_Group)
            {
                /* Final command is a group, navigate into it. */
                QCLI_Context.Current_Group = Find_Result.Data.Group_List_Entry;
            }
            else
            {
                /* Execute the command. */
                Execute_Command(QCLI_Context.Parameter_List[Index - 1].Integer_Value, Find_Result.Data.Command, Parameter_Count - Index, (Parameter_Count > Index) ? &(QCLI_Context.Parameter_List[Index]) : NULL);
            }
        }
        else
        {
            QCLI_Printf("Command \"%s", QCLI_Context.Parameter_List[0].String_Value);

            for(Command_Index = 1; Command_Index < Index; Command_Index ++)
            {
                QCLI_Printf(" %s", QCLI_Context.Parameter_List[Command_Index].String_Value);
            }

            QCLI_Printf("\" not found.\n");
        }
    }
}

/**
  @brief This function will unregister the specified group from the
  command list and recursively unregister any subgroup's that are
  registered for the group.

  @param Group_List_Entry is a pointer to the comand group to be removed.

  @return
  - true if the current group changed as a result of the group being
  unregistered.
  - false if the current group didn't change.
  */
static qbool_t Unregister_Command_Group(Group_List_Entry_t *Group_List_Entry)
{
    qbool_t             Ret_Val;
    Group_List_Entry_t *Current_Entry;
    qbool_t             Group_Is_Valid;

    /* First, remove the group from its parent's list. */
    if(Group_List_Entry->Parent_Group->Subgroup_List == Group_List_Entry)
    {
        /* Group is at the head of the subgroup list. */
        Group_List_Entry->Parent_Group->Subgroup_List = Group_List_Entry->Next_Group_List_Entry;
        Group_Is_Valid = true;
    }
    else
    {
        /* Find the entry in its parent's subgroup list. */
        Current_Entry = Group_List_Entry->Parent_Group->Subgroup_List;

        while((Current_Entry != NULL) && (Current_Entry->Next_Group_List_Entry != Group_List_Entry))
        {
            Current_Entry = Current_Entry->Next_Group_List_Entry;
        }

        if(Current_Entry != NULL)
        {
            Current_Entry->Next_Group_List_Entry = Group_List_Entry->Next_Group_List_Entry;

            Group_Is_Valid = true;
        }
        else
        {
            Group_Is_Valid = false;
        }
    }

    if(Group_Is_Valid)
    {
        /* Unregsiter any subgroups of the command. */
        Current_Entry = Group_List_Entry->Subgroup_List;
        Ret_Val       = false;

        while(Current_Entry != NULL)
        {
            if(Unregister_Command_Group(Current_Entry))
            {
                Ret_Val = true;
            }
        }

        /* If this is the current group, move up to its parent. */
        if(QCLI_Context.Current_Group == Group_List_Entry)
        {
            QCLI_Context.Current_Group = Group_List_Entry->Parent_Group;
            Ret_Val                    = true;
        }

        /* Free the resources for the group. */
        free(Group_List_Entry);
    }
    else
    {
        Ret_Val = false;
    }

    return(Ret_Val);
}

/**
  @brief This function is used to initialize the QCLI module.

  This function must be called before any other QCLI functions.

  @return
  - true if QCLI was initialized successfully.
  - false if initialization failed.
  */
qbool_t QCLI_Initialize(void)
{
    /* Initialize the context information. */
    memset(&QCLI_Context, 0, sizeof(QCLI_Context));
    QCLI_Context.Current_Group = &(QCLI_Context.Root_Group);

    /* Attempt to create a mutex for the QCLI module. */
    qurt_mutex_init(&(QCLI_Context.CLI_Mutex));

    /* Initialize the thread ready event. */
    qurt_signal_init(&QCLI_Context.Thread_Info.Thread_Ready_Event);

    return(true);
}

void QCLI_Set_DataMode(uint32_t enable, uint32_t len)
{
    LOG_AT_OK();
    data_mode = enable;
    data_send_len = len;
}

static void Process_Data(void)
{
    if (qc_api_net_SendData(QCLI_Context.Input_String, data_send_len))
    {
        LOG_ERR("ERROR: Send\r\n");
    }
    else
    {
        LOG_INFO("OK: Send\r\n");
    }
    QCLI_Set_DataMode(0, 0);
}

/**
  @brief This function passes characters input from the command line to
  the QCLI module for processing.

  @param Length is the number of bytes in the provided buffer.
  @param Buffer is a pointer to the buffer containing the inputted data.

  @return
  - true if QCLI was initialized successfully.
  - false if initialization failed.
  */
void QCLI_Process_Input_Data(uint32_t Length, char *Buffer)
{
    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        if((Length) && (Buffer))
        {
            /* Process all received data. */
            while(Length)
            {

                if (data_mode)
                {
                    if(QCLI_Context.Input_Length < MAXIMUM_QCLI_COMMAND_STRING_LENGTH)
                    {
                        QCLI_Context.Input_String[QCLI_Context.Input_Length] = Buffer[0];
                        QCLI_Context.Input_Length++;
                    }
                    if (QCLI_Context.Input_Length == data_send_len)
                    {
#if 0
                        Buffer ++;
                        Length --;
                        if (Length)
                        {
                            QCLI_Printf("\r\nERROR:Send\r\n");
                            Length = 0;
                            QCLI_Set_DataMode(0, 0);
                            break;
                        }
                        else
#endif
                        {
                            QCLI_Printf("\r\nProcess Data now\r\n");
                            Process_Data();
                            /* Set the command length back to zero in preparation of the next
                               command and display the prompt. */
                            QCLI_Context.Input_Length = 0;
                            memset(QCLI_Context.Input_String, '\0', sizeof(QCLI_Context.Input_String));
                            QCLI_Display_Prompt();
                            Length = 0;
                            break;
                        }
                    }
                }
                /* Check for an end of line character. */
                else if(Buffer[0] == PAL_INPUT_END_OF_LINE_CHARACTER)
                {
#if ECHO_CHARACTERS

                    PAL_Console_Write(sizeof(PAL_OUTPUT_END_OF_LINE_STRING), PAL_OUTPUT_END_OF_LINE_STRING);

#endif

                    /* Command is complete, process it now. */
                    Process_Command();

                    /* Set the command length back to zero in preparation of the next
                       command and display the prompt. */
                    QCLI_Context.Input_Length = 0;
                    memset(QCLI_Context.Input_String, '\0', sizeof(QCLI_Context.Input_String));
                    QCLI_Display_Prompt();
                }
                else
                {
                    /* Check for backspace character. */
                    if(Buffer[0] == '\b')
                    {
                        /* Consume a character from the command if one has been
                           entered. */
                        if(QCLI_Context.Input_Length)
                        {
#if ECHO_CHARACTERS

                            PAL_Console_Write(3, "\b \b");

#endif

                            QCLI_Context.Input_Length --;
                            QCLI_Context.Input_String[QCLI_Context.Input_Length] = '\0';
                        }
                    }
                    else
                    {
                        /* Check for a valid character, which here is any non control
                           code lower ASCII (0x20 ' ' to 0x7E '~'). */
                        if((*Buffer >= ' ') && (*Buffer <= '~'))
                        {
                            /* Make sure that the command buffer can fit the character. */
                            if(QCLI_Context.Input_Length < MAXIMUM_QCLI_COMMAND_STRING_LENGTH)
                            {
#if ECHO_CHARACTERS

                                PAL_Console_Write(1, Buffer);

#endif

                                QCLI_Context.Input_String[QCLI_Context.Input_Length] = Buffer[0];
                                QCLI_Context.Input_Length++;
                            }
                        }
                    }
                }

                /* Move to the next character in the buffer. */
                Buffer ++;
                Length --;
            }
        }

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }
}

/**
   @brief This function displays the current command list.

   It is intended to provide a means for the initial command list to be
   displayed once platform initialization is complete.
*/
void QCLI_Display_Command_List(void)
{
    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        Display_Command_List(QCLI_Context.Current_Group);

        QCLI_Display_Prompt();

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }
}

/**
  @brief This function is used to register a command group with the CLI.

  @param Parent_Group is the group which this group should be registerd
  under as a subgroup.  If this parameter is NULL, then the group
  will be registered at the top level.
  @param Command_Group is the command group to be registered.  Note that
  this function assumes the command group information will be
  constant and simply stores a pointer to the data.  If the command
  group and its associated information is not constant, its memory
  MUST be retained until the command is unregistered.

  @return
  - THe handle for the group that was added.
  - NULL if there was an error registering the group.
  */
QCLI_Group_Handle_t QCLI_Register_Command_Group(QCLI_Group_Handle_t Parent_Group, const QCLI_Command_Group_t *Command_Group)
{
    Group_List_Entry_t *New_Entry;
    Group_List_Entry_t *Current_Entry;

    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        /* Create the new entry. */
        New_Entry = (Group_List_Entry_t *)malloc(sizeof(Group_List_Entry_t));
        if(New_Entry)
        {
            New_Entry->Command_Group         = Command_Group;
            New_Entry->Next_Group_List_Entry = NULL;
            New_Entry->Subgroup_List         = NULL;

            if(Parent_Group == NULL)
            {
                New_Entry->Parent_Group = &(QCLI_Context.Root_Group);
            }
            else
            {
                New_Entry->Parent_Group = (Group_List_Entry_t *)Parent_Group;
            }
        }

        /* Add the new entry to its parents subgroup list. */
        if(New_Entry->Parent_Group->Subgroup_List == NULL)
        {
            New_Entry->Parent_Group->Subgroup_List = New_Entry;
        }
        else
        {
            Current_Entry = New_Entry->Parent_Group->Subgroup_List;
            while(Current_Entry->Next_Group_List_Entry != NULL)
            {
                Current_Entry = Current_Entry->Next_Group_List_Entry;
            }

            Current_Entry->Next_Group_List_Entry = New_Entry;
        }

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }
    else
    {
        New_Entry = NULL;
    }

    return((QCLI_Group_Handle_t)New_Entry);
}

/**
  @brief This function is used to usregister a command group from the CLI.

  @param Group_Handle is the handle for the group to be unregistered.
  This will be the value returned form
  QCLI_Register_Command_Group() when the function was registered.
  Note that if the specified group has subgroups, they will be
  unregistred as well.
  */
void QCLI_Unregister_Command_Group(QCLI_Group_Handle_t Group_Handle)
{
    if(Group_Handle != NULL)
    {
        if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
        {
            if(Unregister_Command_Group((Group_List_Entry_t *)Group_Handle))
            {
                /* The current menu level changed so update the prompt. */
                QCLI_Display_Prompt();
            }

            RELEASE_LOCK(QCLI_Context.CLI_Mutex);
        }
    }
}

/**
  @brief This function prints the prompt to the console.

  This provides a means to re-display the prompt after printing data to
  the console from an asynchronous function such as a callback or seperate
  command thread.
  */
void QCLI_Display_Prompt(void)
{
    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        QCLI_Printf("\n");

        /* Recursively display the name for the current group. */
        if(QCLI_Context.Current_Group != &(QCLI_Context.Root_Group))
        {
            Display_Group_Name(QCLI_Context.Current_Group);
        }

        QCLI_Printf("> ");

        /* Display the current command string. */
        if(QCLI_Context.Input_Length != 0)
        {
            PAL_Console_Write(QCLI_Context.Input_Length, QCLI_Context.Input_String);
        }

        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }
}


/**
  @brief This function prints a formated string to the CLI.

  Note that this function will also replace newline characters ('\n') with
  the string specified by PAL_OUTPUT_END_OF_LINE_STRING.

  @param Format is the formatted string to be printed.
  @param ... is the variatic parameter for the format string.
  */
void QCLI_Printf(const char *Format, ...)
{
    uint32_t            Index;
    uint32_t            Next_Print_Index;
    uint32_t            Length;
    va_list             Arg_List;

    if (!data_mode)
    {
        if((Format != NULL))
        {
            if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
            {
                /* Print the string to the buffer. */
                va_start(Arg_List, Format);
                Length = vsnprintf((char *)(QCLI_Context.Printf_Buffer), sizeof(QCLI_Context.Printf_Buffer), (char *)Format, Arg_List);
                va_end(Arg_List);

                /* Make sure the length is not greater than the buffer size (taking the
                   NULL terminator into account). */
                if(Length > sizeof(QCLI_Context.Printf_Buffer) - 1)
                {
                    Length = sizeof(QCLI_Context.Printf_Buffer) - 1;
                }

                /* Write the buffer to the console, setting EOL characters accordingly. */
                Next_Print_Index = 0;
                for(Index = 0; Index < Length; Index ++)
                {
                    if(QCLI_Context.Printf_Buffer[Index] == '\n')
                    {
                        /* Print out the buffer so far and replace the '\n' with the
                           configured EOL string. */
                        if(Index != Next_Print_Index)
                        {
                            PAL_Console_Write(Index - Next_Print_Index, &(QCLI_Context.Printf_Buffer[Next_Print_Index]));
                        }

                        PAL_Console_Write(sizeof(PAL_OUTPUT_END_OF_LINE_STRING) - 1, PAL_OUTPUT_END_OF_LINE_STRING);

                        Next_Print_Index = Index + 1;
                        QCLI_Context.Printf_New_Line = false;
                    }
                    else
                    {
                        QCLI_Context.Printf_New_Line = true;
                    }
                }

                /* Print the remaining buffer after the last newline. */
                if(Length != Next_Print_Index)
                {
                    PAL_Console_Write(Length - Next_Print_Index, &(QCLI_Context.Printf_Buffer[Next_Print_Index]));
                }

                RELEASE_LOCK(QCLI_Context.CLI_Mutex);
            }
        }
    }
}
#if 0
void QCLI_Printf(QCLI_Group_Handle_t Group_Handle, const char *Format, ...)
{
    uint32_t            Index;
    uint32_t            Next_Print_Index;
    uint32_t            Length;
    va_list             Arg_List;
    Group_List_Entry_t *Group_List_Entry;

    if((Group_Handle != NULL) && (Format != NULL))
    {
        if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
        {
            Group_List_Entry = (Group_List_Entry_t *)Group_Handle;

            /* Print the group name first. Note that the main handle indicates the
               message is from the QCLI itself and as such doesn't print a group
               name. */
            if(Group_Handle != QCLI_Context.Current_Printf_Group)
            {
                if(QCLI_Context.Printf_New_Line)
                {
                    PAL_Console_Write(sizeof(PAL_OUTPUT_END_OF_LINE_STRING) - 1, PAL_OUTPUT_END_OF_LINE_STRING);
                    QCLI_Context.Printf_New_Line = false;
                }

                if((Group_Handle != MAIN_PRINTF_HANDLE) && (Format[0] != '\n'))
                {
                    Display_Group_Name(Group_List_Entry);
                    PAL_Console_Write(2, ": ");
                }
            }

            QCLI_Context.Current_Printf_Group = Group_Handle;

            /* Print the string to the buffer. */
            va_start(Arg_List, Format);
            Length = vsnprintf((char *)(QCLI_Context.Printf_Buffer), sizeof(QCLI_Context.Printf_Buffer), (char *)Format, Arg_List);
            va_end(Arg_List);

            /* Make sure the length is not greater than the buffer size (taking the
               NULL terminator into account). */
            if(Length > sizeof(QCLI_Context.Printf_Buffer) - 1)
            {
                Length = sizeof(QCLI_Context.Printf_Buffer) - 1;
            }

            /* Write the buffer to the console, setting EOL characters accordingly. */
            Next_Print_Index = 0;
            for(Index = 0; Index < Length; Index ++)
            {
                if(QCLI_Context.Printf_Buffer[Index] == '\n')
                {
                    /* Print out the buffer so far and replace the '\n' with the
                       configured EOL string. */
                    if(Index != Next_Print_Index)
                    {
                        PAL_Console_Write(Index - Next_Print_Index, &(QCLI_Context.Printf_Buffer[Next_Print_Index]));
                    }

                    PAL_Console_Write(sizeof(PAL_OUTPUT_END_OF_LINE_STRING) - 1, PAL_OUTPUT_END_OF_LINE_STRING);

                    Next_Print_Index = Index + 1;

                    if(Length != (Index + 1))
                    {
                        /* Redsiplay the group name at the start of a new line if its
                           not immidiately succeeded by another new line. */
                        if(QCLI_Context.Printf_Buffer[Index + 1] != '\n')
                        {
                            if(Group_List_Entry && Group_List_Entry->Command_Group)
                            {
                                PAL_Console_Write(strlen((char *)(Group_List_Entry->Command_Group->Group_String)), Group_List_Entry->Command_Group->Group_String);
                                PAL_Console_Write(2, ": ");
                            }
                        }
                    }
                    else
                    {
                        /* This printout stopped on the newline so set the current
                           print group to the main group to prompt the next line to
                           redisplay the group name. */
                        QCLI_Context.Current_Printf_Group = MAIN_PRINTF_HANDLE;
                    }

                    QCLI_Context.Printf_New_Line = false;
                }
                else
                {
                    QCLI_Context.Printf_New_Line = true;
                }
            }

            /* Print the remaining buffer after the last newline. */
            if(Length != Next_Print_Index)
            {
                PAL_Console_Write(Length - Next_Print_Index, &(QCLI_Context.Printf_Buffer[Next_Print_Index]));
            }

            RELEASE_LOCK(QCLI_Context.CLI_Mutex);
        }
    }
}
#endif

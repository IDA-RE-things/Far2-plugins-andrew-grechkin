 /*
   HEADER SECTION
  */



 /*
    MESSAGE DEFINITION SECTION
  */

 /* Categories */

//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_STUBS                   0x3
#define FACILITY_RUNTUME                 0x2
#define FACILITY_IO_ERROR_CODE           0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: EV_CATEGORY_1
//
// MessageText:
//
// Category 1
//
#define EV_CATEGORY_1                    ((WORD)0x00000001L)

 /* Messages */
//
// MessageId: EV_MSG_STRING
//
// MessageText:
//
// %1
//
#define EV_MSG_STRING                    ((DWORD)0x00020100L)

//
// MessageId: EV_MSG_SUCCESS
//
// MessageText:
//
// String_EV_MSG_SUCCESS %1
//
#define EV_MSG_SUCCESS                   ((DWORD)0x00020200L)

//
// MessageId: EV_MSG_INFO
//
// MessageText:
//
// String_EV_MSG_INFO %1
//
#define EV_MSG_INFO                      ((DWORD)0x40020300L)

//
// MessageId: EV_MSG_WARNING
//
// MessageText:
//
// String_EV_MSG_WARNING %1
//
#define EV_MSG_WARNING                   ((DWORD)0x80020400L)

//
// MessageId: EV_MSG_ERROR
//
// MessageText:
//
// String_EV_MSG_ERROR %1
//
#define EV_MSG_ERROR                     ((DWORD)0xC0020500L)

 /* Insert string parameters */
//
// MessageId: PARAM_1
//
// MessageText:
//
// Parameter1
//
#define PARAM_1                          ((DWORD)0x000003E8L)


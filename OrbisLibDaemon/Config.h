#pragma once

#define API_PORT 6900
#define EVENT_PORT 6901

enum APICommands
{
	/* ####### Proc functions ####### */
	PROC_START = 1,

	API_PROC_GET_LIST,
	API_PROC_LOAD_ELF,
	API_PROC_CALL, /* RPC Call. */

	PROC_END,
	/* ############################## */

	/* ####### Apps functions ####### */
	APP_START,

	API_APPS_CHECK_VER,
	API_APPS_GET_DB,
	API_APPS_GET_INFO_STR,
	API_APPS_STATUS,
	API_APPS_START,
	API_APPS_STOP,
	API_APPS_SUSPEND,
	API_APPS_RESUME,
	API_APPS_DELETE,
	API_APPS_SET_VISIBILITY,
	API_APPS_GET_VISIBILITY,

	APP_END,
	/* ############################## */

	/* ##### Debugger functions ##### */
	DBG_START,

	API_DBG_ATTACH, /* Debugger attach to target */
	API_DBG_DETACH, /* Debugger detach from target */
	API_DBG_GET_CURRENT,
	API_DBG_READ,
	API_DBG_WRITE,
	API_DBG_KILL,
	API_DBG_BREAK,
	API_DBG_RESUME,
	API_DBG_SIGNAL,
	API_DBG_STEP,
	API_DBG_STEP_OVER,
	API_DBG_STEP_OUT,
	API_DBG_GET_CALLSTACK,
	API_DBG_GET_REG,
	API_DBG_SET_REG,
	API_DBG_GET_FREG,
	API_DBG_SET_FREG,
	API_DBG_GET_DBGREG,
	API_DBG_SET_DBGREG,

	/* Remote Library functions */
	API_DBG_LOAD_LIBRARY,
	API_DBG_UNLOAD_LIBRARY,
	API_DBG_RELOAD_LIBRARY,
	API_DBG_LIBRARY_LIST,

	/* Thread Management */
	API_DBG_THREAD_LIST,
	API_DBG_THREAD_STOP,
	API_DBG_THREAD_RESUME,

	/* Breakpoint functions */
	API_DBG_BREAKPOINT_GETFREE,
	API_DBG_BREAKPOINT_SET,
	API_DBG_BREAKPOINT_UPDATE,
	API_DBG_BREAKPOINT_REMOVE,
	API_DBG_BREAKPOINT_GETINFO,
	API_DBG_BREAKPOINT_LIST,

	/* Watchpoint functions */
	API_DBG_WATCHPOINT_SET,
	API_DBG_WATCHPOINT_UPDATE,
	API_DBG_WATCHPOINT_REMOVE,
	API_DBG_WATCHPOINT_GETINFO,
	API_DBG_WATCHPOINT_LIST,

	DBG_END,
	/* ############################## */

	/* ###### Kernel functions ###### */
	KERN_START,

	API_KERN_BASE,
	API_KERN_READ,
	API_KERN_WRITE,

	KERN_END,
	/* ############################## */

	/* ###### Target functions ###### */
	TARGET_START,

	API_TARGET_INFO,
	API_TARGET_RESTMODE,
	API_TARGET_SHUTDOWN,
	API_TARGET_REBOOT,
	API_TARGET_NOTIFY,
	API_TARGET_BUZZER,
	API_TARGET_SET_LED,
	API_TARGET_DUMP_PROC,
	API_TARGET_SET_SETTINGS,
	API_TARGET_GETFILE,

	TARGET_END,
	/* ############################## */
};

constexpr const char* CommandList[] = {
	"Invalid Command",

	/* ####### Proc functions ####### */
	"PROC_START",

	"API_PROC_GET_LIST",
	"API_PROC_LOAD_ELF",
	"API_PROC_CALL", /* RPC Call. */

	"PROC_END",
	/* ############################## */

	/* ####### Apps functions ####### */
	"APP_START",

	"API_APPS_GET_LIST",
	"API_APPS_GET_INFO_STR",
	"API_APPS_STATUS",
	"API_APPS_START",
	"API_APPS_STOP",
	"API_APPS_SUSPEND",
	"API_APPS_RESUME",
	"API_APPS_DELETE",
	"API_APPS_SET_VISIBILITY",
	"API_APPS_GET_VISIBILITY",

	"APP_END",
	/* ############################## */

	/* ##### Debugger functions ##### */
	"DBG_START",

	"API_DBG_ATTACH", /* Debugger attach to target */
	"API_DBG_DETACH", /* Debugger detach from target */
	"API_DBG_GET_CURRENT",
	"API_DBG_READ",
	"API_DBG_WRITE",
	"API_DBG_KILL",
	"API_DBG_BREAK",
	"API_DBG_RESUME",
	"API_DBG_SIGNAL",
	"API_DBG_STEP",
	"API_DBG_STEP_OVER",
	"API_DBG_STEP_OUT",
	"API_DBG_GET_CALLSTACK",
	"API_DBG_GET_REG",
	"API_DBG_SET_REG",
	"API_DBG_GET_FREG",
	"API_DBG_SET_FREG",
	"API_DBG_GET_DBGREG",
	"API_DBG_SET_DBGREG",

	/* Remote Library functions */
	"API_DBG_LOAD_LIBRARY",
	"API_DBG_UNLOAD_LIBRARY",
	"API_DBG_RELOAD_LIBRARY",
	"API_DBG_LIBRARY_LIST",

	/* Thread Management */
	"API_DBG_THREAD_LIST",
	"API_DBG_THREAD_STOP",
	"API_DBG_THREAD_RESUME",

	/* Breakpoint functions */
	"API_DBG_BREAKPOINT_GETFREE",
	"API_DBG_BREAKPOINT_SET",
	"API_DBG_BREAKPOINT_UPDATE",
	"API_DBG_BREAKPOINT_REMOVE",
	"API_DBG_BREAKPOINT_GETINFO",
	"API_DBG_BREAKPOINT_LIST",

	/* Watchpoint functions */
	"API_DBG_WATCHPOINT_SET",
	"API_DBG_WATCHPOINT_UPDATE",
	"API_DBG_WATCHPOINT_REMOVE",
	"API_DBG_WATCHPOINT_GETINFO",
	"API_DBG_WATCHPOINT_LIST",

	"DBG_END",
	/* ############################## */

	/* ###### Kernel functions ###### */
	"KERN_START",

	"API_KERN_BASE",
	"API_KERN_READ",
	"API_KERN_WRITE",

	"KERN_END",
	/* ############################## */

	/* ###### Target functions ###### */
	"TARGET_START",

	"API_TARGET_INFO",
	"API_TARGET_RESTMODE",
	"API_TARGET_SHUTDOWN",
	"API_TARGET_REBOOT",
	"API_TARGET_NOTIFY",
	"API_TARGET_BUZZER",
	"API_TARGET_SET_LED",
	"API_TARGET_DUMP_PROC",
	"API_TARGET_SET_SETTINGS",
	"API_TARGET_GETFILE",

	"TARGET_END",
	/* ############################## */

};

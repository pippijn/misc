/*
 * evdev-key-btn-test
 *
 * Copyright (c) 2005 Brad Hards <bradh@frogmouth.net>
 * Copyright (c) 2005-2007 Andreas Schneider <mail@cynapses.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <linux/input.h>

/* this macro is used to tell if "bit" is set in "array"
 * it selects a byte from the array, and does a boolean AND 
 * operation with a byte that only has the relevant bit set. 
 * eg. to check for the 12th bit, we do (array[1] & 1<<4)
 */
#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))

int main (int argc, char **argv) {
	
	int fd = -1;
	uint8_t key_bitmask[KEY_MAX/8 + 1];
	uint8_t rel_bitmask[REL_MAX/8 + 1];
	int yalv;
	int count = 0;
	
	/* ioctl() requires a file descriptor, so we check we got one, and then open it */
	if (argc != 2) {
		fprintf(stderr, "usage: %s event-device - probably /dev/input/event0\n", argv[0]);
	exit(1);
	}
	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		perror("evdev open");
	exit(1);
	}
	
	memset(key_bitmask, 0, sizeof(key_bitmask));
	if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask) < 0) {
		perror("evdev ioctl");
	}

	memset(rel_bitmask, 0, sizeof(rel_bitmask));
	if (ioctl(fd, EVIOCGBIT(EV_REL, sizeof(rel_bitmask)), rel_bitmask) < 0) {
		perror("evdev ioctl");
	}
	
	printf("Supported Keys:\n");
	
	for (yalv = 0; yalv < KEY_MAX; yalv++) {
		if (test_bit(yalv, key_bitmask)) {
			/* this means that the bit is set in the key list */
			printf("  Key  0x%02x ", yalv);
			switch (yalv)
			{
				case KEY_RESERVED : printf(" (Reserved)\n"); count++; break;
				case KEY_ESC : printf(" (Escape)\n"); count++; break;
				case KEY_1 : printf(" (1)\n"); count++; break;
				case KEY_2 : printf(" (2)\n"); count++; break;
				case KEY_3 : printf(" (3)\n"); count++; break;
				case KEY_4 : printf(" (4)\n"); count++; break;
				case KEY_5 : printf(" (5)\n"); count++; break;
				case KEY_6 : printf(" (6)\n"); count++; break;
				case KEY_7 : printf(" (7)\n"); count++; break;
				case KEY_8 : printf(" (8)\n"); count++; break;
				case KEY_9 : printf(" (9)\n"); count++; break;
				case KEY_0 : printf(" (0)\n"); count++; break;
				case KEY_MINUS : printf(" (-)\n"); count++; break;
				case KEY_EQUAL : printf(" (=)\n"); count++; break;
				case KEY_BACKSPACE : printf(" (Backspace)\n"); count++; break;
				case KEY_TAB : printf(" (Tab)\n"); count++; break;
				case KEY_Q : printf(" (Q)\n"); count++; break;
				case KEY_W : printf(" (W)\n"); count++; break;
				case KEY_E : printf(" (E)\n"); count++; break;
				case KEY_R : printf(" (R)\n"); count++; break;
				case KEY_T : printf(" (T)\n"); count++; break;
				case KEY_Y : printf(" (Y)\n"); count++; break;
				case KEY_U : printf(" (U)\n"); count++; break;
				case KEY_I : printf(" (I)\n"); count++; break;
				case KEY_O : printf(" (O)\n"); count++; break;
				case KEY_P : printf(" (P)\n"); count++; break;
				case KEY_LEFTBRACE : printf(" ([)\n"); count++; break;
				case KEY_RIGHTBRACE : printf(" (])\n"); count++; break;
				case KEY_ENTER : printf(" (Enter)\n"); count++; break;
				case KEY_LEFTCTRL : printf(" (LH Control)\n"); count++; break;
				case KEY_A : printf(" (A)\n"); count++; break;
				case KEY_S : printf(" (S)\n"); count++; break;
				case KEY_D : printf(" (D)\n"); count++; break;
				case KEY_F : printf(" (F)\n"); count++; break;
				case KEY_G : printf(" (G)\n"); count++; break;
				case KEY_H : printf(" (H)\n"); count++; break;
				case KEY_J : printf(" (J)\n"); count++; break;
				case KEY_K : printf(" (K)\n"); count++; break;
				case KEY_L : printf(" (L)\n"); count++; break;
				case KEY_SEMICOLON : printf(" (;)\n"); count++; break;
				case KEY_APOSTROPHE : printf(" (')\n"); count++; break;
				case KEY_GRAVE : printf(" (`)\n"); count++; break;
				case KEY_LEFTSHIFT : printf(" (LH Shift)\n"); count++; break;
				case KEY_BACKSLASH : printf(" (\\)\n"); count++; break;
				case KEY_Z : printf(" (Z)\n"); count++; break;
				case KEY_X : printf(" (X)\n"); count++; break;
				case KEY_C : printf(" (C)\n"); count++; break;
				case KEY_V : printf(" (V)\n"); count++; break;
				case KEY_B : printf(" (B)\n"); count++; break;
				case KEY_N : printf(" (N)\n"); count++; break;
				case KEY_M : printf(" (M)\n"); count++; break;
				case KEY_COMMA : printf(" (,)\n"); count++; break;
				case KEY_DOT : printf(" (.)\n"); count++; break;
				case KEY_SLASH : printf(" (/)\n"); count++; break;
				case KEY_RIGHTSHIFT : printf(" (RH Shift)\n"); count++; break;
				case KEY_KPASTERISK : printf(" (*)\n"); count++; break;
				case KEY_LEFTALT : printf(" (LH Alt)\n"); count++; break;
				case KEY_SPACE : printf(" (Space)\n"); count++; break;
				case KEY_CAPSLOCK : printf(" (CapsLock)\n"); count++; break;
				case KEY_F1 : printf(" (F1)\n"); count++; break;
				case KEY_F2 : printf(" (F2)\n"); count++; break;
				case KEY_F3 : printf(" (F3)\n"); count++; break;
				case KEY_F4 : printf(" (F4)\n"); count++; break;
				case KEY_F5 : printf(" (F5)\n"); count++; break;
				case KEY_F6 : printf(" (F6)\n"); count++; break;
				case KEY_F7 : printf(" (F7)\n"); count++; break;
				case KEY_F8 : printf(" (F8)\n"); count++; break;
				case KEY_F9 : printf(" (F9)\n"); count++; break;
				case KEY_F10 : printf(" (F10)\n"); count++; break;
				case KEY_NUMLOCK : printf(" (NumLock)\n"); count++; break;
				case KEY_SCROLLLOCK : printf(" (ScrollLock)\n"); count++; break;
				case KEY_KP7 : printf(" (KeyPad 7)\n"); count++; break;
				case KEY_KP8 : printf(" (KeyPad 8)\n"); count++; break;
				case KEY_KP9 : printf(" (Keypad 9)\n"); count++; break;
				case KEY_KPMINUS : printf(" (KeyPad Minus)\n"); count++; break;
				case KEY_KP4 : printf(" (KeyPad 4)\n"); count++; break;
				case KEY_KP5 : printf(" (KeyPad 5)\n"); count++; break;
				case KEY_KP6 : printf(" (KeyPad 6)\n"); count++; break;
				case KEY_KPPLUS : printf(" (KeyPad Plus)\n"); count++; break;
				case KEY_KP1 : printf(" (KeyPad 1)\n"); count++; break;
				case KEY_KP2 : printf(" (KeyPad 2)\n"); count++; break;
				case KEY_KP3 : printf(" (KeyPad 3)\n"); count++; break;
				case KEY_KPDOT : printf(" (KeyPad decimal point)\n"); count++; break;
				case KEY_F13 : printf(" (F13)\n"); count++; break;
				case KEY_102ND : printf(" (Beats me...)\n"); count++; break;
				case KEY_F11 : printf(" (F11)\n"); count++; break;
				case KEY_F12 : printf(" (F12)\n"); count++; break;
				case KEY_F14 : printf(" (F14)\n"); count++; break;
				case KEY_F15 : printf(" (F15)\n"); count++; break;
				case KEY_F16 : printf(" (F16)\n"); count++; break;
				case KEY_F17 : printf(" (F17)\n"); count++; break;
				case KEY_F18 : printf(" (F18)\n"); count++; break;
				case KEY_F19 : printf(" (F19)\n"); count++; break;
				case KEY_F20 : printf(" (F20)\n"); count++; break;
				case KEY_KPENTER : printf(" (Keypad Enter)\n"); count++; break;
				case KEY_RIGHTCTRL : printf(" (RH Control)\n"); count++; break;
				case KEY_KPSLASH : printf(" (KeyPad Forward Slash)\n"); count++; break;
				case KEY_SYSRQ : printf(" (System Request)\n"); count++; break;
				case KEY_RIGHTALT : printf(" (RH Alternate)\n"); count++; break;
				case KEY_LINEFEED : printf(" (Line Feed)\n"); count++; break;
				case KEY_HOME : printf(" (Home)\n"); count++; break;
				case KEY_UP : printf(" (Up)\n"); count++; break;
				case KEY_PAGEUP : printf(" (Page Up)\n"); count++; break;
				case KEY_LEFT : printf(" (Left)\n"); count++; break;
				case KEY_RIGHT : printf(" (Right)\n"); count++; break;
				case KEY_END : printf(" (End)\n"); count++; break;
				case KEY_DOWN : printf(" (Down)\n"); count++; break;
				case KEY_PAGEDOWN : printf(" (Page Down)\n"); count++; break;
				case KEY_INSERT : printf(" (Insert)\n"); count++; break;
				case KEY_DELETE : printf(" (Delete)\n"); count++; break;
				case KEY_MACRO : printf(" (Macro)\n"); count++; break;
				case KEY_MUTE : printf(" (Mute)\n"); count++; break;
				case KEY_VOLUMEDOWN : printf(" (Volume Down)\n"); count++; break;
				case KEY_VOLUMEUP : printf(" (Volume Up)\n"); count++; break;
				case KEY_POWER : printf(" (Power)\n"); count++; break;
				case KEY_KPEQUAL : printf(" (KeyPad Equal)\n"); count++; break;
				case KEY_KPPLUSMINUS : printf(" (KeyPad +/-)\n"); count++; break;
				case KEY_PAUSE : printf(" (Pause)\n"); count++; break;
				case KEY_F21 : printf(" (F21)\n"); count++; break;
				case KEY_F22 : printf(" (F22)\n"); count++; break;
				case KEY_F23 : printf(" (F23)\n"); count++; break;
				case KEY_F24 : printf(" (F24)\n"); count++; break;
				case KEY_KPCOMMA : printf(" (KeyPad comma)\n"); count++; break;
				case KEY_LEFTMETA : printf(" (LH Meta)\n"); count++; break;
				case KEY_RIGHTMETA : printf(" (RH Meta)\n"); count++; break;
				case KEY_COMPOSE : printf(" (Compose)\n"); count++; break;
				case KEY_STOP : printf(" (Stop)\n"); count++; break;
				case KEY_AGAIN : printf(" (Again)\n"); count++; break;
				case KEY_PROPS : printf(" (Properties)\n"); count++; break;
				case KEY_UNDO : printf(" (Undo)\n"); count++; break;
				case KEY_FRONT : printf(" (Front)\n"); count++; break;
				case KEY_COPY : printf(" (Copy)\n"); count++; break;
				case KEY_OPEN : printf(" (Open)\n"); count++; break;
				case KEY_PASTE : printf(" (Paste)\n"); count++; break;
				case KEY_FIND : printf(" (Find)\n"); count++; break;
				case KEY_CUT : printf(" (Cut)\n"); count++; break;
				case KEY_HELP : printf(" (Help)\n"); count++; break;
				case KEY_MENU : printf(" (Menu)\n"); count++; break;
				case KEY_CALC : printf(" (Calculator)\n"); count++; break;
				case KEY_SETUP : printf(" (Setup)\n"); count++; break;
				case KEY_SLEEP : printf(" (Sleep)\n"); count++; break;
				case KEY_WAKEUP : printf(" (Wakeup)\n"); count++; break;
				case KEY_FILE : printf(" (File)\n"); count++; break;
				case KEY_SENDFILE : printf(" (Send File)\n"); count++; break;
				case KEY_DELETEFILE : printf(" (Delete File)\n"); count++; break;
				case KEY_XFER : printf(" (Transfer)\n"); count++; break;
				case KEY_PROG1 : printf(" (Program 1)\n"); count++; break;
				case KEY_PROG2 : printf(" (Program 2)\n"); count++; break;
				case KEY_WWW : printf(" (Web Browser)\n"); count++; break;
				case KEY_MSDOS : printf(" (DOS mode)\n"); count++; break;
				case KEY_COFFEE : printf(" (Coffee)\n"); count++; break;
				case KEY_DIRECTION : printf(" (Direction)\n"); count++; break;
				case KEY_CYCLEWINDOWS : printf(" (Window cycle)\n"); count++; break;
				case KEY_MAIL : printf(" (Mail)\n"); count++; break;
				case KEY_BOOKMARKS : printf(" (Book Marks)\n"); count++; break;
				case KEY_COMPUTER : printf(" (Computer)\n"); count++; break;
				case KEY_BACK : printf(" (Back)\n"); count++; break;
				case KEY_FORWARD : printf(" (Forward)\n"); count++; break;
				case KEY_CLOSECD : printf(" (Close CD)\n"); count++; break;
				case KEY_EJECTCD : printf(" (Eject CD)\n"); count++; break;
				case KEY_EJECTCLOSECD : printf(" (Eject / Close CD)\n"); count++; break;
				case KEY_NEXTSONG : printf(" (Next Song)\n"); count++; break;
				case KEY_PLAYPAUSE : printf(" (Play and Pause)\n"); count++; break;
				case KEY_PREVIOUSSONG : printf(" (Previous Song)\n"); count++; break;
				case KEY_STOPCD : printf(" (Stop CD)\n"); count++; break;
				case KEY_RECORD : printf(" (Record)\n"); count++; break;
				case KEY_REWIND : printf(" (Rewind)\n"); count++; break;
				case KEY_PHONE : printf(" (Phone)\n"); count++; break;
				case KEY_ISO : printf(" (ISO)\n"); count++; break;
				case KEY_CONFIG : printf(" (Config)\n"); count++; break;
				case KEY_HOMEPAGE : printf(" (Home)\n"); count++; break;
				case KEY_REFRESH : printf(" (Refresh)\n"); count++; break;
				case KEY_EXIT : printf(" (Exit)\n"); count++; break;
				case KEY_MOVE : printf(" (Move)\n"); count++; break;
				case KEY_EDIT : printf(" (Edit)\n"); count++; break;
				case KEY_SCROLLUP : printf(" (Scroll Up)\n"); count++; break;
				case KEY_SCROLLDOWN : printf(" (Scroll Down)\n"); count++; break;
				case KEY_KPLEFTPAREN : printf(" (KeyPad LH parenthesis)\n"); count++; break;
				case KEY_KPRIGHTPAREN : printf(" (KeyPad RH parenthesis)\n"); count++; break;
				case KEY_PLAYCD : printf(" (Play CD)\n"); count++; break;
				case KEY_PAUSECD : printf(" (Pause CD)\n"); count++; break;
				case KEY_PROG3 : printf(" (Program 3)\n"); count++; break;
				case KEY_PROG4 : printf(" (Program 4)\n"); count++; break;
				case KEY_SUSPEND : printf(" (Suspend)\n"); count++; break;
				case KEY_CLOSE : printf(" (Close)\n"); count++; break;
				case KEY_UNKNOWN : printf(" (Specifically unknown)\n"); count++; break;
				case KEY_BRIGHTNESSDOWN : printf(" (Brightness Down)\n"); count++; break;
				case KEY_BRIGHTNESSUP : printf(" (Brightness Up)\n"); count++; break;

				case KEY_MEDIA : printf(" (Media)\n"); count++; break;
				case KEY_OK : printf(" (Ok Key)\n"); count++; break;
				case KEY_SELECT : printf(" (Select)\n"); count++; break;
				case KEY_GOTO : printf(" (Goto)\n"); count++; break;
				case KEY_CLEAR : printf(" (Clear)\n"); count++; break;
				case KEY_POWER2 : printf(" (Power2)\n"); count++; break;
				case KEY_OPTION : printf(" (Option)\n"); count++; break;
				case KEY_INFO : printf(" (Info)\n"); count++; break;
				case KEY_TIME : printf(" (Time)\n"); count++; break;
				case KEY_VENDOR : printf(" (Vendor)\n"); count++; break;
				case KEY_ARCHIVE : printf(" (Archive)\n"); count++; break;
				case KEY_PROGRAM : printf(" (Program)\n"); count++; break;
				case KEY_CHANNEL : printf(" (Channel)\n"); count++; break;
				case KEY_FAVORITES : printf(" (Favorites)\n"); count++; break;
				case KEY_EPG : printf(" (EPG)\n"); count++; break;
				case KEY_PVR : printf(" (PVR)\n"); count++; break;
				case KEY_MHP : printf(" (MHP)\n"); count++; break;
				case KEY_LANGUAGE : printf(" (Language)\n"); count++; break;
				case KEY_TITLE : printf(" (Title)\n"); count++; break;
				case KEY_SUBTITLE : printf(" (Subtitle)\n"); count++; break;
				case KEY_ANGLE : printf(" (Angle)\n"); count++; break;
				case KEY_ZOOM : printf(" (Zoom)\n"); count++; break;
				case KEY_MODE : printf(" (Mode)\n"); count++; break;
				case KEY_KEYBOARD : printf(" (Keyboard)\n"); count++; break;
				case KEY_SCREEN : printf(" (Screen)\n"); count++; break;
				case KEY_PC : printf(" (PC)\n"); count++; break;
				case KEY_TV : printf(" (TV)\n"); count++; break;
				case KEY_TV2 : printf(" (TV2)\n"); count++; break;
				case KEY_VCR : printf(" (VCR)\n"); count++; break;
				case KEY_VCR2 : printf(" (VCR2)\n"); count++; break;
				case KEY_SAT : printf(" (SAT)\n"); count++; break;
				case KEY_SAT2 : printf(" (SAT2)\n"); count++; break;
				case KEY_CD : printf(" (CD)\n"); count++; break;
				case KEY_TAPE : printf(" (Tape)\n"); count++; break;
				case KEY_RADIO : printf(" (Radio)\n"); count++; break;
				case KEY_TUNER : printf(" (TUNER)\n"); count++; break;
				case KEY_PLAYER : printf(" (Player)\n"); count++; break;
				case KEY_TEXT : printf(" (Text)\n"); count++; break;
				case KEY_DVD : printf(" (DVD)\n"); count++; break;
				case KEY_AUX : printf(" (AUX)\n"); count++; break;
				case KEY_MP3 : printf(" (MP3)\n"); count++; break;
				case KEY_AUDIO : printf(" (Audio)\n"); count++; break;
				case KEY_VIDEO : printf(" (Video)\n"); count++; break;
				case KEY_DIRECTORY : printf(" (Directory)\n"); count++; break;
				case KEY_LIST : printf(" (List)\n"); count++; break;
				case KEY_MEMO : printf(" (Memo)\n"); count++; break;
				case KEY_CALENDAR : printf(" (Calendar)\n"); count++; break;
				case KEY_RED : printf(" (Red)\n"); count++; break;
				case KEY_GREEN : printf(" (Green)\n"); count++; break;
				case KEY_YELLOW : printf(" (Yellow)\n"); count++; break;
				case KEY_BLUE : printf(" (Blue)\n"); count++; break;
				case KEY_CHANNELUP : printf(" (Channel up)\n"); count++; break;
				case KEY_CHANNELDOWN : printf(" (Channel down)\n"); count++; break;
				case KEY_FIRST : printf(" (First)\n"); count++; break;
				case KEY_LAST : printf(" (Last)\n"); count++; break;
				case KEY_AB : printf(" (AB)\n"); count++; break;
				case KEY_NEXT : printf(" (Next)\n"); count++; break;
				case KEY_RESTART : printf(" (Restart)\n"); count++; break;
				case KEY_SLOW : printf(" (Slow)\n"); count++; break;
				case KEY_SHUFFLE : printf(" (Shuffle)\n"); count++; break;
				case KEY_BREAK : printf(" (Break)\n"); count++; break;
				case KEY_PREVIOUS : printf(" (Previous)\n"); count++; break;
				case KEY_DIGITS : printf(" (Digits)\n"); count++; break;
				case KEY_TEEN : printf(" (Teen)\n"); count++; break;
				case KEY_TWEN : printf(" (Twen)\n"); count++; break;
				case KEY_MAX : printf(" (MAX)\n"); count++; break;

				case BTN_0 : printf(" (Button 0)\n"); count++; break;
				case BTN_1 : printf(" (Button 1)\n"); count++; break;
				case BTN_2 : printf(" (Button 2)\n"); count++; break;
				case BTN_3 : printf(" (Button 3)\n"); count++; break;
				case BTN_4 : printf(" (Button 4)\n"); count++; break;
				case BTN_5 : printf(" (Button 5)\n"); count++; break;
				case BTN_6 : printf(" (Button 6)\n"); count++; break;
				case BTN_7 : printf(" (Button 7)\n"); count++; break;
				case BTN_8 : printf(" (Button 8)\n"); count++; break;
				case BTN_9 : printf(" (Button 9)\n"); count++; break;
				case BTN_LEFT : printf(" (Left Button)\n"); count++; break;
				case BTN_RIGHT : printf(" (Right Button)\n"); count++; break;
				case BTN_MIDDLE : printf(" (Middle Button)\n"); count++; break;
				case BTN_SIDE : printf(" (Side Button)\n"); count++; break;
				case BTN_EXTRA : printf(" (Extra Button)\n"); count++; break;
				case BTN_TASK : printf(" (Task Button)\n"); count++; break;
				case BTN_FORWARD : printf(" (Forward Button)\n"); count++; break;
				case BTN_BACK : printf(" (Back Button)\n"); count++; break;
				case BTN_TRIGGER : printf(" (Trigger Button)\n"); count++; break;
				case BTN_THUMB : printf(" (Thumb Button)\n"); count++; break;
				case BTN_THUMB2 : printf(" (Second Thumb Button)\n"); count++; break;
				case BTN_TOP : printf(" (Top Button)\n"); count++; break;
				case BTN_TOP2 : printf(" (Second Top Button)\n"); count++; break;
				case BTN_PINKIE : printf(" (Pinkie Button)\n"); count++; break;
				case BTN_BASE : printf(" (Base Button)\n"); count++; break;
				case BTN_BASE2 : printf(" (Second Base Button)\n"); count++; break;
				case BTN_BASE3 : printf(" (Third Base Button)\n"); count++; break;
				case BTN_BASE4 : printf(" (Fourth Base Button)\n"); count++; break;
				case BTN_BASE5 : printf(" (Fifth Base Button)\n"); count++; break;
				case BTN_BASE6 : printf(" (Sixth Base Button)\n"); count++; break;
				case BTN_DEAD : printf(" (Dead Button)\n"); count++; break;
				case BTN_A : printf(" (Button A)\n"); count++; break;
				case BTN_B : printf(" (Button B)\n"); count++; break;
				case BTN_C : printf(" (Button C)\n"); count++; break;
				case BTN_X : printf(" (Button X)\n"); count++; break;
				case BTN_Y : printf(" (Button Y)\n"); count++; break;
				case BTN_Z : printf(" (Button Z)\n"); count++; break;
				case BTN_TL : printf(" (Thumb Left Button)\n"); count++; break;
				case BTN_TR : printf(" (Thumb Right Button )\n"); count++; break;
				case BTN_TL2 : printf(" (Second Thumb Left Button)\n"); count++; break;
				case BTN_TR2 : printf(" (Second Thumb Right Button )\n"); count++; break;
				case BTN_SELECT : printf(" (Select Button)\n"); count++; break;
				case BTN_MODE : printf(" (Mode Button)\n"); count++; break;
				case BTN_THUMBL : printf(" (Another Left Thumb Button )\n"); count++; break;
				case BTN_THUMBR : printf(" (Another Right Thumb Button )\n"); count++; break;
				case BTN_TOOL_PEN : printf(" (Digitiser Pen Tool)\n"); count++; break;
				case BTN_TOOL_RUBBER : printf(" (Digitiser Rubber Tool)\n"); count++; break;
				case BTN_TOOL_BRUSH : printf(" (Digitiser Brush Tool)\n"); count++; break;
				case BTN_TOOL_PENCIL : printf(" (Digitiser Pencil Tool)\n"); count++; break;
				case BTN_TOOL_AIRBRUSH : printf(" (Digitiser Airbrush Tool)\n"); count++; break;
				case BTN_TOOL_FINGER : printf(" (Digitiser Finger Tool)\n"); count++; break;
				case BTN_TOOL_MOUSE : printf(" (Digitiser Mouse Tool)\n"); count++; break;
				case BTN_TOOL_LENS : printf(" (Digitiser Lens Tool)\n"); count++; break;
				case BTN_TOUCH : printf(" (Digitiser Touch Button )\n"); count++; break;
				case BTN_STYLUS : printf(" (Digitiser Stylus Button )\n"); count++; break;
				case BTN_STYLUS2 : printf(" (Second Digitiser Stylus Button )\n"); count++; break;
				case BTN_TOOL_DOUBLETAP : printf(" (Digitiser Doubletap Tool)\n"); count++; break;
				case BTN_TOOL_TRIPLETAP : printf(" (Digitiser Tripletap Tool)\n"); count++; break;
				case BTN_GEAR_DOWN : printf(" (Wheel Up\n"); count++; break;
				case BTN_GEAR_UP : printf(" (Wheel Down\n"); count++; break;
				default:
					printf(" (Unknown key)\n"); count++;
			} /* switch */		 
		} /* if */
	} /* for */

	printf("Buttons/Keys: %3d\n\n", count);

	printf("Supported Relative axes:\n");

	for (yalv = 0; yalv < REL_MAX; yalv++) {
		if (test_bit(yalv, rel_bitmask)) {
			/* this means that the bit is set in the axes list */
			printf("  Relative axis 0x%02x ", yalv);
			switch ( yalv)
			{
				case REL_X :
					printf(" (X Axis)\n");
					break;
				case REL_Y :
					printf(" (Y Axis)\n");
					break;
				case REL_Z :
					printf(" (Z Axis)\n");
					break;
				case REL_HWHEEL :
					printf(" (Horizontal Wheel)\n");
					break;
				case REL_DIAL :
					printf(" (Dial)\n");
					break;
				case REL_WHEEL :
					printf(" (Vertical Wheel)\n");
					break;
				case REL_MISC :
					printf(" (Miscellaneous)\n");
					break;
				default:
					printf(" (Unknown relative feature)\n");
			}
		}
	}
	
	close(fd);
	
	exit(0);
}

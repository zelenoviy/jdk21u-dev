/*
 * Copyright (c) 2012, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#include "Utilities.h"

#include <InterfaceDefs.h>
#include <String.h>
#include <View.h>

#include <pthread.h>

#include "java_awt_Event.h"
#include "java_awt_event_InputEvent.h"
#include "java_awt_event_KeyEvent.h"
#include "java_awt_event_MouseEvent.h"


static pthread_key_t envKey;
static pthread_once_t envKeyOnce = PTHREAD_ONCE_INIT;

static void
EnvDestructor(void*)
{
	jvm->DetachCurrentThread();
}

static void
InitKey()
{
	pthread_key_create(&envKey, EnvDestructor);
}

JNIEnv*
GetEnv()
{
	pthread_once(&envKeyOnce, InitKey);

	// We don't bother getting the value from TLS because GetEnv is
	// faster. We use TLS in order to detach the current thread in
	// the thread-specific destructor.
	JNIEnv* env = NULL;
	jvm->GetEnv((void**)&env, JNI_VERSION_1_2);

	if (env == NULL) {
		jvm->AttachCurrentThread((void**)&env, NULL);
		pthread_setspecific(envKey, env);
	}

	return env;
}


void
DoCallback(jobject obj, const char* name, const char* description, ...)
{
    JNIEnv* env = GetEnv();
    va_list args;
    va_start(args, description);
    JNU_CallMethodByNameV(env, NULL, obj, name, description, args);
    {
		jthrowable exc = env->ExceptionOccurred();
        if (exc) {
            env->DeleteLocalRef(exc);
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }
    DASSERT(!env->ExceptionOccurred());
    va_end(args);
}


jthrowable
safe_ExceptionOccurred(JNIEnv *env) noexcept(true) {
    jthrowable xcp = env->ExceptionOccurred();
    if (xcp != NULL) {
        env->ExceptionClear(); // if we don't do this, FindClass will fail

        jclass outofmem = env->FindClass("java/lang/OutOfMemoryError");
        DASSERT(outofmem != NULL);
        jboolean isOutofmem = env->IsInstanceOf(xcp, outofmem);

        env->DeleteLocalRef(outofmem);

        if (isOutofmem) {
            env->DeleteLocalRef(xcp);
            throw std::bad_alloc();
        } else {
            // rethrow exception
            env->Throw(xcp);
            return xcp;
        }
    }

    return NULL;
}

jint
ConvertMouseButtonToJava(int32 buttons)
{
	if (buttons & B_PRIMARY_MOUSE_BUTTON)
		return java_awt_event_MouseEvent_BUTTON1;
	else if (buttons & B_SECONDARY_MOUSE_BUTTON)
		return java_awt_event_MouseEvent_BUTTON3;
	else if (buttons & B_TERTIARY_MOUSE_BUTTON)
		return java_awt_event_MouseEvent_BUTTON2;
	else
		return java_awt_event_MouseEvent_NOBUTTON;
}

jint
ConvertMouseMaskToJava(int32 buttons)
{
	jint javaButtons = 0;
	if (buttons & B_PRIMARY_MOUSE_BUTTON)
		javaButtons |= java_awt_event_MouseEvent_BUTTON1_DOWN_MASK;
	if (buttons & B_SECONDARY_MOUSE_BUTTON)
		javaButtons |= java_awt_event_MouseEvent_BUTTON3_DOWN_MASK;
	if (buttons & B_TERTIARY_MOUSE_BUTTON)
		javaButtons |= java_awt_event_MouseEvent_BUTTON2_DOWN_MASK;
	return javaButtons;
}

int32
ConvertKeyCodeToNative(jint jkeycode) {
	if (jkeycode == 0) {
		return 0;
	}
	int32 keycode = 0;
	switch (jkeycode) {
	
	// Basic mappings of KeyEvent.<fields> to Haiku keyboard scancodes.
	case java_awt_event_KeyEvent_VK_ESCAPE: keycode = 0x01; break;
	case java_awt_event_KeyEvent_VK_F1: keycode = 0x02; break;
	case java_awt_event_KeyEvent_VK_F2: keycode = 0x03; break;
	case java_awt_event_KeyEvent_VK_F3: keycode = 0x04; break;
	case java_awt_event_KeyEvent_VK_F4: keycode = 0x05; break;
	case java_awt_event_KeyEvent_VK_F5: keycode = 0x06; break;
	case java_awt_event_KeyEvent_VK_F6: keycode = 0x07; break;
	case java_awt_event_KeyEvent_VK_F7: keycode = 0x08; break;
	case java_awt_event_KeyEvent_VK_F8: keycode = 0x09; break;
	case java_awt_event_KeyEvent_VK_F9: keycode = 0x0a; break;
	case java_awt_event_KeyEvent_VK_F10: keycode = 0x0b; break;
	case java_awt_event_KeyEvent_VK_F11: keycode = 0x0c; break;
	case java_awt_event_KeyEvent_VK_F12: keycode = 0x0d; break;

	case java_awt_event_KeyEvent_VK_DEAD_TILDE:
	case java_awt_event_KeyEvent_VK_BACK_QUOTE: keycode = 0x11; break;
	case java_awt_event_KeyEvent_VK_EXCLAMATION_MARK:
	case java_awt_event_KeyEvent_VK_1: keycode = 0x12; break;
	case java_awt_event_KeyEvent_VK_AT:
	case java_awt_event_KeyEvent_VK_2: keycode = 0x13; break;
	case java_awt_event_KeyEvent_VK_NUMBER_SIGN:
	case java_awt_event_KeyEvent_VK_3: keycode = 0x14; break;
	case java_awt_event_KeyEvent_VK_DOLLAR:
	case java_awt_event_KeyEvent_VK_4: keycode = 0x15; break;
	case java_awt_event_KeyEvent_VK_5: keycode = 0x16; break;
	case java_awt_event_KeyEvent_VK_CIRCUMFLEX:
	case java_awt_event_KeyEvent_VK_6: keycode = 0x17; break;
	case java_awt_event_KeyEvent_VK_AMPERSAND:
	case java_awt_event_KeyEvent_VK_7: keycode = 0x18; break;
	case java_awt_event_KeyEvent_VK_ASTERISK:
	case java_awt_event_KeyEvent_VK_8: keycode = 0x19; break;
	case java_awt_event_KeyEvent_VK_LEFT_PARENTHESIS:
	case java_awt_event_KeyEvent_VK_9: keycode = 0x1a; break;
	case java_awt_event_KeyEvent_VK_RIGHT_PARENTHESIS:
	case java_awt_event_KeyEvent_VK_0: keycode = 0x1b; break;
	case java_awt_event_KeyEvent_VK_UNDERSCORE:
	case java_awt_event_KeyEvent_VK_MINUS: keycode = 0x1c; break;
	case java_awt_event_KeyEvent_VK_PLUS:
	case java_awt_event_KeyEvent_VK_EQUALS: keycode = 0x1d; break;
	case java_awt_event_KeyEvent_VK_BACK_SPACE: keycode = 0x1e; break;
	
	case java_awt_event_KeyEvent_VK_TAB: keycode = 0x26; break;
	case java_awt_event_KeyEvent_VK_Q: keycode = 0x27; break;
	case java_awt_event_KeyEvent_VK_W: keycode = 0x28; break;
	case java_awt_event_KeyEvent_VK_E: keycode = 0x29; break;
	case java_awt_event_KeyEvent_VK_R: keycode = 0x2a; break;
	case java_awt_event_KeyEvent_VK_T: keycode = 0x2b; break;
	case java_awt_event_KeyEvent_VK_Y: keycode = 0x2c; break;
	case java_awt_event_KeyEvent_VK_U: keycode = 0x2d; break;
	case java_awt_event_KeyEvent_VK_I: keycode = 0x2e; break;
	case java_awt_event_KeyEvent_VK_O: keycode = 0x2f; break;
	case java_awt_event_KeyEvent_VK_P: keycode = 0x30; break;
	case java_awt_event_KeyEvent_VK_BRACELEFT:
	case java_awt_event_KeyEvent_VK_OPEN_BRACKET: keycode = 0x31; break;
	case java_awt_event_KeyEvent_VK_BRACERIGHT:
	case java_awt_event_KeyEvent_VK_CLOSE_BRACKET: keycode = 0x32; break;
	case java_awt_event_KeyEvent_VK_SEPARATOR:
	case java_awt_event_KeyEvent_VK_BACK_SLASH: keycode = 0x33; break;
	
	case java_awt_event_KeyEvent_VK_CAPS_LOCK: keycode = 0x3b; break;
	case java_awt_event_KeyEvent_VK_A: keycode = 0x3c; break;
	case java_awt_event_KeyEvent_VK_S: keycode = 0x3d; break;
	case java_awt_event_KeyEvent_VK_D: keycode = 0x3e; break;
	case java_awt_event_KeyEvent_VK_F: keycode = 0x3f; break;
	case java_awt_event_KeyEvent_VK_G: keycode = 0x40; break;
	case java_awt_event_KeyEvent_VK_H: keycode = 0x41; break;
	case java_awt_event_KeyEvent_VK_J: keycode = 0x42; break;
	case java_awt_event_KeyEvent_VK_K: keycode = 0x43; break;
	case java_awt_event_KeyEvent_VK_L: keycode = 0x44; break;
	case java_awt_event_KeyEvent_VK_COLON:
	case java_awt_event_KeyEvent_VK_SEMICOLON: keycode = 0x45; break;
	case java_awt_event_KeyEvent_VK_QUOTEDBL:
	case java_awt_event_KeyEvent_VK_QUOTE: keycode = 0x46; break;
	case java_awt_event_KeyEvent_VK_ENTER: keycode = 0x47; break;
	
	case java_awt_event_KeyEvent_VK_SHIFT: keycode = 0x4b; break;
	case java_awt_event_KeyEvent_VK_Z: keycode = 0x4c; break;
	case java_awt_event_KeyEvent_VK_X: keycode = 0x4d; break;
	case java_awt_event_KeyEvent_VK_C: keycode = 0x4e; break;
	case java_awt_event_KeyEvent_VK_V: keycode = 0x4f; break;
	case java_awt_event_KeyEvent_VK_B: keycode = 0x50; break;
	case java_awt_event_KeyEvent_VK_N: keycode = 0x51; break;
	case java_awt_event_KeyEvent_VK_M: keycode = 0x52; break;
	case java_awt_event_KeyEvent_VK_LESS:
	case java_awt_event_KeyEvent_VK_COMMA: keycode = 0x53; break;
	case java_awt_event_KeyEvent_VK_GREATER:
	case java_awt_event_KeyEvent_VK_PERIOD: keycode = 0x54; break;
	case java_awt_event_KeyEvent_VK_SLASH: keycode = 0x55; break;
	// Another SHIFT 0x56
	
	case java_awt_event_KeyEvent_VK_CONTROL: keycode = 0x5c; break;
	case java_awt_event_KeyEvent_VK_META: keycode = 0x66; break;
	case java_awt_event_KeyEvent_VK_ALT: keycode = 0x5d; break;
	case java_awt_event_KeyEvent_VK_SPACE: keycode = 0x5e; break;
	
// Arrow Keys
	case java_awt_event_KeyEvent_VK_LEFT: keycode = 0x61; break;
	case java_awt_event_KeyEvent_VK_UP: keycode = 0x57; break;
	case java_awt_event_KeyEvent_VK_RIGHT: keycode = 0x63; break;
	case java_awt_event_KeyEvent_VK_DOWN: keycode = 0x62; break;
	
// Numeric Keypad
	case java_awt_event_KeyEvent_VK_NUMPAD0: keycode = 0x64; break;
	case java_awt_event_KeyEvent_VK_NUMPAD1: keycode = 0x58; break;
	case java_awt_event_KeyEvent_VK_KP_DOWN:
	case java_awt_event_KeyEvent_VK_NUMPAD2: keycode = 0x59; break;
	case java_awt_event_KeyEvent_VK_NUMPAD3: keycode = 0x5a; break;
	case java_awt_event_KeyEvent_VK_KP_LEFT:
	case java_awt_event_KeyEvent_VK_NUMPAD4: keycode = 0x48; break;
	case java_awt_event_KeyEvent_VK_NUMPAD5: keycode = 0x49; break;
	case java_awt_event_KeyEvent_VK_KP_RIGHT:
	case java_awt_event_KeyEvent_VK_NUMPAD6: keycode = 0x4a; break;
	case java_awt_event_KeyEvent_VK_NUMPAD7: keycode = 0x37; break;
	case java_awt_event_KeyEvent_VK_KP_UP:
	case java_awt_event_KeyEvent_VK_NUMPAD8: keycode = 0x38; break;
	case java_awt_event_KeyEvent_VK_NUMPAD9: keycode = 0x39; break;
	case java_awt_event_KeyEvent_VK_MULTIPLY: keycode = 0x24; break;
	case java_awt_event_KeyEvent_VK_ADD: keycode = 0x3a; break;
	case java_awt_event_KeyEvent_VK_SUBTRACT: keycode = 0x25; break;
	case java_awt_event_KeyEvent_VK_DECIMAL: keycode = 0x65; break;
	case java_awt_event_KeyEvent_VK_DIVIDE: keycode = 0x23; break;
// Page / Mode control	
	case java_awt_event_KeyEvent_VK_PRINTSCREEN: keycode = 0x0e; break;
	case java_awt_event_KeyEvent_VK_PAUSE: keycode = 0x10; break;
	case java_awt_event_KeyEvent_VK_SCROLL_LOCK: keycode = 0x0f; break;
	case java_awt_event_KeyEvent_VK_NUM_LOCK: keycode = 0x22; break;
	
	case java_awt_event_KeyEvent_VK_PAGE_UP: keycode = 0x7f; break;
	case java_awt_event_KeyEvent_VK_PAGE_DOWN: keycode = 0x10; break;
	case java_awt_event_KeyEvent_VK_END: keycode = 0x0f; break;
	case java_awt_event_KeyEvent_VK_HOME: keycode = 0x0f; break;
	case java_awt_event_KeyEvent_VK_INSERT: keycode = 0x7e; break;
	case java_awt_event_KeyEvent_VK_DELETE: keycode = 0x65; break;
	
	
	// Haiku has no direct keycode for these
/*	case java_awt_event_KeyEvent_VK_HELP
	case java_awt_event_KeyEvent_VK_EURO_SIGN
	case java_awt_event_KeyEvent_VK_INVERTED_EXCLAMATION_MARK

	
	// Second row of FKeys Haiku
	case java_awt_event_KeyEvent_VK_F13
	case java_awt_event_KeyEvent_VK_F14
	case java_awt_event_KeyEvent_VK_F15
	case java_awt_event_KeyEvent_VK_F16
	case java_awt_event_KeyEvent_VK_F17
	case java_awt_event_KeyEvent_VK_F18
	case java_awt_event_KeyEvent_VK_F19
	case java_awt_event_KeyEvent_VK_F20
	case java_awt_event_KeyEvent_VK_F21
	case java_awt_event_KeyEvent_VK_F22
	case java_awt_event_KeyEvent_VK_F23
	case java_awt_event_KeyEvent_VK_F24
	
	// Dead
	case java_awt_event_KeyEvent_VK_DEAD_GRAVE
	case java_awt_event_KeyEvent_VK_DEAD_ACUTE
	case java_awt_event_KeyEvent_VK_DEAD_CIRCUMFLEX
	case java_awt_event_KeyEvent_VK_DEAD_MACRON
	case java_awt_event_KeyEvent_VK_DEAD_BREVE
	case java_awt_event_KeyEvent_VK_DEAD_ABOVEDOT
	case java_awt_event_KeyEvent_VK_DEAD_DIAERESIS
	case java_awt_event_KeyEvent_VK_DEAD_ABOVERING
	case java_awt_event_KeyEvent_VK_DEAD_DOUBLEACUTE
	case java_awt_event_KeyEvent_VK_DEAD_CARON
	case java_awt_event_KeyEvent_VK_DEAD_CEDILLA
	case java_awt_event_KeyEvent_VK_DEAD_OGONEK
	case java_awt_event_KeyEvent_VK_DEAD_IOTA
	case java_awt_event_KeyEvent_VK_DEAD_VOICED_SOUND
	case java_awt_event_KeyEvent_VK_DEAD_SEMIVOICED_SOUND
	
	// More keys that aren't on standard 101-key keyboards
	case java_awt_event_KeyEvent_VK_FINAL
	case java_awt_event_KeyEvent_VK_CONVERT
	case java_awt_event_KeyEvent_VK_NONCONVERT
	case java_awt_event_KeyEvent_VK_ACCEPT
	case java_awt_event_KeyEvent_VK_MODECHANGE
	case java_awt_event_KeyEvent_VK_KANA
	case java_awt_event_KeyEvent_VK_KANJI
	case java_awt_event_KeyEvent_VK_ALPHANUMERIC
	case java_awt_event_KeyEvent_VK_KATAKANA
	case java_awt_event_KeyEvent_VK_HIRAGANA
	case java_awt_event_KeyEvent_VK_FULL_WIDTH
	case java_awt_event_KeyEvent_VK_HALF_WIDTH
	case java_awt_event_KeyEvent_VK_ROMAN_CHARACTERS
	case java_awt_event_KeyEvent_VK_ALL_CANDIDATES
	case java_awt_event_KeyEvent_VK_PREVIOUS_CANDIDATE
	case java_awt_event_KeyEvent_VK_CODE_INPUT
	case java_awt_event_KeyEvent_VK_JAPANESE_KATAKANA
	case java_awt_event_KeyEvent_VK_JAPANESE_HIRAGANA
	case java_awt_event_KeyEvent_VK_JAPANESE_ROMAN
	case java_awt_event_KeyEvent_VK_KANA_LOCK
	case java_awt_event_KeyEvent_VK_INPUT_METHOD_ON_OFF
	case java_awt_event_KeyEvent_VK_CUT
	case java_awt_event_KeyEvent_VK_COPY
	case java_awt_event_KeyEvent_VK_PASTE
	case java_awt_event_KeyEvent_VK_UNDO
	case java_awt_event_KeyEvent_VK_AGAIN
	case java_awt_event_KeyEvent_VK_FIND
	case java_awt_event_KeyEvent_VK_PROPS
	case java_awt_event_KeyEvent_VK_STOP
	case java_awt_event_KeyEvent_VK_COMPOSE
	case java_awt_event_KeyEvent_VK_ALT_GRAPH
*/
	case java_awt_event_KeyEvent_VK_UNDEFINED: keycode = 0; break;
	default:
		keycode = 0;
	}
	return keycode;
}


/**
 * Converts from the Haiku keycode to Java keycode/location
 */
void
ConvertKeyCodeToJava(int32 keycode, jint *jkeyCode, jint *jkeyLocation) {
	*jkeyCode = java_awt_event_KeyEvent_VK_UNDEFINED;
	*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_STANDARD;

	switch (keycode) {
		case 0x01: *jkeyCode = java_awt_event_KeyEvent_VK_ESCAPE; break;
		case 0x02: *jkeyCode = java_awt_event_KeyEvent_VK_F1; break;
		case 0x03: *jkeyCode = java_awt_event_KeyEvent_VK_F2; break;
		case 0x04: *jkeyCode = java_awt_event_KeyEvent_VK_F3; break;
		case 0x05: *jkeyCode = java_awt_event_KeyEvent_VK_F4; break;
		case 0x06: *jkeyCode = java_awt_event_KeyEvent_VK_F5; break;
		case 0x07: *jkeyCode = java_awt_event_KeyEvent_VK_F6; break;
		case 0x08: *jkeyCode = java_awt_event_KeyEvent_VK_F7; break;
		case 0x09: *jkeyCode = java_awt_event_KeyEvent_VK_F8; break;
		case 0x0a: *jkeyCode = java_awt_event_KeyEvent_VK_F9; break;
		case 0x0b: *jkeyCode = java_awt_event_KeyEvent_VK_F10; break;
		case 0x0c: *jkeyCode = java_awt_event_KeyEvent_VK_F11; break;
		case 0x0d: *jkeyCode = java_awt_event_KeyEvent_VK_F12; break;
		case 0x0e: *jkeyCode = java_awt_event_KeyEvent_VK_PRINTSCREEN; break;
		case 0x0f: *jkeyCode = java_awt_event_KeyEvent_VK_SCROLL_LOCK; break;	
		case 0x10: *jkeyCode = java_awt_event_KeyEvent_VK_PAUSE; break;		
		case 0x11: *jkeyCode = java_awt_event_KeyEvent_VK_BACK_QUOTE; break;
		case 0x12: *jkeyCode = java_awt_event_KeyEvent_VK_1; break;
		case 0x13: *jkeyCode = java_awt_event_KeyEvent_VK_2; break;
		case 0x14: *jkeyCode = java_awt_event_KeyEvent_VK_3; break;
		case 0x15: *jkeyCode = java_awt_event_KeyEvent_VK_4; break;
		case 0x16: *jkeyCode = java_awt_event_KeyEvent_VK_5; break;
		case 0x17: *jkeyCode = java_awt_event_KeyEvent_VK_6; break;
		case 0x18: *jkeyCode = java_awt_event_KeyEvent_VK_7; break;
		case 0x19: *jkeyCode = java_awt_event_KeyEvent_VK_8; break;
		case 0x1a: *jkeyCode = java_awt_event_KeyEvent_VK_9; break;
		case 0x1b: *jkeyCode = java_awt_event_KeyEvent_VK_0; break;
		case 0x1c: *jkeyCode = java_awt_event_KeyEvent_VK_MINUS; break;
		case 0x1d: *jkeyCode = java_awt_event_KeyEvent_VK_EQUALS; break;
		case 0x1e: *jkeyCode = java_awt_event_KeyEvent_VK_BACK_SPACE; break;
		case 0x1f: *jkeyCode = java_awt_event_KeyEvent_VK_INSERT; break;
		case 0x20: *jkeyCode = java_awt_event_KeyEvent_VK_HOME; break;
		case 0x21: *jkeyCode = java_awt_event_KeyEvent_VK_PAGE_UP; break;
		case 0x22: *jkeyCode = java_awt_event_KeyEvent_VK_NUM_LOCK; break;

		case 0x23:
			*jkeyCode = java_awt_event_KeyEvent_VK_DIVIDE;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD;
			break;

		case 0x24:
			*jkeyCode = java_awt_event_KeyEvent_VK_MULTIPLY;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD;
			break;
		case 0x25:
			*jkeyCode = java_awt_event_KeyEvent_VK_SUBTRACT;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD;
			break;

		case 0x26: *jkeyCode = java_awt_event_KeyEvent_VK_TAB; break;
		case 0x27: *jkeyCode = java_awt_event_KeyEvent_VK_Q; break;
		case 0x28: *jkeyCode = java_awt_event_KeyEvent_VK_W; break;
		case 0x29: *jkeyCode = java_awt_event_KeyEvent_VK_E; break;
		case 0x2a: *jkeyCode = java_awt_event_KeyEvent_VK_R; break;
		case 0x2b: *jkeyCode = java_awt_event_KeyEvent_VK_T; break;
		case 0x2c: *jkeyCode = java_awt_event_KeyEvent_VK_Y; break;
		case 0x2d: *jkeyCode = java_awt_event_KeyEvent_VK_U; break;
		case 0x2e: *jkeyCode = java_awt_event_KeyEvent_VK_I; break;
		case 0x2f: *jkeyCode = java_awt_event_KeyEvent_VK_O; break;
		case 0x30: *jkeyCode = java_awt_event_KeyEvent_VK_P; break;
		case 0x31: *jkeyCode = java_awt_event_KeyEvent_VK_OPEN_BRACKET; break;
		case 0x32: *jkeyCode = java_awt_event_KeyEvent_VK_CLOSE_BRACKET; break;
		case 0x33: *jkeyCode = java_awt_event_KeyEvent_VK_BACK_SLASH; break;
		case 0x34: *jkeyCode = java_awt_event_KeyEvent_VK_DELETE; break;
		case 0x35: *jkeyCode = java_awt_event_KeyEvent_VK_END; break;
		case 0x36: *jkeyCode = java_awt_event_KeyEvent_VK_PAGE_DOWN; break;
		case 0x37: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD7; break;
		case 0x38: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD8; break;
		case 0x39: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD9; break;
	
		case 0x3a:
			*jkeyCode = java_awt_event_KeyEvent_VK_ADD;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD;
			break;
	
		case 0x3b: *jkeyCode = java_awt_event_KeyEvent_VK_CAPS_LOCK; break;
		case 0x3c: *jkeyCode = java_awt_event_KeyEvent_VK_A; break;
		case 0x3d: *jkeyCode = java_awt_event_KeyEvent_VK_S; break;
		case 0x3e: *jkeyCode = java_awt_event_KeyEvent_VK_D; break;
		case 0x3f: *jkeyCode = java_awt_event_KeyEvent_VK_F; break;
		case 0x40: *jkeyCode = java_awt_event_KeyEvent_VK_G; break;
		case 0x41: *jkeyCode = java_awt_event_KeyEvent_VK_H; break;
		case 0x42: *jkeyCode = java_awt_event_KeyEvent_VK_J; break;
		case 0x43: *jkeyCode = java_awt_event_KeyEvent_VK_K; break;
		case 0x44: *jkeyCode = java_awt_event_KeyEvent_VK_L; break;
		case 0x45: *jkeyCode = java_awt_event_KeyEvent_VK_SEMICOLON; break;
		case 0x46: *jkeyCode = java_awt_event_KeyEvent_VK_QUOTE; break;
		case 0x47: *jkeyCode = java_awt_event_KeyEvent_VK_ENTER; break;
		case 0x48: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD4; break;
		case 0x49: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD5; break;
		case 0x4a: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD6; break;

		case 0x4b: 
			*jkeyCode = java_awt_event_KeyEvent_VK_SHIFT; 
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_LEFT;
			break;

		case 0x4c: *jkeyCode = java_awt_event_KeyEvent_VK_Z; break;
		case 0x4d: *jkeyCode = java_awt_event_KeyEvent_VK_X; break;
		case 0x4e: *jkeyCode = java_awt_event_KeyEvent_VK_C; break;
		case 0x4f: *jkeyCode = java_awt_event_KeyEvent_VK_V; break;
		case 0x50: *jkeyCode = java_awt_event_KeyEvent_VK_B; break;
		case 0x51: *jkeyCode = java_awt_event_KeyEvent_VK_N; break;
		case 0x52: *jkeyCode = java_awt_event_KeyEvent_VK_M; break;
		case 0x53: *jkeyCode = java_awt_event_KeyEvent_VK_COMMA; break;
		case 0x54: *jkeyCode = java_awt_event_KeyEvent_VK_PERIOD; break;
		case 0x55: *jkeyCode = java_awt_event_KeyEvent_VK_SLASH; break;

		case 0x56:
			*jkeyCode = java_awt_event_KeyEvent_VK_SHIFT;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_RIGHT;
			break;

		case 0x57: *jkeyCode = java_awt_event_KeyEvent_VK_UP; break;
		case 0x58: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD1; break;
		case 0x59: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD2; break;
		case 0x5a: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD3; break;

		case 0x5b:
			*jkeyCode = java_awt_event_KeyEvent_VK_ENTER;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD;
			break;
		case 0x5c:
			*jkeyCode = java_awt_event_KeyEvent_VK_CONTROL;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_LEFT;
			break;
		case 0x5d:
			*jkeyCode = java_awt_event_KeyEvent_VK_ALT;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_LEFT;
			break;

		case 0x5e: *jkeyCode = java_awt_event_KeyEvent_VK_SPACE; break;

		case 0x5f:
			*jkeyCode = java_awt_event_KeyEvent_VK_ALT;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_RIGHT;
			break;
		case 0x60:
			*jkeyCode = java_awt_event_KeyEvent_VK_CONTROL;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_RIGHT;
			break;

		case 0x61: *jkeyCode = java_awt_event_KeyEvent_VK_LEFT; break;
		case 0x62: *jkeyCode = java_awt_event_KeyEvent_VK_DOWN; break;
		case 0x63: *jkeyCode = java_awt_event_KeyEvent_VK_RIGHT; break;
		case 0x64: *jkeyCode = java_awt_event_KeyEvent_VK_NUMPAD0; break;

		case 0x65:
			*jkeyCode = java_awt_event_KeyEvent_VK_DECIMAL;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_NUMPAD;
			break;
		case 0x66:
			*jkeyCode = java_awt_event_KeyEvent_VK_META;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_LEFT;
			break;
		case 0x67:
			*jkeyCode = java_awt_event_KeyEvent_VK_META;
			*jkeyLocation = java_awt_event_KeyEvent_KEY_LOCATION_RIGHT;
			break;

		case 0x68: *jkeyCode = java_awt_event_KeyEvent_VK_CONTEXT_MENU; break;
		case 0x69: *jkeyCode = java_awt_event_KeyEvent_VK_EURO_SIGN; break;
	}
}

// use this for general Events
jint
ConvertModifiersToJava(uint32 modifiers)
{
	jint jmodifiers = 0;
	if (modifiers & B_SHIFT_KEY) {
		jmodifiers |= java_awt_Event_SHIFT_MASK;
	}
	if (modifiers & B_CONTROL_KEY) {
		jmodifiers |= java_awt_Event_CTRL_MASK;
	}
	if (modifiers & B_OPTION_KEY) {
		jmodifiers |= java_awt_Event_META_MASK;
	}
	if (modifiers & B_COMMAND_KEY) {
		jmodifiers |= java_awt_Event_ALT_MASK;
	}
	return jmodifiers;
}

// use this for subclasses of InputEvent (such as MouseEvent or KeyEvent)
jint
ConvertInputModifiersToJava(uint32 modifiers)
{
	jint jmodifiers = 0;
	if (modifiers & B_SHIFT_KEY) {
		jmodifiers |= java_awt_event_InputEvent_SHIFT_DOWN_MASK;
	}
	if (modifiers & B_CONTROL_KEY) {
		jmodifiers |= java_awt_event_InputEvent_CTRL_DOWN_MASK;
	}
	if (modifiers & B_OPTION_KEY) {
		jmodifiers |= java_awt_event_InputEvent_META_DOWN_MASK;
	}
	if (modifiers & B_COMMAND_KEY) {
		jmodifiers |= java_awt_event_InputEvent_ALT_DOWN_MASK;
	}
	if (modifiers & B_RIGHT_COMMAND_KEY) {
		jmodifiers |= java_awt_event_InputEvent_ALT_GRAPH_DOWN_MASK;
	}
	return jmodifiers;
}


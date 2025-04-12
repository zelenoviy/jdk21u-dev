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
#ifndef UTILITIES_H
#define UTILITIES_H

#include <jni.h>
#include <jni_util.h>
#include <debug_util.h>

#include <new>

#include <Rect.h>

extern "C" {

extern JavaVM* jvm;

}

JNIEnv* GetEnv();
void DoCallback(jobject obj, const char* name, const char* description, ...);

class BString;

jint ConvertMouseButtonToJava(int32 button);
jint ConvertMouseMaskToJava(int32 buttons);
int32 ConvertKeyCodeToNative(jint jkeycode);
void ConvertKeyCodeToJava(int32 keycode, jint *jkeyCode, jint *jkeyLocation);
jint ConvertModifiersToJava(uint32 modifiers);
jint ConvertInputModifiersToJava(uint32 modifiers);


template<typename T, typename C, typename B, void (C::*D)(B)>
class JNIDelete {
public:
	JNIDelete(C* env, T ref) : fEnv(env), fRef(ref) { }
	~JNIDelete() { _Delete(); }

	JNIDelete& operator=(T ref) {
		_Delete();
		fRef = ref;
		return *this;
	}

	T Detach() {
		T ref = fRef;
		fRef = NULL;
		return ref;
	}

	operator T() { return fRef; }
private:
	void _Delete() {
		if (fRef != NULL)
			(fEnv->*D)(fRef);
		fRef = NULL;
	}

	C* fEnv;
	T fRef;
};

template<typename T> struct JNILocal { typedef JNIDelete<T, JNIEnv, jobject, &JNIEnv::DeleteLocalRef> t; };
template<typename T> struct JNIGlobal { typedef JNIDelete<T, JNIEnv, jobject, &JNIEnv::DeleteGlobalRef> t; };
template<typename T> struct JNIWeakGlobal { typedef JNIDelete<T, JNIEnv, jweak, &JNIEnv::DeleteWeakGlobalRef> t; };

typedef JNILocal<jclass>::t jclassLocal;
typedef JNILocal<jobject>::t jobjectLocal;
typedef JNILocal<jstring>::t jstringLocal;
typedef JNILocal<jobjectArray>::t jobjectArrayLocal;


#define EXCEPTION_CHECK(env)          \
	do {                              \
		if (env->ExceptionCheck()) {  \
			env->ExceptionDescribe(); \
			assert(false);            \
		}                             \
	} while(0)                  


/*
 * I copied the following macros from the Windows AWT port.
 * Go look at the Windows DnD stuff for example usage.
 */

jthrowable safe_ExceptionOccurred(JNIEnv *env) noexcept(true);

/*
 * NOTE: You need these macros only if you take care of performance, since they
 * provide proper caching. Otherwise you can use JNU_CallMethodByName etc.
 */

/*
 * This macro defines a function which returns the class for the specified
 * class name with proper caching and error handling.
 */
#define DECLARE_JAVA_CLASS(javaclazz, name)                                    \
static jclass                                                                  \
get_ ## javaclazz(JNIEnv* env) {                                               \
    static jclass javaclazz = NULL;                                            \
                                                                               \
    if (JNU_IsNull(env, javaclazz)) {                                          \
        jclass javaclazz ## Local = env->FindClass(name);                      \
                                                                               \
        if (!JNU_IsNull(env, javaclazz ## Local)) {                            \
            javaclazz = (jclass)env->NewGlobalRef(javaclazz ## Local);         \
            env->DeleteLocalRef(javaclazz ## Local);                           \
            if (JNU_IsNull(env, javaclazz)) {                                  \
                JNU_ThrowOutOfMemoryError(env, "");                            \
            }                                                                  \
        }                                                                      \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
    }                                                                          \
                                                                               \
    DASSERT(!JNU_IsNull(env, javaclazz));                                      \
                                                                               \
    return javaclazz;                                                          \
}

/*
 * The following macros defines blocks of code which retrieve a method of the
 * specified class identified with the specified name and signature.
 * The specified class should be previously declared with DECLARE_JAVA_CLASS.
 * These macros should be placed at the beginning of a block, after definition
 * of local variables, but before the code begins.
 */
#define DECLARE_VOID_JAVA_METHOD(method, javaclazz, name, signature)           \
    static jmethodID method = NULL;                                            \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        jclass clazz = get_ ## javaclazz(env);                                 \
                                                                               \
        if (JNU_IsNull(env, clazz)) {                                          \
            return;                                                            \
        }                                                                      \
                                                                               \
        method = env->GetMethodID(clazz, name, signature);                     \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return;                                                            \
        }                                                                      \
    }

#define DECLARE_STATIC_VOID_JAVA_METHOD(method, javaclazz, name, signature)    \
    static jmethodID method = NULL;                                            \
    jclass clazz = get_ ## javaclazz(env);                                     \
                                                                               \
    if (JNU_IsNull(env, clazz)) {                                              \
        return;                                                                \
    }                                                                          \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        method = env->GetStaticMethodID(clazz, name, signature);               \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return;                                                            \
        }                                                                      \
    }

#define DECLARE_JINT_JAVA_METHOD(method, javaclazz, name, signature)           \
    static jmethodID method = NULL;                                            \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        jclass clazz = get_ ## javaclazz(env);                                 \
                                                                               \
        if (JNU_IsNull(env, clazz)) {                                          \
            return java_awt_dnd_DnDConstants_ACTION_NONE;                      \
        }                                                                      \
                                                                               \
        method = env->GetMethodID(clazz, name, signature);                     \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return java_awt_dnd_DnDConstants_ACTION_NONE;                      \
        }                                                                      \
    }

#define DECLARE_JBOOLEAN_JAVA_METHOD(method, javaclazz, name, signature)       \
    static jmethodID method = NULL;                                            \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        jclass clazz = get_ ## javaclazz(env);                                 \
                                                                               \
        if (JNU_IsNull(env, clazz)) {                                          \
            return JNI_FALSE;                                                  \
        }                                                                      \
                                                                               \
        method = env->GetMethodID(clazz, name, signature);                     \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return JNI_FALSE;                                                  \
        }                                                                      \
    }

#define DECLARE_OBJECT_JAVA_METHOD(method, javaclazz, name, signature)         \
    static jmethodID method = NULL;                                            \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        jclass clazz = get_ ## javaclazz(env);                                 \
                                                                               \
        if (JNU_IsNull(env, clazz)) {                                          \
            return NULL;                                                       \
        }                                                                      \
                                                                               \
        method = env->GetMethodID(clazz, name, signature);                     \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return NULL;                                                       \
        }                                                                      \
    }

#define DECLARE_STATIC_OBJECT_JAVA_METHOD(method, javaclazz, name, signature)  \
    static jmethodID method = NULL;                                            \
    jclass clazz = get_ ## javaclazz(env);                                     \
                                                                               \
    if (JNU_IsNull(env, clazz)) {                                              \
        return NULL;                                                           \
    }                                                                          \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        method = env->GetStaticMethodID(clazz, name, signature);               \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return NULL;                                                       \
        }                                                                      \
    }

#define DECLARE_JAVA_FIELD(field, javaclazz, name, type)                       \
    static jfieldID field = NULL;                                              \
                                                                               \
    if (JNU_IsNull(env, field)) {                                              \
        jclass clazz = get_ ## javaclazz(env);                                 \
                                                                               \
        if (JNU_IsNull(env, clazz)) {                                          \
            return NULL;                                                       \
        }                                                                      \
                                                                               \
        field = env->GetFieldID(clazz, name, type);                            \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, field)) {                                          \
            DASSERT(FALSE);                                                    \
            return NULL;                                                       \
        }                                                                      \
    }

#endif	/* UTILITIES_H */

#pragma once

#include <cstdint>

namespace j2me {
namespace core {

enum Opcode : uint8_t {
    OP_NOP          = 0x00,
    OP_ACONST_NULL  = 0x01,
    OP_ICONST_M1    = 0x02,
    OP_ICONST_0     = 0x03,
    OP_ICONST_1     = 0x04,
    OP_ICONST_2     = 0x05,
    OP_ICONST_3     = 0x06,
    OP_ICONST_4     = 0x07,
    OP_ICONST_5     = 0x08,
    OP_LCONST_0     = 0x09,
    OP_LCONST_1     = 0x0a,
    
    OP_BIPUSH       = 0x10,
    OP_SIPUSH       = 0x11,
    OP_LDC          = 0x12,
    OP_LDC_W        = 0x13,
    OP_LDC2_W       = 0x14,
    
    OP_ILOAD        = 0x15,
    OP_ILOAD_0      = 0x1a,
    OP_ILOAD_1      = 0x1b,
    OP_ILOAD_2      = 0x1c,
    OP_ILOAD_3      = 0x1d,
    
    OP_LLOAD_0      = 0x1e,
    OP_LLOAD_1      = 0x1f,
    OP_LLOAD_2      = 0x20,
    OP_LLOAD_3      = 0x21,
    
    OP_ALOAD        = 0x19,
    OP_ALOAD_0      = 0x2a,
    OP_ALOAD_1      = 0x2b,
    OP_ALOAD_2      = 0x2c,
    OP_ALOAD_3      = 0x2d,
    
    OP_IALOAD       = 0x2e,
    OP_LALOAD       = 0x2f,
    OP_FALOAD       = 0x30,
    OP_DALOAD       = 0x31,
    OP_AALOAD       = 0x32,
    OP_BALOAD       = 0x33,
    OP_CALOAD       = 0x34,
    OP_SALOAD       = 0x35,
    
    OP_ISTORE       = 0x36,
    OP_ISTORE_0     = 0x3b,
    OP_ISTORE_1     = 0x3c,
    OP_ISTORE_2     = 0x3d,
    OP_ISTORE_3     = 0x3e,
    
    OP_ASTORE       = 0x3a,
    OP_ASTORE_0     = 0x4b,
    OP_ASTORE_1     = 0x4c,
    OP_ASTORE_2     = 0x4d,
    OP_ASTORE_3     = 0x4e,
    
    OP_IASTORE      = 0x4f,
    OP_LASTORE      = 0x50,
    OP_FASTORE      = 0x51,
    OP_DASTORE      = 0x52,
    OP_AASTORE      = 0x53,
    OP_BASTORE      = 0x54,
    OP_CASTORE      = 0x55,
    OP_SASTORE      = 0x56,
    
    OP_POP          = 0x57,
    OP_POP2         = 0x58,
    OP_DUP          = 0x59,
    OP_DUP_X1       = 0x5a,
    
    OP_IFEQ         = 0x99,
    OP_IFNE         = 0x9a,
    OP_IFLT         = 0x9b,
    OP_GOTO         = 0xa7,
    OP_IFNULL       = 0xc6,
    OP_IFNONNULL    = 0xc7,
    OP_GOTO_W       = 0xc8,
    
    OP_IADD         = 0x60,
    
    OP_IINC         = 0x84,
    
    OP_NEW          = 0xbb,
    OP_NEWARRAY     = 0xbc,
    OP_ANEWARRAY    = 0xbd,
    OP_ARRAYLENGTH  = 0xbe,
    
    OP_GETSTATIC    = 0xb2,
    OP_PUTSTATIC    = 0xb3,
    OP_GETFIELD     = 0xb4,
    OP_PUTFIELD     = 0xb5,
    
    OP_INVOKEVIRTUAL = 0xb6,
    OP_INVOKESPECIAL = 0xb7,
    OP_INVOKESTATIC  = 0xb8,
    OP_INVOKEINTERFACE = 0xb9,
    
    OP_RETURN       = 0xb1,
    OP_ARETURN      = 0xb0,
    OP_IRETURN      = 0xac,
};

} // namespace core
} // namespace j2me

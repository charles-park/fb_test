//-----------------------------------------------------------------------------
//
// 2022.04.02 my typedefs (chalres-park)
//
//-----------------------------------------------------------------------------
#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

//------------------------------------------------------------------------------------------------
#if defined(__DEBUG__)
	#define	dbg(fmt, args...)	fprintf(stdout,"%s(%d) : " fmt, __func__, __LINE__, ##args)
#else
	#define	dbg(fmt, args...)
#endif

#define	err(fmt, args...)	fprintf(stderr,"[err : %s] : " fmt, __func__, ##args)
#define	info(fmt, args...)	fprintf(stdout,"[INFO] : " fmt, ##args)

//------------------------------------------------------------------------------------------------
typedef unsigned char       byte_t;
typedef unsigned char       uchar_t;
typedef unsigned int        uint_t;
typedef unsigned short      ushort_t;
typedef unsigned short      word_t;
typedef unsigned long       ulong_t;
typedef enum {false, true}  bool;

//------------------------------------------------------------------------------------------------
typedef struct bit8__t {
    uchar_t     b0  :1;
    uchar_t     b1  :1;
    uchar_t     b2  :1;
    uchar_t     b3  :1;
    uchar_t     b4  :1;
    uchar_t     b5  :1;
    uchar_t     b6  :1;
    uchar_t     b7  :1;
}   bit8_t;

typedef union bit8__u {
    uchar_t     uc;
    bit8_t      bits;
}   bit8_u;

//------------------------------------------------------------------------------------------------
typedef struct bit16__t {
    ushort_t    b0  :1;
    ushort_t    b1  :1;
    ushort_t    b2  :1;
    ushort_t    b3  :1;
    ushort_t    b4  :1;
    ushort_t    b5  :1;
    ushort_t    b6  :1;
    ushort_t    b7  :1;

    ushort_t    b8  :1;
    ushort_t    b9  :1;
    ushort_t    b10 :1;
    ushort_t    b11 :1;
    ushort_t    b12 :1;
    ushort_t    b13 :1;
    ushort_t    b14 :1;
    ushort_t    b15 :1;
}   bit16_t;

typedef union bit16__u {
    uchar_t     uc[2];
    ushort_t    us;
    bit16_t     bits;
}   bit16_u;

//------------------------------------------------------------------------------------------------
typedef struct bit32__t {
    uint_t      b0  :1;
    uint_t      b1  :1;
    uint_t      b2  :1;
    uint_t      b3  :1;
    uint_t      b4  :1;
    uint_t      b5  :1;
    uint_t      b6  :1;
    uint_t      b7  :1;

    uint_t      b8  :1;
    uint_t      b9  :1;
    uint_t      b10 :1;
    uint_t      b11 :1;
    uint_t      b12 :1;
    uint_t      b13 :1;
    uint_t      b14 :1;
    uint_t      b15 :1;

    uint_t      b16 :1;
    uint_t      b17 :1;
    uint_t      b18 :1;
    uint_t      b19 :1;
    uint_t      b20 :1;
    uint_t      b21 :1;
    uint_t      b22 :1;
    uint_t      b23 :1;

    uint_t      b24 :1;
    uint_t      b25 :1;
    uint_t      b26 :1;
    uint_t      b27 :1;
    uint_t      b28 :1;
    uint_t      b29 :1;
    uint_t      b30 :1;
    uint_t      b31 :1;
}   bit32_t;

typedef union bit32__u {
    uchar_t     uc[4];
    uint_t      ui;
    ulong_t     ul;
    bit32_t     bits;
}   bit32_u;

//------------------------------------------------------------------------------------------------
#endif  // #define __TYPEDEFS_H__
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

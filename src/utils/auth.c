/*
    auth.c -- Manage passwords for HTTP authorization.

    This file provides facilities for creating passwords for Appweb. It uses basic encoding and decoding using the 
    base64 encoding scheme and the MD5 Message-Digest algorithms developed by RSA. This module is used by both basic 
    and digest authentication services.
 */

/********************************* Includes ***********************************/

#include    "http.h"

/********************************** Locals ************************************/

typedef struct User {
    char        *name;
    char        *realm;
    char        *password;
    bool        enabled;
    struct User *next;
} User;
  
/********************************* Forwards ***********************************/

static User *createUser(cchar *realm, cchar *name, cchar *password, bool enabled);
static void addUser(char *realm, char *user, char *password, bool enabled);
static char *getPassword(char *passBuf, int passLen);
static void printUsage(cchar *programName);
static int  readPassFile(char *passFile);
static char* trimWhiteSpace(char *str);
static int  updatePassFile(char *passFile);

typedef struct {
    uint  state[4];
    uint  count[2];
    uchar buffer[64];
} MD5_CONTEXT;

static char *maMD5(char *string);
static void maMD5Init(MD5_CONTEXT *);
static void maMD5Update(MD5_CONTEXT *, uchar *, uint );
static void maMD5Final(uchar [16], MD5_CONTEXT *);
static char *maMD5binary(uchar *buf, int length);

#if BLD_WIN_LIKE || VXWORKS
static char *getpass(char *prompt);
#endif

#define MAX_PASS    64

/********************************** Locals ************************************/

static User     *users;
static cchar    *programName;

/*********************************** Code *************************************/

int main(int argc, char *argv[])
{
    Mpr     *mpr;
    char    passBuf[MAX_PASS], buf[MAX_PASS * 2];
    char    *password, *passFile, *userName;
    char    *encodedPassword, *realm, *cp;
    int     i, errflg, create, nextArg;
    bool    enable;

    mpr = mprCreate(argc, argv, 0);
    mprSetAppName(argv[0], NULL, NULL);
    programName = mprGetAppName(mpr);

    userName = 0;
    create = errflg = 0;
    password = 0;
    enable = 1;

    for (i = 1; i < argc && !errflg; i++) {
        if (argv[i][0] != '-') {
            break;
        }

        for (cp = &argv[i][1]; *cp && !errflg; cp++) {

            if (*cp == 'c') {
                create++;

            } else if (*cp == 'e') {
                enable = 1;

            } else if (*cp == 'd') {
                enable = 0;

            } else if (*cp == 'p') {
                if (++i == argc) {
                    errflg++;
                } else {
                    password = argv[i];
                    break;
                }

            } else {
                errflg++;
            }
        }
    }
    nextArg = i;

    if ((nextArg + 3) > argc) {
        errflg++;
    }

    if (errflg) {
        printUsage(programName);
        exit(2);
    }   

    passFile = argv[nextArg++];
    realm = argv[nextArg++];
    userName = argv[nextArg++];

    if (!create) {
        if (readPassFile(passFile) < 0) {
            exit(2);
        }
        if (!mprPathExists(passFile, R_OK)) {
            mprError("%s: Can't find %s", programName, passFile);
            exit(3);
        }
        if (!mprPathExists(passFile, W_OK)) {
            mprError("%s: Can't write to %s", programName, passFile);
            exit(4);
        }
    } else {
        if (mprPathExists(passFile, R_OK)) {
            mprError("%s: Can't create %s, already exists", programName, passFile);
            exit(5);
        }
    }

    if (password == 0) {
        password = getPassword(passBuf, sizeof(passBuf));
        if (password == 0) {
            exit(1);
        }
    }

    sprintf(buf, "%s:%s:%s", userName, realm, password);
    encodedPassword = maMD5(buf);

    addUser(realm, userName, encodedPassword, enable);

    if (updatePassFile(passFile) < 0) {
        exit(6);
    }

    if (encodedPassword) {
        free(encodedPassword);
    }
    
    return 0;
}

 

static int readPassFile(char *passFile)
{
    FILE    *fp;
    User    *up;
    char    buf[MAX_PASS * 2];
    char    *tok, *enabledSpec, *name, *realm, *password;
    bool    enabled;
    int     line;

    fp = fopen(passFile, "r" MPR_TEXT);
    if (fp == 0) {
        mprError("%s: Can't open %s", programName, passFile);
        return MPR_ERR_CANT_OPEN;
    }

    line = 0;
    while (fgets(buf, sizeof(buf), fp) != 0) {
        line++;
        enabledSpec = stok(buf, ":", &tok);
        name = stok(0, ":", &tok);
        realm = stok(0, ":", &tok);
        password = stok(0, "\n\r", &tok);
        if (enabledSpec == 0 || name == 0 || realm == 0 || password == 0) {
            mprError("%s: Badly formed password on line %d", programName, line);
            return MPR_ERR_BAD_SYNTAX;
        }
        name = trimWhiteSpace(name);
        if (*name == '#' || *name == '\0') {
            continue;
        }
        enabled = (enabledSpec[0] == '1'); 
        
        realm = trimWhiteSpace(realm);
        password = trimWhiteSpace(password);

        up = createUser(realm, name, password, enabled);
        up->next = users;
        users = up;
    }
    fclose(fp);
    return 0;
}
 

static void manageUser(User *up, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(up->name);
        mprMark(up->realm);
        mprMark(up->password);
    }
}


static User *createUser(cchar *realm, cchar *name, cchar *password, bool enabled)
{
    User    *up;

    if ((up = mprAllocObj(User, manageUser)) != 0) {
        up->realm = sclone(realm);
        up->name = sclone(name);
        up->password = sclone(password);
        up->enabled = enabled;
    }
    return up;
}


static void addUser(char *realm, char *name, char *password, bool enabled)
{
    User    *up;

    up = users;
    while (up) {
        if (strcmp(name, up->name) == 0 && strcmp(realm, up->realm) == 0) {
            up->password = sclone(password);
            up->enabled = enabled;
            return;
        }
        up = up->next;
    }
    up = createUser(realm, name, password, enabled);
    up->next = users;
    users = up;
}



static int updatePassFile(char *passFile)
{
    User        *up;
    char        *tempFile, buf[MPR_BUFSIZE];
    MprFile     *file;

    tempFile = sfmt("%s.tmp", passFile);
    file = mprOpenFile(tempFile, O_CREAT | O_TRUNC | O_WRONLY | O_TEXT, 0664);
    if (file == 0) {
        mprError("%s: Can't open %s", programName, tempFile);
        return MPR_ERR_CANT_OPEN;
    }
    up = users;
    while (up) {
        sprintf(buf, "%d: %s: %s: %s\n", up->enabled, up->name, up->realm, up->password);
        if (mprWriteFile(file, buf, strlen(buf)) < 0) {
            mprError("%s: Can't write to %s", programName, tempFile);
            mprCloseFile(file);
            return MPR_ERR_CANT_WRITE;
        }
        up = up->next;
    }
    mprCloseFile(file);
    if (rename(tempFile, passFile) < 0) {
        mprError("%s: Can't rename %s to %s", programName, tempFile, passFile);
        return MPR_ERR_CANT_COMPLETE;
    }
    return 0;
}
 
 

static char *getPassword(char *passBuf, int passLen)
{
    char    *password, *confirm;

    password = getpass("New password: ");
    strncpy(passBuf, password, passLen - 1);
    confirm = getpass("Confirm password: ");
    if (strcmp(passBuf, confirm) == 0) {
        return passBuf;
    }
    mprError("%s: Error: Password not verified", programName);
    return 0;
}



#if WINCE
static char *getpass(char *prompt)
{
    return "NOT-SUPPORTED";
}

#elif BLD_WIN_LIKE || VXWORKS
static char *getpass(char *prompt)
{
    static char password[MAX_PASS];
    int     c, i;

    fputs(prompt, stderr);
    for (i = 0; i < (int) sizeof(password) - 1; i++) {
#if VXWORKS
        c = getchar();
#else
        c = _getch();
#endif
        if (c == '\r' || c == EOF) {
            break;
        }
        if ((c == '\b' || c == 127) && i > 0) {
            password[--i] = '\0';
            fputs("\b \b", stderr);
            i--;
        } else if (c == 26) {           /* Control Z */
            c = EOF;
            break;
        } else if (c == 3) {            /* Control C */
            fputs("^C\n", stderr);
            exit(255);
        } else if (!iscntrl(c) && (i < (int) sizeof(password) - 1)) {
            password[i] = c;
            fputc('*', stderr);
        } else {
            fputc('', stderr);
            i--;
        }
    }
    if (c == EOF) {
        return "";
    }
    fputc('\n', stderr);
    password[i] = '\0';
    return password;
}

#endif /* BLD_WIN_LIKE */
 
/*
    Display the usage
 */

static void printUsage(cchar *programName)
{
    mprPrintfError("usage: %s [-c] [-p password] passwordFile realm user\n"
        "Options:\n"
        "    -c              Create the password file\n"
        "    -p passWord     Use the specified password\n"
        "    -e              Enable (default)\n"
        "    -d              Disable\n", programName);
}



static char* trimWhiteSpace(char *str)
{
    ssize   len;

    if (str == 0) {
        return str;
    }
    while (isspace((int) *str)) {
        str++;
    }
    len = strlen(str) - 1;
    while (isspace((int) str[len])) {
        str[len--] = '\0';
    }
    return str;
}


/************************************ MD5 Code ********************************/
/*
    Constants for MD5Transform routine.
 */
#define CRYPT_HASH_SIZE 16

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static uchar PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
   F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/*
   ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/*
   FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
   Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (uint)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (uint)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (uint)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (uint)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/*************************** Forward Declarations *****************************/

static void MD5Transform(uint [4], uchar [64]);
static void Encode(uchar *, uint *, unsigned);
static void Decode(uint *, uchar *, unsigned);
static void MD5_memcpy(uchar *, uchar *, unsigned);
static void MD5_memset(uchar *, int, unsigned);

/*********************************** Code *************************************/
/*
    maMD5binary returns the MD5 hash. FUTURE -- better name
 */

char *maMD5binary(uchar *buf, int length)
{
    const char      *hex = "0123456789abcdef";
    MD5_CONTEXT     md5ctx;
    uchar           hash[CRYPT_HASH_SIZE];
    char            *r, *str;
    char            result[(CRYPT_HASH_SIZE * 2) + 1];
    int             i;

    /*
     *  Take the MD5 hash of the string argument.
     */
    maMD5Init(&md5ctx);
    maMD5Update(&md5ctx, buf, (unsigned) length);
    maMD5Final(hash, &md5ctx);

    for (i = 0, r = result; i < 16; i++) {
        *r++ = hex[hash[i] >> 4];
        *r++ = hex[hash[i] & 0xF];
    }
    *r = '\0';

    str = (char*) malloc(sizeof(result));
    strcpy(str, result);

    return str;
}


/*
    Convenience call to webMD5binary 
 */
char *maMD5(char *string)
{
    return maMD5binary((uchar*)string, (int) slen(string));
}


/*
    MD5 initialization. Begins an MD5 operation, writing a new context.
 */ 
void maMD5Init(MD5_CONTEXT *context)
{
    context->count[0] = context->count[1] = 0;

    /*
     * Load constants
     */
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}


/*
    MD5 block update operation. Continues an MD5 message-digest operation, 
    processing another message block, and updating the context.
 */
void maMD5Update(MD5_CONTEXT *context, uchar *input, unsigned inputLen)
{
    unsigned    i, index, partLen;

    index = (unsigned) ((context->count[0] >> 3) & 0x3F);

    if ((context->count[0] += ((uint) inputLen << 3)) < 
            ((uint) inputLen << 3)){
        context->count[1]++;
    }
    context->count[1] += ((uint)inputLen >> 29);
    partLen = 64 - index;

    if (inputLen >= partLen) {
        MD5_memcpy((uchar*) &context->buffer[index], 
            (uchar*) input, partLen);
        MD5Transform(context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64) {
            MD5Transform (context->state, &input[i]);
        }
        index = 0;

    } else {
        i = 0;
    }

    MD5_memcpy((uchar*) &context->buffer[index], 
        (uchar*) &input[i], inputLen-i);
}


/*
    MD5 finalization. Ends an MD5 message-digest operation, writing the message digest and zeroizing the context.
 */ 
void maMD5Final(uchar digest[16], MD5_CONTEXT *context)
{
    uchar   bits[8];
    unsigned        index, padLen;

    /* 
     *  Save number of bits 
     */
    Encode(bits, context->count, 8);

    /*
     *   Pad out to 56 mod 64.
     */
    index = (unsigned)((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    maMD5Update(context, PADDING, padLen);

    /*
     *   Append length (before padding) 
     */
    maMD5Update(context, bits, 8);

    /*
     *   Store state in digest 
     */
    Encode(digest, context->state, 16);

    /*
     *   Zeroize sensitive information.
     */
    MD5_memset((uchar*)context, 0, sizeof (*context));
}


/*
    MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform(uint state[4], uchar block[64])
{
    uint a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode (x, block, 64);

    /*
     *   Round 1 
     */
    FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /*
     *   Round 2 
     */
    GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG (d, a, b, c, x[10], S22, 0x2441453);  /* 22 */
    GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH (b, c, d, a, x[ 6], S34, 0x4881d05);  /* 44 */
    HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* 
     *  Zeroize sensitive information. 
     */     
    MD5_memset ((uchar*) x, 0, sizeof (x));
}


/*
    Encodes input (uint) into output (uchar). Assumes len is a multiple of 4.
 */
static void Encode(uchar *output, uint *input, unsigned len)
{
    unsigned    i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = (uchar) (input[i] & 0xff);
        output[j+1] = (uchar) ((input[i] >> 8) & 0xff);
        output[j+2] = (uchar) ((input[i] >> 16) & 0xff);
        output[j+3] = (uchar) ((input[i] >> 24) & 0xff);
    }
}


/*
    Decodes input (uchar) into output (uint). Assumes len is a multiple of 4.
 */
static void Decode(uint *output, uchar *input, unsigned len)
{
    unsigned    i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((uint) input[j]) | 
            (((uint) input[j+1]) << 8) |
            (((uint) input[j+2]) << 16) | 
            (((uint) input[j+3]) << 24);
}


/*
    FUTURE: Replace "for loop" with standard memcpy if possible.
 */
static void MD5_memcpy(uchar *output, uchar *input, unsigned len)
{
    unsigned    i;

    for (i = 0; i < len; i++)
        output[i] = input[i];
}


/*
   FUTURE: Replace "for loop" with standard memset if possible.
 */
static void MD5_memset(uchar *output, int value, unsigned len)
{
    unsigned    i;

    for (i = 0; i < len; i++)
        ((char*) output)[i] = (char) value;
}


#if VXWORKS
/*
    VxWorks link resolution
 */
int _cleanup() {
    return 0;
}
int _exit() {
    return 0;
}
#endif

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://embedthis.com 
    

    MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
    
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    
    THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
    
    RSA License Details
    -------------------
    
    License to copy and use this software is granted provided that it is 
    identified as the "RSA Data Security, Inc. MD5 Message-Digest Algorithm" 
    in all material mentioning or referencing this software or this function.
    
    License is also granted to make and use derivative works provided that such
    works are identified as "derived from the RSA Data Security, Inc. MD5 
    Message-Digest Algorithm" in all material mentioning or referencing the 
    derived work.
    
    RSA Data Security, Inc. makes no representations concerning either the 
    merchantability of this software or the suitability of this software for 
    any particular purpose. It is provided "as is" without express or implied 
    warranty of any kind.
    
    These notices must be retained in any copies of any part of this documentation 
    and/or software.
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

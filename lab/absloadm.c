/*компиляция используя -lncurses
например, gcc absgraph.c -o absgraph -lncurses
*/
#include <stdio.h>  /*подкл.библ.ф-й ст.в/выв */
#include <string.h> /*подкл.библ.ф-й стр.симв.*/
#include <stdlib.h> /*подкл.библ.ф-й преобр.д.*/
// #include <ctype.h>                                /*подкл.библ.ф-й преобр.с.*/
// #include <curses.h>

#include <wctype.h>
#include <locale.h>
#include <ncursesw/ncurses.h>

#define NSPIS 5    /*разм.списка загр.прогр. */
#define NOBJ 50    /*разм.масс.об'ектных карт*/
#define DOBLZ 1024 /*длина области загрузки  */
#define NOP (6 + 9)/*кол-во обрабатываемых   */
                   /* команд                 */

#define GREEN_RED_LENGTH    67    // 左侧红色绿色区域宽度
#define BLUE_YELLOW_LENGTH  12    // 右侧蓝色和黄色宽度
#define LEN_ROW_TEX         80    // 输入 .tex 文件一行的长度（字符数）

char NFIL[30] = "\x0";

int IOBJC = 0;                   /*инд.вакантн.стр. OBJCARD*/
char OBJCARD[NOBJ][LEN_ROW_TEX]; /*масс.хранен.об'ектн.карт*/

int ISPIS = 0;                   /*инд.вакантн.стр. SPISOK */
char SPISOK[NSPIS][LEN_ROW_TEX]; /*масс.хранен.списка прогр*/

WINDOW *wblue, *wgreen, *wred, *wcyan, *wmargenta, *wyellow;

struct STR_BUF_TXT /*структ.буфера карты TXT */
{
  unsigned char POLE1;     /*место для кода 0x02     */
  unsigned char POLE2[3];  /*поле типа об'ектн.карты */
  unsigned char POLE3;     /*пробел                  */
  unsigned char ADOP[3];   /*относит.адрес опреации  */
  unsigned char POLE5[2];  /*пробелы                 */
  unsigned char DLNOP[2];  /*длина операции          */
  unsigned char POLE7[2];  /*пробелы                 */
  unsigned char POLE71[2]; /*внутренний идент.прогр. */
  unsigned char OPER[56];  /*тело операции           */
  unsigned char POLE9[8];  /*идентификационное поле  */
};

union /*определить об'единение  */
{
  struct STR_BUF_TXT STR_TXT;         /*структура буфера        */
  unsigned char BUF_TXT[LEN_ROW_TEX]; /*буфер карты TXT         */
} TXT;

unsigned char INST[6]; /*массив, содерж. обрабат.*/
                       /*команду                 */


// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 自定义功能 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// 定义一个宏，自动填充函数名和行号
#define LOG_MSG(msg, ...) { log_to_file(__FUNCTION__, __LINE__, msg, ##__VA_ARGS__); }

unsigned int  cc        = 0;  // 条件码：Condition Code, представляет код состояния (Condition Code), который сохраняет результат сравнения команды C: 0 означает равенство, 1 означает меньше, 2 означает больше
unsigned int  com_count = 1;  // 记录执行过多少条指令 является счетчиком команд, который записывает, сколько команд было выполнено
unsigned int  SLL_count = 0;  // 记录 SLL 执行过多少次
// char* state_loop        = "NEVER IN LOOP";
FILE* logFile = NULL;

// 函数用于打开日志文件，返回FILE指针
void create_log_file(const char* file_path) {
    logFile = fopen(file_path, "w"); // 以写入模式打开文件，每次打开文件重写
    if (logFile == NULL) perror("Failed to open log file");
}

// 函数用于关闭日志文件
void close_log_file() {
    if (logFile != NULL) fclose(logFile);
}

// 日志函数的声明，这个函数将日志信息写入通过FILE指针指向的文件
void log_to_file(const char *function, int line, const char *msg, ...) {
    if (logFile == NULL) {
        fprintf(stderr, "Log file is not open.\n");
        return;
    }
    fprintf(logFile, "[%s:%d] ", function, line);

    va_list args;
    va_start(args, msg);
    vfprintf(logFile, msg, args);
    va_end(args);

    fprintf(logFile, "\n");
    fflush(logFile);  // 刷新文件流，确保立即写入文件
}

/**
 * @brief 把指定长度的字符串以十六进制半字的形式返回
 * 
 * @param str 字符串
 * @return int 字符串长度
 */
char* getStrHexBlock(const char* str, const int length)
{
    char* cur_line = malloc(length * 7);
    char hex_str[4];      // 足够保存两个字符的十六进制数及其前缀0x和空终止符
    for (int i = 0; i < length; i++)
    {
      if (i % 16 == 0 && i != 0) strcat(cur_line, "\n");
      sprintf(hex_str, "%.2X ", (unsigned char)str[i]);
      strcat(cur_line, hex_str);
    }
    return cur_line;
}

// 函数用于生成一个 long 类型变量的每个比特位的字符串表示
char *getBitStr(long num)
{
    int bits = sizeof(unsigned long) * 8; // 计算总共有多少比特位
    int stringSize = bits + bits / 4; // 加上空格和终结符的空间
    char* bitString = (char*)malloc(stringSize + 1); // 分配字符串空间
    if (!bitString) return NULL; // 内存分配失败处理

    int pos = 0; // 字符串位置指针
    for (int i = bits - 1; i >= 0; i--) {
        int bit = (num >> i) & 1; // 提取当前位
        bitString[pos++] = bit + '0'; // 存储为字符
        if (i % 4 == 0 && i != 0) { // 每4位后添加一个空格，除了最后一位
            bitString[pos++] = ' ';
        }
    }
    bitString[pos] = '\0'; // 终结字符串

    return bitString;

}

/**
 * @brief 为绿色部分添加换行符(也就是把每行剩余部分全部都填充成空格)。如果不使用这个函数那么可能在 UTF-8 环境下会造成错误（绿色区域显示异常）
 */
void greenNewLine()
{
  int x, y;  // 光标位置
  getyx(wgreen, y, x);
  LOG_MSG("光标位置 x = [%d], y = [%d]", x, y)
  for (int i = 0; i < GREEN_RED_LENGTH - x; i++)
  {
    waddch(wgreen, ' ');
  }
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int FRR(); /*подпр.обр.опер.RR-форм. */
int FRX(); /*подпр.обр.опер.RX-форм. */

// #####################################################################
/** подпр.обр.опер.SS-форм.
 * L  длина операндов SS формата
 * B2 номер базового регистра второго операнда SS формата
 * D2 cмещение второго операнда SS формата
 */
int L, B2, D2;
int FSS(); /*подпр.обр.опер.SS-форм. */
int FRS(); /*подпр.обр.опер.SS-форм. */
// #####################################################################
/*..........................................................................*/

/* 在屏幕上初始化代码 */
int X1 = 1;            /* инициализация коорд.   */
int Y1 = 15;           /* на экране              */

/**
 * R1 - RR 和 RX 格式中第一个寄存器操作数的编号
 * R2 - RX 格式中第二个寄存器操作数的编号
 * D  - RX 格式中的偏移量
 * X  - RX 格式中的索引寄存器编号
 * B  - RX 格式中的基寄存器编号
 */
int R1, /*номер 1-го регистра-операнда в форматах RR и RX*/
    R2, /*номер 2-го регистра-операнда в формате RX      */
    D,  /*смещение в формате RX   */
    X,  /*номер индексн. регистра в формате RX           */
    B;  /*номер базового регистра в формате RX           */

unsigned long I,       /*счетчик адр.тек.ком-ды  */
    BAS_ADDR,          /*адрес начала обл.загруз.*/
    I1, ADDR, ARG, VS; /*вспомогательные перем.  */

//  IBM370 中的 16 个寄存器
unsigned long VR[16], /*массив,содерж.знач.рег. */
    LIGHTPTR;         /*адрес начала обл.отсвет.*/

int x, y, i, j, k, kk; /*рабочие переменные      */

int CUR_IND; /*индекс масс.обл.загр.,  */
             /*соотв.текущ.ком-де прогр*/
int BAS_IND; /*индекс масс.обл.загр.,  */
             /*соотв.первой ком-ды прог*/

/**
 * постоянная часть шаблона отсветки регистров на экране консоли
 * 控制台屏幕上寄存器高亮模式的永久部分     
 */
union U1
{       
  struct
  {
    char NOMREG[3];
    char ZNEQUEL[1];
  } REGS_ASC[16];

  char BUFR[16][4];
} R_ASC;


/**
 * шаблон для расчета элементов абсолютного
 * 计算绝对值元素的模板
 */
union u2
{
  struct
  {
    unsigned int SMESH;
    unsigned int SEGM;
  } VAL_P;
  unsigned char *P_OBLZ;
} POINT;


/**
 * область загрузки трассируемой программы
 * 跟踪程序的加载区
 */
unsigned char OBLZ[DOBLZ];

/*
***** ТАБЛИЦА машинных операций
*/
struct TMOP /*структ.стр.табл.маш.опер*/
{
  unsigned char MNCOP[5]; /*мнемокод операции       */
  unsigned char CODOP;    /*машинный код операции   */
  unsigned char DLOP;     /*длина операции в байтах */
  int (*BXPROG)();        /*указатель на подпр.обраб*/
} T_MOP[NOP] =            /*об'явление табл.маш.опер*/
    {
        {{'B', 'A', 'L', 'R', ' '}, '\x05', 2, FRR}, /*инициализация           */
        {{'B', 'C', 'R', ' ', ' '}, '\x07', 2, FRR}, /*строк                   */
        {{'S', 'T', ' ', ' ', ' '}, '\x50', 4, FRX}, /*таблицы                 */
        {{'L', ' ', ' ', ' ', ' '}, '\x58', 4, FRX}, /*машинных                */
        {{'A', ' ', ' ', ' ', ' '}, '\x5A', 4, FRX}, /*операций                */
        {{'S', ' ', ' ', ' ', ' '}, '\x5B', 4, FRX},
        // #####################################################################
        {{'M', 'V', 'C', ' ', ' '}, '\xD2', 6, FSS},
        {{'C', 'V', 'B', ' ', ' '}, '\x4F', 4, FRX},
        {{'L', 'R', ' ', ' ', ' '}, '\x18', 2, FRR},
        {{'N', ' ', ' ', ' ', ' '}, '\x54', 4, FRX},
        {{'C', ' ', ' ', ' ', ' '}, '\x59', 4, FRX},
        {{'B', 'C', ' ', ' ', ' '}, '\x47', 4, FRX},
        {{'S', 'L', 'L', ' ', ' '}, '\x89', 4, FRS},
        {{'S', 'R', 'L', ' ', ' '}, '\x88', 4, FRS},
        {{'S', 'T', 'H', ' ', ' '}, '\x40', 4, FRX}
        // #####################################################################
};
//..........................................................................
// п р о г р а м м а реализации семантики команды BALR
int P_BALR(void)
{
  if (R2 != 0) I = VR[R2];
  if (R1 != 0) VR[R1] = I;

  return 0;
}

//..........................................................................
// п р о г р а м м а реализации семантики команды BCR с маской 15
int P_BCR(void)
{
  int ret = 1;
  if (R1 == 15)
  {
    ret = 0;
    if ((VR[R2] != 0) && (R2 != 0)) I = VR[R2];
    else
    {
      if (R2 != 0)
      {
        waddstr(wcyan, "переход по адресу = 0 или завершение трассировки программы после нажатия клавиши");
        wrefresh(wcyan);
        ret = 1;
      }
    }
  }

  return ret;
}
/*..........................................................................*/

/*  п р о г р а м м а     */
/*реализации семантики    */
/*команды ST              */
/*рабочие                 */
/*переменные              */
int P_ST()       
{                
  ADDR = VR[B] + VR[X] + D; /*вычисление абс.адреса и */
  int sm = (int)(ADDR - I); /*смещения                */

  /*преобразование содержим.*/
  /*РОН, использованного в  */
  /*качестве первого оп-да, */
  /*к виду, принятому в     */
  /*ЕС ЭВМ                  */
  char bytes[4]; 
  bytes[0] = (VR[R1] - VR[R1] % 0x1000000L) / 0x1000000L;
  bytes[1] = ((VR[R1] - VR[R1] % 0x10000L) / 0x10000L) % 0x100;
  bytes[2] = ((VR[R1] % 0x10000L) - ((VR[R1] % 0x10000L) % 0x100)) / 0x100;
  bytes[3] = (VR[R1] % 0x10000L) % 0x100;

  for (int i = 0; i < 4; i++)                    /*запись преобразованого  */
    OBLZ[BAS_IND + CUR_IND + sm + i] = bytes[i]; /*значения по адресу 2-г  */
                                                 /*операнда                */
  return 0;                                      /*успешное заверш.прогр.  */
}

/*..........................................................................*/

/*  п р о г р а м м а     */
/*реализации семантики    */
/*команды L               */
/*рабочая переменная      */
int P_L()
{
  ADDR = VR[B] + VR[X] + D;                         /*вычисление абс.адреса и */
  int sm = (int)(ADDR - I);                         /*смещения                */
  VR[R1] =                                          /*преобразование содержим.*/
      OBLZ[BAS_IND + CUR_IND + sm]     * 0x1000000L +/*второго операнда к виду,*/
      OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L + /*принятому в IBM PC, и   */
      OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +    /*запись в РОН, использ.в */
      OBLZ[BAS_IND + CUR_IND + sm + 3];             /*качестве 1-го операнда  */

  return 0; /*успешное заверш.прогр.  */
}

/*..........................................................................*/

/*  п р о г р а м м а     */
/*реализации семантики    */
/*команды A               */
/*рабочая переменная      */
int P_A()
{
  ADDR = VR[B] + VR[X] + D;                           /*вычисление абс.адреса и */
  int sm = (int)(ADDR - I);                           /*смещения                */
  ARG = OBLZ[BAS_IND + CUR_IND + sm] * 0x1000000L +   /*формирование содержимого*/
        OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L + /*второго операнда в сог- */
        OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +    /*лашениях ЕС ЭВМ         */
        OBLZ[BAS_IND + CUR_IND + sm + 3];            
                                                      /*и                       */
  VR[R1] = VR[R1] + ARG;                              /*сложение с 1-м операндом*/

  return 0; /*успешное заверш. прогр. */
}

/*..........................................................................*/

/* п р о г р а м м а      */
/* реализации семантики   */
/* команды S              */
/* рабочая переменная      */
int P_S() 
{         
  int sm; 

  ADDR = VR[B] + VR[X] + D; /*вычисление рабочего     */
  sm = (int)(ADDR - I);     /*адреса и смещения       */

  ARG = OBLZ[BAS_IND + CUR_IND + sm] * 0x1000000L +   /*формирование содержимого*/
        OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L + /*второго операнда в сог- */
        OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +    /*лашениях ЕС ЭВМ         */
        OBLZ[BAS_IND + CUR_IND + sm + 3];            
                                                      /* и                      */
  VR[R1] = VR[R1] - ARG;                              /*выч-ие из 1-го операнда */

  return 0; /*успешное заверш.прогр.  */
}


// #####################################################################
// #####################################################################
/**
 * @brief MVC  @BUF+5(3),A    0xD2
 *        D2 02 F0 3F F0 2A
 */
int P_MVC()
{
  ADDR = VR[B] + D;
  int sm = (int)(ADDR - I);

  ADDR = VR[B2] + D2;
  int sm2 = (int)(ADDR - I);
  for (i = 0; i < L + 1; i++)
  {
    OBLZ[BAS_IND + CUR_IND + sm + i] = OBLZ[BAS_IND + CUR_IND + sm2 + i];
  }
  return 0;
}

/**
 * @brief CVB  	@RRAB,@BUF  	0x4F	
 *        4F 30 F0 3A
 */
int P_CVB()
{
  ADDR = VR[B] + VR[X] + D;
  int sm = (int)(ADDR - I);
  ARG = 0;
  for (int i = 0; i < 15; i++)
  {
    ARG *= 10;
    if (i % 2 == 0) {
      ARG += OBLZ[BAS_IND + CUR_IND + sm + (i / 2)] >> 4;
    } else {
      ARG += OBLZ[BAS_IND + CUR_IND + sm + (i / 2)] % 16;
    }
  }
  VR[R1] = ARG;
  return 0;
}

/**
 * @brief LR   	@RRAB1,@RRAB	0x18	
 *        18 43
 */
int P_LR()
{
    VR[R1] = VR[R2];
    return 0;
}

/**
 * @brief N    	@RRAB1,@ONE 	0x54	
 *        54 40 F0 32
 */
int P_N()
{
  int sm;
  ADDR = VR[B] + VR[X] + D;
  sm = (int)(ADDR - I);
  ARG = OBLZ[BAS_IND + CUR_IND + sm]     * 0x1000000L +
        OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L +
        OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100 +
        OBLZ[BAS_IND + CUR_IND + sm + 3];
  VR[R1] = VR[R1] & ARG;
  return 0;
}

/**
 * @brief C    	@RRAB1,@ZERO	0x59	
 *        59 40 F0 36
 */
int P_C()
{
  int sm;
  ADDR = VR[B] + VR[X] + D;
  sm = (int) (ADDR - I);
  unsigned long var1 = VR[R1];
  unsigned long var2 = OBLZ[BAS_IND + CUR_IND + sm]     * 0x1000000L +
                       OBLZ[BAS_IND + CUR_IND + sm + 1] * 0x10000L   +
                       OBLZ[BAS_IND + CUR_IND + sm + 2] * 0x100      +
                       OBLZ[BAS_IND + CUR_IND + sm + 3];

  if      (var1 == var2) cc = 0;
  else if (var1 < var2)  cc = 1;
  else if (var1 > var2)  cc = 2;

  return 0;
}

/**
 * @brief BC   	6,@BREAK    	0x47	Если НЕ равно 0, то перейти к @BREAK (mask-bits: 0110)
 *        47 60 F0 20
 * 
 *        BC   	15,@LOOP    	0x47	Безусловный переход к @LOOP
 *        47 F0 F0 0A
 */
int P_BC()
{
    ADDR = VR[B] + VR[X] + D;
    /** Logical AND, Ex. 2 & 6 = 2, 0 & 6 = 0*/
    if ((int)(cc & R1) != 0 || R1 == 15)
    {
        I = ADDR;
        CUR_IND = (int)(I - BAS_ADDR);
        I1 = I;
    }
    return 0;
}

/**
 * @brief SLL  	@RRAB,1     	0x89	
 *        89 30 00 01
 */
int P_SLL()
{
  SLL_count++;
  ADDR = VR[B] + D;
  VR[R1] = VR[R1] << ADDR;
  return 0;
}

/**
 * @brief SRL  	@RRAB,16    	0x88	
 *        88 30 00 10
 */
int P_SRL()
{
    ADDR = VR[B] + D;
    VR[R1] = VR[R1] >> ADDR;
    return 0;
}

/**
 * @brief STH  	@RRAB, B    	0x40	Сохранине двоичные данные (полслова) в регистре @RRAB из B
 *        40 30 F0 2E
 */
int P_STH()
{
  char bytes[2];
  ADDR = VR[B] + VR[X] + D;
  int sm = (int)(ADDR - I);
  bytes[0] = ((VR[R1] % 0x10000L) - ((VR[R1] % 0x10000L) % 0x100)) / 0x100;
  bytes[1] = (VR[R1] % 0x10000L) % 0x100;
  for (int i = 0; i < sizeof(bytes); i++)
    OBLZ[BAS_IND + CUR_IND + sm + i] = bytes[i];
  return 0;
}
// #####################################################################
// #####################################################################

//..........................................................................
int FRR(void)
{
  int i, j;

  for (i = 0; i < NOP; i++)
  {
    if (INST[0] == T_MOP[i].CODOP)
    {
      waddstr(wgreen, "      ");
      for (j = 0; j < 5; j++)
        waddch(wgreen, T_MOP[i].MNCOP[j]);
      waddstr(wgreen, " ");

      j = (INST[1] - (INST[1] % 0x10)) / 0x10;
      R1 = j;
      wprintw(wgreen, "%1d, ", j);

      j = INST[1] % 0x10;
      R2 = j;
      wprintw(wgreen, "%1d", j);
      greenNewLine();
      break;
    }
  }

  return 0;
}

//...........................................................................
int FRX()
{
  int i, j;

  for (i = 0; i < NOP; i++)
  {
    if (INST[0] == T_MOP[i].CODOP)
    {
      waddstr(wgreen, "  ");
      for (j = 0; j < 5; j++)
        waddch(wgreen, T_MOP[i].MNCOP[j]);
      waddstr(wgreen, " ");

      j = INST[1] >> 4;
      R1 = j;
      wprintw(wgreen, "%.1d, ", j);

      j = INST[2] % 16;
      j = j * 256 + INST[3];
      D = j;
      wprintw(wgreen, "X'%.3X'(", j);

      j = INST[1] % 16;
      X = j;
      wprintw(wgreen, "%1d, ", j);

      j = INST[2] >> 4;
      B = j;
      wprintw(wgreen, "%1d)", j);

      ADDR = VR[B] + VR[X] + D;
      wprintw(wgreen, "\t%.06lX", ADDR);

      // if (ADDR % 4 != 0)
      // {
      //   LOG_MSG("FRX 异常")
      //   return (7);  // ERROR
      // }
      greenNewLine();
      break;
    }
  }

  return 0;
}



// #####################################################################
// #####################################################################
// MVC
int FSS()
{
  int i, j;
  for (i = 0; i < NOP; i++)
  {
    if (INST[0] == T_MOP[i].CODOP)
    {
      waddstr(wgreen, " ");
      for (j = 0; j < 5; j++)
        waddch(wgreen, T_MOP[i].MNCOP[j]);
      // waddstr(wgreen, " ");

      j = INST[1];
      L = j;
      wprintw(wgreen, "%.1d, ", j+1);

      j = INST[2] % 16;
      j = j * 256 + INST[3];
      D = j;
      wprintw(wgreen, "X'%.3X'(", j);

      j = INST[2] >> 4;
      B = j;
      wprintw(wgreen, "%1d), ", j);

      j = INST[4] % 16;
      j = j * 256 + INST[5];
      D2 = j;
      wprintw(wgreen, "X'%.3X'(", j);

      j = INST[4] >> 4;
      B2 = j;
      wprintw(wgreen, "%1d)", j);

      ADDR = VR[B] + D;
      wprintw(wgreen," %.06lX ", ADDR);

      ADDR = VR[B2] + D2;
      wprintw(wgreen,"%.06lX", ADDR);
      greenNewLine();
      break;
    }
  }
  return 0;
}

// FRS/FLR
int FRS(void)
{
    int i, j;
    for (i = 0; i < NOP; i++)
    {
      if (INST[0] == T_MOP[i].CODOP)
      {
        waddstr(wgreen, "  ");
        for (j = 0; j < 5; j++)
          waddch(wgreen, T_MOP[i].MNCOP[j]);
        waddstr(wgreen, " ");

        j = INST[1] >> 4;
        R1 = j;
        wprintw(wgreen, "%.1d, ", j);

        j = INST[2] % 16;
        j = j * 256 + INST[3];
        D = j;
        wprintw(wgreen, "X'%.3X'(", j);

        j = INST[2] >> 4;
        B = j;
        wprintw(wgreen, "%1d)", j);

        ADDR = VR[B] + VR[X] + D;

        greenNewLine();
        // wprintw(wgreen, "\t\t%.07lX\n", ADDR);
        break;
      }
    }
    return 0;
}

// #####################################################################
// #####################################################################

//...........................................................................
// 【已修复】输出中的红色部分 ---------------------------------------------------
int wind(void)
{
  int j1, k, temp;

  x = 0;
  y = 16;

  kk = CUR_IND;
  LIGHTPTR = I1;

  for (j1 = 0; j1 < 8; j1++)
  {
    wprintw(wred, "%.06lX: ", I1);
    for (j = 0; j < 4; j++)
    {
      for (k = 0; k < 4; k++)
        wprintw(wred, "%.02X", OBLZ[BAS_IND + kk + j * 4 + k]);
      waddstr(wred, " ");
    }

    waddstr(wred, "| ");
    for (j = 0; j < 16; j++)
    {
      unsigned char tmp_c = OBLZ[BAS_IND + kk];
      // 不知道为什么会出错，手动增加 tmp_c < 128 的判断（如果移除，UTF8 的环境下会输出一个ASCII 为 240 的非法空字符）
      if (iswprint(tmp_c) && (tmp_c < 128))
      {
        waddch(wred, tmp_c);
        // printf("The ASCII value of `%c` is `%d`\n", OBLZ[BAS_IND + kk], OBLZ[BAS_IND + kk]);
      }
      else
      {
        waddstr(wred, ".");
      }

      wrefresh(wred);
      kk++;
    }

    waddstr(wred, " |\n");
    I1 += 16;
  }
  wrefresh(wred); // вывод на экран
  wclear(wred);   // очистка содержимого окна дампа

  return 0;
}

//---------------------------------------------------------------------------
// программа покомандной интерпретпции(отладки) загруженной программы
// 对已加载程序进行命令解释（调试）的程序
int sys(void)
{
  LOG_MSG("开始对已加载程序进行命令解释")

  int res, temp;
  int ch;  // 键盘输入
  int gr_pos_x, gr_pos_y;
  int ii = 0, jj = 0;
  int gr_y;
  char wstr[LEN_ROW_TEX];
  int zizi = 0, tempI;

  I = BAS_ADDR; // 设置当前地址等于起始地址

  // нижнее поле 创建一个高度为 1、宽度为 LEN_ROW_TEX 的新窗口，位置在屏幕的第 24 行，从第 0 列开始
  wmargenta = newwin(1, LEN_ROW_TEX, 24, 0);
  wbkgd(wmargenta, COLOR_PAIR(COLOR_MAGENTA));
  waddstr(wmargenta, "\"PgUp\",\"PgDn\",\"Up\",\"Down\"->просмотр дампа; \"Enter\"->выполнить очередную команду");

  // строка состояния 高度为 1 的窗口，这次位于屏幕的第 23 行
  wcyan = newwin(1, LEN_ROW_TEX, 23, 0);
  wbkgd(wcyan, COLOR_PAIR(COLOR_CYAN));

  // дамп области загрузки 创建一个高度为 8、宽度为 GREEN_RED_LENGTH 的窗口，开始于屏幕的第 15 行，用于显示程序加载区域的数据或内存转储
  wred = newwin(8, GREEN_RED_LENGTH, 15, 0);
  wbkgd(wred, COLOR_PAIR(COLOR_RED));

  // поле регистров 高度为 16、宽度为 BLUE_YELLOW_LENGTH 的窗口，从第 0 行开始并位于第 68 列，用于显示寄存器
  wblue = newwin(16, BLUE_YELLOW_LENGTH, 0, 68);
  wbkgd(wblue, COLOR_PAIR(COLOR_BLUE));

  // текст 窗口 wgreen，具有 11 行高和 GREEN_RED_LENGTH 列宽，起始位置位于 (0, 14)
  gr_pos_x = 0;
  gr_pos_y = 14;
  gr_y = 11;
  wgreen = newwin(gr_y, GREEN_RED_LENGTH, gr_pos_y, gr_pos_x); // создадим новое окно
  wbkgd(wgreen, COLOR_PAIR(COLOR_GREEN));        // выбор цветовой пары

// #####################################################################
  wyellow = newwin(7, BLUE_YELLOW_LENGTH, 16, 68);
  wbkgd(wyellow, COLOR_PAIR(COLOR_YELLOW));
// #####################################################################

  keypad(wmargenta, TRUE); // разрешить преобразование кодов клавиатуры

BEGIN:
  LOG_MSG("BEGIN: 窗口刷新")
  // все допустимые коды к-нд сравнить с текущей и при совпадениизапомнить номер строки таблицы операций
  // 将所有有效的 k-nd 代码与当前代码进行比较，如果匹配，则记住运算表的行号
  for (i = 0; i < NOP; i++)
  {
    if (OBLZ[BAS_IND + CUR_IND] == T_MOP[i].CODOP)
    {
      LOG_MSG("命中 T_MOP:\t[%.5s]\t[0x%.2X]\t[%d Byte]", T_MOP[i].MNCOP, T_MOP[i].CODOP, T_MOP[i].DLOP)

      k = i;
      wprintw(wgreen, "%.06lX: ", I);
      // рисуем окно, выводим текст
      for (j = 0; j < 6; j++) 
      {                       
        if (j < T_MOP[i].DLOP)
        { /* выдать шестнадцатеричн. запомнить его же в переменной INST, */
          wprintw(wgreen, "%.02X", OBLZ[BAS_IND + CUR_IND + j]);
          INST[j] = OBLZ[BAS_IND + CUR_IND + j];
        }                                 
        else
          INST[j] = '\x00';
      }

      res = T_MOP[i].BXPROG();
      LOG_MSG("\t┗━ T_MOP[%d].BXPROG() = [%d]\n", i, res)
      if (res != 0)   /* уйти в программу отобр.*/
        return (res); /* ассемблерного эквивале-*/
                      /* нта текущей команды,   */

      goto l0; /* перейти к дальнейшей  */
    }
  }
  return (6);

l0:
  // сдвиг окна вверх
  wrefresh(wgreen);
  ii++;
  if (gr_pos_y > 14 - gr_y + 1)
    mvwin(wgreen, gr_pos_y--, gr_pos_x);  // 如果还没有到达滚动位置，只移动窗口
  // при достижении некоторого положения, движение останавливается, и производится прокрутка окна
  else
  {
    for (jj = 0; jj < gr_y - 1; jj++)
    {
      // temp = mvwinnstr(wgreen, jj + 1, 0, wstr, GREEN_RED_LENGTH);
      // mvwaddnstr(wgreen, jj, 0, wstr, GREEN_RED_LENGTH);
      temp = mvwinnstr(wgreen, jj + 1, 0, wstr, -1);
      mvwaddnstr(wgreen, jj, 0, wstr, -1);
    }
  }
  wrefresh(wgreen);


  I += T_MOP[k].DLOP;            /*коррекция счет-ка.адреса*/
  CUR_IND = (int)(I - BAS_ADDR); /*уст-ка текущ. индекса   */
                                 /*в массиве OBLZ          */
  I1 = I;                        /*установка адреса начала */
                                 /*области отсветки        */
  for (i = 0; i < 16; i++)
  {
    if (i < 10)
      waddstr(wblue, "R0");
    else
      waddstr(wblue, "R");
    wprintw(wblue, "%d:", i);
    wprintw(wblue, "%.08lX", VR[i]);
  }
  wrefresh(wblue); // вывод на экран
  wclear(wblue);   // очистка окна регистров


  wind();  // 红色部分


  waddstr(wcyan, "готовность к выполнению очередной команды с адресом ");
  wprintw(wcyan, "%.06lX", I - T_MOP[k].DLOP);
  waddstr(wcyan, "\n");
  wrefresh(wcyan);
  wclear(wcyan);

  // #####################################################################
  waddstr(wyellow, "┏COMM. COUNT");
  waddstr(wyellow, "┗━━━ ");
  wprintw(wyellow, "%.3d", com_count);
  waddstr(wyellow, "\n");

  waddstr(wyellow, "┏SLL COUNT  ");
  waddstr(wyellow, "┗━━━ ");
  wprintw(wyellow, "%.2d", SLL_count);
  waddstr(wyellow, "\n");

  waddstr(wyellow, "┏CONDI. CODE");
  waddstr(wyellow, "┗━━━ ");
  wprintw(wyellow, "%i", cc);
  waddstr(wyellow, "\n");

  // waddstr(wyellow, state_loop);
  wrefresh(wyellow);
  wclear(wyellow);
  // #####################################################################

WAIT:
  CUR_IND = (int)(I - BAS_ADDR);
  ch = wgetch(wmargenta);

  switch (ch)
  {
  case 10:
  {
    goto SKIP;
  }

  case KEY_UP:
  {
    I1 = LIGHTPTR - 16;
    CUR_IND = (int)(I1 - BAS_ADDR);
    wind();
    goto WAIT;
  }

  case KEY_DOWN:
  {
    I1 = LIGHTPTR + 16;
    CUR_IND = (int)(I1 - BAS_ADDR);
    wind();
    goto WAIT;
  }

  case KEY_PPAGE:
  {
    I1 = LIGHTPTR - 128;
    CUR_IND = (int)(I1 - BAS_ADDR);
    wind();
    goto WAIT;
  }

  case KEY_NPAGE:
  {
    I1 = LIGHTPTR + 128;
    CUR_IND = (int)(I1 - BAS_ADDR);
    wind();
    goto WAIT;
  }
  }
  goto WAIT;

SKIP:
  /**
   * согласно  коду команды, селектируемой сч.адреса выбрать подпрогр.интерпретации семантики текущей команды
   * 根据地址计数所选指令的代码，选择一个子程序来解释当前指令的语义
   */
  com_count++;
  switch (T_MOP[k].CODOP)
  {
  case '\x05':
    P_BALR();
    break;

  case '\x07':
  {
    i = P_BCR();
    getch();
    if (i == 1) return 8;
  }
  break;

  case '\x50':
    P_ST();
    break;

  case '\x58':
    P_L();
    break;

  case '\x5A':
    P_A();
    break;

  case '\x5B':
    P_S();
    break;

// #####################################################################
  case (unsigned char)'\xD2':
    P_MVC();
    break;

  case '\x4F':
    P_CVB();
    break;

  case '\x18':
    P_LR();
    LOG_MSG("@LOOP LR @RRAB, RRAB1: 已将 @RRAB 内容写入 RRAB1")
    LOG_MSG("R03-@RRAB : %s", getBitStr(VR[3]))
    LOG_MSG("R04-@RRAB1: %s", getBitStr(VR[4]))
    break;

  case '\x54':
    P_N();
    break;

  case '\x59':
    P_C();
    break;

  case '\x47':
    P_BC();
    break;

  case (unsigned char)'\x89':
    P_SLL();
    LOG_MSG("R03-@RRAB : 0x%.8X", VR[3])
    LOG_MSG("R03-@RRAB : %s", getBitStr(VR[3]))
    break;

  case (unsigned char)'\x88':
    P_SRL();
    LOG_MSG("@BREAK 循环退出，并执行 SRL @RRAB,16")
    LOG_MSG("R03-@RRAB : %s", getBitStr(VR[3]))
    break;

  case '\x40':
    P_STH();
    break;

  default:
    LOG_MSG("错误：未识别的命令")
    break;
// #####################################################################
  }

  goto BEGIN;

  delwin(wblue);
  delwin(wred);
  delwin(wgreen);
  delwin(wmargenta);

  return 0;
}

//...........................................................................
//..........................Инициализация curses..............................
int InitCurses(void)
{
  initscr();            // 初始化 curses 库，它创建一个全屏窗口 stdscr，这是后续所有 curses 操作的基础窗口
  curs_set(0);          // 控制光标显示。参数 0 意味着光标将被隐藏
  noecho();             // 禁止回显输入字符
  cbreak();             // 将终端设置为 cbreak 模式，这意味着输入的字符会立即被程序读取，而不需要等待回车键
  keypad(stdscr, TRUE); // 允许 stdscr 窗口捕捉键盘上的特殊键，并将它们转换为适当的 curses 键码
  scrollok(wgreen, TRUE);  // 允许窗口滚动 出问题的话删除
  start_color();        // 初始化颜色显示功能

  // 定义了一些颜色对。颜色对是前景色和背景色的组合，可以通过 attron(COLOR_PAIR(n)); 来激活
  if (has_colors())
  {
    init_pair(COLOR_BLUE, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_GREEN, COLOR_BLACK, COLOR_GREEN);
    init_pair(COLOR_RED, COLOR_WHITE, COLOR_RED);
    init_pair(COLOR_CYAN, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_MAGENTA, COLOR_WHITE, COLOR_MAGENTA);
    // #####################################################################
    init_pair(COLOR_YELLOW, COLOR_RED, COLOR_YELLOW);
    // #####################################################################
  }

  return 0;
}
//...........................................................................

int main(int argc, char **argv) /* п р о г р а м м а      */
                                /*абсолютного загрузчика  */
                                /*об'ектных файлов        */
{
  // 创建日志
  create_log_file("./log.txt");
  LOG_MSG("============ 主程序开始 ============")

  // 设置 UTF-8 环境
  setlocale(LC_ALL, "en_US.UTF-8");
  LOG_MSG("设置语言为 UTF-8")

  int I, K, N, J0, res; /*рабочие                 */
  unsigned long J;      /*переменные              */
  FILE *fp;             /*программы               */
  char *ptr;

  // main programm

  if (argc != 2)
  {
    printf("%s\n", "Ошибка в командной строке");
    return -1;
  }

  ptr = argv[1];
  strcpy(NFIL, ptr);
  LOG_MSG("NFIL = [%s]", NFIL)

  if (strcmp(&NFIL[strlen(NFIL) - 3], "mod"))
  {
    goto ERR9;
    return -1;
  }

  if ((fp = fopen(NFIL, "rt")) == NULL)
    goto ERR1; /*сообщение об ошибке     */
  else
  {
    LOG_MSG("成功打开 [%s]", NFIL)

    while (!feof(fp))                          /*читать все карты файла  */
    {                                          /*со списком              */
      fgets(SPISOK[ISPIS++], LEN_ROW_TEX, fp); /*в массив SPISOK         */
      LOG_MSG("SPISOK[%d] = [%s]", ISPIS, SPISOK[ISPIS - 1])

      if (ISPIS == NSPIS)                      /*если этот массив пере-  */
      {                                        /*полнен, то:             */
        fclose(fp);                            /*закрыть файл со списком */
        goto ERR4;                             /*и выдать сообщение об ош*/
      }
    }
    fclose(fp); /*закрыть файл SPISOK     */

    /**
     * если список пустой то:  сообщение об ошибке, иначе: продолжить обработку
     * 如果列表为空：错误信息、 否则：继续处理
     */
    if (ISPIS == 0) goto ERR2;
    else            goto CONT1;
  }

CONT1:
  /**
   * перебирая все собираемые объектные файлы, иначе: читать файл до конца, размеcтить записи в массиве OBJCARD и,если считаны не все записи, то: выдать сообщ.об ошиб. закрыть очередной файл и продолжить обработку
   * 读取所有已收集的对象文件，否则：将文件读取到底，将记录放入 OBJCARD 数组，如果未读取所有记录，则：生成错误信息。关闭下一个文件并继续处理。
   */
  LOG_MSG("进入 CONT1，ISPIS = [%d]", ISPIS)
  for (I = 0; I < ISPIS; I++) /*перебирая все собираемые*/
  {
    /*об'ектные файлы,        */
    if ((fp = fopen(SPISOK[I], "rb")) == NULL) goto ERR3;                                    
    else                                             /* иначе:                 */
    {
      LOG_MSG("成功打开 [%s]", SPISOK[I])
      while (!feof(fp))                              /*  читать файл до конца, */
      {                                              /*  размеcтить записи в   */
        fread(OBJCARD[IOBJC++], LEN_ROW_TEX, 1, fp); /*  массиве OBJCARD и,если 可能出错*/

        // 读取 .tex 文件每一行，显示到文件 .........................................
        LOG_MSG("读取 [%s]：OBJCARD[%d] =\n%s\n",
                SPISOK[I], IOBJC - 1, getStrHexBlock(OBJCARD[IOBJC - 1], LEN_ROW_TEX))
        // ......................................................................

        if (IOBJC == NOBJ)                           /*  считаны не все записи,*/
        {                                            /*  то:                   */
          fclose(fp);                                /*   выдать сообщ.об ошиб.*/
          goto ERR5;                                
        }
      }          
      fclose(fp); /*  закрыть очередной файл*/

      goto CONT2; /*  и продолжить обработку*/
    }
  }

CONT2:
  LOG_MSG("成功读取 .tex 的每一行到 OBJCARD 中，IOBJC = [%d]", IOBJC)
  LOG_MSG("------------- 进入 CONT2 -------------")
  POINT.P_OBLZ = OBLZ;  /*расчитать абсолютный    */
  J = POINT.VAL_P.SEGM; /*адрес области загрузки  */
  J = J << 4;           /*OBLZ в переменной J     */
  J += POINT.VAL_P.SMESH;

  if ((J0 = (int)J % 8) == 0) /*выровнять полученное    */
  {
    BAS_ADDR = J; /*значение на границу     */
    BAS_IND = 0;
  }
  else /*двойного слова и запомн.*/
  {
    BAS_ADDR = ((J >> 3) + 1) << 3; /*его в перем.BAS_ADDR,а  */
    BAS_IND = 8 - J0;               /*соотв.индекс масс.OBLZ-в*/
  }                                 /*перем.BAS_IND           */

  for (I = 0; I < IOBJC; I++)                       /*перебирая все считанные */
  {                                                 /*карты об'ектных файлов, */
    if (!memcmp(&OBJCARD[I][1], "TXT", 3))          /*отобрать принадл.к типу */
    {                                               /*TXT и расчитать:        */
      memcpy(TXT.BUF_TXT, OBJCARD[I], LEN_ROW_TEX);
      LOG_MSG("当前处理 TXT.BUF_TXT：\n%s", getStrHexBlock(TXT.BUF_TXT, LEN_ROW_TEX))
      J = TXT.STR_TXT.ADOP[0];                      /* в переменной J начальн.*/
      J = (J << 8) + TXT.STR_TXT.ADOP[1];           /*  индекс загрузки в мас-*/
      J = (J << 8) + TXT.STR_TXT.ADOP[2];           /*  сиве OBLZ             */
      J += BAS_IND;                                 /*и                       */
     
      K = TXT.STR_TXT.DLNOP[0];            /* в переменной K длину   */
      K = (K << 8) + TXT.STR_TXT.DLNOP[1]; /* загружаемых данных     */

      int tmp_J = J;
      LOG_MSG("将 %d Byte 数据开始加载到缓冲区 OBLZ[%d] 位置", K, J)
      for (N = 0; N < K; N++)                 /*загрузить данные с очер.*/
        OBLZ[(int)J++] = TXT.STR_TXT.OPER[N]; /*об'ектной карты         */
      LOG_MSG("加载的数据：%s", getStrHexBlock(&OBLZ[tmp_J], J - tmp_J))
      LOG_MSG("当前缓冲区 OBL：\n%s\n", getStrHexBlock(OBLZ, J))
    }
  }
  LOG_MSG("------------- 成功将所有指令加载到缓冲区 -------------")

  InitCurses();
  LOG_MSG("成功初始化终端色彩")

  res = sys();

  switch (res)
  {
  case 6:
  {
    endwin();
    goto ERR6;
  }
  case 7:
  {
    endwin();
    goto ERR7;
  }
  case 8:
  {
    endwin();
    goto ERR8;
  }
  }

  endwin();

END:
  printf("\n%s\n", "завершение обработки");
  LOG_MSG("============= 程序终止 =============")
  close_log_file();
  return 0;

// Б Л О К  выдачи диагностических сообщений
ERR1:
  printf("%s%s\n", "ошибка открытия файла со списком собираемых ", "модулей");
  goto END;

ERR2:
  printf("%s\n", "пустой файл со списком собираемых модулей");
  goto END;

ERR3:
  printf("%s: %s\n",
         "ошибка открытия файла", SPISOK[I]);
  goto END;

ERR4:
  printf("%s\n",
         "переполнение списка собираемых модулей");
  goto END;

ERR5:
  printf("%s\n",
         "переполнение буфера хранения об'ектных карт");
  goto END;

ERR6:
  printf("%s\n",
         "недопустимый код команды");
  goto END;

ERR7:
  printf("прерывание - ошибка адресации\n");
  goto END;

ERR8:
  goto END;

ERR9:
  printf("%s\n", "Неверный тип файла с исходным текстом");
  goto END;
}

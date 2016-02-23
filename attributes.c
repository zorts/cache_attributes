#include <stdio.h>

static unsigned long int extractCPUAttribute(unsigned long int arg){
  unsigned long int result = 0;
  __asm(" LG 1,%1 \n"
        " ECAG 2,0,0(1) \n"
        " STG 2,%0 \n"
        : "=m"(result)
        : "m"(arg)
        : "r1 r2");
  return result;
}
typedef union{
  unsigned long int value;
  struct{
    unsigned long reserved:54;
    unsigned attributeSetIndication:2;
    unsigned attributeIndication:4;
    unsigned levelIndication:3;
    unsigned typeIndication:1;
  } args;
} ExtractArguments;

typedef struct {
  unsigned reserved:4;
  unsigned scope:2;
  unsigned cacheType:2;
} LevelInfo ;

typedef union{
  unsigned long int value;
  LevelInfo level[8];
} ExtractResults;

typedef enum {
  Summary = 0,
  LineSize = 1,
  TotalSize = 2,
  SetAssociativity = 3
} AttributeIndication;

typedef struct{
  unsigned long int lineSize;
  unsigned long int totalSize; /* in K */
  unsigned long int associativity;
} CacheSizes;

static
CacheSizes getCacheSizes(unsigned int level,
                         unsigned which){
  ExtractArguments arg;
  CacheSizes result;
  arg.value = 0;
  arg.args.levelIndication = level;
  arg.args.typeIndication = which;

  arg.args.attributeIndication = LineSize;
  result.lineSize = extractCPUAttribute(arg.value);
  arg.args.attributeIndication = TotalSize;
  result.totalSize = extractCPUAttribute(arg.value)/1024;
  arg.args.attributeIndication = SetAssociativity;
  result.associativity = extractCPUAttribute(arg.value);
  return result;
}

static
void printTypeAndSizes(unsigned int level, LevelInfo info){
  CacheSizes values;
  switch (info.cacheType){
  case 0: /* separate */
    values = getCacheSizes(level, 0);
    printf("  data: line size=%lu, set associativity=%lu, total size=%luK\n",
           values.lineSize, values.associativity, values.totalSize);
    /* fall through! */
  case 1: /* I only */
    values = getCacheSizes(level, 1);
    printf("  instruction: line size=%lu, set associativity=%lu, total size=%luK\n",
           values.lineSize, values.associativity, values.totalSize);
    break;
  case 2: /* D only */
    values = getCacheSizes(level, 0);
    printf("  data: line size=%lu, set associativity=%lu, total size=%luK\n",
           values.lineSize, values.associativity, values.totalSize);
    break;
  case 3: /* unified */
    values = getCacheSizes(level, 0);
    printf("  unified: line size=%lu, set associativity=%lu, total size=%luK\n",
           values.lineSize, values.associativity, values.totalSize);
    break;
  }
}

static
void produceReport(){
  ExtractArguments arg;
  ExtractResults result;
  arg.value = 0;
  result.value = extractCPUAttribute(arg.value);
  for (unsigned level = 0; level < 8; ++level){
    const char* scope = "undefined";

    if (result.level[level].scope == 0){
      break;
    }

    switch (result.level[level].scope){
    case 1: 
      scope = "private"; 
      break;
    case 2: 
      scope = "shared"; 
      break;
    case 3:
    default:
      break;
    }
    printf("level %u: %s\n", level, scope);
    printTypeAndSizes(level, result.level[level]);
    printf("\n");
  }
}

int main (int argc, char** argv){
  if (argc == 1){
    produceReport();
  } else if (argc != 2){
    printf("usage: %s <arg to EXTRACT CPU ATTRIBUTE>\n", argv[0]);
    return (1);
  } else {
    /* driving by hand */

    unsigned int arg = 0;
    if (1 != sscanf(argv[1], "%i", &arg)) {
      printf("%s is not a valid number\n", argv[1]);
      return (1);
    }

    unsigned long int result = extractCPUAttribute(arg);
    printf("0x%x produced 0x%lx (%lu, %dk)\n", arg, result, result, result/1024);
  }
  return 0;
}

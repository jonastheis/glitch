#include <stdio.h>
#include <stdint.h>

#define JSVAL_TYPE_DOUBLE    ((uint8_t)0x00)
#define JSVAL_TYPE_INT32     ((uint8_t)0x01)
#define JSVAL_TYPE_UNDEFINED ((uint8_t)0x02)
#define JSVAL_TYPE_BOOLEAN   ((uint8_t)0x03)
#define JSVAL_TYPE_MAGIC     ((uint8_t)0x04)
#define JSVAL_TYPE_STRING    ((uint8_t)0x05)
#define JSVAL_TYPE_SYMBOL    ((uint8_t)0x06)
#define JSVAL_TYPE_NULL      ((uint8_t)0x07)
#define JSVAL_TYPE_OBJECT    ((uint8_t)0x08)

#define JSVAL_TAG_CLEAR ((uint32_t)(0xFFFFFF80))
#define JSVAL_TAG_INT32 ((uint32_t)(JSVAL_TAG_CLEAR | JSVAL_TYPE_INT32))
#define JSVAL_TAG_UNDEFINED ((uint32_t)(JSVAL_TAG_CLEAR | \
                                        JSVAL_TYPE_UNDEFINED))
#define JSVAL_TAG_STRING ((uint32_t)(JSVAL_TAG_CLEAR | JSVAL_TYPE_STRING))
#define JSVAL_TAG_SYMBOL ((uint32_t)(JSVAL_TAG_CLEAR | JSVAL_TYPE_SYMBOL))
#define JSVAL_TAG_BOOLEAN ((uint32_t)(JSVAL_TAG_CLEAR | \
                                      JSVAL_TYPE_BOOLEAN))
#define JSVAL_TAG_MAGIC ((uint32_t)(JSVAL_TAG_CLEAR | JSVAL_TYPE_MAGIC))
#define JSVAL_TAG_NULL ((uint32_t)(JSVAL_TAG_CLEAR | JSVAL_TYPE_NULL))
#define JSVAL_TAG_OBJECT ((uint32_t)(JSVAL_TAG_CLEAR | JSVAL_TYPE_OBJECT))

int main() {
  printf("JSVAL_TAG_INT32 = %x\n", JSVAL_TAG_INT32);
  printf("JSVAL_TAG_STRING = %x\n", JSVAL_TAG_STRING);
  printf("JSVAL_TAG_OBJECT = %x\n", JSVAL_TAG_OBJECT);
}
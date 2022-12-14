/**
 * @file queue.c
 * @brief 实现环形队列
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2023-01-07
 *
 * THINK DIFFERENTLY
 */

#include "queue.h"

#undef this
#define this (*ptObj)

#if QUEUE_INT_SAFE
#define __queue_protect() SAFE_ATOM_CODE
#else
#define __queue_protect()
#endif

byte_queue_t *queue_init(byte_queue_t *ptObj, void *pBuffer,
                         uint16_t hwItemSize) {
  if (pBuffer == NULL || hwItemSize == 0 || ptObj == NULL) {
    return NULL;
  }

  __queue_protect() {
    this.pchBuffer = pBuffer;
    this.hwSize = hwItemSize;
    this.hwHead = 0;
    this.hwTail = 0;
    this.hwLength = 0;
    this.hwPeek = this.hwHead;
    this.hwPeekLength = 0;
  }
  return ptObj;
}

bool queue_in_byte(byte_queue_t *ptObj, uint8_t chByte) {
  if (ptObj == NULL) {
    return false;
  }

  if (this.hwHead == this.hwTail && 0 != this.hwLength) {
    return false;
  }

  __queue_protect() {
    this.pchBuffer[this.hwTail++] = chByte;

    if (this.hwTail >= this.hwSize) {
      this.hwTail = 0;
    }

    this.hwLength++;
    this.hwPeekLength++;
  }
  return true;
}

int16_t queue_in(byte_queue_t *ptObj, void *pchByte, uint16_t hwLength) {
  if (ptObj == NULL) {
    return -1;
  }

  if (this.hwHead == this.hwTail && 0 != this.hwLength) {
    return 0;
  }

  __queue_protect() {
    if (hwLength > (this.hwSize - this.hwLength)) {
      hwLength = this.hwSize - this.hwLength;
    }

    do {
      if (hwLength < (this.hwSize - this.hwTail)) {
        memcpy(&this.pchBuffer[this.hwTail], pchByte, hwLength);
        this.hwTail += hwLength;
        break;
      }

      memcpy(&this.pchBuffer[this.hwTail], &pchByte[0],
             this.hwSize - this.hwTail);
      memcpy(&this.pchBuffer[0], &pchByte[this.hwSize - this.hwTail],
             hwLength - (this.hwSize - this.hwTail));
      this.hwTail = hwLength - (this.hwSize - this.hwTail);
    } while (0);

    this.hwLength += hwLength;
    this.hwPeekLength += hwLength;
  }
  return hwLength;
}

bool queue_out_byte(byte_queue_t *ptObj, uint8_t *pchByte) {
  if (pchByte == NULL || ptObj == NULL) {
    return false;
  }

  if (this.hwHead == this.hwTail && 0 == this.hwLength) {
    return false;
  }

  __queue_protect() {
    *pchByte = this.pchBuffer[this.hwHead++];

    if (this.hwHead >= this.hwSize) {
      this.hwHead = 0;
    }

    this.hwLength--;
    this.hwPeek = this.hwHead;
    this.hwPeekLength = this.hwLength;
  }
  return true;
}

int16_t queue_out(byte_queue_t *ptObj, void *pchByte, uint16_t hwLength) {
  if (pchByte == NULL || ptObj == NULL) {
    return -1;
  }

  if (this.hwHead == this.hwTail && 0 == this.hwLength) {
    return 0;
  }

  __queue_protect() {
    if (hwLength > this.hwLength) {
      hwLength = this.hwLength;
    }

    do {
      if (hwLength < (this.hwSize - this.hwHead)) {
        memcpy(pchByte, &this.pchBuffer[this.hwHead], hwLength);
        this.hwHead += hwLength;
        break;
      }

      memcpy(&pchByte[0], &this.pchBuffer[this.hwHead],
             this.hwSize - this.hwHead);
      memcpy(&pchByte[this.hwSize - this.hwHead], &this.pchBuffer[0],
             hwLength - (this.hwSize - this.hwHead));
      this.hwHead = hwLength - (this.hwSize - this.hwHead);
    } while (0);

    this.hwLength -= hwLength;
    this.hwPeek = this.hwHead;
    this.hwPeekLength = this.hwLength;
  }
  return hwLength;
}

bool queue_check_empty(byte_queue_t *ptObj) {
  if (ptObj == NULL) {
    return false;
  }

  if (this.hwHead == this.hwTail && 0 == this.hwLength) {
    return true;
  }

  return false;
}

int16_t queue_get_count(byte_queue_t *ptObj) {
  if (ptObj == NULL) {
    return -1;
  }

  return (this.hwLength);
}

int16_t queue_get_available(byte_queue_t *ptObj) {
  if (ptObj == NULL) {
    return -1;
  }

  return (this.hwSize - this.hwLength);
}

bool queue_peek_check_empty(byte_queue_t *ptObj) {
  if (ptObj == NULL) {
    return false;
  }

  if (this.hwPeek == this.hwTail && 0 == this.hwPeekLength) {
    return true;
  }

  return false;
}

bool queue_peek_byte(byte_queue_t *ptObj, uint8_t *pchByte) {
  if (ptObj == NULL || pchByte == NULL) {
    return false;
  }

  if (this.hwPeek == this.hwTail && 0 == this.hwPeekLength) {
    return false;
  }

  __queue_protect() {
    *pchByte = this.pchBuffer[this.hwPeek++];

    if (this.hwPeek >= this.hwSize) {
      this.hwPeek = 0;
    }

    this.hwPeekLength--;
  }
  return true;
}

int16_t queue_peek(byte_queue_t *ptObj, void *pchByte, uint16_t hwLength) {
  if (ptObj == NULL || pchByte == NULL) {
    return -1;
  }

  if (this.hwPeek == this.hwTail && 0 == this.hwPeekLength) {
    return 0;
  }

  __queue_protect() {
    if (hwLength > this.hwPeekLength) {
      hwLength = this.hwPeekLength;
    }

    do {
      if (hwLength < (this.hwSize - this.hwPeek)) {
        memcpy(pchByte, &this.pchBuffer[this.hwPeek], hwLength);
        this.hwPeek += hwLength;
        break;
      }

      memcpy(&pchByte[0], &this.pchBuffer[this.hwPeek],
             this.hwSize - this.hwPeek);
      memcpy(&pchByte[this.hwSize - this.hwPeek], &this.pchBuffer[0],
             hwLength - (this.hwSize - this.hwPeek));
      this.hwPeek = hwLength - (this.hwSize - this.hwPeek);
    } while (0);

    this.hwPeekLength -= hwLength;
  }
  return hwLength;
}

bool queue_reset_peek_pos(byte_queue_t *ptObj) {
  if (ptObj == NULL) {
    return false;
  }

  __queue_protect() {
    this.hwPeek = this.hwHead;
    this.hwPeekLength = this.hwLength;
  }
  return true;
}

bool queue_pop_peeked(byte_queue_t *ptObj) {
  if (ptObj == NULL) {
    return false;
  }

  __queue_protect() {
    this.hwHead = this.hwPeek;
    this.hwLength = this.hwPeekLength;
  }
  return true;
}

uint16_t queue_get_peek_pos(byte_queue_t *ptObj) {
  uint16_t hwCount;

  if (ptObj == NULL) {
    return false;
  }

  __queue_protect() {
    if (this.hwPeek >= this.hwHead) {
      hwCount = this.hwPeek - this.hwHead;
    } else {
      hwCount = this.hwSize - this.hwHead + this.hwPeek;
    }
  }
  return hwCount;
}

bool queue_set_peek_pos(byte_queue_t *ptObj, uint16_t hwCount) {
  if (ptObj == NULL) {
    return false;
  }

  __queue_protect() {
    if (this.hwHead + hwCount < this.hwSize) {
      this.hwPeek = this.hwHead + hwCount;
    } else {
      this.hwPeek = hwCount - (this.hwSize - this.hwHead);
    }

    this.hwPeekLength = this.hwPeekLength - hwCount;
  }
  return true;
}

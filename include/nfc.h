#ifndef MAGICBOX_NFC_H
#define MAGICBOX_NFC_H

// Async NFC read/write on top of the MFRC522 reader (see hardware.cpp for
// pins). Story code "orders" a read or write; the operation runs from
// nfcPoll() as soon as a tag is presented, then the callback fires.
//
// Tag payload format (same for all supported tag types): 16 bytes holding a
// null-terminated text of at most NFC_TEXT_MAX chars. Stored at block 4 on
// MIFARE Classic (sector 1, factory default key) and pages 4-7 on
// NTAG / MIFARE Ultralight.

// Max text payload stored on a tag: 15 chars + '\0' = one 16-byte block.
constexpr int NFC_TEXT_MAX = 15;

// Fired once when a tag was presented and its text read successfully.
using NfcReadCallback = void (*)(const char* text);
// Fired once when a write completed successfully. On tag errors the request
// stays armed and is retried on the next presented tag, so this never fires
// with false — cancel via nfcCancel() if the write should be abandoned.
using NfcWriteCallback = void (*)(bool success);

// Arm an async read. Replaces any pending request.
void nfcRequestRead(NfcReadCallback cb);

// Arm an async write: `text` (truncated to NFC_TEXT_MAX chars) is copied into
// an internal buffer and written to the next presented tag. Replaces any
// pending request.
void nfcRequestWrite(const char* text, NfcWriteCallback cb);

// Drop any pending request and go back to idle.
void nfcCancel();

// True while a read or write request is pending.
bool nfcBusy();

// Call every loop() iteration; non-blocking (one quick radio poll per call).
void nfcPoll();

#endif // MAGICBOX_NFC_H

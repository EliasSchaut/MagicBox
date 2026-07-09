#include "nfc.h"
#include "hardware.h"
#include "utils.h"

// =============================================================
// Async NFC state machine. nfcPoll() runs one cheap radio poll per loop()
// iteration; when a tag shows up, the pending read/write is executed against
// it and the callback fires. See nfc.h for the tag payload format.
// =============================================================

namespace {

enum class NfcState : uint8_t { IDLE, READ_PENDING, WRITE_PENDING };

NfcState state = NfcState::IDLE;
NfcReadCallback readCb = nullptr;
NfcWriteCallback writeCb = nullptr;
char writeBuf[NFC_TEXT_MAX + 1];

// Debounce: a tag resting on the reader re-wakes after PICC_HaltA and must
// not re-trigger immediately when the next request is armed.
byte lastUid[10];
byte lastUidSize = 0;
unsigned long lastOpMs = 0;
constexpr unsigned long NFC_SAME_TAG_COOLDOWN_MS = 2000;

// Payload location: MIFARE Classic block 4 = sector 1, data block 0 (never
// touch sector trailers — they hold the keys). NTAG/Ultralight user memory
// starts at page 4; one MIFARE_Read of page 4 returns pages 4-7 (16 bytes).
constexpr byte NFC_CLASSIC_BLOCK = 4;
constexpr byte NFC_UL_FIRST_PAGE = 4;

MFRC522::MIFARE_Key defaultKey = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

bool isSameTagWithinCooldown() {
    if (lastUidSize == 0 || rfid.uid.size != lastUidSize) return false;
    if (memcmp(rfid.uid.uidByte, lastUid, lastUidSize) != 0) return false;
    return millis() - lastOpMs < NFC_SAME_TAG_COOLDOWN_MS;
}

void rememberTag() {
    lastUidSize = rfid.uid.size;
    memcpy(lastUid, rfid.uid.uidByte, lastUidSize);
    lastOpMs = millis();
}

// End the transaction with the current tag. PCD_StopCrypto1 is mandatory
// after a Classic authentication and harmless otherwise.
void finishTag() {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

void printNfcError(MFRC522::StatusCode status) {
    printSerial(F("NFC-Fehler: "));
    printSerial(rfid.GetStatusCodeName(status));
    printSerial(F("\n"));
}

// Read the 16-byte payload block into text[17], always null-terminated.
// Returns false (with error printed) on any failure.
bool readText(MFRC522::PICC_Type type, char* text) {
    MFRC522::StatusCode status;
    byte buf[18];
    byte size = sizeof(buf);

    if (type == MFRC522::PICC_TYPE_MIFARE_UL) {
        status = rfid.MIFARE_Read(NFC_UL_FIRST_PAGE, buf, &size);
    } else {
        status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                       NFC_CLASSIC_BLOCK, &defaultKey, &rfid.uid);
        if (status == MFRC522::STATUS_OK) {
            status = rfid.MIFARE_Read(NFC_CLASSIC_BLOCK, buf, &size);
        }
    }
    if (status != MFRC522::STATUS_OK) {
        printNfcError(status);
        return false;
    }
    memcpy(text, buf, 16);
    text[16] = '\0'; // tag content may not be null-terminated
    return true;
}

// Write writeBuf as a zero-padded 16-byte payload. Returns false (with error
// printed) on any failure.
bool writeText(MFRC522::PICC_Type type) {
    MFRC522::StatusCode status = MFRC522::STATUS_OK;
    byte block[16] = {0};
    strncpy(reinterpret_cast<char*>(block), writeBuf, NFC_TEXT_MAX);

    if (type == MFRC522::PICC_TYPE_MIFARE_UL) {
        for (byte i = 0; i < 4 && status == MFRC522::STATUS_OK; i++) {
            status = rfid.MIFARE_Ultralight_Write(NFC_UL_FIRST_PAGE + i, &block[i * 4], 4);
        }
    } else {
        status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                       NFC_CLASSIC_BLOCK, &defaultKey, &rfid.uid);
        if (status == MFRC522::STATUS_OK) {
            status = rfid.MIFARE_Write(NFC_CLASSIC_BLOCK, block, 16);
        }
    }
    if (status != MFRC522::STATUS_OK) {
        printNfcError(status);
        return false;
    }
    return true;
}

} // namespace

void nfcRequestRead(NfcReadCallback cb) {
    state = NfcState::READ_PENDING;
    readCb = cb;
    writeCb = nullptr;
}

void nfcRequestWrite(const char* text, NfcWriteCallback cb) {
    strncpy(writeBuf, text, NFC_TEXT_MAX);
    writeBuf[NFC_TEXT_MAX] = '\0';
    state = NfcState::WRITE_PENDING;
    writeCb = cb;
    readCb = nullptr;
}

void nfcCancel() {
    state = NfcState::IDLE;
    readCb = nullptr;
    writeCb = nullptr;
}

bool nfcBusy() {
    return state != NfcState::IDLE;
}

void nfcPoll() {
    if (state == NfcState::IDLE) return;
    if (!rfid.PICC_IsNewCardPresent()) return;
    if (!rfid.PICC_ReadCardSerial()) return;

    if (isSameTagWithinCooldown()) {
        // Keep refreshing the timestamp so a tag resting on the reader can't
        // re-trigger while held down continuously.
        lastOpMs = millis();
        finishTag();
        return;
    }

    MFRC522::PICC_Type type = MFRC522::PICC_GetType(rfid.uid.sak);
    bool supported = type == MFRC522::PICC_TYPE_MIFARE_MINI ||
                     type == MFRC522::PICC_TYPE_MIFARE_1K ||
                     type == MFRC522::PICC_TYPE_MIFARE_4K ||
                     type == MFRC522::PICC_TYPE_MIFARE_UL;
    if (!supported) {
        printSerial(F("Unbekannter Chip-Typ!\n"));
        rememberTag(); // cooldown, so a resting unknown tag doesn't spam
        finishTag();
        return; // request stays armed
    }

    if (state == NfcState::READ_PENDING) {
        char text[17];
        if (!readText(type, text)) {
            rememberTag();
            finishTag();
            return; // request stays armed, retry after cooldown / other tag
        }
        rememberTag();
        finishTag();
        // Reentrancy: the callback typically re-arms via nfcRequestRead /
        // nfcRequestWrite — reset state BEFORE invoking it.
        NfcReadCallback cb = readCb;
        nfcCancel();
        if (cb) cb(text);
    } else { // WRITE_PENDING
        if (!writeText(type)) {
            rememberTag();
            finishTag();
            return; // request stays armed, retry after cooldown / other tag
        }
        rememberTag();
        finishTag();
        NfcWriteCallback cb = writeCb;
        nfcCancel();
        if (cb) cb(true);
    }
}

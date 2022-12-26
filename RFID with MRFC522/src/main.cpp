#include <Arduino.h>

#include <SPI.h>
#include <MFRC522.h>

#include <avr8-stubs-utilities.h>


//
//* -----------------------------------------------------------------------------------------
//*             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
//*             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
//* Signal      Pin          Pin           Pin       Pin        Pin              Pin
//* -----------------------------------------------------------------------------------------
//* RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
//* SPI SS 1    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
//* SPI SS 2    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
//* SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
//* SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
//* SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
//


#define RST_POWERDOWN_PIN   9    // Configurable - We choose D9 on Nano V3
#define SDA_PIN             10    // COnfigurable - We choose D10 - D10 is also SS on Uno v3


MFRC522 mfrc522Reader(SDA_PIN, RST_POWERDOWN_PIN);


//
// Known keys, see: https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
//
byte KnownKeys[][MFRC522::MF_KEY_SIZE] =  {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
    {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
    {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
    {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
    {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
    {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
    {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};
constexpr int NumberOfKnownKeys = sizeof(KnownKeys) / sizeof(KnownKeys[0]);




bool DumpMifareClassicSectorToSerial(MFRC522::Uid *uid,	MFRC522::MIFARE_Key *key, byte sector) {
	MFRC522::StatusCode status;
	byte firstBlock;		// Address of lowest address to dump actually last block dumped)
	byte no_of_blocks;		// Number of blocks in sector
	bool isSectorTrailer;	// Set to true while handling the "last" (ie highest address) in the sector.
	
	// The access bits are stored in a peculiar fashion.
	// There are four groups:
	//		g[3]	Access bits for the sector trailer, block 3 (for sectors 0-31) or block 15 (for sectors 32-39)
	//		g[2]	Access bits for block 2 (for sectors 0-31) or blocks 10-14 (for sectors 32-39)
	//		g[1]	Access bits for block 1 (for sectors 0-31) or blocks 5-9 (for sectors 32-39)
	//		g[0]	Access bits for block 0 (for sectors 0-31) or blocks 0-4 (for sectors 32-39)
	// Each group has access bits [C1 C2 C3]. In this code C1 is MSB and C3 is LSB
	// The four CX bits are stored together in a nible cx and an inverted nible cx_
	byte c1, c2, c3;		// Nibbles
	byte c1_, c2_, c3_;		// Inverted nibbles
	bool invertedError;		// True if one of the inverted nibbles did not match
	byte g[4];				// Access bits for each of the four groups
	byte group;				// 0-3 - active group for access bits
	bool firstInGroup;		// True for the first block dumped in the group
	
	// Determine position and size of sector.
	if (sector < 32) { // Sectors 0..31 has 4 blocks each
		no_of_blocks = 4;
		firstBlock = sector * no_of_blocks;
	}
	else if (sector < 40) { // Sectors 32-39 has 16 blocks each
		no_of_blocks = 16;
		firstBlock = 128 + (sector - 32) * no_of_blocks;
	}
	else { // Illegal input, no MIFARE Classic PICC has more than 40 sectors
		return false;
	}
		
	// Dump blocks, highest address first
	byte byteCount;
	byte buffer[18];
	byte blockAddr;
	isSectorTrailer = true;
	invertedError = false;
	for (int8_t blockOffset = no_of_blocks - 1; blockOffset >= 0; blockOffset--) {
		blockAddr = firstBlock + blockOffset;
		// Sector number - only on first line
		if (isSectorTrailer) {
			if(sector < 10)
				Serial.print(F("   "));
			else
				Serial.print(F("  "));
			Serial.print(sector);
			Serial.print(F("   "));
		}
		else {
			Serial.print(F("       "));
		}
		// Block number
		if(blockAddr < 10)
			Serial.print(F("   "));
		else {
			if(blockAddr < 100)
				Serial.print(F("  "));
			else
				Serial.print(F(" "));
		}
		Serial.print(blockAddr);
		Serial.print(F("  "));
		// Establish encrypted communications before reading the first block
		if (isSectorTrailer) {
			status = mfrc522Reader.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, firstBlock, key, uid);
			if (status != MFRC522::STATUS_OK) {
				Serial.print(F("PCD_Authenticate() failed: "));
				Serial.println(mfrc522Reader.GetStatusCodeName(status));
				return false;
			}
		}
		// Read block
		byteCount = sizeof(buffer);
		status = mfrc522Reader.MIFARE_Read(blockAddr, buffer, &byteCount);
		if (status != MFRC522::STATUS_OK) {
			Serial.print(F("MIFARE_Read() failed: "));
			Serial.println(mfrc522Reader.GetStatusCodeName(status));
			continue;
		}
		// Dump data
		for (byte index = 0; index < 16; index++) {
			if(buffer[index] < 0x10)
				Serial.print(F(" 0"));
			else
				Serial.print(F(" "));
			Serial.print(buffer[index], HEX);
			if ((index % 4) == 3) {
				Serial.print(F(" "));
			}
		}
		// Parse sector trailer data
		if (isSectorTrailer) {
			c1  = buffer[7] >> 4;
			c2  = buffer[8] & 0xF;
			c3  = buffer[8] >> 4;
			c1_ = buffer[6] & 0xF;
			c2_ = buffer[6] >> 4;
			c3_ = buffer[7] & 0xF;
			invertedError = (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) || (c3 != (~c3_ & 0xF));
			g[0] = ((c1 & 1) << 2) | ((c2 & 1) << 1) | ((c3 & 1) << 0);
			g[1] = ((c1 & 2) << 1) | ((c2 & 2) << 0) | ((c3 & 2) >> 1);
			g[2] = ((c1 & 4) << 0) | ((c2 & 4) >> 1) | ((c3 & 4) >> 2);
			g[3] = ((c1 & 8) >> 1) | ((c2 & 8) >> 2) | ((c3 & 8) >> 3);
			isSectorTrailer = false;
		}
		
		// Which access group is this block in?
		if (no_of_blocks == 4) {
			group = blockOffset;
			firstInGroup = true;
		}
		else {
			group = blockOffset / 5;
			firstInGroup = (group == 3) || (group != (blockOffset + 1) / 5);
		}
		
		if (firstInGroup) {
			// Print access bits
			Serial.print(F(" [ "));
			Serial.print((g[group] >> 2) & 1, DEC); Serial.print(F(" "));
			Serial.print((g[group] >> 1) & 1, DEC); Serial.print(F(" "));
			Serial.print((g[group] >> 0) & 1, DEC);
			Serial.print(F(" ] "));
			if (invertedError) {
				Serial.print(F(" Inverted access bits did not match! "));
			}
		}
		
		if (group != 3 && (g[group] == 1 || g[group] == 6)) { // Not a sector trailer, a value block
			int32_t value = (int32_t(buffer[3])<<24) | (int32_t(buffer[2])<<16) | (int32_t(buffer[1])<<8) | int32_t(buffer[0]);
			Serial.print(F(" Value=0x")); Serial.print(value, HEX);
			Serial.print(F(" Adr=0x")); Serial.print(buffer[12], HEX);
		}
		Serial.println();
	}
	
	return true;
}

bool DumpMifareClassicToSerial(MFRC522::Uid *uid, MFRC522::PICC_Type piccType, MFRC522::MIFARE_Key *key) {
  byte no_of_sectors = 0;
	switch (piccType) {
		case MFRC522::PICC_TYPE_MIFARE_MINI:
			// Has 5 sectors * 4 blocks/sector * 16 bytes/block = 320 bytes
			no_of_sectors = 5;
			break;
			
		case MFRC522::PICC_TYPE_MIFARE_1K:
			// Has 16 sectors * 4 blocks/sector * 16 bytes/block = 1024 bytes
			no_of_sectors = 16;
			break;
			
		case MFRC522::PICC_TYPE_MIFARE_4K:
			// Has (32 sectors * 4 blocks/sector + 8 sectors * 16 blocks/sector) * 16 bytes/block = 4096 bytes
			no_of_sectors = 40;
			break;
			
		default:
			break;
	}

	bool outcome = true;
	if (no_of_sectors) {
		Serial.println(F("Sector Block   0  1  2  3   4  5  6  7   8  9 10 11  12 13 14 15  AccessBits"));
		for (int8_t i = no_of_sectors - 1; i >= 0; i--) {
			outcome = outcome && DumpMifareClassicSectorToSerial(uid, key, i);
		}
	}
	mfrc522Reader.PICC_HaltA();
	mfrc522Reader.PCD_StopCrypto1();

	return outcome;
}

void ReadAndDisplayAllBlocksUsingDefaultKeys() {
  MFRC522::PICC_Type piccType = mfrc522Reader.PICC_GetType(mfrc522Reader.uid.sak);
	switch (piccType) {
		case MFRC522::PICC_TYPE_MIFARE_MINI:
		case MFRC522::PICC_TYPE_MIFARE_1K:
		case MFRC522::PICC_TYPE_MIFARE_4K:
        for (int knownKeyIndex = 0; knownKeyIndex < NumberOfKnownKeys; knownKeyIndex++) {
          MFRC522::MIFARE_Key key;
          memcpy(&key.keyByte, KnownKeys[knownKeyIndex], MFRC522::MF_KEY_SIZE);
          if (DumpMifareClassicToSerial(&(mfrc522Reader.uid), piccType, &key)) {
            break;
          }
        }
			break;
			
		case MFRC522::PICC_TYPE_MIFARE_UL:
      		// TODO: Implement
			break;
			
		case MFRC522::PICC_TYPE_ISO_14443_4:
		case MFRC522::PICC_TYPE_MIFARE_DESFIRE:
		case MFRC522::PICC_TYPE_ISO_18092:
		case MFRC522::PICC_TYPE_MIFARE_PLUS:
		case MFRC522::PICC_TYPE_TNP3XXX:
			Serial.println(F("Dumping memory contents not implemented for that PICC type."));
			break;
			
		case MFRC522::PICC_TYPE_UNKNOWN:
		case MFRC522::PICC_TYPE_NOT_COMPLETE:
		default:
			break;
	}
	
	Serial.println();
	mfrc522Reader.PICC_HaltA();
}



void setup() {
  INIT_DEBUGGER();

  Serial.begin(115200);

  SPI.begin();
  mfrc522Reader.PCD_Init();

  Serial.print(F("Reader "));
  Serial.print(F(": "));
  mfrc522Reader.PCD_DumpVersionToSerial();
}

void loop() {
  if (mfrc522Reader.PICC_IsNewCardPresent() && mfrc522Reader.PICC_ReadCardSerial()) {
    // Show some details of the PICC (that is: the tag/card)
    mfrc522Reader.PICC_DumpDetailsToSerial(&(mfrc522Reader.uid));

    // Try to read all blocks using the default known keys
    ReadAndDisplayAllBlocksUsingDefaultKeys();

    // Halt PICC
    mfrc522Reader.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522Reader.PCD_StopCrypto1();
  } else {
    delay(50);
  }
}
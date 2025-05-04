#include <vitasdk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bus.h"
#include "cart.h"

cart_t rom = {};

typedef struct {
	char tag[3];
	char name[64];
} new_lic_code_t;

static new_lic_code_t new_lic_db[] = {
	{.tag = "00", .name = "None"},
	{.tag = "01", .name = "Nintendo Research & Development 1"},
	{.tag = "08", .name = "Capcom"},
	{.tag = "13", .name = "Electronic Arts"},
	{.tag = "18", .name = "Hudson Soft"},
	{.tag = "19", .name = "B-AI"},
	{.tag = "20", .name = "KSS"},
	{.tag = "22", .name = "Planning Office WADA"},
	{.tag = "24", .name = "PCM Complete"},
	{.tag = "25", .name = "San-X"},
	{.tag = "28", .name = "Kemco"},
	{.tag = "29", .name = "SETA Corporation"},
	{.tag = "30", .name = "Viacom"},
	{.tag = "31", .name = "Nintendo"},
	{.tag = "32", .name = "Bandai"},
	{.tag = "33", .name = "Ocean Software / Acclaim Entertainment"},
	{.tag = "34", .name = "Konami"},
	{.tag = "35", .name = "HectorSoft"},
	{.tag = "37", .name = "Taito"},
	{.tag = "38", .name = "Hudson Soft"},
	{.tag = "39", .name = "Banpresto"},
	{.tag = "41", .name = "Ubi Soft"},
	{.tag = "42", .name = "Atlus"},
	{.tag = "44", .name = "Malibu Interactive"},
	{.tag = "46", .name = "Angel"},
	{.tag = "47", .name = "Bullet-Proof Software"},
	{.tag = "49", .name = "Irem"},
	{.tag = "50", .name = "Absolute"},
	{.tag = "51", .name = "Acclaim Entertainment"},
	{.tag = "52", .name = "Activision"},
	{.tag = "53", .name = "Sammy USA Corporation"},
	{.tag = "54", .name = "Konami"},
	{.tag = "55", .name = "Hi Tech Expressions"},
	{.tag = "56", .name = "LJN"},
	{.tag = "57", .name = "Matchbox"},
	{.tag = "58", .name = "Mattel"},
	{.tag = "59", .name = "Milton Bradley Company"},
	{.tag = "60", .name = "Titus Interactive"},
	{.tag = "61", .name = "Virgin Games Ltd."},
	{.tag = "64", .name = "Lucasfilm Games"},
	{.tag = "67", .name = "Ocean Software"},
	{.tag = "69", .name = "Electronic Arts"},
	{.tag = "70", .name = "Infogrames"},
	{.tag = "71", .name = "Interplay Entertainment"},
	{.tag = "72", .name = "Broderbund"},
	{.tag = "73", .name = "Sculptured Software"},
	{.tag = "75", .name = "The Sales Curve Limited"},
	{.tag = "78", .name = "THQ"},
	{.tag = "79", .name = "Accolade"},
	{.tag = "80", .name = "Misawa Entertainment"},
	{.tag = "83", .name = "lozc"},
	{.tag = "86", .name = "Tokuma Shoten"},
	{.tag = "87", .name = "Tsukuda Original"},
	{.tag = "91", .name = "Chunsoft Co."},
	{.tag = "92", .name = "Video System"},
	{.tag = "93", .name = "Ocean Software / Acclaim Entertainment"},
	{.tag = "95", .name = "Varie"},
	{.tag = "96", .name = "Yonezawa's pal"},
	{.tag = "97", .name = "Kaneko"},
	{.tag = "99", .name = "Pack-In-Video"},
	{.tag = "9H", .name = "Bottom Up"},
	{.tag = "A4", .name = "Konami"},
	{.tag = "BL", .name = "MTO"},
	{.tag = "DK", .name = "Kodansha"},
};

static const char *lic_db[] = {
	[0x00] = "None",
	[0x01] = "Nintendo",
	[0x08] = "Capcom",
	[0x09] = "HOT-B",
	[0x0A] = "Jaleco",
	[0x0B] = "Coconuts Japan",
	[0x0C] = "Elite Systems",
	[0x13] = "Electronic Arts",
	[0x18] = "Hudson Soft",
	[0x19] = "ITC Entertainment",
	[0x1A] = "Yanoman",
	[0x1D] = "Japan Clary",
	[0x1F] = "Virgin Games Ltd.",
	[0x24] = "PCM Complete",
	[0x25] = "San-X",
	[0x28] = "Kemco",
	[0x29] = "SETA Corporation",
	[0x30] = "Infogrames",
	[0x31] = "Nintendo",
	[0x32] = "Bandai",
	[0x33] = "NEW_LICENSEE_CODE",
	[0x34] = "Konami",
	[0x35] = "HectorSoft",
	[0x38] = "Capcom",
	[0x39] = "Banpresto",
	[0x3C] = "Entertainment Interactive",
	[0x3E] = "Gremlin",
	[0x41] = "Ubi Soft",
	[0x42] = "Atlus",
	[0x44] = "Malibu Interactive",
	[0x46] = "Angel",
	[0x47] = "Spectrum HoloByte",
	[0x49] = "Irem",
	[0x4A] = "Virgin Games Ltd.",
	[0x4D] = "Malibu Interactive",
	[0x4F] = "U.S. Gold",
	[0x50] = "Absolute",
	[0x51] = "Acclaim Entertainment",
	[0x52] = "Activision",
	[0x53] = "Sammy USA Corporation",
	[0x54] = "GameTek",
	[0x55] = "Park Place",
	[0x56] = "LJN",
	[0x57] = "Matchbox",
	[0x59] = "Milton Bradley Company",
	[0x5A] = "Mindscape",
	[0x5B] = "Romstar",
	[0x5C] = "Naxat Soft",
	[0x5D] = "Tradewest",
	[0x60] = "Titus Interactive",
	[0x61] = "Virgin Games Ltd.",
	[0x67] = "Ocean Software",
	[0x69] = "Electronic Arts",
	[0x6E] = "Elite Systems",
	[0x6F] = "Electro Brain",
	[0x70] = "Infogrames",
	[0x71] = "Interplay Entertainment",
	[0x72] = "Broderbund",
	[0x73] = "Sculptured Software",
	[0x75] = "The Sales Curve Limited",
	[0x78] = "THQ",
	[0x79] = "Accolade",
	[0x7A] = "Triffix Entertainment",
	[0x7C] = "MicroProse",
	[0x7F] = "Kemco",
	[0x80] = "Misawa Entertainment",
	[0x83] = "LOZC G.",
	[0x86] = "Tokuma Shoten",
	[0x8B] = "Bullet-Proof Software",
	[0x8C] = "Vic Tokai Corp.",
	[0x8E] = "Ape Inc.",
	[0x8F] = "I'Max",
	[0x91] = "Chunsoft Co.",
	[0x92] = "Video System",
	[0x93] = "Tsubaraya Productions",
	[0x95] = "Varie",
	[0x96] = "Yonezawa's Pal",
	[0x97] = "Kemco",
	[0x99] = "Arc",
	[0x9A] = "Nihon Bussan",
	[0x9B] = "Tecmo",
	[0x9C] = "Imagineer",
	[0x9D] = "Banpresto",
	[0x9F] = "Nova",
	[0xA1] = "Hori Electric",
	[0xA2] = "Bandai",
	[0xA4] = "Konami",
	[0xA6] = "Kawada",
	[0xA7] = "Takara",
	[0xA9] = "Technos Japan",
	[0xAA] = "Broderbund",
	[0xAC] = "Toei Animation",
	[0xAD] = "Toho",
	[0xAF] = "Namco",
	[0xB0] = "Acclaim Entertainment",
	[0xB1] = "ASCII Corporation / Nexsoft",
	[0xB2] = "Bandai",
	[0xB4] = "Square Enix",
	[0xB6] = "HAL Laboratory",
	[0xB7] = "SNK",
	[0xB9] = "Pony Canyon",
	[0xBA] = "Culture Brain",
	[0xBB] = "Sunsoft",
	[0xBD] = "Sony Imagesoft",
	[0xBF] = "Sammy Corporation",
	[0xC0] = "Taito",
	[0xC2] = "Kemco",
	[0xC3] = "Square",
	[0xC4] = "Tokuma Shoten",
	[0xC5] = "Data East",
	[0xC6] = "Tonkin House",
	[0xC8] = "Koei",
	[0xC9] = "UFL",
	[0xCA] = "Ultra Games",
	[0xCB] = "VAP, Inc.",
	[0xCC] = "Use Corporation",
	[0xCD] = "Meldac",
	[0xCE] = "Pony Canyon",
	[0xCF] = "Angel",
	[0xD0] = "Taito",
	[0xD1] = "SOFEL",
	[0xD2] = "Quest",
	[0xD3] = "Sigma Enterprises",
	[0xD4] = "ASK Kodansha Co.",
	[0xD6] = "Naxat Soft",
	[0xD7] = "Copya System",
	[0xD9] = "Banpresto",
	[0xDA] = "Tomy",
	[0xDB] = "LJN",
	[0xDD] = "Nippon Computer Systems",
	[0xDE] = "Human Ent.",
	[0xDF] = "Altron",
	[0xE0] = "Jaleco",
	[0xE1] = "Towa Chiki",
	[0xE2] = "Yutaka",
	[0xE3] = "Varie",
	[0xE5] = "Epoch",
	[0xE7] = "Athena",
	[0xE8] = "Asmik Ace Entertainment",
	[0xE9] = "Natsume",
	[0xEA] = "King Records",
	[0xEB] = "Atlus",
	[0xEC] = "Epic / Sony Records",
	[0xEE] = "IGS",
	[0xF0] = "A Wave",
	[0xF3] = "Extreme Entertainment",
	[0xFF] = "LJN",
};

const char *cart_types[] = {
	[0x00] = "ROM",
	[0x01] = "MBC1",
	[0x02] = "MBC1+RAM",
	[0x03] = "MBC1+RAM+BATTERY",
	[0x05] = "MBC2",
	[0x06] = "MBC2+BATTERY",
	[0x08] = "ROM+RAM",
	[0x09] = "ROM+RAM+BATTERY",
	[0x0B] = "MMM01",
	[0x0C] = "MMM01+RAM",
	[0x0D] = "MMM01+RAM+BATTERY",
	[0x0F] = "MBC3+TIMER+BATTERY",
	[0x10] = "MBC3+TIMER+RAM+BATTERY",
	[0x11] = "MBC3",
	[0x12] = "MBC3+RAM",
	[0x13] = "MBC3+RAM+BATTERY",
	[0x19] = "MBC5",
	[0x1A] = "MBC5+RAM",
	[0x1B] = "MBC5+RAM+BATTERY",
	[0x1C] = "MBC5+RUMBLE",
	[0x1D] = "MBC5+RUMBLE+RAM",
	[0x1E] = "MBC5+RUMBLE+RAM+BATTERY",
	[0x20] = "MBC6",
	[0x22] = "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
	[0xFC] = "POCKET CAMERA",
	[0xFD] = "BANDAI TAMA5",
	[0xFE] = "HuC3",
	[0xFF] = "HuC1+RAM+BATTERY",
};

static void cart_init_ram_banks() {
	for (int i = 0; i < 16; i++) {
		if (rom.ram_banks[i]) {
			free(rom.ram_banks[i]);
		}
		if (i < rom.ram_size / 8) {
			rom.ram_banks[i] = (uint8_t *)malloc(0x2000);
			sceClibMemset(rom.ram_banks[i], 0, 0x2000);
		} else {
			rom.ram_banks[i] = NULL;
		}
	}
	
	rom.ram_bank = rom.ram_banks[0];
	rom.ram_bank_num = 0;
	rom.rom_bank_num = 0;
	rom.rom_bank = rom.data + ADDR_ROM_BANK_1;
}

static void cart_load_battery() {
	char path[256];
	sprintf(path, "%s.battery", rom.fname);
	FILE *f = fopen(path, "rb");
	if (f) {
		fread(rom.ram_banks[0], 1, 0x2000, f);
		fclose(f);
	}
}

void cart_save_battery() {
	if (rom.ram_banks[rom.ram_bank_num]) {
		char path[256];
		sprintf(path, "%s.battery", rom.fname);
		FILE *f = fopen(path, "wb");
		fwrite(rom.ram_banks[rom.ram_bank_num], 1, 0x2000, f);
		fclose(f);
		rom.save_battery = 0;
	}
}

void cart_load(const char *path) {
	strcpy(rom.fname, path);
	if (rom.data) {
		free(rom.data);
	}
	FILE *f = fopen(path, "rb");
	fseek(f, 0, SEEK_END);
	size_t sz = ftell(f);
	rom.data = malloc(sz);
	fseek(f, 0, SEEK_SET);
	fread(rom.data, 1, sz, f);
	fclose(f);
	
	// Parsing the header
	sceClibMemcpy(rom.name, &rom.data[CART_TITLE], 16);
	rom.name[16] = 0;
	rom.type = rom.data[CART_TYPE];
	rom.size = 32 << rom.data[CART_ROM_SIZE];
	if (rom.data[CART_LIC_CODE] == 0x33) {
		for (int i = 0; i < sizeof(new_lic_db) / sizeof(*new_lic_db); i++) {
			if (!(strncmp(&rom.data[CART_NEW_LIC_CODE], new_lic_db[i].tag, 2))) {
				strcpy(rom.licensee, new_lic_db[i].name);
				break;
			}
		}
	} else {
		strcpy(rom.licensee, lic_db[rom.data[CART_LIC_CODE]]);
	}
	
	// Init RAM
	if (strstr(cart_types[rom.type], "RAM")) {
		switch (rom.data[CART_RAM_SIZE]) {
		case 0:
			rom.ram_size = 0;
			break;
		case 1:
			rom.ram_size = 2;
			break;
		case 2:
			rom.ram_size = 8;
			break;
		case 3:
			rom.ram_size = 32;
			break;
		case 4:
			rom.ram_size = 128;
			break;
		case 5:
			rom.ram_size = 64;
			break;
		default:
			break;
		}
	}
	cart_init_ram_banks();
	
	// Parsing battery/mapper info
	rom.save_battery = 0;
	if (strstr(cart_types[rom.type], "MBC1")) {
		rom.mbc1 = 1;
	} else {
		rom.mbc1 = 0;
	}
	if (strstr(cart_types[rom.type], "BATTERY")) {
		rom.battery = 1;
		cart_load_battery();
	} else {
		rom.battery = 0;
	}
	
	
	// Header checksum check
	uint16_t x;
	for (int i = 0x134; i < 0x14D; i++) {
		x -= rom.data[i] + 1;
	}
	sceClibPrintf("Header checksum %s\n", ((x & 0xFF) == rom.data[CART_HDR_CHECKSUM]) ? "passed" : "failed");

	// Debug logging
	sceClibPrintf("Game Title: %s\n", rom.name);
	sceClibPrintf("Type: %s\n", cart_types[rom.type]);
	sceClibPrintf("ROM Size: %d KBs\n", rom.size);
	sceClibPrintf("RAM Size: %d KBs\n", rom.ram_size);
	sceClibPrintf("Licensed by: %s\n", rom.licensee);
}

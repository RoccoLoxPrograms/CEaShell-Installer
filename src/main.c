#include <debug.h>
#include <fileioc.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ti/screen.h>
#include <ti/getcsc.h>
#include <ti/info.h>

// pretty sure you'll run out of flash past this point, lol
#define MAX_APPVARS 24
#define APPVAR_SIZE 60000

#define MAIN_SECTION_OFFSET 0x112

extern struct {
	void *data;
	size_t size;
} locations[MAX_APPVARS];

extern uint8_t num_appvars;
extern size_t app_size;
extern uint24_t code_offset;
extern void *install_loc;

enum error {
	SUCCESS,
	ALREADY_INSTALLED,
	MISSING_VAR,
	PORT_SETUP_FAILED,
	NO_SPACE,
};

uint8_t install();
bool port_setup();
bool confirm_delete_vars();

uint8_t try_install() {
	for (uint8_t i = 0; i < MAX_APPVARS; i++) {
		char filename[] = "AppInstA";
		filename[7] = 'A' + i;
		ti_var_t f = ti_Open(filename, "r");
		if (!f) {
			dbg_printf(dbgerr, "Appvar %s not found.\n", filename);
			return MISSING_VAR;
		}
		locations[i].data = ti_GetDataPtr(f);
		size_t size = ti_GetSize(f);
		locations[i].size = size;
		app_size += size;
		num_appvars++;
		ti_Close(f);
		if (size != APPVAR_SIZE) break;
	}
	code_offset = *(uint24_t*)(locations[0].data + MAIN_SECTION_OFFSET) + 0x100;
	if (port_setup()) {
		dbg_printf(dbgerr, "Failed to set up ports.\n");
		return PORT_SETUP_FAILED;
	}
	uint8_t err = install();
	dbg_printf("Installed to %p\n", install_loc);
	dbg_printf("Execution start: %p\n", install_loc + 0x100 + *(uint24_t*)(install_loc + 0x11B));
	return err;
}

void delete_vars(const char *prgmname) {
	for (uint8_t i = 0; i < num_appvars; i++) {
		char filename[] = "AppInstA";
		filename[7] = 'A' + i;
		ti_Delete(filename);
	}
	ti_DeleteVar(prgmname, OS_TYPE_PROT_PRGM);
}

int main(int argc, char **argv) {
	const system_info_t *system_info = os_GetSystemInfo();
	bool french = false;
	if (system_info->language == 0x10C) {
		french = true;
	}
	os_HomeUp();
	os_ClrLCDFull();
	if (!french) {
		os_PutStrFull("Installing app.");
		os_NewLine();
		os_PutStrFull("Please wait...");
	} else {
		os_PutStrFull("Installation de");
		os_NewLine();
		os_PutStrFull("l'application.");
		os_NewLine();
		os_PutStrFull("Veuillez patienter...");
	}
	uint8_t error = try_install();
	os_HomeUp();
	os_ClrLCDFull();
	switch (error) {
		case SUCCESS:
			if (!french) {
				os_PutStrFull("Successfully installed.");
				os_NewLine();
				os_NewLine();
				os_PutStrFull("Delete installer files?");
			} else {
				os_PutStrFull("Install""\x96"" avec succ""\x97""s.");
				os_NewLine();
				os_NewLine();
				os_PutStrFull("Supprimer l'installeur ?");
			}
			if (confirm_delete_vars()) {
				delete_vars(argv[0]);
			}
			return SUCCESS;
			break;
		case ALREADY_INSTALLED:
			if (!french) {
				os_PutStrFull("Already installed.");
				os_NewLine();
				os_PutStrFull("Delete app from the");
				os_NewLine();
				os_PutStrFull("mem menu to reinstall.");
			} else {
				os_PutStrFull("D""\x96""j""\x8F"" install""\x96"".");
				os_NewLine();
				os_PutStrFull("Supprimez l'appli depuis");
				os_NewLine();
				os_PutStrFull("le menu m""\x96""m pour");
				os_NewLine();
				os_PutStrFull("reinstaller.");
			}
			break;
		case MISSING_VAR:
			if (!french) {
				os_PutStrFull("Install failed.");
				os_NewLine();
				os_PutStrFull("Missing an appvar.");
			} else {
				os_PutStrFull("Echec de l'installation.");
				os_NewLine();
				os_PutStrFull("Appvar manquante.");
			}
			break;
		case PORT_SETUP_FAILED:
			if (!french) {
				os_PutStrFull("Install failed.");
				os_NewLine();
				os_PutStrFull("Unsupported OS version.");
			} else {
				os_PutStrFull("Echec de l'installation.");
				os_NewLine();
				os_PutStrFull("Version de l'OS");
				os_NewLine();
				os_PutStrFull("incompatible.");
			}
			break;
		case NO_SPACE:
			if (!french) {
				os_PutStrFull("Install failed.");
				os_NewLine();
				os_PutStrFull("Out of archive space.");
				os_NewLine();
				os_PutStrFull("Try running the");
				os_NewLine();
				os_PutStrFull("GarbageCollect command.");
			} else {
				os_PutStrFull("Echec de l'installation.");
				os_NewLine();
				os_PutStrFull("M""\x96""moire insuffisante.");
				os_NewLine();
				os_PutStrFull("Essayez de lancer la");
				os_NewLine();
				os_PutStrFull("commande \"RamasseMiettes\".");
			}
			break;
	}
	while (os_GetCSC());
	while (!os_GetCSC());
	return error;
}

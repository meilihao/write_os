
OBJ_DIR = objs
CC = cl.exe
LINK = link.exe
ASM  = ml.exe
#### Set your NASM path here
NASM = D:\ProjectUEFI\BochsLoader\BOOT16\nasm.exe

CONSOLE_APP_LINK_FLAGS = /MANIFEST /NXCOMPAT /DEBUG /MACHINE:X86 /OPT:REF /SAFESEH /INCREMENTAL:NO  /SUBSYSTEM:CONSOLE /MANIFESTUAC:"level='asInvoker' uiAccess='false'"  /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO /TLBID:1 
CONSOLE_APP_LINK_LIBS = "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib"
CONSOLE_APP_CC_FLAGS = /Fo:"$(OBJ_DIR)\\"

PE32_LOADER_ASM_FLAGS = /c /nologo /Zi /Fo"$(OBJ_DIR)\\" /Fl"$(OBJ_DIR)\\" /W3 /errorReport:prompt 
PE32_LOADER_CC_FLAGS = /permissive- /ifcOutput "$(OBJ_DIR)\\" /GS- /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /sdl- /Fd"$(OBJ_DIR)\vc142.pdb" /Zc:inline /fp:precise /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi- /MDd /FC /nologo /Fo"$(OBJ_DIR)\\" /Fp"$(OBJ_DIR)\PE32Krnl.pch" /diagnostics:column 
PE32_LOADER_LINK_FLAGS = /OUT:"$(OBJ_DIR)\PE32Krnl.exe" /SAFESEH:NO /MANIFEST:NO /NXCOMPAT:NO /PDB:"$(OBJ_DIR)\PE32Krnl.pdb"  /DYNAMICBASE:NO /FIXED /BASE:"0x400000" /MACHINE:X86 /ENTRY:"main" /OPT:REF /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /ERRORREPORT:PROMPT /NOLOGO /NODEFAULTLIB /TLBID:1 


all: fatbootsector mkbtfat32.exe PEToBin.exe sysldr_asm PE32Krnl.exe

prepare:
	- mkdir $(OBJ_DIR)

fatbootsector: prepare
	$(NASM) BOOT16\FAT32LDR.asm -o $(OBJ_DIR)\FAT32LDR.bin

sysldr_asm: prepare
	$(NASM) BOOT16\SYSLDR16.asm -o $(OBJ_DIR)\SYSLDR16

mkbtfat32.exe: prepare
	$(CC) /c mkbtfat32\mkbtfat32.c $(CONSOLE_APP_CC_FLAGS) 
	$(LINK) $(OBJ_DIR)\mkbtfat32.obj /OUT:"$(OBJ_DIR)\mkbtfat32.exe" $(CONSOLE_APP_LINK_FLAGS) /PDB:"$(OBJ_DIR)\mkbtfat32.pdb" /DYNAMICBASE $(CONSOLE_APP_LINK_LIBS)  /ManifestFile:"$(OBJ_DIR)\mkbtfat32.exe.intermediate.manifest"

PEToBin.exe: prepare
	$(CC) /c PEToBin\PEToBin.c $(CONSOLE_APP_CC_FLAGS)
	$(LINK) $(OBJ_DIR)\PEToBin.obj /OUT:"$(OBJ_DIR)\PEToBin.exe" $(CONSOLE_APP_LINK_FLAGS) /PDB:"$(OBJ_DIR)\PEToBin.pdb" /DYNAMICBASE $(CONSOLE_APP_LINK_LIBS)  /ManifestFile:"$(OBJ_DIR)\PEToBin.exe.intermediate.manifest"

PE32Krnl.exe: prepare PEToBin.exe sysldr_asm
	$(CC) /c PE32Krnl\PE32Krnl.c $(PE32_LOADER_CC_FLAGS) 
	$(CC) /c PE32Krnl\ata_direct.c $(PE32_LOADER_CC_FLAGS) 
	$(CC) /c PE32Krnl\fatfs.c $(PE32_LOADER_CC_FLAGS) 
	$(CC) /c PE32Krnl\goto64.c $(PE32_LOADER_CC_FLAGS)
	$(CC) /c PE32Krnl\siodebug.c $(PE32_LOADER_CC_FLAGS) 
	$(CC) /c PE32Krnl\stdio.c $(PE32_LOADER_CC_FLAGS) 
	$(CC) /c PE32Krnl\string.c $(PE32_LOADER_CC_FLAGS)

	$(ASM) $(PE32_LOADER_ASM_FLAGS) /Ta PE32Krnl\sys32.asm
	$(ASM) $(PE32_LOADER_ASM_FLAGS) /Ta PE32Krnl\mode_change.asm
	
	$(LINK) "$(OBJ_DIR)\PE32Krnl.obj" \
			"$(OBJ_DIR)\ata_direct.obj" \
			"$(OBJ_DIR)\fatfs.obj" \
			"$(OBJ_DIR)\goto64.obj" \
			"$(OBJ_DIR)\siodebug.obj" \
			"$(OBJ_DIR)\stdio.obj" \
			"$(OBJ_DIR)\string.obj" \
			"$(OBJ_DIR)\sys32.obj" \
			"$(OBJ_DIR)\mode_change.obj" \
			$(PE32_LOADER_LINK_FLAGS)
	
	$(OBJ_DIR)\PEToBin.exe $(OBJ_DIR)\PE32Krnl.exe $(OBJ_DIR)\SYSLDR16 $(OBJ_DIR)\SYSLDR.BIN

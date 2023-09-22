
OBJ_DIR = objs
CC = cl.exe
LINK = link.exe
ASM  = ml64.exe

KRNL64_ASM_FLAGS = /c /nologo /Zi /Fo"$(OBJ_DIR)\\" /Fl"$(OBJ_DIR)\\" /W3 /errorReport:prompt 
KRNL64_CC_FLAGS = /permissive- /ifcOutput "$(OBJ_DIR)\\" /GS- /W3 /Zc:wchar_t /ZI /Gm-  /Od /sdl- /Fd"$(OBJ_DIR)\vc142.pdb" /Zc:inline /fp:precise /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi- /MDd /FC /nologo /Fo"$(OBJ_DIR)\\" /Fp"$(OBJ_DIR)\HELLO64.pch" /diagnostics:column 
KRNL64_LINK_FLAGS = /OUT:"$(OBJ_DIR)\HELLO64.exe" /SAFESEH:NO /MANIFEST:NO /NXCOMPAT:NO /PDB:"$(OBJ_DIR)\HELLO64.pdb"  /DYNAMICBASE:NO /FIXED /BASE:"0x800000" /MACHINE:X64 /ENTRY:"main" /SUBSYSTEM:EFI_APPLICATION /ERRORREPORT:PROMPT /NOLOGO /NODEFAULTLIB /TLBID:1 
PUBLIC_HEADERS = /I"include" 

all: hello64

prepare:
	- mkdir $(OBJ_DIR)

hello64: prepare
	$(CC) /c src\main.c $(PUBLIC_HEADERS) $(KRNL64_CC_FLAGS) 
	$(CC) /c src\siodebug.c $(PUBLIC_HEADERS) $(KRNL64_CC_FLAGS) 
	$(CC) /c src\string.c $(PUBLIC_HEADERS) $(KRNL64_CC_FLAGS)

	$(ASM) $(KRNL64_ASM_FLAGS) /Ta src\sys64.asm
	
	$(LINK) "$(OBJ_DIR)\main.obj" \
			"$(OBJ_DIR)\siodebug.obj" \
			"$(OBJ_DIR)\string.obj" \
			"$(OBJ_DIR)\sys64.obj" \
			$(KRNL64_LINK_FLAGS)

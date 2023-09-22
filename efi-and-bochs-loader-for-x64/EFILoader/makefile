
OBJ_DIR = objs
CC = cl.exe
LINK = link.exe
ASM  = ml64.exe

EFIAPP_ASM_FLAGS = /c /nologo /Zi /Fo"$(OBJ_DIR)\\" /Fl"$(OBJ_DIR)\\" /W3 /errorReport:prompt 
EFIAPP_CC_FLAGS = /permissive- /ifcOutput "$(OBJ_DIR)\\" /GS- /W3 /Zc:wchar_t /ZI /Gm-  /Od /sdl- /Fd"$(OBJ_DIR)\vc142.pdb" /Zc:inline /fp:precise /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi- /MDd /FC /nologo /Fo"$(OBJ_DIR)\\" /Fp"$(OBJ_DIR)\EFILoader.pch" /diagnostics:column 
EFIAPP_LINK_FLAGS = /OUT:"$(OBJ_DIR)\EFILoader.exe" /SAFESEH:NO /MANIFEST:NO /NXCOMPAT:NO /PDB:"$(OBJ_DIR)\EFILoader.pdb"  /DYNAMICBASE:NO /FIXED:NO /MACHINE:X64 /ENTRY:"efi_main" /SUBSYSTEM:EFI_APPLICATION /ERRORREPORT:PROMPT /NOLOGO /NODEFAULTLIB /TLBID:1 
# modify it to EFI_ToolKit include path
PUBLIC_HEADERS = /I"D:\ProjectUEFI\EFI_Toolkit_2.0.0.1\EFI_Toolkit_2.0\include\efi\em64t" /I"D:\ProjectUEFI\EFI_Toolkit_2.0.0.1\EFI_Toolkit_2.0\include\efi"

all: hello64

prepare:
	- mkdir $(OBJ_DIR)

hello64: prepare
	$(CC) /c src\EFILoader.c $(PUBLIC_HEADERS) $(EFIAPP_CC_FLAGS) 
	$(CC) /c src\LoadPE.c $(PUBLIC_HEADERS) $(EFIAPP_CC_FLAGS) 
	$(CC) /c src\siodebug.c $(PUBLIC_HEADERS) $(EFIAPP_CC_FLAGS) 
	$(CC) /c src\string.c $(PUBLIC_HEADERS) $(EFIAPP_CC_FLAGS)

	$(ASM) $(EFIAPP_ASM_FLAGS) /Ta src\mode_change.asm
	$(ASM) $(EFIAPP_ASM_FLAGS) /Ta src\sys64.asm
	
	$(LINK) "$(OBJ_DIR)\EFILoader.obj" \
			"$(OBJ_DIR)\LoadPE.obj" \
			"$(OBJ_DIR)\siodebug.obj" \
			"$(OBJ_DIR)\string.obj" \
			"$(OBJ_DIR)\mode_change.obj" \
			"$(OBJ_DIR)\sys64.obj" \
			$(EFIAPP_LINK_FLAGS)

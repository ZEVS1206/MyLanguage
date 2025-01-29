FOLDER_ASSEMBLER = ./Assembler
FOLDER_PROCESSOR = ./Processor
FOLDER_READER = ./Reader
FILE_CMD = Assembler/source/cpu_commands.txt

dir_asm = ../assembler.exe
dir_proc = ../processor.exe
dir_read = ../reader.exe
compile ?= all
start ?= all
cleaning ?= all

start_proc = ./processor.exe
start_asm = ./assembler.exe
start_reader = ./reader.exe

dir_build_asm = Assembler/build
dir_build_proc = Processor/build
dir_build_reader = Reader/build

dir_dump_reader = Reader/dump

ASM = asm
PROC = proc
READ = rd

.PHONY = all, run


ifeq ($(compile),$(ASM))
all::
	make -C $(FOLDER_ASSEMBLER) dir=$(dir_asm)
else
ifeq ($(compile), $(PROC))
all::
	make -C $(FOLDER_PROCESSOR) dir=$(dir_proc)
else
ifeq ($(compile), $(READ))
all::
	make -C $(FOLDER_READER) dir=$(dir_read)
else
all::
	make -C $(FOLDER_ASSEMBLER) dir=$(dir_asm)
	make -C $(FOLDER_PROCESSOR) dir=$(dir_proc)
	make -C $(FOLDER_READER) dir=$(dir_read)
endif
endif
endif


ifeq ($(cleaning), $(ASM))
clean::
	rm -rf $(start_asm)
	rm -rf $(dir_build_asm)
else
ifeq ($(cleaning), $(PROC))
clean::
	rm -rf $(dir_build_proc)
	rm -rf $(start_proc)
else
ifeq ($(cleaning), $(READ))
clean::
	rm -rf $(dir_build_reader)
	rm -rf $(start_reader)
	rm -rf $(dir_dump_reader)
else
clean::
	rm -rf $(dir_build_asm)
	rm -rf $(dir_build_proc)
	rm -rf $(FILE_CMD)
	rm -rf $(start_asm)
	rm -rf $(start_proc)
	rm -rf $(dir_build_reader)
	rm -rf $(start_reader)
	rm -rf $(dir_dump_reader)
endif
endif
endif


ifeq ($(start), $(ASM))
run::
	$(start_asm)
else
ifeq ($(start), $(PROC))
run::
	$(start_proc)
else
ifeq ($(start), $(READ))
run::
	$(start_reader)
else
run::
	$(start_asm)
	$(start_proc)
	$(start_reader)
endif
endif
endif




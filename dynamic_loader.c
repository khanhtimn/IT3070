
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function prototypes
void load_elf(const char *filename);

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <elf-file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  load_elf(argv[1]);
  return 0;
}

void load_elf(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  Elf64_Ehdr ehdr;
  if (fread(&ehdr, 1, sizeof(ehdr), file) != sizeof(ehdr)) {
    fprintf(stderr, "Failed to read ELF header\n");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
    fprintf(stderr, "Not an ELF file\n");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  printf("ELF header read successfully\n");

  // Read program headers
  fseek(file, ehdr.e_phoff, SEEK_SET);
  Elf64_Phdr phdr;
  void *base_address = NULL;
  size_t segment_size = 0;
  Elf64_Addr strtab_offset = 0;
  void *strtab_base = NULL;

  for (int i = 0; i < ehdr.e_phnum; i++) {
    if (fread(&phdr, 1, sizeof(phdr), file) != sizeof(phdr)) {
      fprintf(stderr, "Failed to read program header\n");
      fclose(file);
      exit(EXIT_FAILURE);
    }
    if (phdr.p_type == PT_LOAD) {
      printf("Loaded segment at offset 0x%lx (virtual address: 0x%lx, size: "
             "0x%lx)\n",
             phdr.p_offset, phdr.p_vaddr, phdr.p_memsz);
      if (base_address == NULL) {
        base_address = (void *)(uintptr_t)phdr.p_vaddr;
        segment_size = phdr.p_memsz;
      }
    }
    if (phdr.p_type == PT_DYNAMIC) {
      fseek(file, phdr.p_offset, SEEK_SET);
      Elf64_Dyn *dyn = (Elf64_Dyn *)malloc(phdr.p_memsz);
      if (!dyn) {
        perror("malloc");
        fclose(file);
        exit(EXIT_FAILURE);
      }
      if (fread(dyn, 1, phdr.p_memsz, file) != phdr.p_memsz) {
        fprintf(stderr, "Failed to read dynamic section\n");
        free(dyn);
        fclose(file);
        exit(EXIT_FAILURE);
      }

      // Find the string table offset
      for (int j = 0; j < phdr.p_memsz / sizeof(Elf64_Dyn); j++) {
        if (dyn[j].d_tag == DT_STRTAB) {
          strtab_offset = dyn[j].d_un.d_ptr;
          printf("String table offset: 0x%lx\n", strtab_offset);
          break;
        }
      }

      // Print the needed libraries
      for (int j = 0; j < phdr.p_memsz / sizeof(Elf64_Dyn); j++) {
        if (dyn[j].d_tag == DT_NEEDED) {
          if (strtab_offset != 0) {
            // Calculate the correct address of the string table
            uintptr_t strtab_addr = 0;
            for (int k = 0; k < ehdr.e_phnum; k++) {
              fseek(file, ehdr.e_phoff + k * sizeof(phdr), SEEK_SET);
              if (fread(&phdr, 1, sizeof(phdr), file) != sizeof(phdr)) {
                fprintf(stderr, "Failed to read program header\n");
                free(dyn);
                fclose(file);
                exit(EXIT_FAILURE);
              }
              if (phdr.p_type == PT_LOAD && strtab_offset >= phdr.p_vaddr &&
                  strtab_offset < phdr.p_vaddr + phdr.p_memsz) {
                strtab_addr = phdr.p_offset + (strtab_offset - phdr.p_vaddr);
                break;
              }
            }
            fseek(file, strtab_addr + dyn[j].d_un.d_val, SEEK_SET);
            char lib_name[256];
            if (fread(lib_name, 1, sizeof(lib_name), file) > 0) {
              printf("Needs library: %s\n", lib_name);
            } else {
              fprintf(stderr, "Failed to read library name\n");
            }
          } else {
            fprintf(stderr, "String table offset not found.\n");
          }
        }
      }
      free(dyn);
    }
  }

  fclose(file);
}

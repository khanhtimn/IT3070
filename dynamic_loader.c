#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void load_elf(const char *filename);

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Hãy cung cấp đường dẫn đến file thực thi: %s <elf-file>\n",
            argv[0]); // Sử dụng: %s <tệp elf>
    exit(EXIT_FAILURE);
  }

  load_elf(argv[1]);
  return 0;
}

/**
 * Hàm load_elf
 * @param filename Tên tệp ELF cần tải
 * Hàm này mở tệp ELF, đọc tiêu đề ELF và các tiêu đề chương trình, sau đó tải
 * các phân đoạn cần thiết.
 */
void load_elf(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    perror("fopen"); // mở tệp thất bại
    exit(EXIT_FAILURE);
  }

  Elf64_Ehdr ehdr;
  if (fread(&ehdr, 1, sizeof(ehdr), file) != sizeof(ehdr)) {
    fprintf(stderr, "Lỗi đọc file ELF\n"); // Đọc tiêu đề ELF thất bại
    fclose(file);
    exit(EXIT_FAILURE);
  }

  if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
    fprintf(stderr, "Không phải một file ELF\n"); // Không phải là tệp ELF
    fclose(file);
    exit(EXIT_FAILURE);
  }

  printf("Đọc tiêu đề ELF thành công\n");

  // Đọc các tiêu đề chương trình
  fseek(file, ehdr.e_phoff, SEEK_SET);
  Elf64_Phdr phdr;
  void *base_address = NULL;
  size_t segment_size = 0;
  Elf64_Addr strtab_offset = 0;
  void *strtab_base = NULL;

  for (int i = 0; i < ehdr.e_phnum; i++) {
    if (fread(&phdr, 1, sizeof(phdr), file) != sizeof(phdr)) {
      fprintf(stderr, "Không đọc được phần Header của file chương trình\n");
      fclose(file);
      exit(EXIT_FAILURE);
    }
    if (phdr.p_type == PT_LOAD) {
      printf("Đã tải phân đoạn tại offset 0x%lx (địa chỉ ảo: 0x%lx, kích "
             "thước: 0x%lx)\n",
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
        perror("malloc"); // cấp phát bộ nhớ thất bại
        fclose(file);
        exit(EXIT_FAILURE);
      }
      if (fread(dyn, 1, phdr.p_memsz, file) != phdr.p_memsz) {
        fprintf(stderr, "Không đọc được phần liên kết động\n");
        free(dyn);
        fclose(file);
        exit(EXIT_FAILURE);
      }

      // Tìm offset của bảng chuỗi
      for (int j = 0; j < phdr.p_memsz / sizeof(Elf64_Dyn); j++) {
        if (dyn[j].d_tag == DT_STRTAB) {
          strtab_offset = dyn[j].d_un.d_ptr;
          printf("Offset của bảng chuỗi: 0x%lx\n", strtab_offset);
          break;
        }
      }

      // In các thư viện cần thiết
      for (int j = 0; j < phdr.p_memsz / sizeof(Elf64_Dyn); j++) {
        if (dyn[j].d_tag == DT_NEEDED) {
          if (strtab_offset != 0) {
            // Tính toán địa chỉ chính xác của bảng chuỗi
            uintptr_t strtab_addr = 0;
            for (int k = 0; k < ehdr.e_phnum; k++) {
              fseek(file, ehdr.e_phoff + k * sizeof(phdr), SEEK_SET);
              if (fread(&phdr, 1, sizeof(phdr), file) != sizeof(phdr)) {
                fprintf(stderr,
                        "Không đọc được phần Header của file chương trình\n");
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
              printf("Cần thư viện: %s\n", lib_name);
            } else {
              fprintf(stderr, "Đọc tên thư viện thất bại\n");
            }
          } else {
            fprintf(stderr, "Không tìm thấy offset của bảng chuỗi.\n");
          }
        }
      }
      free(dyn);
    }
  }

  fclose(file);
}

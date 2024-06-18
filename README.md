# Mock Dynamic Linker

## Tổng quan

Dự án này minh họa quá trình tải và diễn giải một tệp ELF
để liên kết động các thư viện chia sẻ tại thời gian chạy.

## Cơ chế thực thi của Dynamic Linker

Trong liên kết động, một chương trình có thể sử dụng mã không có trong chính tệp chương trình mà nằm trong các tệp thư viện riêng biệt dùng chung

Để so sánh, một chương trình được liên kết tĩnh chứa tất cả mã và dữ liệu mà nó cần để chạy từ đầu cho đến khi chương trình hoàn thành. Chương trình sẽ được Linux tải từ đĩa vào không gian địa chỉ ảo và quyền điều khiển được chuyển cho đoạn chương trình được ánh xạ trong không gian địa chỉ để thực thi sau đó.

```text
                          @vm
                         |        |
 @disk                   |--------|
+--------+   execve(2)   |        | <- $rip
| prog A | ------------> | prog A |
+--------+               |        |
                         |--------|
                         |        |
```

Mặt khác, một chương trình được liên kết động cần chỉ định một `dynamic linker` về cơ bản là một trình thông dịch tại thời gian chạy (runtime interpreter). Linux sẽ tải thêm trình thông dịch đó vào không gian địa chỉ ảo và trao quyền kiểm soát cho trình thông dịch thay vì chương trình người dùng. Trình thông dịch sẽ chuẩn bị môi trường thực thi cho chương trình và chuyển quyền điều khiển cho chương trình đó sau đó. Nhiệm vụ điển hình của trình phiên dịch là:

- Tải các đối tượng được chia sẻ vào bộ nhớ (Dependencies).
- Thực hiện việc tái cấu trúc không gian địa chỉ.
- Chạy các thủ tục khởi tạo.

```text
                                @vm                      @vm
                               |        |               |          |
 @disk                         |--------|               |----------|
+--------------+   execve(2)   |        |               |          | <- $rip
| prog A       | ------------> | prog A |               | prog A   |
+--------------+               |        |   load deps   |          |
| interp ldso  |               |--------| ------------> |----------|
+--------------+               |        |               |          |
| dep libgreet |               |--------|               |----------|
+--------------+               | ldso   | <- $rip       | ldso     |
                               |--------|               |----------|
                                                        |          |
                                                        |----------|
                                                        | libgreet |
                                                        |----------|
```

Trong định dạng file `ELF`, tên của trình liên kết động được chỉ định là 1
chuỗi ký tự trong phần đặc biệt `.interp`.

```bash
readelf -W --string-dump .interp main

String dump of section '.interp':
  [     0]  /lib64/ld-linux-x86-64.so.2
```

Phần `.interp` được tham chiếu bởi đoạn `PT_INTERP` trong phần header của chương trình. Phân đoạn này được nhân Linux sử dụng trong `execve(2)`
syscall trong hàm [`load_elf_binary`] để kiểm tra xem
chương trình cần một trình liên kết động và nếu có thì sẽ lấy được tên của nó.

```bash
readelf -W --sections --program-headers main

Section Headers:
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  [ 0]                   NULL            0000000000000000 000000 000000 00      0   0  0
  [ 1] .interp           PROGBITS        00000000000002a8 0002a8 00001c 00   A  0   0  1
  ...

Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
    PHDR           0x000040 0x0000000000000040 0x0000000000000040 0x000268 0x000268 R   0x8
    INTERP         0x0002a8 0x00000000000002a8 0x00000000000002a8 0x00001c 0x00001c R   0x1
        [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
    ...
```

## Những điều cần ghi nhớ

- Các chương trình được liên kết động sử dụng mã có trong các file thư viện riêng biệt.
- `Trình liên kết động` là một trình thông dịch được nhân Linux tải và nhận
  điều khiển trước chương trình người dùng.
- Một chương trình được liên kết động chỉ định trình liên kết động cần thiết trong
  phần `.interp`.

## Thực hiện

### Bước 1: Biên Dịch Thư Viện Chia Sẻ

Biên dịch `example.c` để tạo thư viện chia sẻ `libexample.so`.

```sh
gcc -shared -fPIC -o libexample.so example.c
```

### Bước 2: Biên Dịch Trình Nạp Liên Kết Động

Biên dịch dynamic_loader.c để tạo tệp thực thi dynamic_loader.

```sh
gcc -o dynamic_loader dynamic_loader.c
```

### Bước 3: Thực Thi

```sh
./dynamic_loader main
```

### Kết Quả Mong Đợi

Kết quả sẽ hiển thị thông tin tiêu đề ELF, các phân đoạn đã tải,
offset của bảng chuỗi và tên của các thư viện cần thiết. Ví dụ:

```sh
Đọc tiêu đề ELF thành công
Đã tải phân đoạn tại offset 0x0 (địa chỉ ảo: 0x0, kích thước: 0x640)
Đã tải phân đoạn tại offset 0x1000 (địa chỉ ảo: 0x1000, kích thước: 0x15d)
Đã tải phân đoạn tại offset 0x2000 (địa chỉ ảo: 0x2000, kích thước: 0xa4)
Đã tải phân đoạn tại offset 0x2dc0 (địa chỉ ảo: 0x3dc0, kích thước: 0x260)
Offset của bảng chuỗi: 0x488
Cần thư viện: libexample.so
Cần thư viện: libc.so.6
```
